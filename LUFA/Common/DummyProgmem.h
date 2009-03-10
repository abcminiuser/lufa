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
 *  TODO
 *
 *  \note Do not include this file directly, rather include the Common.h header file instead to gain this file's
 *        functionality.
 */

#ifndef __DUMMYPROGMEM_H__
#define __DUMMYPROGMEM_H__

	/* Preprocessor Checks: */
		#if !defined(__COMMON_H__)
			#error Do not include this file directly. Include LUFA/Common/Common.h instead to gain this functionality.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			#if (MCU_ARCHITECTURE != ARCH_AVR8)
				#define PROGMEM
				#define pgm_read_byte(...)
				#define pgm_read_word(...)
				#define pgm_read_dword(...)
				#define pgm_read_float(...)

				#define pgm_read_byte_near(...)
				#define pgm_read_word_near(...)
				#define pgm_read_dword_near(...)
				#define pgm_read_float_near(...)

				#define pgm_read_byte_far(...)
				#define pgm_read_word_far(...)
				#define pgm_read_dword_far(...)
				#define pgm_read_float_far(...)
			#endif
#endif
