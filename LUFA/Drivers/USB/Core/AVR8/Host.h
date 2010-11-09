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

#ifndef __USBHOST_AVR8_H__
#define __USBHOST_AVR8_H__

	/* Includes: */
		#include <avr/io.h>
		#include <stdbool.h>
		#include <util/delay.h>

		#include "../../../../Common/Common.h"
		#include "../StdDescriptors.h"
		#include "../Pipe.h"
		#include "../USBInterrupt.h"

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB/USB.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
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

