/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
 *  \brief Standard avr-libc character stream driver for the USART.
 *
 *  Serial stream driver for the USART subsystem on supported USB AVRs. This makes use of the functions in the
 *  regular USART driver (see \ref Group_Serial), but allows the avr-libc standard stream functions (printf,
 *  puts, etc.) to work with the
 *  USART.
 */

/** \ingroup Group_PeripheralDrivers
 *  @defgroup Group_SerialStream Serial Stream Driver - LUFA/Drivers/Peripheral/SerialStream.h
 *
 *  \section Sec_Dependencies Module Source Dependencies
 *  The following files must be built with any user project that uses this module:
 *    - LUFA/Drivers/Peripheral/SerialStream.c <i>(Makefile source module name: LUFA_SRC_SERIALSTREAM)</i>
 *
 *  \section Module Description
 *  Serial stream driver for the USART subsystem on supported USB AVRs. This makes use of the functions in the
 *  regular USART driver (see \ref Group_Serial), but allows the avr-libc standard stream functions (printf,
 *  puts, etc.) to work with the
 *  USART.
 *
 *  @{
 */

#ifndef __SERIAL_STREAM_H__
#define __SERIAL_STREAM_H__

	/* Macros: */
	#if !defined(__DOXYGEN__)
		#define __INCLUDE_FROM_SPI_H
	#endif

	/* Includes: */
		#if (ARCH == ARCH_AVR8)
			#include "AVR8/SerialStream.h"
		#else
			#error "Serial UART Streams are not available for the currently selected architecture."
		#endif

#endif

/** @} */

