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

static void downsampling();
static void record_stop();

uint8_t MainComponent::num_of_objects = 0;


void MainComponent::init_playback()
{
	static AudioFormatReader* audio_format_reader;
	String playback_file_name(
		"C:\\Work\\audio_files\\"
		"Eagles_-_Hotel_California_Lyrics[ListenVid.com].wav");
	File audio_file(playback_file_name);

	playbackGui.set_main_component(this);
	formatManager.registerBasicFormats();

	// no need to delete audio_format_reader, it will be deleted by
	// readerSource
	audio_format_reader = formatManager.createReaderFor(audio_file);
	if (audio_format_reader != nullptr)
	{
		playbackGui.set_filename_str(playback_file_name);

		std::unique_ptr<juce::AudioFormatReaderSource> newSource(
				new juce::AudioFormatReaderSource (audio_format_reader, true));
		transportSource.setSource(
				newSource.get(), 0, nullptr, audio_format_reader->sampleRate);
		playbackGui.changeState(PLAYBACK_STOPPED);
		readerSource.reset(newSource.release());
	}
	else
	{
		playbackGui.set_filename_str("no playback file selected");
	}
	transportSource.addChangeListener(this);
}


#define RECORDER_SAMPLE_RATE  16000
#define RECORDER_CHANNEL_NUM  2
#define RECORDER_WORD_SIZE    16


// the thread that will write our audio data to disk
static TimeSliceThread record_thread { "Audio Recorder Thread" };

// the FIFO used to buffer the incoming data
static std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter;

static 	std::unique_ptr< FileOutputStream >  fileStream;
static 	AudioFormatWriter* wav_writer;
static  std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
static  CriticalSection writerLock;
static File record_audio_file;

void MainComponent::init_recorder()
{
	String record_file_name("C:\\Work\\audio_files\\records\\output.wav");
	record_audio_file = record_file_name;

	recordGui.set_main_component(this);

	recordGui.set_filename_str(record_file_name);
	recordGui.changeState(RECORD_STOPPED);
    record_thread.startThread();
}


void MainComponent::init_audio()
{
	auto_init_api();
	init_playback();
	init_recorder();

	dsp_management_api_init(DSP_BUFF_SIZE);
	init_chain();
	if (num_of_objects) // trying to create more then 1 objects
	{
		CRITICAL_ERROR("only one object can exist");
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

	threadedWriter.reset();

//	if (nullptr != threadedWriter.get())
//	{
//		threadedWriter.reset();
//		// close of threadedWriter will release wav_writer and
//		// fileStream too, so no need to continue
//		return;
//	}
//
//	if (nullptr != wav_writer)
//	{
//		delete wav_writer;
//
//		// delete of wav_writer will release fileStream too,
//		// so no need to continue
//		return;
//	}
//	fileStream.reset();
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



void get_input_buffer_pointers(AudioDeviceManager *deviceManager,
		const juce::AudioSourceChannelInfo& bufferToFill,
		const float **new_mic_left, const float **new_mic_right)
{
	auto* device = deviceManager->getCurrentAudioDevice();
	auto activeInputChannels  = device->getActiveInputChannels();
	auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
	int actualInputChannel;

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
}


void get_output_buffer_pointers(AudioDeviceManager *deviceManager,
		const juce::AudioSourceChannelInfo& bufferToFill,
		float **new_playback_left, float **new_playback_right)
{
	auto* device = deviceManager->getCurrentAudioDevice();
	auto activeOutputChannels = device->getActiveOutputChannels();
	auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
	int actualOutputChannel;

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
		const float *new_buf_left, const float *new_buf_right,
		uint8_t updating_playback_buffers)
{
	size_t samples_to_write;
	size_t free_space_in_buffer;
	size_t tmp_in_audio_write_pointer;
	size_t tmp_valid_data_in_input_buffer;

	tmp_in_audio_write_pointer = in_audio_write_pointer;
	tmp_valid_data_in_input_buffer = valid_data_in_input_buffer;
	while (num_of_samples)
	{
		free_space_in_buffer = CYCLIC_BUFF_SIZE - tmp_valid_data_in_input_buffer;
		if (0 == free_space_in_buffer)
		{
			//TODO: report overflow
			return;
		}

		if (tmp_in_audio_write_pointer >= in_audio_read_pointer)
		{
			samples_to_write = CYCLIC_BUFF_SIZE - tmp_in_audio_write_pointer;
		}
		else
		{
			samples_to_write = in_audio_read_pointer - tmp_in_audio_write_pointer;
		}

		if (num_of_samples < samples_to_write)
		{
			samples_to_write = num_of_samples;
		}
		if (free_space_in_buffer < samples_to_write)
		{
			samples_to_write = free_space_in_buffer;
		}

		if (0 == updating_playback_buffers)
		{
			memcpy(&mic_buff_left_cyclic[tmp_in_audio_write_pointer],
						new_buf_left, samples_to_write * sizeof(float));
			memcpy(&mic_buff_right_cyclic[tmp_in_audio_write_pointer],
						new_buf_right, samples_to_write * sizeof(float));
		}
		else
		{
			memcpy(&playback_buff_left_cyclic[tmp_in_audio_write_pointer],
						new_buf_left, samples_to_write * sizeof(float));
			memcpy(&playback_buff_right_cyclic[tmp_in_audio_write_pointer],
						new_buf_right, samples_to_write * sizeof(float));
		}

		tmp_in_audio_write_pointer =
			(tmp_in_audio_write_pointer + samples_to_write) % CYCLIC_BUFF_SIZE;
		tmp_valid_data_in_input_buffer += samples_to_write;
		num_of_samples -= samples_to_write;
		new_buf_left += samples_to_write;
		new_buf_right += samples_to_write;
	}
	if (updating_playback_buffers)
	{
		in_audio_write_pointer = tmp_in_audio_write_pointer;
		valid_data_in_input_buffer = tmp_valid_data_in_input_buffer;
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
static void downsampling()
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


void MainComponent::do_record()
{
    const ScopedLock sl (writerLock);
	int record_sample_rate = 16000;
	float *data_to_record[2];

	if ((RECORD_RECORDING != record_state) ||
			(curr_rec_duration_ms >= max_rec_duration_ms))
	{
		return;
	}

	data_to_record[0] = output_buff_left;
	data_to_record[1] = output_buff_right;
    if (activeWriter.load() != nullptr)
    {
        activeWriter.load()->write (data_to_record, DSP_BUFF_SIZE);
    }

	curr_rec_duration_ms +=
			(int) (DSP_BUFF_SIZE * 1000) / record_sample_rate;

	if (curr_rec_duration_ms >= max_rec_duration_ms)
	{
		record_stop();
		recordGui.changeState(RECORD_STOPPED);
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
	double sample_rate;
	auto* device = deviceManager.getCurrentAudioDevice();

	num_of_samples = bufferToFill.numSamples;
	verify_zero_buff(num_of_samples);
	sample_rate = device->getCurrentSampleRate();

	if ((enable_audio_process) &&
			(true == device->isOpen()) && (48000 == sample_rate ))
	{
		get_input_buffer_pointers(
			&deviceManager, bufferToFill, &new_mic_left, &new_mic_right);
		add_new_input_buffers(num_of_samples, new_mic_left, new_mic_right, 0);
	}

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
	if (48000 != sample_rate ) return;

	get_output_buffer_pointers(&deviceManager, bufferToFill,
								&new_playback_left, &new_playback_right);
	add_new_input_buffers(
			num_of_samples, new_playback_left, new_playback_right, 1);

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
	do_record();
}


void MainComponent::change_playback_state(
				enum playback_transport_state_e new_state)
{
	if (playback_state == new_state)
	{
		return;
	}

	playback_state = new_state;
	switch (playback_state)
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


static int record_start(RecordGui &recordGui)
{
	WavAudioFormat wavFormat;
	record_audio_file.deleteFile();

//	std::unique_ptr<FileOutputStream> out_stream =
//						record_audio_file.createOutputStream();
	fileStream = record_audio_file.createOutputStream();
	if (nullptr == fileStream.get())
	{
		recordGui.set_filename_str("failed to open stream to wav file");
		return 1;
	}


	wav_writer = wavFormat.createWriterFor(
			fileStream.get(), RECORDER_SAMPLE_RATE, RECORDER_CHANNEL_NUM,
			RECORDER_WORD_SIZE, {}, 0);

	if (nullptr == wav_writer)
	{
		fileStream.reset();
		recordGui.set_filename_str("failed to open wav writer");
		return 1;
	}

	// (passes responsibility for deleting the stream to the writer object
	// that is now using it)
	fileStream.release();


    // Now we'll create one of these helper objects which will act as a FIFO
	// buffer, and will write the data to disk on our background thread.
    threadedWriter.reset(
    	new AudioFormatWriter::ThreadedWriter(
    								wav_writer, record_thread, 32768)
    	);

    if (nullptr == threadedWriter.get())
    {
    	delete wav_writer;
    	return 1;
    }
    // And now, swap over our active writer pointer so
    // that the audio callback will start using it..
    const ScopedLock sl (writerLock);
    activeWriter = threadedWriter.get();
    return 0;
}


static void record_stop()
{
    // First, clear this pointer to stop the audio callback
	// from using our writer object..
    {
        const ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }

    // Now we can delete the writer object. It's done in this order because
    // the deletion could take a little time while remaining data gets flushed
    // to disk, so it's best to avoid blocking the audio
    // callback while this happens.
    threadedWriter.reset();
}


void MainComponent::change_recording_state(
				enum record_transport_state_e new_state, int duration_ms)
{
	if (record_state == new_state)
	{
		return;
	}

	switch (new_state)
	{
//	case RECORD_STOPPED:
//		recordGui.changeState(RECORD_STOPPED);
//		break;
//
	case RECORD_STARTING:
		if (0 == record_start(recordGui))
		{
			curr_rec_duration_ms = 0;
			max_rec_duration_ms = duration_ms;
			record_state = RECORD_RECORDING;
			recordGui.changeState(RECORD_RECORDING);
		}
		break;

	case RECORD_STOPPING:
		record_stop();
		record_state = RECORD_STOPPED;
		recordGui.changeState(RECORD_STOPPED);
		break;

//	case RECORD_RECORDING:
//		recordGui.changeState(RECORD_RECORDING);
//		break;
	}
}


void MainComponent::playback_transport_changed()
{
	if (transportSource.isPlaying())
		change_playback_state(PLAYBACK_PLAYING);
	else
		change_playback_state(PLAYBACK_STOPPED);
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
