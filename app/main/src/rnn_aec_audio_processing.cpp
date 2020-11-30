#include "MainComponent.h"

extern "C" {
	#include "auto_init_api.h"
	#include "dsp_management_api.h"
	#include "cyclic_buffer_api.h"
	void init_chain();
	extern struct dsp_chain_t *pMain_dsp_chain;

}

#ifdef UPSAMPLE_OUTPUT_TO_48000
	#define REC_SAMPLE_RATE  48000
#else
	#define REC_SAMPLE_RATE  16000
#endif

static float mic_buff_left[DSP_BUFF_SIZE] = {0};
static float mic_buff_right[DSP_BUFF_SIZE] = {0};
static float playback_buff_left[DSP_BUFF_SIZE] = {0};
static float playback_buff_right[DSP_BUFF_SIZE] = {0};

#define SIZE_OF_PROCESSED_PLBCK_BUFF (DSP_BUFF_SIZE * 5)
static float processed_playback_buff_left[SIZE_OF_PROCESSED_PLBCK_BUFF];
static float processed_playback_buff_right[SIZE_OF_PROCESSED_PLBCK_BUFF];

static float output_buff_left[DSP_BUFF_SIZE];
static float output_buff_right[DSP_BUFF_SIZE];


#define ZERO_BUFF_SIZE_FOR_DSP_CHAIN  (DSP_BUFF_SIZE * 8)
#define DUMMY_BUFF_SIZE_FOR_DSP_CHAIN  (DSP_BUFF_SIZE * 8)

uint8_t zero_buff_for_DSP_chain[ZERO_BUFF_SIZE_FOR_DSP_CHAIN] = {0};
uint8_t dummy_buff_for_DSP_chain[DUMMY_BUFF_SIZE_FOR_DSP_CHAIN];

static int downsample_factor;

#define CYCLIC_BUFF_SIZE  (512 * 16)
#define CYCLIC_PROCESSED_PLBCK_BUFF_SIZE  (512 * 1024)

// initial data in playback buffer. serves as output 'delay'
// must be less than CYCLIC_PROCESSED_PLBCK_BUFF_SIZE
#define NUM_OF_PREFILL_CHUNKS   50
#define CYCLIC_PROCESSED_PREFILL_SAMPLES  (512 * NUM_OF_PREFILL_CHUNKS)

cyclic_buffer_t  mic_cyclic_buffer_left;
cyclic_buffer_t  mic_cyclic_buffer_right;
cyclic_buffer_t  playback_cyclic_buffer_left;
cyclic_buffer_t  playback_cyclic_buffer_right;
cyclic_buffer_t  proccessed_playback_cyclic_buffer_left;
cyclic_buffer_t  proccessed_playback_cyclic_buffer_right;



void MainComponent::init_audio_processing()
{
	uint8_t ret_val;
	size_t test_num_of_samples;
	size_t i;

	dsp_management_api_init(
			zero_buff_for_DSP_chain, ZERO_BUFF_SIZE_FOR_DSP_CHAIN,
			dummy_buff_for_DSP_chain, DUMMY_BUFF_SIZE_FOR_DSP_CHAIN);
	init_chain();
	record_sample_rate = REC_SAMPLE_RATE;
	downsample_factor = 48000 / record_sample_rate;

	ret_val = 0;
	ret_val += cyclic_buffer_init(
			sizeof(float), CYCLIC_BUFF_SIZE, &mic_cyclic_buffer_left);
	ret_val += cyclic_buffer_init(
			sizeof(float), CYCLIC_BUFF_SIZE, &mic_cyclic_buffer_right);
	ret_val += cyclic_buffer_init(
			sizeof(float), CYCLIC_BUFF_SIZE, &playback_cyclic_buffer_left);
	ret_val += cyclic_buffer_init(
			sizeof(float), CYCLIC_BUFF_SIZE, &playback_cyclic_buffer_right);
	ret_val += cyclic_buffer_init(sizeof(float),
				CYCLIC_PROCESSED_PLBCK_BUFF_SIZE,
				&proccessed_playback_cyclic_buffer_left);
	ret_val += cyclic_buffer_init(sizeof(float),
				CYCLIC_PROCESSED_PLBCK_BUFF_SIZE,
				&proccessed_playback_cyclic_buffer_right);
	if (CYCLIC_BUFFER_NO_ERRORS != ret_val)
	{
		CRITICAL_ERROR("cannot create cyclic buffer");

	}

	for (i = 0; i < NUM_OF_PREFILL_CHUNKS; i++)
	{
		test_num_of_samples = cyclic_buffer_add_items(
				proccessed_playback_cyclic_buffer_left,
				output_buff_right, 512);
		if (test_num_of_samples != 512)
		{
			CRITICAL_ERROR("cannot put requested samples");
		}
		test_num_of_samples = cyclic_buffer_add_items(
				proccessed_playback_cyclic_buffer_right,
				output_buff_right, 512);
		if (test_num_of_samples != 512)
		{
			CRITICAL_ERROR("cannot put requested samples");
		}
	}
}


uint8_t get_buffers_for_processing()
{
	size_t test_num_of_samples;

	if (DSP_BUFF_SIZE > cyclic_buffer_get_num_of_items(mic_cyclic_buffer_left))
	{
		return 0;// not enough data in buffer
	}

	test_num_of_samples = cyclic_buffer_get_items(
			mic_cyclic_buffer_left, mic_buff_left, DSP_BUFF_SIZE);
	if (test_num_of_samples != DSP_BUFF_SIZE)
	{
		CRITICAL_ERROR("cannot get mic_L requested samples");
	}
	test_num_of_samples = cyclic_buffer_get_items(
			mic_cyclic_buffer_right, mic_buff_right, DSP_BUFF_SIZE);
	if (test_num_of_samples != DSP_BUFF_SIZE)
	{
		CRITICAL_ERROR("cannot get mic_R requested samples");
	}
	test_num_of_samples = cyclic_buffer_get_items(
			playback_cyclic_buffer_left, playback_buff_left, DSP_BUFF_SIZE);
	if (test_num_of_samples != DSP_BUFF_SIZE)
	{
		CRITICAL_ERROR("cannot get plbck_R requested samples");
	}
	test_num_of_samples = cyclic_buffer_get_items(
			playback_cyclic_buffer_right, playback_buff_right, DSP_BUFF_SIZE);
	if (test_num_of_samples != DSP_BUFF_SIZE)
	{
		CRITICAL_ERROR("cannot get plbck_R requested samples");
	}

	return 1;
}


void add_new_input_buffers(size_t num_of_samples,
		const float *new_playback_left, const float *new_playback_right)
{
	size_t test_num_of_samples;

	test_num_of_samples = cyclic_buffer_add_items(
			playback_cyclic_buffer_left, new_playback_left, num_of_samples);
	if (test_num_of_samples != num_of_samples)
	{
		CRITICAL_ERROR("cannot put requested plbck_L samples to cyclic buffer");
	}
	test_num_of_samples = cyclic_buffer_add_items(
			playback_cyclic_buffer_right, new_playback_right, num_of_samples);
	if (test_num_of_samples != num_of_samples)
	{
		CRITICAL_ERROR("cannot put requested plbck_R samples to cyclic buffer");
	}
}


void MainComponent::process_audio(
		float **left_buff_to_record, float **right_buff_to_record,
		size_t *samples_to_record,
		float *playback_buf_left, float *playback_buf_right,
		size_t num_of_samples)
{
	size_t i;
	uint8_t data_is_valid_for_proccessing;
	size_t rec_num_of_samples;
	size_t test_num_of_samples;

	test_num_of_samples = cyclic_buffer_get_items(
			proccessed_playback_cyclic_buffer_left,
			processed_playback_buff_left, num_of_samples);
	if (test_num_of_samples != num_of_samples)
	{
		CRITICAL_ERROR("cannot get requested samples");
	}
	test_num_of_samples = cyclic_buffer_get_items(
			proccessed_playback_cyclic_buffer_right,
			processed_playback_buff_right, num_of_samples);
	if (test_num_of_samples != num_of_samples)
	{
		CRITICAL_ERROR("cannot get requested samples");
	}

	for (i = 0; i < num_of_samples; i++)
	{
		playback_buf_left[i] += processed_playback_buff_left[i];
		playback_buf_right[i] += processed_playback_buff_right[i];
	}

	add_new_input_buffers(num_of_samples,
		playback_buf_left, playback_buf_right);

	data_is_valid_for_proccessing = get_buffers_for_processing();

	if (0 == data_is_valid_for_proccessing) return;

	rec_num_of_samples = DSP_BUFF_SIZE / downsample_factor;

	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(0),
					(uint8_t *)mic_buff_left, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(1),
					(uint8_t *)mic_buff_right, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(2),
				(uint8_t *)playback_buff_left, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_input_buffer(pMain_dsp_chain, IN_PAD(3),
				(uint8_t *)playback_buff_right, DSP_BUFF_SIZE * sizeof(float));
	dsp_management_api_set_chain_output_buffer(pMain_dsp_chain, OUT_PAD(0),
				(uint8_t *)output_buff_left, rec_num_of_samples * sizeof(float));
	dsp_management_api_set_chain_output_buffer(pMain_dsp_chain, OUT_PAD(1),
			(uint8_t *)output_buff_right, rec_num_of_samples * sizeof(float));
	dsp_management_api_process_chain(pMain_dsp_chain);

	// put the same right buffer into left buffer
	test_num_of_samples = cyclic_buffer_add_items(
			proccessed_playback_cyclic_buffer_left,
			output_buff_right, rec_num_of_samples);
	if (test_num_of_samples != rec_num_of_samples)
	{
		CRITICAL_ERROR("cannot put requested samples");
	}
	test_num_of_samples = cyclic_buffer_add_items(
			proccessed_playback_cyclic_buffer_right,
			output_buff_right, rec_num_of_samples);
	if (test_num_of_samples != rec_num_of_samples)
	{
		CRITICAL_ERROR("cannot put requested samples");
	}

	*left_buff_to_record = output_buff_left;
	*right_buff_to_record = output_buff_right;
	*samples_to_record = rec_num_of_samples;
}
