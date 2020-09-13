#include "MainComponent.h"

extern "C" {
	#include "auto_init_api.h"
	#include "dsp_management_api.h"

	void init_chain();
	extern struct dsp_chain_t *pMain_dsp_chain;

}

// length should be (buff_size_of_DSP_chain * const)
#define DSP_BUFF_SIZE  512
#define CYCLIC_BUFF_SIZE  (DSP_BUFF_SIZE * 7)

static float mic_buff_left_cyclic[CYCLIC_BUFF_SIZE];
static float mic_buff_right_cyclic[CYCLIC_BUFF_SIZE];
static float playback_buff_left_cyclic[CYCLIC_BUFF_SIZE];
static float playback_buff_right_cyclic[CYCLIC_BUFF_SIZE];

static float mic_buff_left_x3[DSP_BUFF_SIZE * 3];
static float mic_buff_right_x3[DSP_BUFF_SIZE * 3];
static float playback_buff_left_x3[DSP_BUFF_SIZE * 3];
static float playback_buff_right_x3[DSP_BUFF_SIZE * 3];

static float mic_buff_left[DSP_BUFF_SIZE];
static float mic_buff_right[DSP_BUFF_SIZE];
static float playback_buff_left[DSP_BUFF_SIZE];
static float playback_buff_right[DSP_BUFF_SIZE];

static float output_buff_left[DSP_BUFF_SIZE];
static float output_buff_right[DSP_BUFF_SIZE];

static size_t in_audio_write_pointer = 0;
static size_t in_audio_read_pointer = 0;
static size_t valid_data_in_input_buffer = 0;

static size_t zero_buff_size = 0;
static float *zero_buff = NULL;

static uint8_t error_state = 0;

void downsampling();

uint8_t MainComponent::num_of_objects = 0;


void MainComponent::init_audio()
{
	static AudioFormatReader* audio_format_reader;

	auto_init_api();

	String file_name(
		"C:\\Work\\audio_files\\"
		"Eagles_-_Hotel_California_Lyrics[ListenVid.com].wav");
	File audio_file(file_name);

	formatManager.registerBasicFormats();

	// no need to delete audio_format_reader, it will be deleted by
	// readerSource
	audio_format_reader = formatManager.createReaderFor(audio_file);
	playbackGui.set_main_component(this);

	if (audio_format_reader != nullptr)
	{
		playbackGui.set_filename_str(file_name);

		std::unique_ptr<juce::AudioFormatReaderSource> newSource(
				new juce::AudioFormatReaderSource (audio_format_reader, true));
		transportSource.setSource(
				newSource.get(), 0, nullptr, audio_format_reader->sampleRate);
		playbackGui.changeState(PLAYBACK_STOPPED);
		readerSource.reset(newSource.release());
	}
	else
	{
		playbackGui.set_filename_str("no playback file added");
	}
	transportSource.addChangeListener(this);

	dsp_management_api_init(DSP_BUFF_SIZE);
	init_chain();

	if (num_of_objects) // trying to create more then 1 objects
	{
		error_state = 2;
	}
	num_of_objects++;
}


void MainComponent::free_audio()
{
	transportSource.releaseResources();
	free(zero_buff);
	dsp_management_api_delete_chain(pMain_dsp_chain);
	dsp_management_api_delete();
}


void verify_zero_buff(size_t req_buf_size)
{
	if (req_buf_size > zero_buff_size)
	{
		zero_buff = (float*)realloc(zero_buff, sizeof(float) * req_buf_size);
		if (NULL == zero_buff)
		{
			error_state = 1;
			return;
		}
		zero_buff_size = req_buf_size;
		for (size_t i = 0; i < zero_buff_size; i++)
		{
			zero_buff[i] = 0;
		}
	}
}


void MainComponent::prepareToPlay(
		int samplesPerBlockExpected, double sampleRate)
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
	verify_zero_buff((size_t)samplesPerBlockExpected);
}



void get_all_buffer_pointers(AudioDeviceManager *deviceManager,
		const juce::AudioSourceChannelInfo& bufferToFill,
		const float **new_mic_left, const float **new_mic_right,
		float **new_playback_left, float **new_playback_right)
{
	auto* device = deviceManager->getCurrentAudioDevice();
	auto activeInputChannels  = device->getActiveInputChannels();
	auto activeOutputChannels = device->getActiveOutputChannels();
	auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
	auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
	int actualInputChannel;
	int actualOutputChannel;

	actualInputChannel = 0 % maxInputChannels;
	if (activeInputChannels[0])
	{
		*new_mic_left = bufferToFill.buffer->getReadPointer(
						actualInputChannel, bufferToFill.startSample);
	}
	else
	{
		*new_mic_left = zero_buff;
	}

	actualInputChannel = 1 % maxInputChannels;
	if (activeInputChannels[1])
	{
		*new_mic_right = bufferToFill.buffer->getReadPointer(
						actualInputChannel, bufferToFill.startSample);
	}
	else
	{
		*new_mic_right = zero_buff;
	}

	actualOutputChannel = 0 % maxOutputChannels;
	if (activeOutputChannels[0])
	{
		*new_playback_left = bufferToFill.buffer->getWritePointer(
								actualOutputChannel, bufferToFill.startSample);
	}
	else
	{
		*new_playback_left = zero_buff;
	}

	actualOutputChannel = 1 % maxOutputChannels;
	if (activeOutputChannels[1])
	{
		*new_playback_right = bufferToFill.buffer->getWritePointer(
								actualOutputChannel, bufferToFill.startSample);
	}
	else
	{
		*new_playback_right = zero_buff;
	}
}


void add_new_input_buffers(size_t num_of_samples,
		const float *new_mic_left, const float *new_mic_right,
		float *new_playback_left, float *new_playback_right)
{
	size_t samples_to_write;
	size_t free_space_in_buffer;

	while (num_of_samples)
	{
		free_space_in_buffer = CYCLIC_BUFF_SIZE - valid_data_in_input_buffer;
		if (0 == free_space_in_buffer)
		{
			//TODO: report overflow
			return;
		}

		if (in_audio_write_pointer >= in_audio_read_pointer)
		{
			samples_to_write = CYCLIC_BUFF_SIZE - in_audio_write_pointer;
		}
		else
		{
			samples_to_write = in_audio_read_pointer - in_audio_write_pointer;
		}

		if (num_of_samples < samples_to_write)
		{
			samples_to_write = num_of_samples;
		}
		if (free_space_in_buffer < samples_to_write)
		{
			samples_to_write = free_space_in_buffer;
		}

		memcpy(&mic_buff_left_cyclic[in_audio_write_pointer], new_mic_left,
										samples_to_write * sizeof(float));
		memcpy(&mic_buff_right_cyclic[in_audio_write_pointer], new_mic_right,
										samples_to_write * sizeof(float));
		memcpy(&playback_buff_left_cyclic[in_audio_write_pointer],
					new_playback_left, samples_to_write * sizeof(float));
		memcpy(&playback_buff_right_cyclic[in_audio_write_pointer],
					new_playback_right, samples_to_write * sizeof(float));

		in_audio_write_pointer =
				(in_audio_write_pointer + samples_to_write) % CYCLIC_BUFF_SIZE;
		valid_data_in_input_buffer += samples_to_write;
		num_of_samples -= samples_to_write;
		new_mic_left += samples_to_write;
		new_mic_right += samples_to_write;
		new_playback_left += samples_to_write;
		new_playback_right += samples_to_write;
	}
}


uint8_t get_buffers_for_processing()
{
	size_t samples_to_copy;
	size_t curr_samples_to_copy;
	size_t curr_dest_buff_pos;

	samples_to_copy = DSP_BUFF_SIZE * 3;
	if (samples_to_copy > valid_data_in_input_buffer)
	{
		return 0;// not enough data in buffer
	}

	curr_dest_buff_pos = 0;
	while (samples_to_copy)
	{
		if (in_audio_write_pointer >= in_audio_read_pointer)
		{
			curr_samples_to_copy =
					in_audio_write_pointer - in_audio_read_pointer;
		}
		else
		{
			curr_samples_to_copy = CYCLIC_BUFF_SIZE - in_audio_read_pointer;
		}

		if (curr_samples_to_copy > valid_data_in_input_buffer)
		{
			curr_samples_to_copy = valid_data_in_input_buffer;
		}
		if (curr_samples_to_copy > samples_to_copy)
		{
			curr_samples_to_copy = samples_to_copy;
		}

		memcpy(&mic_buff_left_x3[curr_dest_buff_pos],
				&mic_buff_left_cyclic[in_audio_read_pointer],
				curr_samples_to_copy * sizeof(float));
		memcpy(&mic_buff_right_x3[curr_dest_buff_pos],
				&mic_buff_right_cyclic[in_audio_read_pointer],
				curr_samples_to_copy * sizeof(float));
		memcpy(&playback_buff_left_x3[curr_dest_buff_pos],
				&playback_buff_left_cyclic[in_audio_read_pointer],
				curr_samples_to_copy * sizeof(float));
		memcpy(&playback_buff_right_x3[curr_dest_buff_pos],
				&playback_buff_right_cyclic[in_audio_read_pointer],
				curr_samples_to_copy * sizeof(float));

		in_audio_read_pointer =
			(in_audio_read_pointer + curr_samples_to_copy) % CYCLIC_BUFF_SIZE;
		curr_dest_buff_pos += curr_samples_to_copy;
		valid_data_in_input_buffer -= curr_samples_to_copy;
		samples_to_copy -= curr_samples_to_copy;
	}

	downsampling();
	return 1;
}


// TODO: add correct downsampling
void downsampling()
{
	size_t i;

	for(i = 0; i < DSP_BUFF_SIZE; i++)
	{
		mic_buff_left[i] = mic_buff_left_x3[i * 3];
		mic_buff_right[i] = mic_buff_right_x3[i * 3];
		playback_buff_left[i] = playback_buff_left_x3[i * 3];
		playback_buff_right[i] = playback_buff_right_x3[i * 3];
	}
}


void MainComponent::getNextAudioBlock(
		const juce::AudioSourceChannelInfo& bufferToFill)
{
	const float *new_mic_left;
	const float *new_mic_right;
	float *new_playback_left;
	float *new_playback_right;
	uint8_t data_is_valid_for_proccessing;
	size_t num_of_samples;
	auto* device = deviceManager.getCurrentAudioDevice();

	num_of_samples = bufferToFill.numSamples;
	verify_zero_buff(num_of_samples);

	if ((error_state) || (readerSource.get() == nullptr))
	{
		bufferToFill.clearActiveBufferRegion();
		if (error_state) return;
	}
	else
	{
		transportSource.getNextAudioBlock (bufferToFill);
	}

	if (0 == enable_audio_process) return;
	if (false == device->isOpen()) return;
	if (48000 != device->getCurrentSampleRate()) return;

	get_all_buffer_pointers(&deviceManager, bufferToFill,
		&new_mic_left, &new_mic_right, &new_playback_left, &new_playback_right);
	add_new_input_buffers(num_of_samples,
			new_mic_left, new_mic_right, new_playback_left, new_playback_right);

	data_is_valid_for_proccessing = get_buffers_for_processing();

	if (0 == data_is_valid_for_proccessing) return;
#if 1

	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain,
			IN_PAD(0), (uint8_t *)mic_buff_left, DSP_BUFF_SIZE);
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain,
			IN_PAD(1), (uint8_t *)mic_buff_right, DSP_BUFF_SIZE);
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain,
			IN_PAD(2), (uint8_t *)playback_buff_left, DSP_BUFF_SIZE);
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain,
			IN_PAD(3), (uint8_t *)playback_buff_right, DSP_BUFF_SIZE);
	dsp_management_api_set_chain_output_buffer(pMain_dsp_chain,
			OUT_PAD(0), (uint8_t *)output_buff_left, DSP_BUFF_SIZE);
	dsp_management_api_set_chain_output_buffer(pMain_dsp_chain,
			OUT_PAD(1), (uint8_t *)output_buff_right, DSP_BUFF_SIZE);
	dsp_management_api_process_chain(pMain_dsp_chain);
#endif
}


void MainComponent::changeState(enum playback_transport_state_e new_state)
{
	if (state != new_state)
	{
		state = new_state;
		switch (state)
		{
			case PLAYBACK_STOPPED:
				playbackGui.changeState(PLAYBACK_STOPPED);
				transportSource.setPosition (0.0);
				break;

			case PLAYBACK_STARTING:
				playbackGui.changeState(PLAYBACK_STARTING);
				transportSource.start();
				break;

			case PLAYBACK_PLAYING:
				playbackGui.changeState(PLAYBACK_PLAYING);
				break;

			case PLAYBACK_STOPPING:
				playbackGui.changeState(PLAYBACK_STOPPING);
				transportSource.stop();
				break;
		}
	}
}


void MainComponent::playback_transport_changed()
{
	if (transportSource.isPlaying())
		changeState (PLAYBACK_PLAYING);
	else
		changeState (PLAYBACK_STOPPED);
}


void MainComponent::enProcessButtonClicked ()
{
	if ( true  == (enable_process_btn.getToggleState()))
	{
		enable_audio_process = 1;
	}
	else
	{
		enable_audio_process = 0;
	}
}
