/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *  \brief Board specific LED driver header for the  Microchip AVR64DU32 Curiosity Nano.
 *  \copydetails Group_LEDs_AVR64DU32_CNANO
 *
 *  \note This file should not be included directly. It is automatically included as needed by the LEDs driver
 *        dispatch header located in LUFA/Drivers/Board/LEDs.h.
 */

/** \ingroup Group_LEDs
 *  \defgroup Group_LEDs_AVR64DU32_CNANO AVR64DU32_CNANO
 *  \brief Board specific LED driver header for the  Microchip AVR64DU32 Curiosity Nano.
 *
 *  Board specific LED driver header for the  Microchip AVR64DU32 Curiosity Nano.
 *
 *  <table>
 *    <tr><th>Name</th><th>Color</th><th>Info</th><th>Active Level</th><th>Port Pin</th></tr>
 *    <tr><td>LEDS_LED1</td><td>Yellow</td><td>LED0 LED</td><td>Low</td><td>PORTF.2</td></tr>
 *  </table>
 *
 *  @{
 */

#ifndef __LEDS_AVR64DU32_CNANO_H__
#define __LEDS_AVR64DU32_CNANO_H__

	/* Includes: */
		#include <avr/io.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_LEDS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/LEDS.h instead.
		#endif

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Macros: */
			#define LEDS_PORTF_LEDS       (LEDS_LED1)
	#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** LED mask for the first LED on the board. */
			#define LEDS_LED1        PIN2_bm

			/** LED mask for all the LEDs on the board. */
			#define LEDS_ALL_LEDS    (LEDS_LED1)

			/** LED mask for none of the board LEDs. */
			#define LEDS_NO_LEDS     0

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void LEDs_Init(void)
			{
				PORTF.DIRSET    = LEDS_PORTF_LEDS;
				PORTF.OUTCLR    = LEDS_PORTF_LEDS;

				PORTF.PIN2CTRL  = PORT_INVEN_bm;
			}

			static inline void LEDs_Disable(void)
			{
				PORTF.DIRCLR    = LEDS_PORTF_LEDS;
				PORTF.OUTCLR    = LEDS_PORTF_LEDS;

				PORTF.PIN2CTRL  = 0;
			}

			static inline void LEDs_TurnOnLEDs(const uint8_t LEDMask)
			{
				PORTF.OUTSET = LEDMask & LEDS_PORTF_LEDS;
			}

			static inline void LEDs_TurnOffLEDs(const uint8_t LEDMask)
			{
				PORTF.OUTCLR = LEDMask & LEDS_PORTF_LEDS;
			}

			static inline void LEDs_SetAllLEDs(const uint8_t LEDMask)
			{
				PORTF.OUTCLR = LEDS_PORTF_LEDS;

				PORTF.OUTSET = (LEDMask & LEDS_PORTF_LEDS);
			}

			static inline void LEDs_ChangeLEDs(const uint8_t LEDMask,
			                                   const uint8_t ActiveMask)
			{
				PORTF.OUTCLR = (LEDMask & LEDS_PORTF_LEDS);

				PORTF.OUTSET = (ActiveMask & LEDS_PORTF_LEDS);
			}

			static inline void LEDs_ToggleLEDs(const uint8_t LEDMask)
			{
				PORTF.OUTTGL = (LEDMask & LEDS_PORTF_LEDS);
			}

			ATTR_WARN_UNUSED_RESULT
			static inline uint8_t LEDs_GetLEDs(void)
			{
				return (PORTF_OUT & LEDS_PORTF_LEDS);
			}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

