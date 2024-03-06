/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#include "../../../Common/Common.h"
#if (ARCH == ARCH_AVRDX)

#define  __INCLUDE_FROM_TWI_C
#include "../TWI.h"

uint8_t TWI_StartTransmission(TWI_t* const TWI,
                              const uint8_t SlaveAddress,
                              const uint8_t TimeoutMS)
{
	uint16_t TimeoutRemaining;

	TWI->MADDR = SlaveAddress;

	TimeoutRemaining = (TimeoutMS * 100);
	while (TimeoutRemaining)
	{
		uint8_t status = TWI->MSTATUS;

		if ((status & (TWI_WIF_bm | TWI_ARBLOST_bm)) == (TWI_WIF_bm | TWI_ARBLOST_bm))
		{
			TWI->MADDR = SlaveAddress;
		}
		else if ((status & (TWI_WIF_bm | TWI_RXACK_bm)) == (TWI_WIF_bm | TWI_RXACK_bm))
		{
			TWI_StopTransmission(TWI);
			return TWI_ERROR_SlaveResponseTimeout;
		}
		else if (status & (TWI_WIF_bm | TWI_RIF_bm))
		{
			return TWI_ERROR_NoError;
		}

		_delay_us(10);
		TimeoutRemaining--;
	}

	if (!(TimeoutRemaining)) {
		if (TWI->MSTATUS & TWI_CLKHOLD_bm) {
			TWI_StopTransmission(TWI);
		}
	}

	return TWI_ERROR_BusCaptureTimeout;
}

bool TWI_SendByte(TWI_t* const TWI,
                  const uint8_t Byte)
{
	TWI->MDATA = Byte;

	while (!(TWI->MSTATUS & TWI_WIF_bm));

	return (TWI->MSTATUS & TWI_WIF_bm) && !(TWI->MSTATUS & TWI_RXACK_bm);
}

bool TWI_ReceiveByte(TWI_t* const TWI,
                     uint8_t* const Byte,
                     const bool LastByte)
{
	if ((TWI->MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm)) == (TWI_BUSERR_bm | TWI_ARBLOST_bm)) {
		return false;
	}

	while (!(TWI->MSTATUS & TWI_RIF_bm));

	*Byte = TWI->MDATA;

	if (LastByte)
	  TWI->MCTRLB = TWI_ACKACT_bm | TWI_MCMD_STOP_gc;
	else
	  TWI->MCTRLB = TWI_MCMD_RECVTRANS_gc;

	return true;
}

uint8_t TWI_ReadPacket(TWI_t* const TWI,
                       const uint8_t SlaveAddress,
                       const uint8_t TimeoutMS,
                       const uint8_t* InternalAddress,
                       uint8_t InternalAddressLen,
                       uint8_t* Buffer,
                       uint16_t Length)
{
	uint8_t ErrorCode;

	if ((ErrorCode = TWI_StartTransmission(TWI, (SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
	                                       TimeoutMS)) == TWI_ERROR_NoError)
	{
		while (InternalAddressLen--)
		{
			if (!(TWI_SendByte(TWI, *(InternalAddress++))))
			{
				ErrorCode = TWI_ERROR_SlaveNAK;
				break;
			}
		}

		if ((ErrorCode = TWI_StartTransmission(TWI, (SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_READ,
		                                       TimeoutMS)) == TWI_ERROR_NoError)
		{
			while (Length--)
			{
				if (!(TWI_ReceiveByte(TWI, Buffer++, (Length == 0))))
				{
					ErrorCode = TWI_ERROR_SlaveNAK;
					break;
				}
			}
		}

		TWI_StopTransmission(TWI);
	}

	return ErrorCode;
}

uint8_t TWI_WritePacket(TWI_t* const TWI,
                        const uint8_t SlaveAddress,
                        const uint8_t TimeoutMS,
                        const uint8_t* InternalAddress,
                        uint8_t InternalAddressLen,
                        const uint8_t* Buffer,
                        uint16_t Length)
{
	uint8_t ErrorCode;

	if ((ErrorCode = TWI_StartTransmission(TWI, (SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
	                                       TimeoutMS)) == TWI_ERROR_NoError)
	{
		while (InternalAddressLen--)
		{
			if (!(TWI_SendByte(TWI, *(InternalAddress++))))
			{
				ErrorCode = TWI_ERROR_SlaveNAK;
				break;
			}
		}

		while (Length--)
		{
			if (!(TWI_SendByte(TWI, *(Buffer++))))
			{
				ErrorCode = TWI_ERROR_SlaveNAK;
				break;
			}
		}

		TWI_StopTransmission(TWI);
	}

	return ErrorCode;
}

#endif
