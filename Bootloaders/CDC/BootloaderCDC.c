/*
             LUFA Library
     Copyright (C) Dean Camera, 2013.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2013  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
 *  Main source file for the CDC class bootloader. This file contains the complete bootloader logic.
 */

#define  INCLUDE_FROM_BOOTLOADERCDC_C
#include "BootloaderCDC.h"

/** Contains the current baud rate and other settings of the first virtual serial port. This must be retained as some
 *  operating systems will not open the port unless the settings can be set successfully.
 */
static CDC_LineEncoding_t LineEncoding = { .BaudRateBPS = 0,
                                           .CharFormat  = CDC_LINEENCODING_OneStopBit,
                                           .ParityType  = CDC_PARITY_None,
                                           .DataBits    = 8                            };

/** Current address counter. This stores the current address of the FLASH or EEPROM as set by the host,
 *  and is used when reading or writing to the AVRs memory (either FLASH or EEPROM depending on the issued
 *  command.)
 */
static uint32_t CurrAddress;

/** Flag to indicate if the bootloader should be running, or should exit and allow the application code to run
 *  via a watchdog reset. When cleared the bootloader will exit, starting the watchdog and entering an infinite
 *  loop until the AVR restarts and the application runs.
 */
static bool RunBootloader = true;

/** Magic lock for forced application start. If the HWBE fuse is programmed and BOOTRST is unprogrammed, the bootloader
 *  will start if the /HWB line of the AVR is held low and the system is reset. However, if the /HWB line is still held
 *  low when the application attempts to start via a watchdog reset, the bootloader will re-start. If set to the value
 *  \ref MAGIC_BOOT_KEY the special init function \ref Application_Jump_Check() will force the application to start.
 */
uint16_t MagicBootKey ATTR_NO_INIT;

/** Routine to check if the reset source was the watchdog. */
static inline uint8_t Reset_Source_Was_Watchdog() {
#if (ARCH == ARCH_AVR8)
	return (MCUSR & (1 << WDRF));
#elif (ARCH == ARCH_XMEGA)
	return (RST.STATUS & RST_WDRF_bm);
#endif
}

/** Routine to turn off the watchdog reset status bit. */
static inline void Turn_Off_Watchdog_Reset_Status() {
#if (ARCH == ARCH_AVR8)
	MSUSR &= ~(1 << WDRF);
#elif (ARCH == ARCH_XMEGA)
	RST.STATUS = RST_WDRF_bm;
#endif
}

/** wdt_disable() is not defined by avr/wdt.h for XMEGA devices.  */
#if (ARCH == ARCH_XMEGA)
static inline void wdt_disable(void);
static inline void wdt_disable() {
  uint8_t temp = (WDT.CTRL & ~WDT_ENABLE_bm) | WDT_CEN_bm;
  CCP = CCP_IOREG_gc;
  WDT.CTRL = temp;
}
#endif

/** Special startup routine to check if the bootloader was started via a watchdog reset, and if the magic application
 *  start key has been loaded into \ref MagicBootKey. If the bootloader started via the watchdog and the key is valid,
 *  this will force the user application to start via a software jump.
 */
void Application_Jump_Check(void)
{
	bool JumpToApplication = false;

	#if ((BOARD == BOARD_XPLAIN) || (BOARD == BOARD_XPLAIN_REV1))
		/* Disable JTAG debugging */
		JTAG_DISABLE();

		/* Enable pull-up on the JTAG TCK pin so we can use it to select the mode */
		PORTF |= (1 << 4);
		Delay_MS(10);

		/* If the TCK pin is not jumpered to ground, start the user application instead */
		JumpToApplication |= ((PINF & (1 << 4)) != 0);

		/* Re-enable JTAG debugging */
		JTAG_ENABLE();
	#endif

	#if (BOARD == BOARD_A3BU_XPLAINED)
		/* Check if SW0 is pressed. If not, go into the application. */
		JumpToApplication |= PORTE.IN & _BV(5);
	#endif

	/* If the reset source was the bootloader and the key is correct, clear it and jump to the application */
	if (Reset_Source_Was_Watchdog() && (MagicBootKey == MAGIC_BOOT_KEY))
		JumpToApplication |= true;

	/* If a request has been made to jump to the user application, honor it */
	if (JumpToApplication)
	{
		/* Turn off the watchdog */
		Turn_Off_Watchdog_Reset_Status();
		wdt_disable();

		/* Clear the boot key and jump to the user application */
		MagicBootKey = 0;

		// cppcheck-suppress constStatement
		asm("jmp 0"); // Note: same as ((void (*)(void))0x0000)();
	}
}

/** Main program entry point. This routine configures the hardware required by the bootloader, then continuously
 *  runs the bootloader processing routine until instructed to soft-exit, or hard-reset via the watchdog to start
 *  the loaded application code.
 */
int main(void)
{
	/* Setup hardware required for the bootloader */
	SetupHardware();

	/* Turn on first LED on the board to indicate that the bootloader has started */
	LEDs_SetAllLEDs(LEDS_LED1);

	/* Enable global interrupts so that the USB stack can function */
	GlobalInterruptEnable();

	while (RunBootloader)
	{
		CDC_Task();
		USB_USBTask();
	}

	/* Disconnect from the host - USB interface will be reset later along with the AVR */
	USB_Detach();

	/* Unlock the forced application start mode of the bootloader if it is restarted */
	MagicBootKey = MAGIC_BOOT_KEY;

	/* Enable the watchdog and force a timeout to reset the AVR */
	wdt_enable(WDTO_250MS);

	for (;;);
}

/** Configures all hardware required for the bootloader. */
static void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
  Turn_Off_Watchdog_Reset_Status();
	wdt_disable();

#if (ARCH == ARCH_AVR8)
	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Relocate the interrupt vector table to the bootloader section */
	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);
#elif (ARCH == ARCH_XMEGA)
	/** Set the frequency correctly */
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);
	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

	uint8_t temp = PMIC.CTRL | PMIC_IVSEL_bm;
	CCP = CCP_IOREG_gc;
	PMIC.CTRL = temp;
#endif

	/* Initialize the USB and other board hardware drivers */
	USB_Init();
	LEDs_Init();

#if (ARCH == ARCH_AVR8)
	/* Bootloader active LED toggle timer initialization */
	TIMSK1 = (1 << TOIE1);
	TCCR1B = ((1 << CS11) | (1 << CS10));
#elif (ARCH == ARCH_XMEGA)
  TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
  TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
#endif
}

/** ISR to periodically toggle the LEDs on the board to indicate that the bootloader is active. */
#if (ARCH == ARCH_AVR8)
ISR(TIMER1_OVF_vect, ISR_BLOCK)
#elif (ARCH == ARCH_XMEGA)
ISR(TCC0_OVF_vect, ISR_BLOCK)
#endif
{
	LEDs_ToggleLEDs(LEDS_LED1 | LEDS_LED2);
}

/** Event handler for the USB_ConfigurationChanged event. This configures the device's endpoints ready
 *  to relay data to and from the attached USB host.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	/* Setup CDC Notification, Rx and Tx Endpoints */
	Endpoint_ConfigureEndpoint(CDC_NOTIFICATION_EPADDR, EP_TYPE_INTERRUPT,
	                           CDC_NOTIFICATION_EPSIZE, 1);

	Endpoint_ConfigureEndpoint(CDC_TX_EPADDR, EP_TYPE_BULK, CDC_TXRX_EPSIZE, 1);

	Endpoint_ConfigureEndpoint(CDC_RX_EPADDR, EP_TYPE_BULK, CDC_TXRX_EPSIZE, 1);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	/* Ignore any requests that aren't directed to the CDC interface */
	if ((USB_ControlRequest.bmRequestType & (CONTROL_REQTYPE_TYPE | CONTROL_REQTYPE_RECIPIENT)) !=
	    (REQTYPE_CLASS | REQREC_INTERFACE))
	{
		return;
	}

	/* Activity - toggle indicator LEDs */
	LEDs_ToggleLEDs(LEDS_LED1 | LEDS_LED2);

	/* Process CDC specific control requests */
	switch (USB_ControlRequest.bRequest)
	{
		case CDC_REQ_GetLineEncoding:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

				/* Write the line coding data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(&LineEncoding, sizeof(CDC_LineEncoding_t));
				Endpoint_ClearOUT();
			}

			break;
		case CDC_REQ_SetLineEncoding:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

				/* Read the line coding data in from the host into the global struct */
				Endpoint_Read_Control_Stream_LE(&LineEncoding, sizeof(CDC_LineEncoding_t));
				Endpoint_ClearIN();
			}

			break;
        case CDC_REQ_SetControlLineState:
	        if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
	        {
	            Endpoint_ClearSETUP();
	            Endpoint_ClearStatusStage();
	        }

	        break;
	}
}

#if !defined(NO_BLOCK_SUPPORT)
/** Reads or writes a block of EEPROM or FLASH memory to or from the appropriate CDC data endpoint, depending
 *  on the AVR910 protocol command issued.
 *
 *  \param[in] Command  Single character AVR910 protocol command indicating what memory operation to perform
 */
static void ReadWriteMemoryBlock(const uint8_t Command)
{
	uint16_t BlockSize;
	char     MemoryType;

	uint8_t  HighByte = 0;
	uint8_t  LowByte  = 0;

	BlockSize  = (FetchNextCommandByte() << 8);
	BlockSize |=  FetchNextCommandByte();

	MemoryType =  FetchNextCommandByte();

	if ((MemoryType != MEMORY_TYPE_FLASH) && (MemoryType != MEMORY_TYPE_EEPROM))
	{
		/* Send error byte back to the host */
		WriteNextResponseByte('?');

		return;
	}

	/* Check if command is to read a memory block */
	if (Command == AVR109_COMMAND_BlockRead)
	{
		/* Re-enable RWW section */
#if (ARCH == ARCH_AVR8)
		boot_rww_enable();
#endif

		while (BlockSize--)
		{
			if (MemoryType == MEMORY_TYPE_FLASH)
			{
				/* Read the next FLASH byte from the current FLASH page */
				#if (FLASHEND > 0xFFFF)
				LEDs_TurnOnLEDs(LEDS_LED3);
				WriteNextResponseByte(pgm_read_byte_far(CurrAddress | HighByte));
				LEDs_TurnOffLEDs(LEDS_LED3);
				#else
				WriteNextResponseByte(pgm_read_byte(CurrAddress | HighByte));
				#endif

				/* If both bytes in current word have been read, increment the address counter */
				if (HighByte)
				  CurrAddress += 2;

				HighByte = !HighByte;
			}
			else
			{
				/* Read the next EEPROM byte into the endpoint */
				WriteNextResponseByte(eeprom_read_byte((uint8_t*)(intptr_t)(CurrAddress >> 1)));

				/* Increment the address counter after use */
				CurrAddress += 2;
			}
		}
	}
	else
	{
		uint32_t PageStartAddress = CurrAddress;

		if (MemoryType == MEMORY_TYPE_FLASH)
		{
#if (ARCH == ARCH_AVR8)
			boot_page_erase(PageStartAddress);
			boot_spm_busy_wait();
			while (BlockSize--)
			{
				/* If both bytes in current word have been written, increment the address counter */
				if (HighByte)
				{
					/* Write the next FLASH word to the current FLASH page */
					boot_page_fill(CurrAddress, ((FetchNextCommandByte() << 8) | LowByte));

					/* Increment the address counter after use */
					CurrAddress += 2;
				}
				else
				{
					LowByte = FetchNextCommandByte();
				}

				HighByte = !HighByte;
			}
			/* Commit the flash page to memory */
			boot_page_write(PageStartAddress);

			/* Wait until write operation has completed */
			boot_spm_busy_wait();
#elif (ARCH == ARCH_XMEGA)
			uint8_t buffer[SPM_PAGESIZE];
			for (int i = 0; i < BlockSize; i++) {
				buffer[i] = FetchNextCommandByte();
			}
			uint32_t tempaddress = CurrAddress;
			LEDs_TurnOnLEDs(LEDS_LED4);
			SP_LoadFlashPage(buffer);
			SP_EraseWriteApplicationPage(tempaddress);
			SP_WaitForSPM();
			CurrAddress += BlockSize;
			LEDs_TurnOffLEDs(LEDS_LED4);
#endif
		}
		else
		{
			while (BlockSize--) {
				/* Write the next EEPROM byte from the endpoint */
				eeprom_write_byte((uint8_t*)((intptr_t)(CurrAddress >> 1)), FetchNextCommandByte());

				/* Increment the address counter after use */
				CurrAddress += 1;
			}
		}

  /* Send response byte back to the host */
	WriteNextResponseByte('\r');
	}
}
#endif

/** Retrieves the next byte from the host in the CDC data OUT endpoint, and clears the endpoint bank if needed
 *  to allow reception of the next data packet from the host.
 *
 *  \return Next received byte from the host in the CDC data OUT endpoint
 */
static uint8_t FetchNextCommandByte(void)
{
	/* Select the OUT endpoint so that the next data byte can be read */
	Endpoint_SelectEndpoint(CDC_RX_EPADDR);

	/* If OUT endpoint empty, clear it and wait for the next packet from the host */
	while (!(Endpoint_IsReadWriteAllowed()))
	{
		Endpoint_ClearOUT();

		while (!(Endpoint_IsOUTReceived()))
		{
			if (USB_DeviceState == DEVICE_STATE_Unattached)
			  return 0;
		}
	}

	/* Fetch the next byte from the OUT endpoint */
	return Endpoint_Read_8();
}

/** Writes the next response byte to the CDC data IN endpoint, and sends the endpoint back if needed to free up the
 *  bank when full ready for the next byte in the packet to the host.
 *
 *  \param[in] Response  Next response byte to send to the host
 */
static void WriteNextResponseByte(const uint8_t Response)
{
	/* Select the IN endpoint so that the next data byte can be written */
	Endpoint_SelectEndpoint(CDC_TX_EPADDR);

	/* If IN endpoint full, clear it and wait until ready for the next packet to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		Endpoint_ClearIN();

		while (!(Endpoint_IsINReady()))
		{
			if (USB_DeviceState == DEVICE_STATE_Unattached)
			  return;
		}
	}

	/* Write the next byte to the IN endpoint */
	Endpoint_Write_8(Response);
}

/** Task to read in AVR910 commands from the CDC data OUT endpoint, process them, perform the required actions
 *  and send the appropriate response back to the host.
 */
static void CDC_Task(void)
{
	/* Select the OUT endpoint */
	Endpoint_SelectEndpoint(CDC_RX_EPADDR);

	/* Check if endpoint has a command in it sent from the host */
	if (!(Endpoint_IsOUTReceived()))
	  return;

	/* Read in the bootloader command (first byte sent from host) */
	uint8_t Command = FetchNextCommandByte();

  uint16_t tmpword = 0;

	if (Command == AVR109_COMMAND_ExitBootloader)
	{
		RunBootloader = false;

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if ((Command == AVR109_COMMAND_SetLED) || (Command == AVR109_COMMAND_ClearLED) ||
	         (Command == AVR109_COMMAND_SelectDeviceType))
	{
		FetchNextCommandByte();

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if ((Command == AVR109_COMMAND_EnterProgrammingMode) || (Command == AVR109_COMMAND_LeaveProgrammingMode))
	{
		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if (Command == AVR109_COMMAND_ReadPartCode)
	{
		/* Return ATMEGA128 part code - this is only to allow AVRProg to use the bootloader */
		WriteNextResponseByte(0x44);
		WriteNextResponseByte(0x00);
	}
	else if (Command == AVR109_COMMAND_ReadAutoAddressIncrement)
	{
		/* Indicate auto-address increment is supported */
		WriteNextResponseByte('Y');
	}
	else if (Command == AVR109_COMMAND_SetCurrentAddress)
	{
		/* Set the current address to that given by the host (translate 16-bit word address to byte address) */
		CurrAddress   = (FetchNextCommandByte() << 9);
		CurrAddress  |= (FetchNextCommandByte() << 1);

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if (Command == AVR109_COMMAND_ReadBootloaderInterface)
	{
		/* Indicate serial programmer back to the host */
		WriteNextResponseByte('S');
	}
	else if (Command == AVR109_COMMAND_ReadBootloaderIdentifier)
	{
		/* Write the 7-byte software identifier to the endpoint */
		for (uint8_t CurrByte = 0; CurrByte < 7; CurrByte++)
		  WriteNextResponseByte(SOFTWARE_IDENTIFIER[CurrByte]);
	}
	else if (Command == AVR109_COMMAND_ReadBootloaderSWVersion)
	{
		WriteNextResponseByte('0' + BOOTLOADER_VERSION_MAJOR);
		WriteNextResponseByte('0' + BOOTLOADER_VERSION_MINOR);
	}
	else if (Command == AVR109_COMMAND_ReadSignature)
	{
		WriteNextResponseByte(AVR_SIGNATURE_3);
		WriteNextResponseByte(AVR_SIGNATURE_2);
		WriteNextResponseByte(AVR_SIGNATURE_1);
	}
	else if (Command == AVR109_COMMAND_EraseFLASH)
	{
		/* Clear the application section of flash */
		for (uint32_t CurrFlashAddress = 0; CurrFlashAddress < (uint32_t)BOOT_START_ADDR; CurrFlashAddress += SPM_PAGESIZE)
		{
#if (ARCH == ARCH_AVR8)
			boot_page_erase(CurrFlashAddress);
			boot_spm_busy_wait();
			boot_page_write(CurrFlashAddress);
			boot_spm_busy_wait();
#elif (ARCH == ARCH_XMEGA)
      SP_EraseApplicationSection();
      SP_WaitForSPM();
#endif
		}

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	#if !defined(NO_LOCK_BYTE_WRITE_SUPPORT)
	else if (Command == AVR109_COMMAND_WriteLockbits)
	{
		/* Set the lock bits to those given by the host */
#if (ARCH == ARCH_AVR8)
		boot_lock_bits_set(FetchNextCommandByte());
#elif (ARCH == ARCH_XMEGA)
    SP_WriteLockBits( FetchNextCommandByte());
#endif
		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	#endif
	else if (Command == AVR109_COMMAND_ReadLockbits)
	{
#if (ARCH == ARCH_AVR8)
		WriteNextResponseByte(boot_lock_fuse_bits_get(GET_LOCK_BITS));
#elif (ARCH == ARCH_XMEGA)
		WriteNextResponseByte(SP_ReadLockBits());
#endif
	}
	else if (Command == AVR109_COMMAND_ReadLowFuses)
	{
#if (ARCH == ARCH_AVR8)
		WriteNextResponseByte(boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS));
#elif (ARCH == ARCH_XMEGA)
		WriteNextResponseByte(SP_ReadFuseByte(0));
#endif
	}
	else if (Command == AVR109_COMMAND_ReadHighFuses)
	{
#if (ARCH == ARCH_AVR8)
		WriteNextResponseByte(boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS));
#elif (ARCH == ARCH_XMEGA)
		WriteNextResponseByte(SP_ReadFuseByte(1));
#endif
	}
	else if (Command == AVR109_COMMAND_ReadExtendedFuses)
	{
#if (ARCH == ARCH_AVR8)
		WriteNextResponseByte(boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS));
#elif (ARCH == ARCH_XMEGA)
		WriteNextResponseByte(SP_ReadFuseByte(2));
#endif
	}
	#if !defined(NO_BLOCK_SUPPORT)
	else if (Command == AVR109_COMMAND_GetBlockWriteSupport)
	{
		WriteNextResponseByte('Y');

		/* Send block size to the host */
		WriteNextResponseByte(SPM_PAGESIZE >> 8);
		WriteNextResponseByte(SPM_PAGESIZE & 0xFF);
	}
	else if ((Command == AVR109_COMMAND_BlockWrite) || (Command == AVR109_COMMAND_BlockRead))
	{
		/* Delegate the block write/read to a separate function for clarity */
		ReadWriteMemoryBlock(Command);
	}
	#endif
	#if !defined(NO_FLASH_BYTE_SUPPORT)
	else if (Command == AVR109_COMMAND_FillFlashPageWordHigh)
	{
#if (ARCH == ARCH_AVR8)
		/* Write the high byte to the current flash page */
		boot_page_fill(CurrAddress, FetchNextCommandByte());
#elif (ARCH == ARCH_XMEGA)
		tmpword |= FetchNextCommandByte() << 8;
		/* Write the word to the flash */
		SP_LoadFlashWord(CurrAddress << 1, tmpword);
		CurrAddress += 2;
#endif

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if (Command == AVR109_COMMAND_FillFlashPageWordLow)
	{
#if (ARCH == ARCH_AVR8)
		/* Write the low byte to the current flash page */
		boot_page_fill(CurrAddress | 0x01, FetchNextCommandByte());
		/* Increment the address */
		CurrAddress += 2;
#elif (ARCH == ARCH_XMEGA)
		tmpword = FetchNextCommandByte();
#endif

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if (Command == AVR109_COMMAND_WriteFlashPage)
	{
#if (ARCH == ARCH_AVR8)
		/* Commit the flash page to memory */
		boot_page_write(CurrAddress);

		/* Wait until write operation has completed */
		boot_spm_busy_wait();
#elif (ARCH == ARCH_XMEGA)
		SP_WriteApplicationPage(CurrAddress << 1);
		SP_WaitForSPM();
#endif

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if (Command == AVR109_COMMAND_ReadFLASHWord)
	{
		#if (FLASHEND > 0xFFFF)
		uint16_t ProgramWord = pgm_read_word_far(CurrAddress);
		#else
		uint16_t ProgramWord = pgm_read_word(CurrAddress);
		#endif

		WriteNextResponseByte(ProgramWord >> 8);
		WriteNextResponseByte(ProgramWord & 0xFF);
	}
	#endif
	#if !defined(NO_EEPROM_BYTE_SUPPORT)
	else if (Command == AVR109_COMMAND_WriteEEPROM)
	{
		/* Read the byte from the endpoint and write it to the EEPROM */
		eeprom_write_byte((uint8_t*)((intptr_t)(CurrAddress >> 1)), FetchNextCommandByte());

		/* Increment the address after use */
		CurrAddress += 2;

		/* Send confirmation byte back to the host */
		WriteNextResponseByte('\r');
	}
	else if (Command == AVR109_COMMAND_ReadEEPROM)
	{
		/* Read the EEPROM byte and write it to the endpoint */
		WriteNextResponseByte(eeprom_read_byte((uint8_t*)((intptr_t)(CurrAddress >> 1))));

		/* Increment the address after use */
		CurrAddress += 2;
	}
	#endif
	else if (Command != AVR109_COMMAND_Sync)
	{
		/* Unknown (non-sync) command, return fail code */
		WriteNextResponseByte('?');
	}

	/* Select the IN endpoint */
	Endpoint_SelectEndpoint(CDC_TX_EPADDR);

	/* Remember if the endpoint is completely full before clearing it */
	bool IsEndpointFull = !(Endpoint_IsReadWriteAllowed());

	/* Send the endpoint data to the host */
	Endpoint_ClearIN();

	/* If a full endpoint's worth of data was sent, we need to send an empty packet afterwards to signal end of transfer */
	if (IsEndpointFull)
	{
		while (!(Endpoint_IsINReady()))
		{
			if (USB_DeviceState == DEVICE_STATE_Unattached)
			  return;
		}

		Endpoint_ClearIN();
	}

	/* Wait until the data has been sent to the host */
	while (!(Endpoint_IsINReady()))
	{
		if (USB_DeviceState == DEVICE_STATE_Unattached)
		  return;
	}

	/* Select the OUT endpoint */
	Endpoint_SelectEndpoint(CDC_RX_EPADDR);

	/* Acknowledge the command from the host */
	Endpoint_ClearOUT();
}

