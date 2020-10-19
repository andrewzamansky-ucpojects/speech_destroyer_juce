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


#include  "NuAEC_rnn_chain.uml.c"

#define DMIC_BYTES_PER_PCM_CHANNEL  4
struct dsp_chain_t *pMain_dsp_chain;

void init_chain()
{
	float gain ;

	pMain_dsp_chain = DSP_MANAGEMENT_API_CREATE_STATIC_CHAIN(chain);

	gain = 6.0f; // 0 - no gain
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, rnn_gain,
			IOCTL_MULTIPLIER_SET_WEIGHT_DB, &gain );

	gain = 0.0f; // 0 - no gain
	dsp_management_api_ioctl_1_params(pMain_dsp_chain, passthrough_gain,
			IOCTL_MULTIPLIER_SET_WEIGHT_DB, &gain );

}
