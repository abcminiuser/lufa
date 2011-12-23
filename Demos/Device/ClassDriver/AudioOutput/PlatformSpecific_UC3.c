/*
             LUFA Library
     Copyright (C) Dean Camera, 2011.

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
	/* Start the master external oscillator which will be used as the main clock reference */
	AVR32CLK_StartExternalOscillator(0, EXOSC_MODE_8MHZ_OR_MORE, EXOSC_START_0CLK);
	
	/* Start the PLL for the CPU clock, switch CPU to it */
	AVR32CLK_StartPLL(0, CLOCK_SRC_OSC0, 12000000, F_CPU);
	AVR32CLK_SetCPUClockSource(CLOCK_SRC_PLL0, F_CPU);

	/* Start the PLL for the USB Generic Clock module */
	AVR32CLK_StartPLL(1, CLOCK_SRC_OSC0, 12000000, F_USB);
	
	/* Initialize interrupt subsystem */
	INTC_Init();
	INTC_RegisterGroupHandler(INTC_IRQ_GROUP(AVR32_USBB_IRQ), AVR32_INTC_INT0, USB_GEN_vect);
	
	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();
}

void ConfigureAudioOutput(bool Enable)
{
	if (Enable)
	{
		/* Sample reload timer initialization */
		INTC_RegisterGroupHandler(AVR32_TC_IRQ0, AVR32_INTC_INT0, TC_CH0_vect);
		AVR32_TC.channel[0].IER.cpcs  = true;
		AVR32_TC.channel[0].cmr       = (AVR32_TC_CMR0_WAVE_MASK |
										 (AVR32_TC_CMR0_WAVSEL_UP_AUTO << AVR32_TC_CMR0_WAVSEL_OFFSET) |
										 (AVR32_TC_CMR0_TCCLKS_TIMER_CLOCK3 << AVR32_TC_CMR0_TCCLKS_OFFSET));
		AVR32_TC.channel[0].rc        = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);		
		AVR32_TC.channel[0].ccr       = AVR32_TC_SWTRG_MASK | AVR32_TC_CLKEN_MASK;

		/* Configure GPIO pins for speaker output as the PWM alternative functions */
		AVR32_GPIO.port[PWM_GPIO_LEFT / 32].gper   &= ~(1UL << (PWM_GPIO_LEFT % 32));
		AVR32_GPIO.port[PWM_GPIO_LEFT / 32].pmr0   |=  (1UL << (PWM_GPIO_LEFT % 32));
		AVR32_GPIO.port[PWM_GPIO_LEFT / 32].pmr1   &= ~(1UL << (PWM_GPIO_LEFT % 32));
		AVR32_GPIO.port[PWM_GPIO_RIGHT / 32].gper  &= ~(1UL << (PWM_GPIO_RIGHT % 32));
		AVR32_GPIO.port[PWM_GPIO_RIGHT / 32].pmr0  |=  (1UL << (PWM_GPIO_RIGHT % 32));
		AVR32_GPIO.port[PWM_GPIO_RIGHT / 32].pmr1  &= ~(1UL << (PWM_GPIO_RIGHT % 32));
		
		/* Set speakers as outputs */
		AVR32_GPIO.port[PWM_GPIO_LEFT / 32].oder   &= ~(1UL << (PWM_GPIO_LEFT % 32));
		AVR32_GPIO.port[PWM_GPIO_RIGHT / 32].oder  &= ~(1UL << (PWM_GPIO_RIGHT % 32));

		/* PWM unit initialization */
		AVR32_PWM.ena                                |= ((1UL << PWM_CHANNEL_LEFT) | (1UL << PWM_CHANNEL_RIGHT));
		AVR32_PWM.channel[PWM_CHANNEL_LEFT].CMR.calg  = true;
		AVR32_PWM.channel[PWM_CHANNEL_LEFT].cprd      = 0xFF;
		AVR32_PWM.channel[PWM_CHANNEL_LEFT].cdty      = 0;
		AVR32_PWM.channel[PWM_CHANNEL_RIGHT].CMR.calg = true;
		AVR32_PWM.channel[PWM_CHANNEL_RIGHT].cprd     = 0xFF;
		AVR32_PWM.channel[PWM_CHANNEL_RIGHT].cdty     = 0;
	}
	else
	{
		/* Stop the sample reload timer */
		AVR32_TC.channel[0].CCR.clken = false;
		
		/* Set speakers as inputs to reduce current draw */
		AVR32_GPIO.port[PWM_GPIO_LEFT / 32].oder  &= ~(1UL << (PWM_GPIO_LEFT % 32));
		AVR32_GPIO.port[PWM_GPIO_RIGHT / 32].oder &= ~(1UL << (PWM_GPIO_RIGHT % 32));
	}
}

void SetAudioSampleFrequency(const uint32_t SampleFrequency)
{
	/* Adjust sample reload timer to the new frequency */
	AVR32_TC.channel[0].RC.rc = ((F_CPU / 8 / AudioSampleFrequency) - 1);		
}

/** ISR to handle the reloading of the PWM timer with the next sample. */
ISR(TC_CH0_vect)
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
		AVR32_PWM.channel[PWM_CHANNEL_LEFT].cupd  = (LeftSample_8Bit  + 1);
		AVR32_PWM.channel[PWM_CHANNEL_RIGHT].cupd = (RightSample_8Bit + 1);
		
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

	/* Perform dummy read of the TC Status Register to clear interrupt flags */
	(void)AVR32_TC.channel[0].sr;
	
	Endpoint_SelectEndpoint(PrevEndpoint);	
}

