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

#define  __INCLUDE_FROM_USB_DRIVER
#include "../HighLevel/USBMode.h"

#if defined(USB_CAN_BE_HOST)

#include "Pipe.h"

uint8_t USB_ControlPipeSize = PIPE_CONTROLPIPE_DEFAULT_SIZE;

bool Pipe_ConfigurePipe(const uint8_t Number,
                        const uint8_t Type,
                        const uint8_t Token,
                        const uint8_t EndpointNumber,
                        const uint16_t Size,
                        const uint8_t Banks)
{
	Pipe_SelectPipe(Number);
	Pipe_EnablePipe();

	UPCFG1X = 0;

	UPCFG0X = ((Type << EPTYPE0) | Token | ((EndpointNumber & PIPE_EPNUM_MASK) << PEPNUM0));
	UPCFG1X = ((1 << ALLOC) | Banks | Pipe_BytesToEPSizeMask(Size));

	Pipe_SetInfiniteINRequests();

	return Pipe_IsConfigured();
}

void Pipe_ClearPipes(void)
{
	UPINT = 0;

	for (uint8_t PNum = 0; PNum < PIPE_TOTAL_PIPES; PNum++)
	{
		Pipe_SelectPipe(PNum);
		UPIENX  = 0;
		UPINTX  = 0;
		UPCFG1X = 0;
		Pipe_DisablePipe();
	}
}

bool Pipe_IsEndpointBound(const uint8_t EndpointAddress)
{
	uint8_t PrevPipeNumber = Pipe_GetCurrentPipe();

	for (uint8_t PNum = 0; PNum < PIPE_TOTAL_PIPES; PNum++)
	{
		Pipe_SelectPipe(PNum);

		if (!(Pipe_IsConfigured()))
		  continue;

		uint8_t PipeToken        = Pipe_GetPipeToken();
		bool    PipeTokenCorrect = true;

		if (PipeToken != PIPE_TOKEN_SETUP)
		  PipeTokenCorrect = (PipeToken == ((EndpointAddress & PIPE_EPDIR_MASK) ? PIPE_TOKEN_IN : PIPE_TOKEN_OUT));

		if (PipeTokenCorrect && (Pipe_BoundEndpointNumber() == (EndpointAddress & PIPE_EPNUM_MASK)))
		  return true;
	}

	Pipe_SelectPipe(PrevPipeNumber);
	return false;
}

uint8_t Pipe_WaitUntilReady(void)
{
	#if (USB_STREAM_TIMEOUT_MS < 0xFF)
	uint8_t  TimeoutMSRem = USB_STREAM_TIMEOUT_MS;
	#else
	uint16_t TimeoutMSRem = USB_STREAM_TIMEOUT_MS;
	#endif

	uint16_t PreviousFrameNumber = USB_Host_GetFrameNumber();

	for (;;)
	{
		if (Pipe_GetPipeToken() == PIPE_TOKEN_IN)
		{
			if (Pipe_IsINReceived())
			  return PIPE_READYWAIT_NoError;
		}
		else
		{
			if (Pipe_IsOUTReady())
			  return PIPE_READYWAIT_NoError;
		}

		if (Pipe_IsStalled())
		  return PIPE_READYWAIT_PipeStalled;
		else if (USB_HostState == HOST_STATE_Unattached)
		  return PIPE_READYWAIT_DeviceDisconnected;

		uint16_t CurrentFrameNumber = USB_Host_GetFrameNumber();

		if (CurrentFrameNumber != PreviousFrameNumber)
		{
			PreviousFrameNumber = CurrentFrameNumber;

			if (!(TimeoutMSRem--))
			  return PIPE_READYWAIT_Timeout;
		}
	}
}

#endif

