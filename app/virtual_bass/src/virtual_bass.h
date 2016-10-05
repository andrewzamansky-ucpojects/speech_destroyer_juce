/*
 * file : VIRTUAL_BASS.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _VIRTUAL_BASS_H
#define _VIRTUAL_BASS_H

#include "src/_virtual_bass_prerequirements_check.h"


/***************   typedefs    *******************/



typedef struct
{
	float envelope_folower;
	float harmonic_out;
	float 	hu_1_B;
	float	hu_1_A;
	float 	threshold;
	float	attack;
	float	one_minus_attack;
	float	release;
	float	one_minus_release;
	float 	gain;
} VIRTUAL_BASS_Instance_t;




#endif /* */
