/*
 * defines.h
 *
 *  Created on: Oct 28, 2019
 *      Author: krad2
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include <msp430.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <config.h>

/* Determine structure sizes */

#if defined(__MSP430X__)
	#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
		#define RTOS_20BIT__
	#else
		#define RTOS_16BIT__
	#endif

	#define RTOS_MSP430X_TRAPFRAME__
#else
	#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
		#error "20-bit settings cannot be compiled for 16-bit kernel / machine"
	#else
		#define RTOS_16BIT__
	#endif

	#define RTOS_MSP430_TRAPFRAME__
#endif

/* Define structure sizes */

#if defined(RTOS_20BIT__)
	typedef uint32_t word_t;
#elif defined(RTOS_16BIT__)
	typedef uint16_t word_t;
#endif

#define RTOS_STACK_CHECK__

#endif /* DEFINES_H_ */
