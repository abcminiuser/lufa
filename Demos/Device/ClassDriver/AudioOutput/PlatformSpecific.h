/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
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
 *  Header file for the demo's platform specific source PlatformSpecific_{ARCH}.c.
 */

#ifndef _PLATFORM_SPECIFIC_H_
#define _PLATFORM_SPECIFIC_H_

	/* Includes: */
		#if (ARCH == ARCH_AVR8)		
			#include <avr/io.h>
			#include <avr/wdt.h>
			#include <avr/power.h>
			#include <avr/interrupt.h>
			#include <stdlib.h>
		#elif (ARCH == ARCH_UC3)
			#include <LUFA/Platform/UC3/InterruptManagement.h>
			#include <LUFA/Platform/UC3/ClockManagement.h>

			#include <avr32/io.h>			
		#endif

		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/USB/USB.h>

	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      LEDS_LED1

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)

		#if (ARCH == ARCH_UC3)
			#if (BOARD == BOARD_EVK1101)
				#define PWM_GPIO_LEFT        13
				#define PWM_CHANNEL_LEFT     3
				#define PWM_GPIO_RIGHT       14
				#define PWM_CHANNEL_RIGHT    4
			#endif
		#endif
		
	/* Function Prototypes: */
		void SetupHardware(void);
		void ConfigureAudioOutput(bool Enable);
		void SetAudioSampleFrequency(const uint32_t SampleFrequency);
		
#endif

