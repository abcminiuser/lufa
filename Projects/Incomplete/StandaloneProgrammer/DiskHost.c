/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#include "DiskHost.h"

#if defined(USB_CAN_BE_HOST)
/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Host_t DiskHost_MS_Interface =
	{
		.Config =
			{
				.DataINPipe             =
					{
						.Address        = (PIPE_DIR_IN  | 1),
						.Banks          = 1,
					},
				.DataOUTPipe            =
					{
						.Address        = (PIPE_DIR_OUT | 2),
						.Banks          = 1,
					},
			},
	};


void DiskHost_USBTask(void)
{
	MS_Host_USBTask(&DiskHost_MS_Interface);
}

void EVENT_USB_Host_DeviceEnumerationComplete(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);

	uint16_t ConfigDescriptorSize;
	uint8_t  ConfigDescriptorData[512];

	if (USB_Host_GetDeviceConfigDescriptor(1, &ConfigDescriptorSize, ConfigDescriptorData,
										   sizeof(ConfigDescriptorData)) != HOST_GETCONFIG_Successful)
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	if (MS_Host_ConfigurePipes(&DiskHost_MS_Interface,
							   ConfigDescriptorSize, ConfigDescriptorData) != MS_ENUMERROR_NoError)
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	if (USB_Host_SetDeviceConfiguration(1) != HOST_SENDCONTROL_Successful)
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	uint8_t MaxLUNIndex;
	if (MS_Host_GetMaxLUN(&DiskHost_MS_Interface, &MaxLUNIndex))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	if (MS_Host_ResetMSInterface(&DiskHost_MS_Interface))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	SCSI_Request_Sense_Response_t SenseData;
	if (MS_Host_RequestSense(&DiskHost_MS_Interface, 0, &SenseData) != 0)
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	pf_mount(&DiskFATState);

	LEDs_SetAllLEDs(LEDMASK_USB_READY);
}

void EVENT_USB_Host_DeviceAttached(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

void EVENT_USB_Host_DeviceUnattached(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

#endif

