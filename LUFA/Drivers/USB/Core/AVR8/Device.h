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
 *  \brief USB device mode definitions.
 *
 *  This file contains structures, function prototypes and macros related to USB device mode.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB driver
 *        dispatch header located in LUFA/Drivers/USB/USB.h.
 */

/** \ingroup Group_USB
 *  @defgroup Group_Device Device Management
 *
 *  USB Device mode related macros and enums. This module contains macros and enums which are used when
 *  the USB controller is initialized in device mode.
 *
 *  @{
 */

#ifndef __USBDEVICE_AVR8_H__
#define __USBDEVICE_AVR8_H__

	/* Includes: */
		#include <avr/io.h>
		#include <avr/pgmspace.h>
		#include <avr/eeprom.h>
		#include <avr/boot.h>
		#include <util/atomic.h>

		#include "../../../../Common/Common.h"
		#include "../StdDescriptors.h"
		#include "../USBInterrupt.h"
		#include "../Endpoint.h"
		#include "../EndpointStream.h"

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB/USB.h instead.
		#endif
			
		/* Function Prototypes: */
			/** Sends a Remote Wakeup request to the host. This signals to the host that the device should
			 *  be taken out of suspended mode, and communications should resume.
			 *
			 *  Typically, this is implemented so that HID devices (mice, keyboards, etc.) can wake up the
			 *  host computer when the host has suspended all USB devices to enter a low power state.
			 *
			 *  \note This macro should only be used if the device has indicated to the host that it
			 *        supports the Remote Wakeup feature in the device descriptors, and should only be
			 *        issued if the host is currently allowing remote wakeup events from the device (i.e.,
			 *        the \ref USB_RemoteWakeupEnabled flag is set). When the NO_DEVICE_REMOTE_WAKEUP compile
			 *        time option is used, this macro is unavailable.
			 *        \n\n
			 *
			 *  \note The USB clock must be running for this function to operate. If the stack is initialized with
			 *        the \ref USB_OPT_MANUAL_PLL option enabled, the user must ensure that the PLL is running
			 *        before attempting to call this function.
			 *
			 *  \see \ref Group_Descriptors for more information on the RMWAKEUP feature and device descriptors.
			 */
			void USB_Device_SendRemoteWakeup(void);

		/* Inline Functions: */
			/** Returns the current USB frame number, when in device mode. Every millisecond the USB bus is active (i.e. enumerated to a host)
			 *  the frame number is incremented by one.
			 */
			static inline uint16_t USB_Device_GetFrameNumber(void)
			{
				return UDFNUM;
			}

			#if !defined(NO_SOF_EVENTS)
				/** Enables the device mode Start Of Frame events. When enabled, this causes the
				 *  \ref EVENT_USB_Device_StartOfFrame() event to fire once per millisecond, synchronized to the USB bus,
				 *  at the start of each USB frame when enumerated in device mode.
				 *
				 *  \note Not available when the NO_SOF_EVENTS compile time token is defined.
				 */
				static inline void USB_Device_EnableSOFEvents(void) ATTR_ALWAYS_INLINE;
				static inline void USB_Device_EnableSOFEvents(void)
				{
					USB_INT_Enable(USB_INT_SOFI);
				}

				/** Disables the device mode Start Of Frame events. When disabled, this stops the firing of the
				 *  \ref EVENT_USB_Device_StartOfFrame() event when enumerated in device mode.
				 *
				 *  \note Not available when the NO_SOF_EVENTS compile time token is defined.
				 */
				static inline void USB_Device_DisableSOFEvents(void) ATTR_ALWAYS_INLINE;
				static inline void USB_Device_DisableSOFEvents(void)
				{
					USB_INT_Disable(USB_INT_SOFI);
				}
			#endif

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Inline Functions: */
			#if !defined(NO_INTERNAL_SERIAL) && (USE_INTERNAL_SERIAL != NO_DESCRIPTOR)
			static inline void USB_Device_GetInternalSerialDescriptor(USB_Descriptor_String_t* const Descriptor,
			                                                          const uint8_t Length)
			{
				Descriptor->Header.Type = DTYPE_String;
				Descriptor->Header.Size = sizeof(USB_Descriptor_String_t) + (Length * sizeof(wchar_t));

				ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
				{
					uint8_t SigReadAddress = 0x0E;

					for (uint8_t SerialCharNum = 0; SerialCharNum < Length; SerialCharNum++)
					{
						uint8_t SerialByte = boot_signature_byte_get(SigReadAddress);

						if (SerialCharNum & 0x01)
						{
							SerialByte >>= 4;
							SigReadAddress++;
						}

						Descriptor->UnicodeString[SerialCharNum] = (SerialByte >= 10) ?
						                                            (('A' - 10) + SerialByte) : ('0' + SerialByte);
					}
				}
			}
			#endif

			#if (defined(USB_SERIES_4_AVR) || defined(USB_SERIES_6_AVR) || defined(USB_SERIES_7_AVR))
			static inline void USB_Device_SetLowSpeed(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Device_SetLowSpeed(void)
			{
				UDCON |=  (1 << LSM);
			}

			static inline void USB_Device_SetFullSpeed(void) ATTR_ALWAYS_INLINE;
			static inline void USB_Device_SetFullSpeed(void)
			{
				UDCON &= ~(1 << LSM);
			}
			#endif

			static inline void USB_Device_SetDeviceAddress(const uint8_t Address) ATTR_ALWAYS_INLINE;
			static inline void USB_Device_SetDeviceAddress(const uint8_t Address)
			{
				UDADDR  = ((UDADDR & (1 << ADDEN)) | (Address & 0x7F));
				UDADDR |= (1 << ADDEN);
			}

			static inline bool USB_Device_IsAddressSet(void) ATTR_ALWAYS_INLINE;
			static inline bool USB_Device_IsAddressSet(void)
			{
				return (UDADDR & (1 << ADDEN));
			}
	#endif

#endif

/** @} */

