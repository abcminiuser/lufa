/*
             LUFA Library
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Board specific pushbuttons driver header for the STK525.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the pushbuttons driver
 *        dispatch header located in LUFA/Drivers/Board/Pushbuttons.h.
 */
 
#ifndef __PUSHBUTTONS_STK525_H__
#define __PUSHBUTTONS_STK525_H__

	/* Includes: */
		#include <avr/io.h>

		#include "../../../Common/Common.h"

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(INCLUDE_FROM_PUSHBUTTONS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/Pushbuttons.h instead.
		#endif
		
	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Total number of pushbuttons on the selected board */
			#define TOTAL_PUSHBUTTONS     1
	
			/** Mask of the first button on the board */
			#define BUTTON_1              (1 << 2)

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void Pushbuttons_Init(void)
			{
				DDRE  &= ~BUTTON_1;
				PORTE |=  BUTTON_1;
			}

			static inline uint8_t Pushbuttons_GetStatus(void) ATTR_WARN_UNUSED_RESULT;
			static inline uint8_t Pushbuttons_GetStatus(void)
			{
				return (!(PINE & BUTTON_1));
			}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif
				
#endif
