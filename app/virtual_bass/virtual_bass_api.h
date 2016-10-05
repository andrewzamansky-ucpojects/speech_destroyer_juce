
#ifndef _virtual_bass_API_H_
#define _virtual_bass_API_H_

#include "src/_virtual_bass_prerequirements_check.h"

#include "common_dsp_api.h"

/*****************  defines  **************/


/**********  define API  types ************/


typedef enum
{
	IOCTL_VIRTUAL_BASS_SET_FIRST_HPF = IOCTL_DSP_LAST_COMMON_IOCTL+1,
	IOCTL_VIRTUAL_BASS_SET_SECOND_HPF,
	IOCTL_VIRTUAL_BASS_SET_GAIN
}VIRTUAL_BASS_API_ioctl_t;



/**********  define API  functions  ************/


extern char virtual_bass_module_name[] ;
#define VIRTUAL_BASS_API_MODULE_NAME	virtual_bass_module_name

#endif
