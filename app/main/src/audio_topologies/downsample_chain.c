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
#include "math.h"
#include "downsampling_by_int_api.h"

#include "nuvoton_aec_api.h"
#include  "downsample_chain.uml.c"

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
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, downsample_filter,
			IOCTL_DOWNSAMPLING_BY_INT_SET_PARAMS, &downsample_set_params);
}
