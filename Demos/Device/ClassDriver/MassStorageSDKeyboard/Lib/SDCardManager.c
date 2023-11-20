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

/*
ported from Mathieu Sonet lufa-sdcard-mass-storage-demo USB Mass storage on SD card with a 
Teensy2/ATMEGA32U4 https://bitbucket.org/elasticsheep/lufa-sdcard-mass-storage-demo/src/default/
to AT90USBKEY2 at90usb1287

ported from LUFA_091223 to lufa-LUFA-170418

ported from http://www.roland-riegel.de/sd-reader/index.html MMC/SD/SDHC card reader library 
to FatFS - Generic FAT Filesystem Module - http://elm-chan.org/fsw/ff/00index_e.html

by David Reguera Garcia aka Dreg - dreg@fr33project.org
https://github.com/David-Reguera-Garcia-Dreg - http://www.fr33project.org
*/

/** \file
 *
 *  Functions to manage the physical dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */

#define  INCLUDE_FROM_SDCARDMANAGER_C
#include "SDCardManager.h"
#include "mmc_avr.h"
#include "diskio.h"

static uint32_t CachedTotalBlocks = 0;

void SDCardManager_Init(void)
{
	printf_P(PSTR("\r\n\r\n"));
	do
	{
		printf_P(PSTR("trying ... disk_initialize\r\n"));
	} while(disk_initialize(0) != FR_OK);

	printf_P(PSTR("disk_initialize success\r\n"));
}

uint32_t SDCardManager_GetNbBlocks(void)
{
	DWORD ioct_rsp = 0;
	
	if (CachedTotalBlocks != 0)
		return CachedTotalBlocks;
		
	disk_ioctl(0, GET_SECTOR_COUNT, &ioct_rsp);

	CachedTotalBlocks = ioct_rsp;
	printf_P(PSTR("SD blocks: %li\r\n"), CachedTotalBlocks);
	
	return CachedTotalBlocks;
}

uintptr_t SDCardManager_WriteBlockHandler(uint8_t* buff_to_write, void* a, void* p)
{
	/* Check if the endpoint is currently empty */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the current endpoint bank */
		Endpoint_ClearOUT();
		
		/* Wait until the host has sent another packet */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
	
	/* Write one 16-byte chunk of data to the dataflash */
	buff_to_write[0] = Endpoint_Read_8();
	buff_to_write[1] = Endpoint_Read_8();
	buff_to_write[2] = Endpoint_Read_8();
	buff_to_write[3] = Endpoint_Read_8();
	buff_to_write[4] = Endpoint_Read_8();
	buff_to_write[5] = Endpoint_Read_8();
	buff_to_write[6] = Endpoint_Read_8();
	buff_to_write[7] = Endpoint_Read_8();
	buff_to_write[8] = Endpoint_Read_8();
	buff_to_write[9] = Endpoint_Read_8();
	buff_to_write[10] = Endpoint_Read_8();
	buff_to_write[11] = Endpoint_Read_8();
	buff_to_write[12] = Endpoint_Read_8();
	buff_to_write[13] = Endpoint_Read_8();
	buff_to_write[14] = Endpoint_Read_8();
	buff_to_write[15] = Endpoint_Read_8();
	
	return 16;
}

void SDCardManager_WriteBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	int i;
	static uint8_t buff_to_write[512];
	
	printf_P(PSTR("W %li %i\r\n"), BlockAddress, TotalBlocks);

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		for (i = 0; i < 32; i++)
		{
			SDCardManager_WriteBlockHandler(buff_to_write + (16 * i), NULL, NULL);	
		}
		
		disk_write(0, buff_to_write, BlockAddress, 1);
				
		/* Check if the current command is being aborted by the host */
		if (MSInterfaceInfo->State.IsMassStoreReset)
		  return;
			
		/* Decrement the blocks remaining counter and reset the sub block counter */
		BlockAddress++;
		TotalBlocks--;
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearOUT();
}

static inline uint8_t SDCardManager_ReadBlockHandler(uint8_t* buffer, void* a, void* p)
{
	/* Check if the endpoint is currently full */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the endpoint bank to send its contents to the host */
		Endpoint_ClearIN();
		
		/* Wait until the endpoint is ready for more data */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
		
	Endpoint_Write_8(buffer[0]);
	Endpoint_Write_8(buffer[1]);
	Endpoint_Write_8(buffer[2]);
	Endpoint_Write_8(buffer[3]);
	Endpoint_Write_8(buffer[4]);
	Endpoint_Write_8(buffer[5]);
	Endpoint_Write_8(buffer[6]);
	Endpoint_Write_8(buffer[7]);
	Endpoint_Write_8(buffer[8]);
	Endpoint_Write_8(buffer[9]);
	Endpoint_Write_8(buffer[10]);
	Endpoint_Write_8(buffer[11]);
	Endpoint_Write_8(buffer[12]);
	Endpoint_Write_8(buffer[13]);
	Endpoint_Write_8(buffer[14]);
	Endpoint_Write_8(buffer[15]);
	
	return 1;
}

void SDCardManager_ReadBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	static uint8_t buff_rd[512];
	int i;
	
	printf_P(PSTR("R %li %i\r\n"), BlockAddress, TotalBlocks);
	
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		disk_read(0, buff_rd, BlockAddress, 1);
		for (i = 0; i < 32; i++)
		{
			SDCardManager_ReadBlockHandler(buff_rd + (16 * i), NULL, NULL);	
		}
		/* Decrement the blocks remaining counter */
		BlockAddress++;
		TotalBlocks--;
	}
	
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearIN();
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean true if all media chips are working, false otherwise
 */
bool SDCardManager_CheckDataflashOperation(void)
{	
	return true;
}
