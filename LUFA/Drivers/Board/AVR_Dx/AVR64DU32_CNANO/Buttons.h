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
 *  \brief Board specific Buttons driver header for the  Microchip AVR64DU32 Curiosity Nano.
 *  \copydetails Group_Buttons_AVR64DU32_CNANO
 *
 *  \note This file should not be included directly. It is automatically included as needed by the Buttons driver
 *        dispatch header located in LUFA/Drivers/Board/Buttons.h.
 */

/** \ingroup Group_Buttons
 *  \defgroup Group_Buttons_AVR64DU32_CNANO AVR64DU32_CNANO
 *  \brief Board specific Buttons driver header for the  Microchip AVR64DU32 Curiosity Nano.
 *
 *  Board specific Buttons driver header for the  Microchip AVR64DU32 Curiosity Nano.
 *
 *  <table>
 *    <tr><th>Name</th><th>Info</th><th>Active Level</th><th>Port Pin</th></tr>
 *    <tr><td>BUTTONS_BUTTON1</td><td>SW0 Button</td><td>Low</td><td>PORTF.6</td></tr>
 *  </table>
 *
 *  @{
 */

#ifndef __BUTTONS_AVR64DU32_CNANO_H__
#define __BUTTONS_AVR64DU32_CNANO_H__

	/* Includes: */
		#include <avr/io.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_BUTTONS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/Buttons.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Button mask for the first button on the board. */
			#define BUTTONS_BUTTON1          PIN6_bm

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void Buttons_Init(void)
			{
				PORTF.OUTCLR   = BUTTONS_BUTTON1;
				PORTF.PIN6CTRL = (PORT_PULLUPEN_bm | PORT_INVEN_bm);
			}

			static inline void Buttons_Disable(void)
			{
				PORTF.OUTCLR   = BUTTONS_BUTTON1;
				PORTF.PIN6CTRL = 0;
			}

			ATTR_WARN_UNUSED_RESULT
			static inline uint8_t Buttons_GetStatus(void)
			{
				return (VPORTF.IN & BUTTONS_BUTTON1);
			}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

