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
#include "multiplier_1ch_api.h"
#include "fir_filter_api.h"

#include "nuvoton_aec_api.h"
#include  "fir_chain.uml.c"

struct dsp_chain_t *pMain_dsp_chain;

void init_chain()
{
	float gain ;
	struct fir_filter_api_set_params_t fir_set_params;

	pMain_dsp_chain = DSP_MANAGEMENT_API_CREATE_STATIC_CHAIN(chain);

	fir_set_params.set_coefficients_type = FIR_CALCULATE_COEFFICIENTS_BY_PARAMS;
	fir_set_params.coeff_by_params.filter_mode = FIR_LOWPASS_MODE;
	fir_set_params.coeff_by_params.fc = 6000;
	fir_set_params.coeff_by_params.dfc = 1000;
	fir_set_params.coeff_by_params.A_stop = 90;
	fir_set_params.coeff_by_params.sample_rate_Hz = 48000;
	fir_set_params.number_of_filter_coefficients = 25;//401;
	fir_set_params.predefined_data_block_size = 128;
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, fir_filter,
					IOCTL_FIR_FILTER_SET_FIR_COEFFICIENTS, &fir_set_params);

	gain = 0.0f; // 0 - no gain
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, passthrough_gain,
			IOCTL_MULTIPLIER_SET_WEIGHT_DB, &gain );

}
