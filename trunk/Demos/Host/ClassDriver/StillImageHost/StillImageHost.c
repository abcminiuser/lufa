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
 *
 *  Main source file for the StillImageHost demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "StillImageHost.h"

/** LUFA Still Image Class driver interface configuration and state information. This structure is
 *  passed to all Still Image Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_SI_Host_t DigitalCamera_SI_Interface =
	{
		.Config =
			{
				.DataINPipeNumber       = 1,
				.DataINPipeDoubleBank   = false,

				.DataOUTPipeNumber      = 2,
				.DataOUTPipeDoubleBank  = false,

				.EventsPipeNumber       = 3,
				.EventsPipeDoubleBank   = false,
			},
	};

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	puts_P(PSTR(ESC_FG_CYAN "Still Image Host Demo running.\r\n" ESC_FG_WHITE));

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	for (;;)
	{
		switch (USB_HostState)
		{
			case HOST_STATE_Addressed:
				LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);

				uint16_t ConfigDescriptorSize;
				uint8_t  ConfigDescriptorData[512];

				if (USB_Host_GetDeviceConfigDescriptor(1, &ConfigDescriptorSize, ConfigDescriptorData,
				                                       sizeof(ConfigDescriptorData)) != HOST_GETCONFIG_Successful)
				{
					puts_P(PSTR("Error Retrieving Configuration Descriptor.\r\n"));
					LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				if (SI_Host_ConfigurePipes(&DigitalCamera_SI_Interface,
				                           ConfigDescriptorSize, ConfigDescriptorData) != SI_ENUMERROR_NoError)
				{
					puts_P(PSTR("Attached Device Not a Valid Still Image Class Device.\r\n"));
					LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				if (USB_Host_SetDeviceConfiguration(1) != HOST_SENDCONTROL_Successful)
				{
					puts_P(PSTR("Error Setting Device Configuration.\r\n"));
					LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				puts_P(PSTR("Still Image Device Enumerated.\r\n"));
				LEDs_SetAllLEDs(LEDMASK_USB_READY);
				USB_HostState = HOST_STATE_Configured;
				break;
			case HOST_STATE_Configured:
				puts_P(PSTR("Opening Session...\r\n"));

				if (SI_Host_OpenSession(&DigitalCamera_SI_Interface) != PIPE_RWSTREAM_NoError)
				{
					puts_P(PSTR("Could not open PIMA session.\r\n"));
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				puts_P(PSTR("Turning off Device...\r\n"));

				SI_Host_SendCommand(&DigitalCamera_SI_Interface, 0x1013, 0, NULL);
				if (SI_Host_ReceiveResponse(&DigitalCamera_SI_Interface))
				{
					puts_P(PSTR("Could not turn off device.\r\n"));
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				puts_P(PSTR("Device Off.\r\n"));

				puts_P(PSTR("Closing Session...\r\n"));

				if (SI_Host_CloseSession(&DigitalCamera_SI_Interface) != PIPE_RWSTREAM_NoError)
				{
					puts_P(PSTR("Could not close PIMA session.\r\n"));
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				LEDs_SetAllLEDs(LEDMASK_USB_READY);
				USB_HostState = HOST_STATE_WaitForDeviceRemoval;
				break;
		}

		SI_Host_USBTask(&DigitalCamera_SI_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	SerialStream_Init(9600, false);
	LEDs_Init();
	USB_Init();
}

/** Event handler for the USB_DeviceAttached event. This indicates that a device has been attached to the host, and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Host_DeviceAttached(void)
{
	puts_P(PSTR("Device Attached.\r\n"));
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the USB_DeviceUnattached event. This indicates that a device has been removed from the host, and
 *  stops the library USB task management process.
 */
void EVENT_USB_Host_DeviceUnattached(void)
{
	puts_P(PSTR("\r\nDevice Unattached.\r\n"));
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the USB_DeviceEnumerationComplete event. This indicates that a device has been successfully
 *  enumerated by the host and is now ready to be used by the application.
 */
void EVENT_USB_Host_DeviceEnumerationComplete(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_READY);
}

/** Event handler for the USB_HostError event. This indicates that a hardware error occurred while in host mode. */
void EVENT_USB_Host_HostError(const uint8_t ErrorCode)
{
	USB_ShutDown();

	printf_P(PSTR(ESC_FG_RED "Host Mode Error\r\n"
	                         " -- Error Code %d\r\n" ESC_FG_WHITE), ErrorCode);

	LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
	for(;;);
}

/** Event handler for the USB_DeviceEnumerationFailed event. This indicates that a problem occurred while
 *  enumerating an attached USB device.
 */
void EVENT_USB_Host_DeviceEnumerationFailed(const uint8_t ErrorCode,
                                            const uint8_t SubErrorCode)
{
	printf_P(PSTR(ESC_FG_RED "Dev Enum Error\r\n"
	                         " -- Error Code %d\r\n"
	                         " -- Sub Error Code %d\r\n"
	                         " -- In State %d\r\n" ESC_FG_WHITE), ErrorCode, SubErrorCode, USB_HostState);

	LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
}

