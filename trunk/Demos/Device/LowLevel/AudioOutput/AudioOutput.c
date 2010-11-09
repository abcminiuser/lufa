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
 *  Main source file for the AudioOutput demo. This file contains the main tasks of the demo and
 *  is responsible for the initial application hardware configuration.
 */

#include "AudioOutput.h"

/** Flag to indicate if the streaming audio alternative interface has been selected by the host. */
bool StreamingAudioInterfaceSelected = false;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	for (;;)
	{
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
	LEDs_Init();
	USB_Init();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs, and
 *  configures the sample update and PWM timers.
 */
void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);

	/* Sample reload timer initialization */
	TIMSK0 = (1 << OCIE0A);
	OCR0A  = ((F_CPU / 8 / AUDIO_SAMPLE_FREQUENCY) - 1);
	TCCR0A = (1 << WGM01);  // CTC mode
	TCCR0B = (1 << CS01);   // Fcpu/8 speed

	#if defined(AUDIO_OUT_MONO)
	/* Set speaker as output */
	DDRC  |= (1 << 6);
	#elif defined(AUDIO_OUT_STEREO)
	/* Set speakers as outputs */
	DDRC  |= ((1 << 6) | (1 << 5));
	#elif defined(AUDIO_OUT_PORTC)
	/* Set PORTC as outputs */
	DDRC  |= 0xFF;
	#endif

	#if (defined(AUDIO_OUT_MONO) || defined(AUDIO_OUT_STEREO))
	/* PWM speaker timer initialization */
	TCCR3A = ((1 << WGM30) | (1 << COM3A1) | (1 << COM3A0)
	                       | (1 << COM3B1) | (1 << COM3B0)); // Set on match, clear on TOP
	TCCR3B = ((1 << WGM32) | (1 << CS30));  // Fast 8-Bit PWM, F_CPU speed
	#endif
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs, disables the sample update and PWM output timers and stops the USB and Audio management tasks.
 */
void EVENT_USB_Device_Disconnect(void)
{
	/* Stop the timers */
	TCCR0B = 0;
	#if (defined(AUDIO_OUT_MONO) || defined(AUDIO_OUT_STEREO))
	TCCR3B = 0;
	#endif

	#if defined(AUDIO_OUT_MONO)
	/* Set speaker as input to reduce current draw */
	DDRC  &= ~(1 << 6);
	#elif defined(AUDIO_OUT_STEREO)
	/* Set speakers as inputs to reduce current draw */
	DDRC  &= ~((1 << 6) | (1 << 5));
	#elif defined(AUDIO_OUT_PORTC)
	/* Set PORTC low */
	PORTC = 0x00;
	#endif

	/* Indicate streaming audio interface not selected */
	StreamingAudioInterfaceSelected = false;

	/* Indicate USB not ready */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup Audio Stream Endpoint */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(AUDIO_STREAM_EPNUM, EP_TYPE_ISOCHRONOUS, ENDPOINT_DIR_OUT,
	                                            AUDIO_STREAM_EPSIZE, ENDPOINT_BANK_DOUBLE);

	/* Indicate endpoint configuration success or failure */
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	/* Process General and Audio specific control requests */
	switch (USB_ControlRequest.bRequest)
	{
		case REQ_SetInterface:
			/* Set Interface is not handled by the library, as its function is application-specific */
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_STANDARD | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();
				Endpoint_ClearStatusStage();

				/* Check if the host is enabling the audio interface (setting AlternateSetting to 1) */
				StreamingAudioInterfaceSelected = ((USB_ControlRequest.wValue) != 0);
			}

			break;
	}
}

/** ISR to handle the reloading of the PWM timer with the next sample. */
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
	uint8_t PrevEndpoint = Endpoint_GetCurrentEndpoint();

	/* Select the audio stream endpoint */
	Endpoint_SelectEndpoint(AUDIO_STREAM_EPNUM);

	/* Check if the current endpoint can be read from (contains a packet) and the host is sending data */
	if (Endpoint_IsOUTReceived() && StreamingAudioInterfaceSelected)
	{
		/* Retrieve the signed 16-bit left and right audio samples, convert to 8-bit */
		int8_t LeftSample_8Bit   = ((int16_t)Endpoint_Read_Word_LE() >> 8);
		int8_t RightSample_8Bit  = ((int16_t)Endpoint_Read_Word_LE() >> 8);

		/* Mix the two channels together to produce a mono, 8-bit sample */
		int8_t MixedSample_8Bit  = (((int16_t)LeftSample_8Bit + (int16_t)RightSample_8Bit) >> 1);

		/* Check to see if the bank is now empty */
		if (!(Endpoint_IsReadWriteAllowed()))
		{
			/* Acknowledge the packet, clear the bank ready for the next packet */
			Endpoint_ClearOUT();
		}

		#if defined(AUDIO_OUT_MONO)
		/* Load the sample into the PWM timer channel */
		OCR3A = (MixedSample_8Bit ^ (1 << 7));
		#elif defined(AUDIO_OUT_STEREO)
		/* Load the dual 8-bit samples into the PWM timer channels */
		OCR3A = (LeftSample_8Bit  ^ (1 << 7));
		OCR3B = (RightSample_8Bit ^ (1 << 7));
		#elif defined(AUDIO_OUT_PORTC)
		/* Load the 8-bit mixed sample into PORTC */
		PORTC = MixedSample_8Bit;
		#endif

		uint8_t LEDMask = LEDS_NO_LEDS;

		/* Turn on LEDs as the sample amplitude increases */
		if (MixedSample_8Bit > 16)
		  LEDMask = (LEDS_LED1 | LEDS_LED2 | LEDS_LED3 | LEDS_LED4);
		else if (MixedSample_8Bit > 8)
		  LEDMask = (LEDS_LED1 | LEDS_LED2 | LEDS_LED3);
		else if (MixedSample_8Bit > 4)
		  LEDMask = (LEDS_LED1 | LEDS_LED2);
		else if (MixedSample_8Bit > 2)
		  LEDMask = (LEDS_LED1);

		LEDs_SetAllLEDs(LEDMask);
	}
	
	Endpoint_SelectEndpoint(PrevEndpoint);
}

