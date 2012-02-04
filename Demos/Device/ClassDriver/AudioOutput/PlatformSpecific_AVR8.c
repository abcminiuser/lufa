/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

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

#include "PlatformSpecific.h"

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

void ConfigureAudioOutput(bool Enable)
{
	if (Enable)
	{
		/* Sample reload timer initialization */
		TIMSK0  = (1 << OCIE0A);
		OCR0A   = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
		TCCR0A  = (1 << WGM01);  // CTC mode
		TCCR0B  = (1 << CS01);   // Fcpu/8 speed

		/* Set speakers as outputs */
		DDRC   |= ((1 << 6) | (1 << 5));

		/* PWM speaker timer initialization */
		TCCR3A  = ((1 << WGM30) | (1 << COM3A1) | (1 << COM3A0) |
				   (1 << COM3B1) | (1 << COM3B0)); // Set on match, clear on TOP
		TCCR3B  = ((1 << WGM32) | (1 << CS30));  // Fast 8-Bit PWM, F_CPU speed
	}
	else
	{
		/* Stop the sample reload timer */
		TCCR0B = 0;

		/* Stop the PWM generation timer */
		TCCR3B = 0;

		/* Set speakers as inputs to reduce current draw */
		DDRC  &= ~((1 << 6) | (1 << 5));	
	}
}

void SetAudioSampleFrequency(const uint32_t SampleFrequency)
{
	/* Adjust sample reload timer to the new frequency */
	OCR0A = ((F_CPU / 8 / SampleFrequency) - 1);
}

/** ISR to handle the reloading of the PWM timer with the next sample. */
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
	uint8_t PrevEndpoint = Endpoint_GetCurrentEndpoint();
	
	/* Check that the USB bus is ready for the next sample to read */
	if (Audio_Device_IsSampleReceived(&Speaker_Audio_Interface))
	{
		/* Retrieve the signed 16-bit left and right audio samples */
		int16_t LeftSample_16Bit  = Audio_Device_ReadSample16(&Speaker_Audio_Interface);
		int16_t RightSample_16Bit = Audio_Device_ReadSample16(&Speaker_Audio_Interface);

		/* Convert the signed 16-bit left and right audio samples to 8-bit */
		int8_t LeftSample_8Bit  = (LeftSample_16Bit  >> 8);
		int8_t RightSample_8Bit = (RightSample_16Bit >> 8);

		/* Mix the two channels together to produce a mono, 8-bit sample */
		int8_t MixedSample_8Bit = (((int16_t)LeftSample_8Bit + (int16_t)RightSample_8Bit) >> 1);

		/* Load the dual 8-bit samples into the PWM timer channels */
		OCR3A = (LeftSample_8Bit  ^ (1 << 7));
		OCR3B = (RightSample_8Bit ^ (1 << 7));
		
		uint_reg_t LEDMask = LEDS_NO_LEDS;

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

