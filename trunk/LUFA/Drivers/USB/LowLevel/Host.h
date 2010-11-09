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

	/* Includes: */
		#include <avr/io.h>
		#include <stdbool.h>
		#include <util/delay.h>

		#include "../../../Common/Common.h"
		#include "../HighLevel/StdDescriptors.h"
		#include "Pipe.h"
		#include "USBInterrupt.h"

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

		/* Inline Functions: */
			/** Returns the current USB frame number, when in host mode. Every millisecond the USB bus is active (i.e. not suspended)
			 *  the frame number is incremented by one.
			 */
			static inline uint16_t USB_Host_GetFrameNumber(void)
			{
				return UHFNUM;
			}

			#if !defined(NO_SOF_EVENTS)
				/** Enables the host mode Start Of Frame events. When enabled, this causes the
				 *  \ref EVENT_USB_Host_StartOfFrame() event to fire once per millisecond, synchronized to the USB bus,
				 *  at the start of each USB frame when a device is enumerated while in host mode.
				 *
				 *  \note Not available when the NO_SOF_EVENTS compile time token is defined.
				 */
				static inline void USB_Host_EnableSOFEvents(void) ATTR_ALWAYS_INLINE;
				static inline void USB_Host_EnableSOFEvents(void)
				{
					USB_INT_Enable(USB_INT_HSOFI);
				}

				/** Disables the host mode Start Of Frame events. When disabled, this stops the firing of the
				 *  \ref EVENT_USB_Host_StartOfFrame() event when enumerated in host mode.
				 *
				 *  \note Not available when the NO_SOF_EVENTS compile time token is defined.
				 */
				static inline void USB_Host_DisableSOFEvents(void) ATTR_ALWAYS_INLINE;
				static inline void USB_Host_DisableSOFEvents(void)
				{
					USB_INT_Disable(USB_INT_HSOFI);
				}
			#endif

			/** Resets the USB bus, including the endpoints in any attached device and pipes on the AVR host.
			 *  USB bus resets leave the default control pipe configured (if already configured).
			 *
			 *  If the USB bus has been suspended prior to issuing a bus reset, the attached device will be
			 *  woken up automatically and the bus resumed after the reset has been correctly issued.
			 */
			static inline void USB_Host_ResetBus(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_ResetBus(void)
			{
				UHCON |=  (1 << RESET);
			}

			/** Determines if a previously issued bus reset (via the \ref USB_Host_ResetBus() macro) has
			 *  completed.
			 *
			 *  \return Boolean true if no bus reset is currently being sent, false otherwise.
			 */
			static inline bool USB_Host_IsBusResetComplete(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool USB_Host_IsBusResetComplete(void)
			{
				return ((UHCON & (1 << RESET)) ? false : true);
			}

			/** Resumes USB communications with an attached and enumerated device, by resuming the transmission
			 *  of the 1MS Start Of Frame messages to the device. When resumed, USB communications between the
			 *  host and attached device may occur.
			 */
			static inline void USB_Host_ResumeBus(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_ResumeBus(void)
			{
				UHCON |=  (1 << SOFEN);
			}

			/** Suspends the USB bus, preventing any communications from occurring between the host and attached
			 *  device until the bus has been resumed. This stops the transmission of the 1MS Start Of Frame
			 *  messages to the device.
			 */
			static inline void USB_Host_SuspendBus(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_SuspendBus(void)
			{
				UHCON &= ~(1 << SOFEN);
			}

			/** Determines if the USB bus has been suspended via the use of the \ref USB_Host_SuspendBus() macro,
			 *  false otherwise. While suspended, no USB communications can occur until the bus is resumed,
			 *  except for the Remote Wakeup event from the device if supported.
			 *
			 *  \return Boolean true if the bus is currently suspended, false otherwise.
			 */
			static inline bool USB_Host_IsBusSuspended(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool USB_Host_IsBusSuspended(void)
			{
				return ((UHCON & (1 << SOFEN)) ? false : true);
			}

			/** Determines if the attached device is currently enumerated in Full Speed mode (12Mb/s), or
			 *  false if the attached device is enumerated in Low Speed mode (1.5Mb/s).
			 *
			 *  \return Boolean true if the attached device is enumerated in Full Speed mode, false otherwise.
			 */
			static inline bool USB_Host_IsDeviceFullSpeed(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool USB_Host_IsDeviceFullSpeed(void)
			{
				return ((USBSTA & (1 << SPEED)) ? true : false);
			}

			/** Determines if the attached device is currently issuing a Remote Wakeup request, requesting
			 *  that the host resume the USB bus and wake up the device, false otherwise.
			 *
			 *  \return Boolean true if the attached device has sent a Remote Wakeup request, false otherwise.
			 */
			static inline bool USB_Host_IsRemoteWakeupSent(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool USB_Host_IsRemoteWakeupSent(void)
			{
				return ((UHINT & (1 << RXRSMI)) ? true : false);
			}

			/** Clears the flag indicating that a Remote Wakeup request has been issued by an attached device. */
			static inline void USB_Host_ClearRemoteWakeupSent(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_ClearRemoteWakeupSent(void)
			{
				UHINT &= ~(1 << RXRSMI);
			}

			/** Accepts a Remote Wakeup request from an attached device. This must be issued in response to
			 *  a device's Remote Wakeup request within 2ms for the request to be accepted and the bus to
			 *  be resumed.
			 */
			static inline void USB_Host_ResumeFromWakeupRequest(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_ResumeFromWakeupRequest(void)
			{
				UHCON |=  (1 << RESUME);
			}

			/** Determines if a resume from Remote Wakeup request is currently being sent to an attached
			 *  device.
			 *
			 *  \return Boolean true if no resume request is currently being sent, false otherwise.
			 */
			static inline bool USB_Host_IsResumeFromWakeupRequestSent(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool USB_Host_IsResumeFromWakeupRequestSent(void)
			{
				return ((UHCON & (1 << RESUME)) ? false : true);
			}

		/* Function Prototypes: */
			/** Convenience function. This routine sends a SetConfiguration standard request to the attached
			 *  device, with the given configuration index. This can be used to easily set the device
			 *  configuration without creating and sending the request manually.
			 *
			 *  \note After this routine returns, the control pipe will be selected.
			 *
			 *  \param[in] ConfigNumber  Configuration index to send to the device.
			 *
			 *  \return A value from the \ref USB_Host_SendControlErrorCodes_t enum to indicate the result.
			 */
			uint8_t USB_Host_SetDeviceConfiguration(const uint8_t ConfigNumber);

			/** Convenience function. This routine sends a GetDescriptor standard request to the attached
			 *  device, requesting the device descriptor. This can be used to easily retrieve information
			 *  about the device such as its VID, PID and power requirements.
			 *
			 *  \note After this routine returns, the control pipe will be selected.
			 *
			 *  \param[out] DeviceDescriptorPtr  Pointer to the destination device descriptor structure where
			 *                                   the read data is to be stored.
			 *
			 *  \return A value from the \ref USB_Host_SendControlErrorCodes_t enum to indicate the result.
			 */
			uint8_t USB_Host_GetDeviceDescriptor(void* const DeviceDescriptorPtr);

			/** Convenience function. This routine sends a GetDescriptor standard request to the attached
			 *  device, requesting the string descriptor of the specified index. This can be used to easily
			 *  retrieve string descriptors from the device by index, after the index is obtained from the
			 *  Device or Configuration descriptors.
			 *
			 *  \note After this routine returns, the control pipe will be selected.
			 *
			 *  \param[in]  Index        Index of the string index to retrieve.
			 *  \param[out] Buffer       Pointer to the destination buffer where the retrieved string descriptor is
			 *                           to be stored.
			 *  \param[in] BufferLength  Maximum size of the string descriptor which can be stored into the buffer.
			 *
			 *  \return A value from the \ref USB_Host_SendControlErrorCodes_t enum to indicate the result.
			 */
			uint8_t USB_Host_GetDeviceStringDescriptor(const uint8_t Index,
			                                           void* const Buffer,
			                                           const uint8_t BufferLength);

			/** Clears a stall condition on the given pipe, via a ClearFeature request to the attached device.
			 *
			 *  \note After this routine returns, the control pipe will be selected.
			 *
			 *  \param[in] EndpointIndex  Index of the endpoint to clear, including the endpoint's direction.
			 *
			 *  \return A value from the \ref USB_Host_SendControlErrorCodes_t enum to indicate the result.
			 */
			uint8_t USB_Host_ClearPipeStall(const uint8_t EndpointIndex);

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Macros: */
			static inline void USB_Host_HostMode_On(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_HostMode_On(void)
			{
				USBCON |=  (1 << HOST);
			}

			static inline void USB_Host_HostMode_Off(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_HostMode_Off(void)
			{
				USBCON &= ~(1 << HOST);
			}

			static inline void USB_Host_VBUS_Auto_Enable(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_VBUS_Auto_Enable(void)
			{
				OTGCON &= ~(1 << VBUSHWC);
				UHWCON |=  (1 << UVCONE);
			}

			static inline void USB_Host_VBUS_Manual_Enable(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_VBUS_Manual_Enable(void)
			{
				OTGCON |=  (1 << VBUSHWC);
				UHWCON &= ~(1 << UVCONE);

				DDRE   |=  (1 << 7);
			}

			static inline void USB_Host_VBUS_Auto_On(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_VBUS_Auto_On(void)
			{
				OTGCON |=  (1 << VBUSREQ);
			}

			static inline void USB_Host_VBUS_Manual_On(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_VBUS_Manual_On(void)
			{
				PORTE  |=  (1 << 7);
			}

			static inline void USB_Host_VBUS_Auto_Off(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_VBUS_Auto_Off(void)
			{
				OTGCON |=  (1 << VBUSRQC);
			}

			static inline void USB_Host_VBUS_Manual_Off(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_VBUS_Manual_Off(void)
			{
				PORTE  &= ~(1 << 7);
			}

			static inline void USB_Host_SetDeviceAddress(const uint8_t Address) ATTR_ALWAYS_INLINE;
			static inline void USB_Host_SetDeviceAddress(const uint8_t Address)
			{
				UHADDR  =  (Address & 0x7F);
			}

		/* Enums: */
			enum USB_Host_WaitMSErrorCodes_t
			{
				HOST_WAITERROR_Successful       = 0,
				HOST_WAITERROR_DeviceDisconnect = 1,
				HOST_WAITERROR_PipeError        = 2,
				HOST_WAITERROR_SetupStalled     = 3,
			};

		/* Function Prototypes: */
			void    USB_Host_ProcessNextHostState(void);
			uint8_t USB_Host_WaitMS(uint8_t MS);

			#if defined(__INCLUDE_FROM_HOST_C)
				static void USB_Host_ResetDevice(void);
			#endif
	#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

