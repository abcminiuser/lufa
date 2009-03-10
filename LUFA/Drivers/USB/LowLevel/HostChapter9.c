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

#include "../HighLevel/USBMode.h"
#if defined(USB_CAN_BE_HOST)

#define  INCLUDE_FROM_HOSTCHAPTER9_C
#include "HostChapter9.h"

USB_Host_Request_Header_t USB_HostRequest;

uint8_t USB_Host_SendControlRequest(void* BufferPtr)
{
	uint8_t* HeaderStream   = (uint8_t*)&USB_HostRequest;
	uint8_t* DataStream     = (uint8_t*)BufferPtr;
	bool     BusSuspended   = USB_Host_IsBusSuspended();
	uint8_t  ReturnStatus   = HOST_SENDCONTROL_Successful;
	uint16_t DataLen        = USB_HostRequest.wLength;

	USB_Host_ResumeBus();
	
	if ((ReturnStatus = USB_Host_WaitMS(1)) != HOST_WAITERROR_Successful)
	  return ReturnStatus;

	Pipe_SelectPipe(PIPE_CONTROLPIPE);
	Pipe_SetToken(PIPE_TOKEN_SETUP);
	Pipe_ClearErrorFlags();
	Pipe_ClearSetupSent();

	Pipe_Unfreeze();

	for (uint8_t HeaderByte = 0; HeaderByte < sizeof(USB_Host_Request_Header_t); HeaderByte++)
	  Pipe_Write_Byte(*(HeaderStream++));

	Pipe_ClearSetupOUT();
	
	if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_SetupSent)))
	  goto End_Of_Control_Send;

	Pipe_ClearSetupSent();
	Pipe_Freeze();

	if ((ReturnStatus = USB_Host_WaitMS(1)) != HOST_WAITERROR_Successful)
	  goto End_Of_Control_Send;

	if ((USB_HostRequest.bmRequestType & CONTROL_REQTYPE_DIRECTION) == REQDIR_DEVICETOHOST)
	{
		Pipe_SetToken(PIPE_TOKEN_IN);
		
		if (DataStream != NULL)
		{
			while (DataLen)
			{
				Pipe_Unfreeze();

				if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_InReceived)))
				  goto End_Of_Control_Send;
							
				if (!(Pipe_BytesInPipe()))
				  DataLen = 0;
				
				while (Pipe_BytesInPipe() && DataLen)
				{
					*(DataStream++) = Pipe_Read_Byte();
					DataLen--;
				}

				Pipe_Freeze();
				Pipe_ClearSetupIN();
			}
		}

		Pipe_SetToken(PIPE_TOKEN_OUT);
		Pipe_Unfreeze();
		
		if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_OutReady)))
		  goto End_Of_Control_Send;

		Pipe_ClearSetupOUT();

		if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_OutReady)))
		  goto End_Of_Control_Send;
	}
	else
	{
		if (DataStream != NULL)
		{
			Pipe_SetToken(PIPE_TOKEN_OUT);
			Pipe_Unfreeze();	

			while (DataLen)
			{
				if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_OutReady)))
				  goto End_Of_Control_Send;

				while (DataLen && (Pipe_BytesInPipe() < USB_ControlPipeSize))
				{					
					Pipe_Write_Byte(*(DataStream++));
					DataLen--;
				}
				
				Pipe_ClearSetupOUT();
			}

			if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_OutReady)))
			  goto End_Of_Control_Send;

			Pipe_Freeze();
		}
		
		Pipe_SetToken(PIPE_TOKEN_IN);
		Pipe_Unfreeze();

		if ((ReturnStatus = USB_Host_Wait_For_Setup_IOS(USB_HOST_WAITFOR_InReceived)))
		  goto End_Of_Control_Send;

		Pipe_ClearSetupIN();
	}

End_Of_Control_Send:
	Pipe_Freeze();
	
	if (BusSuspended)
	  USB_Host_SuspendBus();

	Pipe_ResetPipe(PIPE_CONTROLPIPE);

	return ReturnStatus;
}

static uint8_t USB_Host_Wait_For_Setup_IOS(const uint8_t WaitType)
{
	uint16_t TimeoutCounter = USB_HOST_TIMEOUT_MS;
	
	while (!(((WaitType == USB_HOST_WAITFOR_SetupSent)  && Pipe_IsSetupSent())       ||
	         ((WaitType == USB_HOST_WAITFOR_InReceived) && Pipe_IsSetupINReceived()) ||
	         ((WaitType == USB_HOST_WAITFOR_OutReady)   && Pipe_IsSetupOUTReady())))
	{
		uint8_t ErrorCode;

		if ((ErrorCode = USB_Host_WaitMS(1)) != HOST_WAITERROR_Successful)
		  return ErrorCode;
			
		if (!(TimeoutCounter--))
		  return HOST_SENDCONTROL_SoftwareTimeOut;
	}

	return HOST_SENDCONTROL_Successful;
}

#endif
