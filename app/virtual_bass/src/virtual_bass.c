/*
 *
 * file :   virtual_bass.c
 *
 *
 *
 *
 *
 */



/********  includes *********************/

#include "_virtual_bass_prerequirements_check.h"

#include "virtual_bass_api.h" //place first to test that header file is self-contained
#include "virtual_bass.h"
#include "common_dsp_api.h"

#include "auto_init_api.h"

#include "math.h"

#ifdef CONFIG_USE_HW_DSP
  #include "cpu_config.h"
  #include "arm_math.h"
#endif

#define DEBUG
//#include "gpio_api.h"
#include "equalizer_api.h"

/********  defines *********************/
#define ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT	1.0f

#define HU1_A	1.0f
#define HU1_B	0.03f
#define HU1_A_ADJUSTED	HU1_A //(HU1_A  / ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT)
#define HU1_B_ADJUSTED	HU1_B //(HU1_B / ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT)


/********  types  *********************/

/********  externals *********************/

extern pdev_descriptor_t debug_io0_dev, debug_io1_dev;

/********  exported variables *********************/

char virtual_bass_module_name[] = "virtual_bass";


/**********   external variables    **************/



/***********   local variables    **************/





/* ----------------------------------------------------------------------

** Fast approximation to the log2() function.  It uses a two step

** process.  First, it decomposes the floating-point number into

** a fractional component F and an exponent E.  The fraction component

** is used in a polynomial approximation and then the exponent added

** to the result.  A 3rd order polynomial is used and the result

** when computing db20() is accurate to 7.984884e-003 dB.

** ------------------------------------------------------------------- */


float log2f_approx_coeff[4] = {1.23149591368684f, -4.11852516267426f, 6.02197014179219f, -3.13396450166353f};


float log2f_approx(float X)
{
  float *C = &log2f_approx_coeff[0];
  float Y;
  float F;
  int E;

  // This is the approximation to log2()
  F = frexpf(fabsf(X), &E);

  //  Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;

  Y = *C++;
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y += E;

  return(Y);
}


#define LOG_COEFF	(20.0f/3.321928095f)
#define THRESHOLD	0.001778f

#define COMPR_ATTACK  	0.3875f
#define ONE_MINUS_COMPR_ATTACK  	(1.0f - COMPR_ATTACK)
#define COMPR_REALESE	0.2f
#define  ONE_MINUS_COMPR_REALESE	(1.0f - COMPR_REALESE)
#define RATIO			3.2f
//float vb_volume = 20;
float volatile mon ;

dsp_descriptor_t first_filter;
static dsp_chain_t *first_dsp_chain;

dsp_descriptor_t second_filter;
static dsp_chain_t *second_dsp_chain;

float tmp_buff[I2S_BUFF_LEN];
/*---------------------------------------------------------------------------------------------------------*/
/* Function:        virtual_bass_dsp                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
void virtual_bass_dsp(pdsp_descriptor apdsp , size_t data_len ,
		dsp_pad_t *in_pads[MAX_NUM_OF_OUTPUT_PADS] , dsp_pad_t out_pads[MAX_NUM_OF_OUTPUT_PADS])
{

	float *apCh1In ;
	float *apCh1Out  ;
	float curr_ratio ;
	VIRTUAL_BASS_Instance_t *handle;
	float harmonic_out;
	float *tmp_in_buf;
	float *tmp_out_buf;
	uint16_t i;

//	uint8_t envelope_folower_sample_count =ENVELOP_FOLLOWER_SAMPLE_RATE;
	float envelope_folower;
	float curr_y,curr_x;
	float 	hu_1_B;
	float	hu_1_A;
	float 	threshold;
	float	attack;
	float	one_minus_attack;
	float	release;
	float	one_minus_release;
	float	gain;

	handle = apdsp->handle;
	envelope_folower = handle->envelope_folower ;
	harmonic_out = handle->harmonic_out ;

	hu_1_B = handle->hu_1_B;
	hu_1_A = handle->hu_1_A;
	threshold = handle->threshold;
	attack = handle->attack;
	one_minus_attack = handle->one_minus_attack;
	release = handle->release;
	one_minus_release = handle->one_minus_release;
	gain = handle->gain;

	apCh1In = in_pads[0]->buff;
	apCh1Out = out_pads[0].buff;


#if 1
	tmp_in_buf = apCh1In;
	tmp_out_buf = tmp_buff;
	for(i = 0 ; i < data_len ; i++)
	{
		float	tmp ;
		curr_x = *tmp_in_buf++;

		if(curr_x < harmonic_out  )
		{
			tmp = hu_1_B;
		}
		else
		{
			tmp = hu_1_A;
		}
		harmonic_out *= (1.0f-tmp);
		tmp *=  curr_x ;
		harmonic_out += tmp ;
		*tmp_out_buf++ = harmonic_out ;//+ curr_x;
	}
#else
	memcpy(tmp_buff , apCh1In ,data_len * sizeof(float) );
#endif

#if 1
	DSP_SET_CHAIN_INPUT_BUFFER(first_dsp_chain,DSP_INPUT_PAD_0,tmp_buff );
	DSP_SET_CHAIN_OUTPUT_BUFFER(first_dsp_chain, DSP_OUTPUT_PAD_0, apCh1Out );// to save memory put harmonics temporary in  apCh1Out
	DSP_PROCESS_CHAIN(first_dsp_chain , data_len );
#else
	memcpy(apCh1Out , tmp_buff ,data_len * sizeof(float) );
#endif

#if 1

#if 1
	tmp_in_buf = apCh1Out;
	tmp_out_buf = tmp_buff;
	for(i = 0 ; i < data_len ; i++)
	{
		volatile float delta ;
		volatile float tmp ;
		volatile float curr_x_abs;


		curr_x = *tmp_in_buf++;
#if 1

		curr_x_abs = fabsf(curr_x);
//		mon = curr_x;


		delta = 1;
		if(curr_x_abs > threshold)
		{
			delta = threshold / curr_x_abs;
		}
//		mon = delta;

		if(delta > envelope_folower)
		{
			envelope_folower *= attack ;
			tmp = delta * one_minus_attack ;
		}
		else
		{
			envelope_folower *= release ;
			tmp = delta * one_minus_release ;
		}
		envelope_folower += tmp ;


//		mon = envelope_folower;

		tmp = RATIO;

		tmp = 1.0f/tmp;

		if(0==envelope_folower)
		{
			while(1);
		}	//	mon = tmp;
		envelope_folower = 1.0f / envelope_folower;


		curr_ratio = fast_pow(envelope_folower , tmp);
//		mon = curr_ratio;
#else
		curr_ratio = 1;
#endif
		curr_y = curr_x * curr_ratio;
		curr_y *= gain;
		*tmp_out_buf++ = curr_y;
	}
#else
	memcpy(tmp_buff , apCh1Out ,data_len * sizeof(float) );
#endif
	DSP_SET_CHAIN_INPUT_BUFFER(second_dsp_chain,DSP_INPUT_PAD_0,tmp_buff );
	DSP_SET_CHAIN_OUTPUT_BUFFER(second_dsp_chain, DSP_OUTPUT_PAD_0, apCh1Out );
	DSP_PROCESS_CHAIN(second_dsp_chain , data_len );

#else
	//memcpy(apCh1Out ,tmp_buff  ,data_len * sizeof(float) );

#endif
	handle->harmonic_out = harmonic_out ;
	handle->envelope_folower =envelope_folower ;

}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        virtual_bass_ioctl                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t virtual_bass_ioctl(pdsp_descriptor apdsp ,const uint8_t aIoctl_num , void * aIoctl_param1 , void * aIoctl_param2)
{
	VIRTUAL_BASS_Instance_t *handle = apdsp->handle;;
	equalizer_api_band_set_t band_set;
	equalizer_api_band_set_params_t  *p_band_set_params;
	float freq;
	p_band_set_params = &band_set.band_set_params;

	p_band_set_params->Gain = 1;

	switch(aIoctl_num)
	{

		case IOCTL_DSP_INIT :

			handle->envelope_folower = 1 ;
			handle->harmonic_out = 0 ;

			handle->hu_1_B = HU1_B_ADJUSTED;
			handle->hu_1_A = HU1_A_ADJUSTED;
			handle->threshold = THRESHOLD;
			handle->attack = COMPR_REALESE;
			handle->one_minus_attack = ONE_MINUS_COMPR_ATTACK ;
			handle->release = COMPR_REALESE;
			handle->one_minus_release = ONE_MINUS_COMPR_REALESE;
			handle->gain = 20;

			/*  first dsp chain*/
			first_dsp_chain = DSP_CREATE_CHAIN(2);
			DSP_ADD_MODULE_TO_CHAIN(first_dsp_chain, EQUALIZER_API_MODULE_NAME, &first_filter);

			DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(first_dsp_chain, DSP_INPUT_PAD_0, &first_filter, DSP_INPUT_PAD_0);
			DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(first_dsp_chain, DSP_OUTPUT_PAD_0, &first_filter, DSP_OUTPUT_PAD_0);

			DSP_IOCTL_1_PARAMS(&first_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , 3 );

			/*  second dsp chain*/
			second_dsp_chain = DSP_CREATE_CHAIN(2);
			DSP_ADD_MODULE_TO_CHAIN(second_dsp_chain, EQUALIZER_API_MODULE_NAME, &second_filter);

			DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(second_dsp_chain, DSP_INPUT_PAD_0, &second_filter, DSP_INPUT_PAD_0);
			DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(second_dsp_chain, DSP_OUTPUT_PAD_0, &second_filter, DSP_OUTPUT_PAD_0);

			DSP_IOCTL_1_PARAMS(&second_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , 3 );

			break;

		case IOCTL_VIRTUAL_BASS_SET_FIRST_HPF :
			freq = *(float*)aIoctl_param1;
			p_band_set_params->QValue = 0.707;//0.836;//0.707;
			p_band_set_params->Gain = 1;
			p_band_set_params->Fc = 10;
			p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			band_set.band_num = 0;
			DSP_IOCTL_1_PARAMS(&first_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );
			p_band_set_params->QValue = 0.707;//0.836;//0.707;
			p_band_set_params->Fc = freq;
			p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			band_set.band_num = 1;
			DSP_IOCTL_1_PARAMS(&first_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );
			band_set.band_num = 2;
			DSP_IOCTL_1_PARAMS(&first_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );

			p_band_set_params->QValue = 0.8;//0.836;//0.707;
			p_band_set_params->Gain = 1;
			p_band_set_params->Fc = freq;
			p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
			band_set.band_num = 0;
			DSP_IOCTL_1_PARAMS(&second_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );

			break ;

		case IOCTL_VIRTUAL_BASS_SET_SECOND_HPF :
			freq = *(float*)aIoctl_param1;
			p_band_set_params->Gain = 1;
			p_band_set_params->QValue = 0.8;//0.836;//0.707;
			p_band_set_params->Fc = freq;
			p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			band_set.band_num = 1;
			DSP_IOCTL_1_PARAMS(&second_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );
			band_set.band_num = 2;
			DSP_IOCTL_1_PARAMS(&second_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );

			break ;
		case IOCTL_VIRTUAL_BASS_SET_GAIN :
			handle->gain = *(float*)aIoctl_param1;
			break;
		default :
			return 1;
	}
	return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        virtual_bass_init                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
void  virtual_bass_init(void)
{
	DSP_REGISTER_NEW_MODULE("virtual_bass",virtual_bass_ioctl , virtual_bass_dsp , VIRTUAL_BASS_Instance_t);
}

AUTO_INIT_FUNCTION(virtual_bass_init);
