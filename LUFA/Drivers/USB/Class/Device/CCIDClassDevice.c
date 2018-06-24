/*
             LUFA Library
     Copyright (C) Dean Camera, 2018.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)
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

#define  __INCLUDE_FROM_USB_DRIVER
#include "../../Core/USBMode.h"

#if defined(USB_CAN_BE_DEVICE)

#define	 __INCLUDE_FROM_CCID_DRIVER
#define	 __INCLUDE_FROM_CCID_DEVICE_C
#include "CCIDClassDevice.h"


bool CCID_CheckStatusNoError(int status)
{
	return (status & 0xC0) == 0x0;
}

void CCID_Device_ProcessControlRequest(USB_ClassInfo_CCID_Device_t* const CCIDInterfaceInfo)
{
	if (!(Endpoint_IsSETUPReceived()))
		return;

	if (USB_ControlRequest.wIndex != CCIDInterfaceInfo->Config.InterfaceNumber)
		return;

	switch (USB_ControlRequest.bRequest)
	{
		case CCID_ABORT:
		{
			// Initiates the abort process.
			// The host should send 2 messages in the following order:
			//	- CCID_ABORT control request
			//	- CCID_PC_t_PCo_RDR_Abort command
			//
			// If the device is still processing a message, it should fail it until receiving a CCIRPC_to_RDR_Abort
			// command.
			//
			// When the device receives the CCIRPC_to_RDR_Abort message, it replies with RDR_to_PC_SlotStatus
			// and the abort process ends.

			// The wValue field contains the slot number (bSlot) in the low byte and the sequence number (bSeq) in
			// the high	byte
			uint8_t Slot = USB_ControlRequest.wValue & 0xFF;
			uint8_t Seq  = USB_ControlRequest.wValue >> 8;

			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE) && Slot == 0)
			{
				Endpoint_ClearSETUP();

				CCIDInterfaceInfo->State.Aborted = true;
				CCIDInterfaceInfo->State.AbortedSeq = Seq;

				Endpoint_ClearOUT();
			}

			break;
		}
		case CCID_GET_CLOCK_FREQUENCIES:
		{
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{

				Endpoint_ClearSETUP();
				Endpoint_Write_8(0); // Not supported
				Endpoint_ClearOUT();
			}
			break;
		}
		case CCID_GET_DATA_RATES:
		{
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{

				Endpoint_ClearSETUP();
				Endpoint_Write_8(0); // Not supported
				Endpoint_ClearOUT();
			}
			break;
		}
	}
}

bool CCID_Device_ConfigureEndpoints(USB_ClassInfo_CCID_Device_t* const CCIDInterfaceInfo)
{
	CCIDInterfaceInfo->Config.DataINEndpoint.Type	= EP_TYPE_BULK;
	CCIDInterfaceInfo->Config.DataOUTEndpoint.Type = EP_TYPE_BULK;

	if (!(Endpoint_ConfigureEndpointTable(&CCIDInterfaceInfo->Config.DataINEndpoint, 1)))
		return false;

	if (!(Endpoint_ConfigureEndpointTable(&CCIDInterfaceInfo->Config.DataOUTEndpoint, 1)))
		return false;

	return true;
}

void CCID_Device_USBTask(USB_ClassInfo_CCID_Device_t* const CCIDInterfaceInfo)
{
	Endpoint_SelectEndpoint(CCIDInterfaceInfo->Config.DataOUTEndpoint.Address);

	uint8_t BlockBuffer[0x20];
	CCIDInterfaceInfo->State.Aborted    = false;
	CCIDInterfaceInfo->State.AbortedSeq = -1;

	if (Endpoint_IsOUTReceived())
	{
		USB_CCID_BulkMessage_Header_t CCIDHeader;
		CCIDHeader.MessageType = Endpoint_Read_8();
		CCIDHeader.Length      = Endpoint_Read_32_LE();
		CCIDHeader.Slot        = Endpoint_Read_8();
		CCIDHeader.Seq         = Endpoint_Read_8();

		uint8_t Status;
		uint8_t Error = CCID_ERROR_NO_ERROR;

		switch (CCIDHeader.MessageType)
		{
			case CCID_PC_to_RDR_IccPowerOn:
			{
				uint8_t AtrLength;
				USB_CCID_RDR_to_PC_DataBlock_t* ResponseATR = (USB_CCID_RDR_to_PC_DataBlock_t*)&BlockBuffer;

				ResponseATR->CCIDHeader.MessageType = CCID_RDR_to_PC_DataBlock;
				ResponseATR->CCIDHeader.Slot        = CCIDHeader.Slot;
				ResponseATR->CCIDHeader.Seq         = CCIDHeader.Seq;
				ResponseATR->ChainParam             = 0;

				Status = CALLBACK_CCID_IccPowerOn(ResponseATR->CCIDHeader.Slot, (uint8_t*)ResponseATR->Data, &AtrLength, &Error);

				if (CCID_CheckStatusNoError(Status) && !CCIDInterfaceInfo->State.Aborted)
				{
					ResponseATR->CCIDHeader.Length = AtrLength;
				}
				else if (CCIDInterfaceInfo->State.Aborted)
				{
					Status = CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_PRESENTANDACTIVE;
					Error  = CCID_ERROR_CMD_ABORTED;
					AtrLength = 0;
				}
				else
				{
					AtrLength = 0;
				}

				ResponseATR->Status = Status;
				ResponseATR->Error  = Error;

				Endpoint_ClearOUT();

				Endpoint_SelectEndpoint(CCIDInterfaceInfo->Config.DataINEndpoint.Address);
				Endpoint_Write_Stream_LE(ResponseATR, sizeof(USB_CCID_RDR_to_PC_DataBlock_t) + AtrLength, NULL);
				Endpoint_ClearIN();
				break;
			}

			case CCID_PC_to_RDR_IccPowerOff:
			{
				USB_CCID_RDR_to_PC_SlotStatus_t* ResponsePowerOff = (USB_CCID_RDR_to_PC_SlotStatus_t*)&BlockBuffer;
				ResponsePowerOff->CCIDHeader.MessageType = CCID_RDR_to_PC_SlotStatus;
				ResponsePowerOff->CCIDHeader.Length      = 0;
				ResponsePowerOff->CCIDHeader.Slot        = CCIDHeader.Slot;
				ResponsePowerOff->CCIDHeader.Seq         = CCIDHeader.Seq;

				ResponsePowerOff->ClockStatus = 0;

				Status = CALLBACK_CCID_IccPowerOff(CCIDHeader.Slot, &Error);

				ResponsePowerOff->Status = Status;
				ResponsePowerOff->Error  = Error;

				Endpoint_ClearOUT();

				Endpoint_SelectEndpoint(CCIDInterfaceInfo->Config.DataINEndpoint.Address);
				Endpoint_Write_Stream_LE(ResponsePowerOff, sizeof(USB_CCID_RDR_to_PC_SlotStatus_t), NULL);
				Endpoint_ClearIN();
				break;
			}

			case CCID_PC_to_RDR_GetSlotStatus:
			{
				USB_CCID_RDR_to_PC_SlotStatus_t* ResponseSlotStatus = (USB_CCID_RDR_to_PC_SlotStatus_t*)&BlockBuffer;
				ResponseSlotStatus->CCIDHeader.MessageType = CCID_RDR_to_PC_SlotStatus;
				ResponseSlotStatus->CCIDHeader.Length      = 0;
				ResponseSlotStatus->CCIDHeader.Slot        = CCIDHeader.Slot;
				ResponseSlotStatus->CCIDHeader.Seq         = CCIDHeader.Seq;

				ResponseSlotStatus->ClockStatus = 0;

				Status = CALLBACK_CCID_GetSlotStatus(CCIDHeader.Slot, &Error);

				ResponseSlotStatus->Status = Status;
				ResponseSlotStatus->Error  = Error;

				Endpoint_ClearOUT();

				Endpoint_SelectEndpoint(CCIDInterfaceInfo->Config.DataINEndpoint.Address);
				Endpoint_Write_Stream_LE(ResponseSlotStatus, sizeof(USB_CCID_RDR_to_PC_SlotStatus_t), NULL);
				Endpoint_ClearIN();
				break;
			}

			case CCID_PC_to_RDR_Abort:
			{
				USB_CCID_RDR_to_PC_SlotStatus_t* ResponseAbort = (USB_CCID_RDR_to_PC_SlotStatus_t*)&BlockBuffer;
				ResponseAbort->CCIDHeader.MessageType = CCID_RDR_to_PC_SlotStatus;
				ResponseAbort->CCIDHeader.Length      = 0;
				ResponseAbort->CCIDHeader.Slot        = CCIDHeader.Slot;
				ResponseAbort->CCIDHeader.Seq         = CCIDHeader.Seq;

				ResponseAbort->ClockStatus = 0;

				Status = CALLBACK_CCID_Abort(CCIDHeader.Slot, CCIDHeader.Seq, &Error);

				ResponseAbort->Status = Status;
				ResponseAbort->Error  = Error;

				Endpoint_ClearOUT();

				Endpoint_SelectEndpoint(CCIDInterfaceInfo->Config.DataINEndpoint.Address);
				Endpoint_Write_Stream_LE(ResponseAbort, sizeof(USB_CCID_RDR_to_PC_SlotStatus_t), NULL);
				Endpoint_ClearIN();
				break;
			}
			default:
			{
				memset(BlockBuffer, 0x00, sizeof(BlockBuffer));

				Endpoint_SelectEndpoint(CCIDInterfaceInfo->Config.DataINEndpoint.Address);
				Endpoint_Write_Stream_LE(BlockBuffer, sizeof(BlockBuffer), NULL);
				Endpoint_ClearIN();
			}
		}
	}
}

#endif
