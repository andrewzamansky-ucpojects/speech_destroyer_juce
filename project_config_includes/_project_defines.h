/*
 * file : project_defines.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _project_defines_H
#define _project_defines_H

#include "project_config.h"
#include <stddef.h> // include for NULL


#ifdef __cplusplus
	#define  EXTERN_C_FUNCTION    extern "C"
#else
	#define  EXTERN_C_FUNCTION
#endif


#define OS_TICK_IN_MICRO_SEC		1000


#define I2S_BUFF_LEN 		512
#define LATENCY_LENGTH		64
#define	NUM_OF_BYTES_PER_AUDIO_WORD		2// 2- 16bits , 4- 32bits



/******   task priorities *********/
#define HEARTBEAT_THREAD_PRIORITY				(tskIDLE_PRIORITY + 1)
#define APP_DEV_THREAD_PRIORITY					(tskIDLE_PRIORITY + 4)
#define SHELL_TASK_PRIORITY						(tskIDLE_PRIORITY + 2)
#define ASYNC_TX_WRAPPER_TASK_PRIORITY			(tskIDLE_PRIORITY + 2)
/*********************************/


#define ADDITIONAL_STACK_SAFETY_MARGIN	 20
#define DEFINE_STACK_SIZE(n)    (n + ADDITIONAL_STACK_SAFETY_MARGIN)
/******   task stack sizes *********/
#define HEARTBEAT_STACK_SIZE_BYTES				DEFINE_STACK_SIZE( 260 )
#define MAIN_STACK_SIZE_BYTES					DEFINE_STACK_SIZE( 360 )
#define SHELL_TASK_STACK_SIZE					DEFINE_STACK_SIZE( 700 )
#define ASYNC_TX_WRAPPER_TASK_STACK_SIZE		DEFINE_STACK_SIZE( 260 )
/***********************************/


/***********************************/

#endif /* */
