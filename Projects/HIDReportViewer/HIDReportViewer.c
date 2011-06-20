/*
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
 *  Main source file for the HIDReportViewer project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */
 
#include "HIDReportViewer.h"

/** Processed HID report descriptor items structure, containing information on each HID report element */
static HID_ReportInfo_t HIDReportInfo;

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Host_t Device_HID_Interface =
	{
		.Config =
			{
				.DataINPipeNumber       = 1,
				.DataINPipeDoubleBank   = false,

				.DataOUTPipeNumber      = 2,
				.DataOUTPipeDoubleBank  = false,

				.HIDInterfaceProtocol   = HID_CSCP_NonBootProtocol,

				.HIDParserData          = &HIDReportInfo
			},
	};


/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	puts_P(PSTR(ESC_FG_CYAN "HID Device Report Viewer Running.\r\n" ESC_FG_WHITE));

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

				if (HID_Host_ConfigurePipes(&Device_HID_Interface,
				                            ConfigDescriptorSize, ConfigDescriptorData) != HID_ENUMERROR_NoError)
				{
					puts_P(PSTR("Attached Device Not a Valid HID Device.\r\n"));
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

				if (HID_Host_SetReportProtocol(&Device_HID_Interface) != 0)
				{
					puts_P(PSTR("Error Setting Report Protocol Mode.\r\n"));
					LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
					USB_HostState = HOST_STATE_WaitForDeviceRemoval;
					break;
				}

				puts_P(PSTR("HID Device Enumerated.\r\n"));
				LEDs_SetAllLEDs(LEDMASK_USB_READY);
				USB_HostState = HOST_STATE_Configured;
				break;
			case HOST_STATE_Configured:
				LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
				
				printf_P(PSTR("\r\n\r\nTotal Reports: %" PRId8 "\r\n"), HIDReportInfo.TotalDeviceReports);
				for (uint8_t ReportIndex = 0; ReportIndex < HIDReportInfo.TotalDeviceReports; ReportIndex++)
				{
					HID_ReportSizeInfo_t* CurrReportIDInfo = &HIDReportInfo.ReportIDSizes[ReportIndex];

					uint8_t ReportSizeInBits      = CurrReportIDInfo->ReportSizeBits[HID_REPORT_ITEM_In];
					uint8_t ReportSizeOutBits     = CurrReportIDInfo->ReportSizeBits[HID_REPORT_ITEM_Out];
					uint8_t ReportSizeFeatureBits = CurrReportIDInfo->ReportSizeBits[HID_REPORT_ITEM_Feature];

					/* Print out the byte sizes of each report within the device */
					printf_P(PSTR("  + Report ID %" PRId8 " - In: %" PRId8 " bytes, Out: %" PRId8 " bytes, Feature: %" PRId8 " bytes\r\n"),
					         CurrReportIDInfo->ReportID,
							 ((ReportSizeInBits      >> 3) + ((ReportSizeInBits      & 0x07) != 0)),
							 ((ReportSizeOutBits     >> 3) + ((ReportSizeOutBits     & 0x07) != 0)),
							 ((ReportSizeFeatureBits >> 3) + ((ReportSizeFeatureBits & 0x07) != 0)));
				}
				
				printf_P(PSTR("\r\n\r\nReport Items: %d\r\n"), HIDReportInfo.TotalDeviceReports);
				for (uint8_t ItemIndex = 0; ItemIndex < HIDReportInfo.TotalReportItems; ItemIndex++)
				{
					const HID_ReportItem_t* RItem = &HIDReportInfo.ReportItems[ItemIndex];

					printf_P(PSTR("  + Item %" PRId8 ":\r\n"
					              "    - Report ID:          0x%02" PRIX8  "\r\n"
					              "    - Data Direction:     %s\r\n"
					              "    - Item Flags:         0x%02" PRIX8  "\r\n"
					              "    - Item Offset (Bits): 0x%02" PRIX8  "\r\n"
					              "    - Item Size (Bits):   0x%02" PRIX8  "\r\n"
					              "    - Usage Page:         0x%04" PRIX16 "\r\n"
					              "    - Usage:              0x%04" PRIX16 "\r\n"
					              "    - Unit Type:          0x%08" PRIX32 "\r\n"
					              "    - Unit Exponent:      0x%02" PRIX8  "\r\n"
					              "    - Logical Minimum:    0x%08" PRIX32 "\r\n"
					              "    - Logical Maximum:    0x%08" PRIX32 "\r\n"
					              "    - Physical Minimum:   0x%08" PRIX32 "\r\n"
					              "    - Physical Maximum:   0x%08" PRIX32 "\r\n"
					              "    - Collection Path:\r\n"),
					         ItemIndex,
					         RItem->ReportID,
					         ((RItem->ItemType == HID_REPORT_ITEM_In) ? "IN" : ((RItem->ItemType == HID_REPORT_ITEM_Out) ? "OUT" : "FEATURE")),
					         RItem->ItemFlags,
					         RItem->BitOffset,
					         RItem->Attributes.BitSize,
					         RItem->Attributes.Usage.Page,
					         RItem->Attributes.Usage.Usage,
					         RItem->Attributes.Unit.Type,
					         RItem->Attributes.Unit.Exponent,
					         RItem->Attributes.Logical.Minimum,
					         RItem->Attributes.Logical.Maximum,
					         RItem->Attributes.Physical.Minimum,
					         RItem->Attributes.Physical.Maximum);
					
					const HID_CollectionPath_t* CollectionPath  = RItem->CollectionPath;
					uint8_t                     CollectionDepth = 6;
					
					while (CollectionPath != NULL)
					{
						for (uint8_t i = 0; i < CollectionDepth; i++)
						  putchar(' ');

						printf_P(PSTR("- Type:  0x%02" PRIX8 "\r\n"), CollectionPath->Type);
					
						for (uint8_t i = 0; i < CollectionDepth; i++)
						  putchar(' ');

						printf_P(PSTR("- Usage: 0x%02" PRIX8 "\r\n"), CollectionPath->Usage);

						CollectionDepth += 3;
						CollectionPath   = CollectionPath->Parent;
					}
				}
				
				LEDs_SetAllLEDs(LEDMASK_USB_READY);
				USB_HostState = HOST_STATE_WaitForDeviceRemoval;
				break;
		}

		HID_Host_USBTask(&Device_HID_Interface);
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
	Serial_Init(9600, false);
	LEDs_Init();
	USB_Init();

	/* Create a stdio stream for the serial port for stdin and stdout */
	Serial_CreateStream(NULL);
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
	USB_Disable();

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

/** Callback for the HID Report Parser. This function is called each time the HID report parser is about to store
 *  an IN, OUT or FEATURE item into the HIDReportInfo structure. To save on RAM, we are able to filter out items
 *  we aren't interested in (preventing us from being able to extract them later on, but saving on the RAM they would
 *  have occupied).
 *
 *  \param[in] CurrentItem  Pointer to the item the HID report parser is currently working with
 *
 *  \return Boolean true if the item should be stored into the HID report structure, false if it should be discarded
 */
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem)
{
	return true;
}
