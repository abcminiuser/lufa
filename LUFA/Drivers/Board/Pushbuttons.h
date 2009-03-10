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
 *  This file is the master dispatch header file for the board-specific pushbuttons driver, for boards containing
 *  one or more physical pushbuttons connected to the AVR's I/O pins.
 *
 *  User code should include this file, which will in turn include the correct pushbuttons driver header file for the
 *  currently selected board.
 *
 *  If the BOARD value is set to BOARD_USER, this will include the /Board/Pushbuttons.h file in the user project
 *  directory.
 */
 
#ifndef __PUSHBUTTONS_H__
#define __PUSHBUTTONS_H__

	/* Macros: */
	#if !defined(__DOXYGEN__)
		#define INCLUDE_FROM_PUSHBUTTONS_H
		#define INCLUDE_FROM_BOARD_DRIVER
	#endif

	/* Includes: */
	#include "../../Common/Common.h"
	
	#if !defined(BOARD)
		#error BOARD must be set in makefile to a value specified in BoardTypes.h.
	#elif (BOARD == BOARD_USBKEY)
		#include "USBKEY/Pushbuttons.h"
	#elif (BOARD == BOARD_STK525)
		#include "STK525/Pushbuttons.h"
	#elif (BOARD == BOARD_STK526)
		#include "STK526/Pushbuttons.h"
	#elif (BOARD == BOARD_ATAVRUSBRF01)
		#include "ATAVRUSBRF01/Pushbuttons.h"
	#elif (BOARD == BOARD_EVK1101)
		#include "EVK1101/Pushbuttons.h"
	#elif (BOARD == BOARD_USER)
		#include "Board/Pushbuttons.h"		
	#else
		#error The selected board does not contain pushbuttons.
	#endif
	
	/* Psudo-Functions for Doxygen: */
	#if defined(__DOXYGEN__)
		/** Initializes the pushbuttons driver, so that the current button positions can be read. This sets the appropriate
		 *  I/O pins to inputs with pull-ups enabled.
		 *
		 *  This must be called before any pushbutton functions are used.
		 */
		static inline void Pushbuttons_Init(void);
		
		/** Returns the current position of the pushbuttons on the board.
		 *
		 *  \return BUTTON_* mask of currently pushed buttons
		 */
		static inline uint8_t Pushbuttons_GetStatus(void) ATTR_WARN_UNUSED_RESULT;
	#endif

#endif
