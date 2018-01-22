/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2018  Filipe Rodrigues (filipepazrodrigues [at] gmail [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
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
 *  Main source file for the CCID demo. This file contains the main tasks of the demo and
 *  is responsible for the initial application hardware configuration.

 *  WARNING: This code is not production ready and should not by any means be considered safe.
 *  If you plan to integrate it into your application, you should seriously consider strong
 *  encryption algorithms or a secure microprocessor. Since Atmel AVR microprocessors do not 
 *  have any security requirement (therefore they don't offer any known protection against
 *  side channel attacks or fault injection) a secure microprocessor is the best option.
 */

#define  INCLUDE_FROM_CCID_C
#include "CCID.h"
#include <stdlib.h>



uint8_t CALLBACK_CCID_IccPowerOff(uint8_t slot, uint8_t *error) {
	if(slot == 0) {
		*error = CCID_ERROR_NOERROR;
		return CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR | CCID_ICCSTATUS_NOICCPRESENT;
	} else {
		*error = CCID_ERROR_SLOTNOTFOUND;
                return CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_NOICCPRESENT;
	}
}

uint8_t CALLBACK_CCID_GetSlotStatus(uint8_t slot, uint8_t *error) {
	if(slot == 0) {
		*error = CCID_ERROR_NOERROR;
		return CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR | CCID_ICCSTATUS_PRESENTANDACTIVE;
	} else {
		 *error = CCID_ERROR_SLOTNOTFOUND;
                return CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_NOICCPRESENT;
	}
}

bool CCID_NoError(int status) {
	return (status && 0xC0) == 0x0;
}
void CCID_Device_USB_Task() {

	Endpoint_SelectEndpoint(CCID_OUT_EPADDR);
	if (Endpoint_IsOUTReceived())
	{
		
		USB_CCID_BulkMessage_Header_t CCIDHeader;
		CCIDHeader.MessageType = Endpoint_Read_8();
		CCIDHeader.Length = Endpoint_Read_32_LE();
		CCIDHeader.Slot = Endpoint_Read_8();
		CCIDHeader.Seq = Endpoint_Read_8();
	
		uint8_t status;
		uint8_t error = CCID_ERROR_NOERROR;

		if(CCIDHeader.MessageType == PC_to_RDR_IccPowerOn) {


			int atrLength;

			uint8_t atrBuffer[17] = {0x3B, 0x8C, 0x80, 0x01, 0x59, 0x75, 0x62, 0x69, 0x6B, 0x65, 0x79, 0x4E, 0x45, 0x4F, 0x72, 0x33, 0x58};
			atrLength = 17;
			status = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR | CCID_ICCSTATUS_PRESENTANDACTIVE;

			USB_CCID_RDR_to_PC_DataBlock_t* ResponseATR = malloc(sizeof(USB_CCID_RDR_to_PC_DataBlock_t) + (atrLength * sizeof(uint8_t)));

			ResponseATR->CCIDHeader.MessageType = RDR_to_PC_DataBlock;
			ResponseATR->CCIDHeader.Slot = CCIDHeader.Slot;
			ResponseATR->CCIDHeader.Seq = CCIDHeader.Seq;
			ResponseATR->ChainParam = 0;

			if(CCID_NoError(status)) {
				ResponseATR->CCIDHeader.Length = atrLength;
				memcpy(&ResponseATR->Data, atrBuffer, sizeof(uint8_t) * atrLength);
				
			} else {
				atrLength = 0;
			}
			
			ResponseATR->Status = status;
			ResponseATR->Error = error;

			Endpoint_ClearOUT();

			Endpoint_SelectEndpoint(CCID_IN_EPADDR);
			Endpoint_Write_Stream_LE(ResponseATR, sizeof(USB_CCID_RDR_to_PC_DataBlock_t)  + (atrLength * sizeof(uint8_t)), NULL);
			Endpoint_ClearIN();
			free(ResponseATR);
		} else if(CCIDHeader.MessageType == PC_to_RDR_IccPowerOff) {
			USB_CCID_RDR_to_PC_SlotStatus_t ResponsePowerOff;
			ResponsePowerOff.CCIDHeader.MessageType = RDR_to_PC_SlotStatus;
			ResponsePowerOff.CCIDHeader.Length = 0;
			ResponsePowerOff.CCIDHeader.Slot = CCIDHeader.Slot;
			ResponsePowerOff.CCIDHeader.Seq = CCIDHeader.Seq;

			ResponsePowerOff.ClockStatus = 0;

			uint8_t status = CALLBACK_CCID_IccPowerOff(CCIDHeader.Slot, &error);

			ResponsePowerOff.Status = status;
			ResponsePowerOff.Error = error;

			Endpoint_ClearOUT();

			Endpoint_SelectEndpoint(CCID_IN_EPADDR);
			Endpoint_Write_Stream_LE(&ResponsePowerOff, sizeof(USB_CCID_RDR_to_PC_SlotStatus_t), NULL);
			Endpoint_ClearIN();
		} else if(CCIDHeader.MessageType == PC_to_RDR_GetSlotStatus) {
			USB_CCID_RDR_to_PC_SlotStatus_t ResponseSlotStatus;
			ResponseSlotStatus.CCIDHeader.MessageType = RDR_to_PC_SlotStatus;
			ResponseSlotStatus.CCIDHeader.Length = 0;
			ResponseSlotStatus.CCIDHeader.Slot = CCIDHeader.Slot;
			ResponseSlotStatus.CCIDHeader.Seq = CCIDHeader.Seq;

			ResponseSlotStatus.ClockStatus = 0;

			uint8_t status = CALLBACK_CCID_GetSlotStatus(CCIDHeader.Slot, &error);

			ResponseSlotStatus.Status = status;
			ResponseSlotStatus.Error = error;

			Endpoint_ClearOUT();

			Endpoint_SelectEndpoint(CCID_IN_EPADDR);
			Endpoint_Write_Stream_LE(&ResponseSlotStatus, sizeof(USB_CCID_RDR_to_PC_SlotStatus_t), NULL);
			Endpoint_ClearIN();

		} else if(CCIDHeader.MessageType == PC_to_RDR_XfrBlock) {
			uint8_t bwi = Endpoint_Read_8();
			uint16_t levelParameter = Endpoint_Read_16_LE();

			uint8_t receivedBuffer[0x4];
			uint8_t sendBuffer[0x2] = {0x90, 0x00};
			uint8_t sendLength = sizeof(sendBuffer);
			status = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR | CCID_ICCSTATUS_PRESENTANDACTIVE;

			USB_CCID_RDR_to_PC_DataBlock_t ResponseBlock;
			ResponseBlock.CCIDHeader.MessageType = RDR_to_PC_DataBlock;
			ResponseBlock.CCIDHeader.Slot = CCIDHeader.Slot;
			ResponseBlock.CCIDHeader.Seq = CCIDHeader.Seq;
			ResponseBlock.ChainParam = 0;

			Endpoint_Read_Stream_LE(receivedBuffer, sizeof(receivedBuffer), NULL);
		
			if(CCID_NoError(status)) {
				ResponseBlock.CCIDHeader.Length = sendLength;
				memcpy(&ResponseBlock.Data, sendBuffer, sizeof(uint8_t) * sendLength);

			} else {
				sendLength = 0;
			}

			ResponseBlock.Status = status;
			ResponseBlock.Error = error;

			Endpoint_ClearOUT();

			Endpoint_SelectEndpoint(CCID_IN_EPADDR);
			Endpoint_Write_Stream_LE(&ResponseBlock, sizeof(USB_CCID_RDR_to_PC_DataBlock_t) + (sendLength * sizeof(uint8_t)), NULL);
			Endpoint_ClearIN();
			
		} else {
			//replying back with zeroes

			uint8_t SentData[0x20];
			memset(SentData, 0x00, sizeof(0x20));
			Endpoint_SelectEndpoint(CCID_IN_EPADDR);
			Endpoint_Write_Stream_LE(&SentData, 0x20, NULL);
			Endpoint_ClearIN();
		}
	}
}


/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
		USB_USBTask();
		CCID_Device_USB_Task();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs. */
void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs.
 */
void EVENT_USB_Device_Disconnect(void)
{
	/* Indicate USB not ready */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup CCID Data Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(CCID_IN_EPADDR,  EP_TYPE_BULK, CCID_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(CCID_OUT_EPADDR, EP_TYPE_BULK, CCID_EPSIZE, 1);

	/* Indicate endpoint configuration success or failure */
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
    if (!(Endpoint_IsSETUPReceived()))
	  return;

	//if (USB_ControlRequest.wIndex != MSInterfaceInfo->Config.InterfaceNumber)
	  //return;

	switch (USB_ControlRequest.bRequest)
	{
		case CCID_ABORT:
			//
			break;
		case CCID_GET_CLOCK_FREQUENCIES:
			//
			break;
		case CCID_GET_DATA_RATES:
			//
			break;
	}                                                                    
}
