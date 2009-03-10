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

/*
   This is a stub driver header file, for implementing custom board
   layout hardware with compatible LUFA board specific drivers. If
   the library is configured to use the BOARD_USER board mode, this
   driver file should be completed and copied into the "/Board/" folder
   inside the application's folder.

   This stub is for the board-specific component of the LUFA pushbuttons
   driver.
*/
 
#ifndef __PUSHBUTTONS_USER_H__
#define __PUSHBUTTONS_USER_H__

	/* Includes: */
		#include <avr/io.h>

		#include "../../../Common/Common.h"

		// TODO: Add any required includes here

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
			#define BUTTON_1              // TODO: Add mask for first board pushbutton here

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void Pushbuttons_Init(void)
			{
				// TODO: Initialize the appropriate port pin as an input here, with pullup
			}

			static inline uint8_t Pushbuttons_GetStatus(void) ATTR_WARN_UNUSED_RESULT;
			static inline uint8_t Pushbuttons_GetStatus(void)
			{
				// TODO: Return current button status here, debounced if required
			}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif
			
#endif
