/*
 *
 * file :   rnn_chain.c
 *
 *
 */

#include "_project.h"
#include "dev_management_api.h"


#define DEBUG
#include "PRINTF_api.h"


#include "dsp_management_api.h"
#include "biquad_filter_api.h"
#include "pcm_splitter_api.h"
#include "pcm_mixer_api.h"
#include "math.h"
#include "multiplier_1ch_api.h"
#include "multiplier_2ch_api.h"
#include "mixer2x1_api.h"
#include "rnn_api.h"
#if defined(USE_SENSORY)
	#include "sensory_api.h"
#endif

#include "nuvoton_aec_api.h"
#include "downsampling_by_int_api.h"


#include  "NuAEC_rnn_chain.uml.c"

#define DMIC_BYTES_PER_PCM_CHANNEL  4
struct dsp_chain_t *pMain_dsp_chain;

void init_chain()
{
	float gain ;
	struct downsampling_by_int_api_set_params_t downsample_set_params;

	pMain_dsp_chain = DSP_MANAGEMENT_API_CREATE_STATIC_CHAIN(chain);

	downsample_set_params.input_sample_rate_Hz = 48000;
	downsample_set_params.output_sample_rate_Hz = 16000;
	downsample_set_params.number_of_coefficients_in_lowpass_filter = 25;
	downsample_set_params.predefined_data_block_size = 125;
	dsp_management_api_ioctl_1_params(
			pMain_dsp_chain, downsample_filter_mic_left,
			IOCTL_DOWNSAMPLING_BY_INT_SET_PARAMS, &downsample_set_params);
	dsp_management_api_ioctl_1_params(
			pMain_dsp_chain, downsample_filter_mic_right,
			IOCTL_DOWNSAMPLING_BY_INT_SET_PARAMS, &downsample_set_params);
	dsp_management_api_ioctl_1_params(
			pMain_dsp_chain, downsample_filter_playback_left,
			IOCTL_DOWNSAMPLING_BY_INT_SET_PARAMS, &downsample_set_params);
	dsp_management_api_ioctl_1_params(
			pMain_dsp_chain, downsample_filter_playback_right,
			IOCTL_DOWNSAMPLING_BY_INT_SET_PARAMS, &downsample_set_params);

	gain = 6.0f; // 0 - no gain
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, rnn_gain,
			IOCTL_MULTIPLIER_SET_WEIGHT_DB, &gain );

	gain = 0.0f; // 0 - no gain
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, passthrough_gain,
			IOCTL_MULTIPLIER_SET_WEIGHT_DB, &gain );
}
