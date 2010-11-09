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
 *  \brief USB low level USB controller definitions.
 *
 *  This file contains structures, function prototypes and macros related to the low level configuration of the
 *  USB controller, to start, stop and reset the USB library core.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB driver
 *        dispatch header located in LUFA/Drivers/USB/USB.h.
 */

/** \ingroup Group_USB
 *  @defgroup Group_USBManagement USB Interface Management
 *
 *  Functions, macros, variables, enums and types related to the setup and management of the USB interface.
 *
 *  @{
 */

#ifndef __USBCONTROLLER_H__
#define __USBCONTROLLER_H__

	/* Includes: */
		#include <stdint.h>
		#include <stdbool.h>

		#include "USBMode.h"

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks and Defines: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB/USB.h instead.
		#endif

		#if !defined(F_CLOCK)
			#error F_CLOCK is not defined. You must define F_CLOCK to the frequency of the unprescaled input clock in your project makefile.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** \name USB Controller Option Masks */
			//@{
			/** Regulator disable option mask for \ref USB_Init(). This indicates that the internal 3.3V USB data pad
			 *  regulator should be enabled to regulate the data pin voltages to within the USB standard.
			 *
			 *  \note See USB AVR data sheet for more information on the internal pad regulator.
			 */
			#define USB_OPT_REG_DISABLED               (1 << 1)

			/** Regulator enable option mask for \ref USB_Init(). This indicates that the internal 3.3V USB data pad
			 *  regulator should be disabled and the AVR's VCC level used for the data pads.
			 *
			 *  \note See USB AVR data sheet for more information on the internal pad regulator.
			 */
			#define USB_OPT_REG_ENABLED                (0 << 1)

			/** Manual PLL control option mask for \ref USB_Init(). This indicates to the library that the user application
			 *  will take full responsibility for controlling the AVR's PLL (used to generate the high frequency clock
			 *  that the USB controller requires) and ensuring that it is locked at the correct frequency for USB operations.
			 */
			#define USB_OPT_MANUAL_PLL                 (1 << 2)

			/** Automatic PLL control option mask for \ref USB_Init(). This indicates to the library that the library should
			 *  take full responsibility for controlling the AVR's PLL (used to generate the high frequency clock
			 *  that the USB controller requires) and ensuring that it is locked at the correct frequency for USB operations.
			 */
			#define USB_OPT_AUTO_PLL                   (0 << 2)
			//@}
			
			/** \name Endpoint/Pipe Type Masks */
			//@{
			/** Mask for a CONTROL type endpoint or pipe.
			 *
			 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
			 */
			#define EP_TYPE_CONTROL                    0x00

			/** Mask for an ISOCHRONOUS type endpoint or pipe.
			 *
			 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
			 */
			#define EP_TYPE_ISOCHRONOUS                0x01

			/** Mask for a BULK type endpoint or pipe.
			 *
			 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
			 */
			#define EP_TYPE_BULK                       0x02

			/** Mask for an INTERRUPT type endpoint or pipe.
			 *
			 *  \note See \ref Group_EndpointManagement and \ref Group_PipeManagement for endpoint/pipe functions.
			 */
			#define EP_TYPE_INTERRUPT                  0x03
			//@}

			#if !defined(USB_STREAM_TIMEOUT_MS) || defined(__DOXYGEN__)
				/** Constant for the maximum software timeout period of the USB data stream transfer functions
				 *  (both control and standard) when in either device or host mode. If the next packet of a stream
				 *  is not received or acknowledged within this time period, the stream function will fail.
				 *
				 *  This value may be overridden in the user project makefile as the value of the
				 *  \ref USB_STREAM_TIMEOUT_MS token, and passed to the compiler using the -D switch.
				 */
				#define USB_STREAM_TIMEOUT_MS       100
			#endif

		/* Global Variables: */
			#if (!defined(USB_HOST_ONLY) && !defined(USB_DEVICE_ONLY)) || defined(__DOXYGEN__)
				/** Indicates the mode that the USB interface is currently initialized to, a value from the
				 *  \ref USB_Modes_t enum.
				 *
				 *  \note This variable should be treated as read-only in the user application, and never manually
				 *        changed in value.
				 *        \n\n
				 *
				 *  \note When the controller is initialized into UID autodetection mode, this variable will hold the
				 *        currently selected USB mode (i.e. \ref USB_MODE_Device or \ref USB_MODE_Host). If the controller
				 *        is fixed into a specific mode (either through the USB_DEVICE_ONLY or USB_HOST_ONLY compile time
				 *        options, or a limitation of the USB controller in the chosen device model) this will evaluate to
				 *        a constant of the appropriate value and will never evaluate to \ref USB_MODE_None even when the
				 *        USB interface is not initialized.
				 */
				extern volatile uint8_t USB_CurrentMode;
			#elif defined(USB_HOST_ONLY)
				#define USB_CurrentMode USB_MODE_Host
			#elif defined(USB_DEVICE_ONLY)
				#define USB_CurrentMode USB_MODE_Device
			#endif

			#if !defined(USE_STATIC_OPTIONS) || defined(__DOXYGEN__)
				/** Indicates the current USB options that the USB interface was initialized with when \ref USB_Init()
				 *  was called. This value will be one of the USB_MODE_* masks defined elsewhere in this module.
				 *
				 *  \note This variable should be treated as read-only in the user application, and never manually
				 *        changed in value.
				 */
				extern volatile uint8_t USB_Options;
			#elif defined(USE_STATIC_OPTIONS)
				#define USB_Options USE_STATIC_OPTIONS
			#endif

		/* Enums: */
			/** Enum for the possible USB controller modes, for initialization via \ref USB_Init() and indication back to the
			 *  user application via \ref USB_CurrentMode.
			 */
			enum USB_Modes_t
			{
				USB_MODE_None   = 0, /**< Indicates that the controller is currently not initialized in any specific USB mode. */
				USB_MODE_Device = 1, /**< Indicates that the controller is currently initialized in USB Device mode. */
				USB_MODE_Host   = 2, /**< Indicates that the controller is currently initialized in USB Host mode. */
				USB_MODE_UID    = 3, /**< Indicates that the controller should determine the USB mode from the UID pin of the
				                      *   USB connector.
				                      */
			};

	/* Includes: */
		#if (ARCH == ARCH_AVR8)
			#include "AVR8/USBController.h"
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

