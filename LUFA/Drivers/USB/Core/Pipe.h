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
 *  \brief USB host pipe management definitions.
 *
 *  This file contains structures, function prototypes and macros related to the management of the device's
 *  data pipes when the library is initialized in USB host mode.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB driver
 *        dispatch header located in LUFA/Drivers/USB/USB.h.
 */

/** \ingroup Group_PipeManagement
 *  @defgroup Group_PipeRW Pipe Data Reading and Writing
 *
 *  Functions, macros, variables, enums and types related to data reading and writing from and to pipes.
 */

/** \ingroup Group_PipeRW
 *  @defgroup Group_PipePrimitiveRW Read/Write of Primitive Data Types
 *
 *  Functions, macros, variables, enums and types related to data reading and writing of primitive data types
 *  from and to pipes.
 */

/** \ingroup Group_PipeManagement
 *  @defgroup Group_PipePacketManagement Pipe Packet Management
 *
 *  Functions, macros, variables, enums and types related to packet management of pipes.
 */

/** \ingroup Group_PipeManagement
 *  @defgroup Group_PipeControlReq Pipe Control Request Management
 *
 *  Module for host mode request processing. This module allows for the transmission of standard, class and
 *  vendor control requests to the default control endpoint of an attached device while in host mode.
 *
 *  \see Chapter 9 of the USB 2.0 specification.
 */

/** \ingroup Group_USB
 *  @defgroup Group_PipeManagement Pipe Management
 *
 *  This module contains functions, macros and enums related to pipe management when in USB Host mode. This
 *  module contains the pipe management macros, as well as pipe interrupt and data send/receive functions
 *  for various data types.
 *
 *  @{
 */

#ifndef __PIPE_H__
#define __PIPE_H__

	/* Includes: */
		#include <avr/io.h>
		#include <stdbool.h>

		#include "../../../Common/Common.h"
		#include "USBTask.h"
		
		#if (ARCH == ARCH_AVR8)
			#include "AVR8/Pipe.h"
		#endif

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB/USB.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Enums: */
			/** Enum for the possible error return codes of the Pipe_WaitUntilReady function.
			 *
			 *  \ingroup Group_PipeRW
			 */
			enum Pipe_WaitUntilReady_ErrorCodes_t
			{
				PIPE_READYWAIT_NoError                 = 0, /**< Pipe ready for next packet, no error. */
				PIPE_READYWAIT_PipeStalled             = 1,	/**< The device stalled the pipe while waiting. */
				PIPE_READYWAIT_DeviceDisconnected      = 2,	/**< Device was disconnected from the host while waiting. */
				PIPE_READYWAIT_Timeout                 = 3, /**< The device failed to accept or send the next packet
				                                             *   within the software timeout period set by the
				                                             *   \ref USB_STREAM_TIMEOUT_MS macro.
				                                             */
			};

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

