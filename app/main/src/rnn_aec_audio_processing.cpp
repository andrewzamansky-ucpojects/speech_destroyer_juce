#include "MainComponent.h"

extern "C" {
	#include "auto_init_api.h"
	#include "dsp_management_api.h"

	void init_chain();
	extern struct dsp_chain_t *pMain_dsp_chain;

}


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


#define ZERO_BUFF_SIZE_FOR_DSP_CHAIN  (DSP_BUFF_SIZE * 8)
#define DUMMY_BUFF_SIZE_FOR_DSP_CHAIN  (DSP_BUFF_SIZE * 8)

uint8_t zero_buff_for_DSP_chain[ZERO_BUFF_SIZE_FOR_DSP_CHAIN];
uint8_t dummy_buff_for_DSP_chain[DUMMY_BUFF_SIZE_FOR_DSP_CHAIN];


void MainComponent::init_audio_processing()
{
	dsp_management_api_init(
			zero_buff_for_DSP_chain, ZERO_BUFF_SIZE_FOR_DSP_CHAIN,
			dummy_buff_for_DSP_chain, DUMMY_BUFF_SIZE_FOR_DSP_CHAIN);
	init_chain();
	record_sample_rate = 16000;
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


uint8_t MainComponent::get_buffers_for_processing()
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



void MainComponent::process_audio(
		float **left_buff_to_record, float **right_buff_to_record,
		size_t *samples_to_record)
{
	uint8_t data_is_valid_for_proccessing;

	data_is_valid_for_proccessing = get_buffers_for_processing();

	if (0 == data_is_valid_for_proccessing) return;
#if 1

	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(0),
					(uint8_t *)mic_buff_left, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(1),
					(uint8_t *)mic_buff_right, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(2),
				(uint8_t *)playback_buff_left, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(3),
				(uint8_t *)playback_buff_right, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_output_buffer(pMain_dsp_chain, OUT_PAD(0),
				(uint8_t *)output_buff_left, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_output_buffer(pMain_dsp_chain, OUT_PAD(1),
				(uint8_t *)output_buff_right, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_process_chain(pMain_dsp_chain);
#endif
	*left_buff_to_record = output_buff_left;
	*right_buff_to_record = output_buff_right;
	*samples_to_record = DSP_BUFF_SIZE;
}
