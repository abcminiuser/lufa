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

/** \file
 *
 *  Main source file for the AudioOutput demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "AudioOutput.h"

/** LUFA Audio Class driver interface configuration and state information. This structure is
 *  passed to all Audio Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_Audio_Device_t Speaker_Audio_Interface =
	{
		.Config =
			{
				.StreamingInterfaceNumber = 1,

				.DataOUTEndpointNumber    = AUDIO_STREAM_EPNUM,
				.DataOUTEndpointSize      = AUDIO_STREAM_EPSIZE,
			},
	};

/** Current audio sampling frequency of the streaming audio endpoint. */
static uint32_t CurrentAudioSampleFrequency = 48000;


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
	
	for (;;)
	{
		Audio_Device_USBTask(&Speaker_Audio_Interface);
		USB_USBTask();
	}
}

#define F_XTAL0 12000000

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	#if (ARCH == ARCH_AVR8)
		/* Disable watchdog if enabled by bootloader/fuses */
		MCUSR &= ~(1 << WDRF);
		wdt_disable();

		/* Disable clock division */
		clock_prescale_set(clock_div_1);
	#elif (ARCH == ARCH_UC3)
		/* Start the master external oscillator which will be used as the main clock reference */
		AVR32CLK_StartExternalOscillator(0, EXOSC_MODE_8MHZ_OR_MORE, EXOSC_START_0CLK);
		
		/* Start the PLL for the CPU clock, switch CPU to it */
		AVR32CLK_StartPLL(0, CLOCK_SRC_OSC0, 12000000, F_CPU);
		AVR32CLK_SetCPUClockSource(CLOCK_SRC_PLL0, F_CPU);

		/* Start the PLL for the USB Generic Clock module */
		AVR32CLK_StartPLL(1, CLOCK_SRC_OSC0, 12000000, 48000000);
		
		/* Initialize interrupt subsystem */
		INTC_Init();
		INTC_RegisterGroupHandler(AVR32_USBB_IRQ, AVR32_INTC_INT0, USB_GEN_vect);
	#endif
	
	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();
}

/** ISR to handle the reloading of the PWM timer with the next sample. */
#if (ARCH == ARCH_AVR)
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
#elif (ARCH == ARCH_UC3)
ISR(TC_CH0_vect)
#endif
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

		#if (ARCH == ARCH_AVR8)
			/* Load the dual 8-bit samples into the PWM timer channels */
			OCR3A = (LeftSample_8Bit  ^ (1 << 7));
			OCR3B = (RightSample_8Bit ^ (1 << 7));
		#elif (ARCH == ARCH_UC3)
			/* Load the dual 16-bit samples into the PWM timer channels */
			AVR32_TC.channel[2].ra = (LeftSample_8Bit  ^ (1 << 7));
			AVR32_TC.channel[2].rb = (RightSample_8Bit ^ (1 << 7));
		#endif
		
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
	
	#if (ARCH == ARCH_UC3)
		/* Perform dummy read of the TC Status Register to clear interrupt flags */
		(void)AVR32_TC.channel[0].sr;
	#endif
	
	Endpoint_SelectEndpoint(PrevEndpoint);	
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);

	#if (ARCH == ARCH_AVR8)
		/* Sample reload timer initialization */
		TIMSK0  = (1 << OCIE0A);
		OCR0A   = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
		TCCR0A  = (1 << WGM01);  // CTC mode
		TCCR0B  = (1 << CS01);   // Fcpu/8 speed

		/* Set speakers as outputs */
		DDRC   |= ((1 << 6) | (1 << 5));

		/* PWM speaker timer initialization */
		TCCR3A  = ((1 << WGM30) | (1 << COM3A1) | (1 << COM3A0)
		        | (1 << COM3B1) | (1 << COM3B0)); // Set on match, clear on TOP
		TCCR3B  = ((1 << WGM32) | (1 << CS30));  // Fast 8-Bit PWM, F_CPU speed
	#elif (ARCH == ARCH_UC3)
		/* Sample reload timer initialization */
		INTC_RegisterGroupHandler(AVR32_TC_IRQ0, AVR32_INTC_INT0, TC_CH0_vect);
		AVR32_TC.channel[0].IER.cpcs = true;
		AVR32_TC.channel[0].cmr      = AVR32_TC_CMR0_WAVE_MASK |
		                               (AVR32_TC_CMR0_WAVSEL_UP_AUTO << AVR32_TC_CMR0_WAVSEL_OFFSET) |
		                               (AVR32_TC_CMR0_TCCLKS_TIMER_CLOCK3 << AVR32_TC_CMR0_TCCLKS_OFFSET);
		AVR32_TC.channel[0].rc       = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);		
		AVR32_TC.channel[0].ccr      = AVR32_TC_SWTRG_MASK | AVR32_TC_CLKEN_MASK;

		/* Configure GPIO pins for speaker output as the timer alternative functions */
		AVR32_GPIO.port[1].gper     &= ~((1UL << 10) | (1UL << 11));
		AVR32_GPIO.port[1].pmr0     |=  ((1UL << 10) | (1UL << 11));
		AVR32_GPIO.port[1].pmr1     &= ~((1UL << 10) | (1UL << 11));

		/* Set speakers as outputs */
		AVR32_GPIO.port[1].oder     |=  ((1UL << 10) | (1UL << 11));

		/* PWM speaker timer initialization */
		AVR32_TC.channel[2].cmr      = AVR32_TC_CMR2_WAVE_MASK |
		                               (AVR32_TC_CMR2_EEVT_XC0_OUTPUT << AVR32_TC_CMR2_EEVT_OFFSET) |
		                               (AVR32_TC_CMR2_WAVSEL_UP_AUTO << AVR32_TC_CMR2_WAVSEL_OFFSET) |
		                               (AVR32_TC_CMR2_TCCLKS_TIMER_CLOCK2 << AVR32_TC_CMR2_TCCLKS_OFFSET) |
		                               (AVR32_TC_CMR2_ACPA_SET   << AVR32_TC_CMR2_ACPA_OFFSET) |
		                               (AVR32_TC_CMR2_ACPC_CLEAR << AVR32_TC_CMR2_ACPC_OFFSET) |
		                               (AVR32_TC_CMR2_BCPB_SET   << AVR32_TC_CMR2_BCPB_OFFSET) |
		                               (AVR32_TC_CMR2_BCPC_CLEAR << AVR32_TC_CMR2_BCPC_OFFSET);
		AVR32_TC.channel[2].rc       = 0x00000FF;
		AVR32_TC.channel[2].ccr      = AVR32_TC_SWTRG_MASK | AVR32_TC_CLKEN_MASK;		
	#endif
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

	#if (ARCH == ARCH_AVR8)
		/* Stop the sample reload timer */
		TCCR0B = 0;

		/* Stop the PWM generation timer */
		TCCR3B = 0;

		/* Set speakers as inputs to reduce current draw */
		DDRC  &= ~((1 << 6) | (1 << 5));
	#elif (ARCH == ARCH_UC3)
		/* Stop the sample reload timer */
		AVR32_TC.channel[0].CCR.clken = false;

		/* Stop the PWM generation timer */
		AVR32_TC.channel[2].CCR.clken = false;
		
		/* Set speakers as inputs to reduce current draw */
		AVR32_GPIO.port[1].oder &= ~((1UL << 10) | (1UL << 11));		
	#endif
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= Audio_Device_ConfigureEndpoints(&Speaker_Audio_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	Audio_Device_ProcessControlRequest(&Speaker_Audio_Interface);
}

/** Audio class driver callback for the setting and retrieval of streaming endpoint properties. This callback must be implemented
 *  in the user application to handle property manipulations on streaming audio endpoints.
 *
 *  When the DataLength parameter is NULL, this callback should only indicate whether the specified operation is valid for
 *  the given endpoint index, and should return as fast as possible. When non-NULL, this value may be altered for GET operations
 *  to indicate the size of the retreived data.
 *
 *  \note The length of the retrieved data stored into the Data buffer on GET operations should not exceed the initial value
 *        of the \c DataLength parameter.
 *
 *  \param[in,out] AudioInterfaceInfo  Pointer to a structure containing an Audio Class configuration and state.
 *  \param[in]     EndpointProperty    Property of the endpoint to get or set, a value from Audio_ClassRequests_t.
 *  \param[in]     EndpointAddress     Address of the streaming endpoint whose property is being referenced.
 *  \param[in]     EndpointControl     Parameter of the endpoint to get or set, a value from Audio_EndpointControls_t.
 *  \param[in,out] DataLength          For SET operations, the length of the parameter data to set. For GET operations, the maximum
 *                                     length of the retrieved data. When NULL, the function should return whether the given property
 *                                     and parameter is valid for the requested endpoint without reading or modifying the Data buffer.
 *  \param[in,out] Data                Pointer to a location where the parameter data is stored for SET operations, or where
 *                                     the retrieved data is to be stored for GET operations.
 *
 *  \return Boolean true if the property get/set was successful, false otherwise
 */
bool CALLBACK_Audio_Device_GetSetEndpointProperty(USB_ClassInfo_Audio_Device_t* const AudioInterfaceInfo,
                                                  const uint8_t EndpointProperty,
                                                  const uint8_t EndpointAddress,
                                                  const uint8_t EndpointControl,
                                                  uint16_t* const DataLength,
                                                  uint8_t* Data)
{
	/* Check the requested endpoint to see if a supported endpoint is being manipulated */
	if (EndpointAddress == (ENDPOINT_DESCRIPTOR_DIR_OUT | Speaker_Audio_Interface.Config.DataOUTEndpointNumber))
	{
		/* Check the requested control to see if a supported control is being manipulated */
		if (EndpointControl == AUDIO_EPCONTROL_SamplingFreq)
		{
			/* Check the requested property to see if a supported property is being manipulated */
			switch (EndpointProperty)
			{
				case AUDIO_REQ_SetCurrent:
					/* Check if we are just testing for a valid property, or actually adjusting it */
					if (DataLength != NULL)
					{
						/* Set the new sampling frequency to the value given by the host */
						CurrentAudioSampleFrequency = (((uint32_t)Data[2] << 16) | ((uint32_t)Data[1] << 8) | (uint32_t)Data[0]);

						#if (ARCH == ARCH_AVR8)
							/* Adjust sample reload timer to the new frequency */
							OCR0A = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
						#elif (ARCH == ARCH_UC3)
							/* Adjust sample reload timer to the new frequency */
							AVR32_TC.channel[0].RC.rc = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);		
						#endif
					}
					
					return true;
				case AUDIO_REQ_GetCurrent:
					/* Check if we are just testing for a valid property, or actually reading it */
					if (DataLength != NULL)
					{
						*DataLength = 3;

						Data[2] = (CurrentAudioSampleFrequency >> 16);
						Data[1] = (CurrentAudioSampleFrequency >> 8);
						Data[0] = (CurrentAudioSampleFrequency &  0xFF);					
					}
					
					return true;
			}
		}
	}
	
	return false;
}
