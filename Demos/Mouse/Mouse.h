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
 *  Header file for Mouse.c.
 */

#ifndef _MOUSE_H_
#define _MOUSE_H_

	/* Includes: */
		#include <LUFA/Common/Common.h>
		#include <LUFA/Version.h>                    // Library Version Information
		#include <LUFA/Common/ButtLoadTag.h>         // PROGMEM tags readable by the ButtLoad project
		#include <LUFA/Drivers/USB/USB.h>            // USB Functionality
		#include <LUFA/Drivers/Board/Joystick.h>     // Joystick driver
		#include <LUFA/Drivers/Board/LEDs.h>         // LEDs driver
		#include <LUFA/Drivers/Board/Pushbuttons.h>  // Pushbuttons driver
		#include <LUFA/Scheduler/Scheduler.h>        // Simple scheduler for task management
		
		#if (MCU_ARCHITECTURE == ARCH_AVR8)
			#include <avr/io.h>
			#include <avr/wdt.h>
			#include <avr/interrupt.h>
			#include <avr/power.h>
		#else
			#include <avr32/io.h>
		#endif
		
		#include <stdbool.h>
		#include <string.h>

		#include "Descriptors.h"
		
	/* Task Definitions: */
		TASK(USB_Mouse_Report);

	/* Macros: */
		/** HID Class specific request to get the next HID report from the device. */
		#define REQ_GetReport      0x01

		/** HID Class specific request to get the idle timeout period of the device. */
		#define REQ_GetIdle        0x02

		/** HID Class specific request to send the next HID report to the device. */
		#define REQ_SetReport      0x09

		/** HID Class specific request to set the idle timeout period of the device. */
		#define REQ_SetIdle        0x0A

		/** HID Class specific request to get the current HID protocol in use, either report or boot. */
		#define REQ_GetProtocol    0x03

		/** HID Class specific request to set the current HID protocol in use, either report or boot. */
		#define REQ_SetProtocol    0x0B

	/* Type Defines: */
		/** Type define for the mouse HID report structure, for creating and sending HID reports to the host PC.
		 *  This mirrors the layout described to the host in the HID report descriptor, in Descriptors.c.
		 */
		typedef struct
		{
			uint8_t Button; /**< Bit mask of the currently pressed mouse buttons */
			int8_t  X; /**< Current mouse delta X movement, as a signed 8-bit integer */
			int8_t  Y; /**< Current mouse delta Y movement, as a signed 8-bit integer */
		} USB_MouseReport_Data_t;
		
	/* Enums: */
		/** Enum for the possible status codes for passing to the UpdateStatus() function. */
		enum Mouse_StatusCodes_t
		{
			Status_USBNotReady    = 0, /**< USB is not ready (disconnected from a USB host) */
			Status_USBEnumerating = 1, /**< USB interface is enumerating */
			Status_USBReady       = 2, /**< USB interface is connected and ready */
		};
			
	/* Event Handlers: */
		/** Indicates that this module will catch the USB_Connect event when thrown by the library. */
		HANDLES_EVENT(USB_Connect);

		/** Indicates that this module will catch the USB_Disconnect event when thrown by the library. */
		HANDLES_EVENT(USB_Disconnect);

		/** Indicates that this module will catch the USB_ConfigurationChanged event when thrown by the library. */
		HANDLES_EVENT(USB_ConfigurationChanged);

		/** Indicates that this module will catch the USB_UnhandledControlPacket event when thrown by the library. */
		HANDLES_EVENT(USB_UnhandledControlPacket);

	/* Function Prototypes: */
		bool GetNextReport(USB_MouseReport_Data_t* ReportData);
		void UpdateStatus(uint8_t CurrentStatus);

#endif
