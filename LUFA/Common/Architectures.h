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
 *  \brief Supported architecture defines.
 *
 *  This file contains constants which can be passed to the compiler (via setting the macro ARCH) in the
 *  user project makefile using the -D option to configure the library board-specific drivers.
 *
 *  \note Do not include this file directly, rather include the Common.h header file instead to gain this file's
 *        functionality.
 */

/** \ingroup Group_Common
 *  @defgroup Group_Architectures Architectures
 *
 *  Macros for indicating the chosen physical architecture to the library. These macros should be used when
 *  defining the ARCH token to the chosen hardware via the -D switch in the project makefile.
 *
 *  @{
 */

#ifndef __ARCHITECTURES_H__
#define __ARCHITECTURES_H__

	/* Preprocessor Checks: */
		#if !defined(__COMMON_H__)
			#error Do not include this file directly. Include LUFA/Common/Common.h instead to gain this functionality.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Selects the 8-bit MEGA AVR (AT90USB*, ATMEGA*) architecture. */
			#define ARCH_AVR8MEGA      0

			/** Selects the 32-bit UC3B AVR (AT32UC3B*) architecture. */
			#define ARCH_AVR32         1

			#if !defined(__DOXYGEN__)
				#define ARCH_           AVR8

				#if !defined(ARCH)
					#define ARCH        ARCH_AVR8
				#endif
			#endif

#endif

/** @} */

