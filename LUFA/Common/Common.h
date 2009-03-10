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
 *  This file contains macros which are common to all library elements, and which may be useful in user code. It
 *  also includes other common headees, such as Atomic.h, FunctionAttributes.h and BoardTypes.h.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

	/* Includes: */
		#include "Architecture.h"
		#include "FunctionAttributes.h"
		#include "BoardTypes.h"
		#include "DummyProgmem.h"
		
		#include <stdint.h>
		#include <stdio.h>
		#include <alloca.h>

	/* Public Interface - May be used in end-application: */
		/* Macros: */		
			/** Macro for encasing other multi-statment macros. This should be used along with an opening brace
			 *  before the start of any multi-statement macro, so that the macros contents as a whole are treated
			 *  as a discreete block and not as a list of seperate statements which may cause problems when used as
			 *  a block (such as inline IF statments).
			 */
			#define MACROS                  do

			/** Macro for encasing other multi-statment macros. This should be used along with a preceeding closing
			 *  brace at the end of any multi-statement macro, so that the macros contents as a whole are treated
			 *  as a discreete block and not as a list of seperate statements which may cause problems when used as
			 *  a block (such as inline IF statments).
			 */
			#define MACROE                  while (0)
			
			/** Defines a volatile NOP statment which cannot be optimized out by the compiler, and thus can always
			 *  be set as a breakpoint in the resulting code. Useful for debugging purposes, where the optimizer
			 *  removes/reorders code to the point where break points cannot reliably be set.
			 */
			#define JTAG_DEBUG_POINT()      asm volatile ("NOP" ::)

			/** Defines an explicit JTAG break point in the resulting binary via the ASM BREAK statment. When
			 *  a JTAG is used, this causes the program execution to halt when reached until manually resumed. */
			#define JTAG_DEBUG_BREAK()      asm volatile ("BREAK" ::)
			
			/** Macro for testing condition "x" and breaking via JTAG_DEBUG_BREAK() if the condition is false. */
			#define JTAG_DEBUG_ASSERT(x)    MACROS{ if (!(x)) { JTAG_DEBUG_BREAK(); } }MACROE

			/** Macro for testing condition "x" and writing debug data to the serial stream if false. As a
			 *  prerequisite for this macro, the serial stream should be configured via the Serial_Stream driver.
			 *
			 *  The serial output takes the form "{FILENAME}: Function {FUNCTION NAME}, Line {LINE NUMBER}: Assertion
			 *  {x} failed."
			 */
			#define SERIAL_STREAM_ASSERT(x) MACROS{ if (!(x)) { printf_P(PSTR("%s: Function \"%s\", Line %d: "   \
																"Assertion \"%s\" failed.\r\n"),   \
																__FILE__, __func__, __LINE__, #x); \
			                                } }MACROE

		/* Inline Functions: */
			/** Function to reverse the individual bits in a byte - i.e. bit 7 is moved to bit 0, bit 6 to bit 1,
			 *  etc.
			 *
			 *  \param Byte   Byte of data whose bits are to be reversed
			 */
			static inline uint8_t BitReverse(uint8_t Byte) ATTR_WARN_UNUSED_RESULT ATTR_CONST;
			static inline uint8_t BitReverse(uint8_t Byte)
			{
				Byte = (((Byte & 0xF0) >> 4) | ((Byte & 0x0F) << 4));
				Byte = (((Byte & 0xCC) >> 2) | ((Byte & 0x33) << 2));
				Byte = (((Byte & 0xAA) >> 1) | ((Byte & 0x55) << 1));

				return Byte;
			}
			
			/** Function to reverse the byte ordering of the individual bytes in a 16 bit number.
			 *
			 *  \param Word   Word of data whose bytes are to be swapped
			 */
			static inline uint16_t SwapEndian_16(uint16_t Word) ATTR_WARN_UNUSED_RESULT ATTR_CONST;
			static inline uint16_t SwapEndian_16(uint16_t Word)
			{
				return ((Word >> 8) | (Word << 8));				
			}

			/** Function to reverse the byte ordering of the individual bytes in a 32 bit number.
			 *
			 *  \param DWord   Double word of data whose bytes are to be swapped
			 */
			static inline uint32_t SwapEndian_32(uint32_t DWord) ATTR_WARN_UNUSED_RESULT ATTR_CONST;
			static inline uint32_t SwapEndian_32(uint32_t DWord)
			{
				return (((DWord & 0xFF000000) >> 24) |
				        ((DWord & 0x00FF0000) >> 8)  |
						((DWord & 0x0000FF00) << 8)  |
						((DWord & 0x000000FF) << 24));
			}

			/** Function to reverse the byte ordering of the individual bytes in a n byte number.
			 *
			 *  \param Data   Pointer to a number containing an even number of bytes to be reversed
			 *  \param Bytes  Length of the data in bytes
			 */
			static inline void SwapEndian_n(uint8_t* Data, uint8_t Bytes);
			static inline void SwapEndian_n(uint8_t* Data, uint8_t Bytes)
			{
				uint8_t Temp;
				
				while (Bytes)
				{
					Temp = *Data;
					*Data = *(Data + Bytes - 1);
					*(Data + Bytes) = Temp;

					Data++;
					Bytes -= 2;
				}
			}

#endif
