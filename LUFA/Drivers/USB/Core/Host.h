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
 *  \brief USB host mode definitions.
 *
 *  This file contains structures, function prototypes and macros related to USB host mode.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB driver
 *        dispatch header located in LUFA/Drivers/USB/USB.h.
 */

/** \ingroup Group_USB
 *  @defgroup Group_Host Host Management
 *
 *  USB Host mode related macros and enums. This module contains macros and enums which are used when
 *  the USB controller is initialized in host mode.
 *
 *  @{
 */

#ifndef __USBHOST_H__
#define __USBHOST_H__

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB/USB.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Indicates the fixed USB device address which any attached device is enumerated to when in
			 *  host mode. As only one USB device may be attached to the AVR in host mode at any one time
			 *  and that the address used is not important (other than the fact that it is non-zero), a
			 *  fixed value is specified by the library.
			 */
			#define USB_HOST_DEVICEADDRESS                 1

			#if !defined(USB_HOST_TIMEOUT_MS) || defined(__DOXYGEN__)
				/** Constant for the maximum software timeout period of sent USB control transactions to an attached
				 *  device. If a device fails to respond to a sent control request within this period, the
				 *  library will return a timeout error code.
				 *
				 *  This value may be overridden in the user project makefile as the value of the
				 *  \ref USB_HOST_TIMEOUT_MS token, and passed to the compiler using the -D switch.
				 */
				#define USB_HOST_TIMEOUT_MS                1000
			#endif

			#if !defined(HOST_DEVICE_SETTLE_DELAY_MS) || defined(__DOXYGEN__)
				/** Constant for the delay in milliseconds after a device is connected before the library
				 *  will start the enumeration process. Some devices require a delay of up to 5 seconds
				 *  after connection before the enumeration process can start or incorrect operation will
				 *  occur.
				 *
				 *  The default delay value may be overridden in the user project makefile by definining the
				 *  HOST_DEVICE_SETTLE_DELAY_MS token to tbe required delay in milliseconds, and passed to the
				 *  compiler using the -D switch.
				 */
				#define HOST_DEVICE_SETTLE_DELAY_MS        1000
			#endif

		/* Enums: */
			/** Enum for the various states of the USB Host state machine. Only some states are
			 *  implemented in the LUFA library - other states are left to the user to implement.
			 *
			 *  For information on each possible USB host state, refer to the USB 2.0 specification.
			 *  Several of the USB host states are broken up further into multiple smaller sub-states,
			 *  so that they can be internally implemented inside the library in an efficient manner.
			 *
			 *  \see \ref USB_HostState, which stores the current host state machine state.
			 */
			enum USB_Host_States_t
			{
				HOST_STATE_WaitForDeviceRemoval         = 0,  /**< Internally implemented by the library. This state can be
				                                               *   used by the library to wait until the attached device is
				                                               *   removed by the user - useful for when an error occurs or
				                                               *   further communication with the device is not needed. This
				                                               *   allows for other code to run while the state machine is
				                                               *   effectively disabled.
				                                               */
				HOST_STATE_WaitForDevice                = 1,  /**< Internally implemented by the library. This state indicates
				                                               *   that the stack is waiting for an interval to elapse before
				                                               *   continuing with the next step of the device enumeration
				                                               *   process.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Unattached                   = 2,  /**< Internally implemented by the library. This state indicates
				                                               *   that the host state machine is waiting for a device to be
				                                               *   attached so that it can start the enumeration process.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Powered                      = 3,  /**< Internally implemented by the library. This state indicates
				                                               *   that a device has been attached, and the library's internals
				                                               *   are being configured to begin the enumeration process.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Powered_WaitForDeviceSettle  = 4,  /**< Internally implemented by the library. This state indicates
				                                               *   that the stack is waiting for the initial settling period to
				                                               *   elapse before beginning the enumeration process.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Powered_WaitForConnect       = 5,  /**< Internally implemented by the library. This state indicates
				                                               *   that the stack is waiting for a connection event from the USB
				                                               *   controller to indicate a valid USB device has been attached to
				                                               *   the bus and is ready to be enumerated.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Powered_DoReset              = 6,  /**< Internally implemented by the library. This state indicates
				                                               *   that a valid USB device has been attached, and that it is
				                                               *   will now be reset to ensure it is ready for enumeration.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Powered_ConfigPipe           = 7,  /**< Internally implemented by the library. This state indicates
				                                               *   that the attached device is currently powered and reset, and
				                                               *   that the control pipe is now being configured by the stack.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Default                      = 8,  /**< Internally implemented by the library. This state indicates
				                                               *   that the stack is currently retrieving the control endpoint's
				                                               *   size from the device, so that the control pipe can be altered
				                                               *   to match.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Default_PostReset            = 9,  /**< Internally implemented by the library. This state indicates that
				                                               *   the control pipe is being reconfigured to match the retrieved
				                                               *   control endpoint size from the device, and the device's USB bus
				                                               *   address is being set.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Default_PostAddressSet       = 10, /**< Internally implemented by the library. This state indicates that
				                                               *   the device's address has now been set, and the stack is has now
				                                               *   completed the device enumeration process. This state causes the
				                                               *   stack to change the current USB device address to that set for
				                                               *   the connected device, before progressing to the user-implemented
				                                               *   \ref HOST_STATE_Addressed state for further communications.
				                                               *
				                                               *   \note Do not manually change to this state in the user code.
				                                               */
				HOST_STATE_Addressed                    = 11, /**< May be implemented by the user project. This state should
				                                               *   set the device configuration before progressing to the
				                                               *   \ref HOST_STATE_Configured state. Other processing (such as the
				                                               *   retrieval and processing of the device descriptor) should also
				                                               *   be placed in this state.
				                                               */
				HOST_STATE_Configured                   = 12, /**< May be implemented by the user project. This state should implement the
				                                               *   actual work performed on the attached device and changed to the
				                                               *   \ref HOST_STATE_Suspended or \ref HOST_STATE_WaitForDeviceRemoval states as needed.
				                                               */
				HOST_STATE_Suspended                    = 15, /**< May be implemented by the user project. This state should be maintained
				                                               *   while the bus is suspended, and changed to either the \ref HOST_STATE_Configured
				                                               *   (after resuming the bus with the USB_Host_ResumeBus() macro) or the
				                                               *   \ref HOST_STATE_WaitForDeviceRemoval states as needed.
				                                               */
			};

			/** Enum for the error codes for the \ref EVENT_USB_Host_HostError() event.
			 *
			 *  \see \ref Group_Events for more information on this event.
			 */
			enum USB_Host_ErrorCodes_t
			{
				HOST_ERROR_VBusVoltageDip       = 0, /**< VBUS voltage dipped to an unacceptable level. This
				                                      *   error may be the result of an attached device drawing
				                                      *   too much current from the VBUS line, or due to the
				                                      *   AVR's power source being unable to supply sufficient
				                                      *   current.
				                                      */
			};

			/** Enum for the error codes for the \ref EVENT_USB_Host_DeviceEnumerationFailed() event.
			 *
			 *  \see \ref Group_Events for more information on this event.
			 */
			enum USB_Host_EnumerationErrorCodes_t
			{
				HOST_ENUMERROR_NoError          = 0, /**< No error occurred. Used internally, this is not a valid
				                                      *   ErrorCode parameter value for the \ref EVENT_USB_Host_DeviceEnumerationFailed()
				                                      *   event.
				                                      */
				HOST_ENUMERROR_WaitStage        = 1, /**< One of the delays between enumeration steps failed
				                                      *   to complete successfully, due to a timeout or other
				                                      *   error.
				                                      */
				HOST_ENUMERROR_NoDeviceDetected = 2, /**< No device was detected, despite the USB data lines
				                                      *   indicating the attachment of a device.
				                                      */
				HOST_ENUMERROR_ControlError     = 3, /**< One of the enumeration control requests failed to
				                                      *   complete successfully.
				                                      */
				HOST_ENUMERROR_PipeConfigError  = 4, /**< The default control pipe (address 0) failed to
				                                      *   configure correctly.
				                                      */
			};

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Enums: */
			enum USB_Host_WaitMSErrorCodes_t
			{
				HOST_WAITERROR_Successful       = 0,
				HOST_WAITERROR_DeviceDisconnect = 1,
				HOST_WAITERROR_PipeError        = 2,
				HOST_WAITERROR_SetupStalled     = 3,
			};
	#endif

	/* Includes: */
		#if (ARCH == ARCH_AVR8)
			#include "AVR8/Host.h"
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

