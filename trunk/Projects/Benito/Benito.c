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
 *  Main source file for the Benito project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "Benito.h"

/** Circular buffer to hold data from the serial port before it is sent to the host. */
RingBuff_t Tx_Buffer;

/** Pulse generation counters to keep track of the number of milliseconds remaining for each pulse type */
volatile struct
{
	uint8_t ResetPulse; /**< Milliseconds remaining for target /RESET pulse */
	uint8_t TxLEDPulse; /**< Milliseconds remaining for data Tx LED pulse */
	uint8_t RxLEDPulse; /**< Milliseconds remaining for data Rx LED pulse */
	uint8_t PingPongLEDPulse; /**< Milliseconds remaining for enumeration Tx/Rx ping-pong LED pulse */
} PulseMSRemaining;

/** Previous state of the virtual DTR control line from the host */
bool PreviousDTRState = false;

/** Milliseconds remaining until the receive buffer is flushed to the USB host */
uint8_t FlushPeriodRemaining = RECEIVE_BUFFER_FLUSH_MS;

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = 0,

				.DataINEndpointNumber           = CDC_TX_EPNUM,
				.DataINEndpointSize             = CDC_TXRX_EPSIZE,
				.DataINEndpointDoubleBank       = false,

				.DataOUTEndpointNumber          = CDC_RX_EPNUM,
				.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
				.DataOUTEndpointDoubleBank      = false,

				.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
				.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
				.NotificationEndpointDoubleBank = false,
			},
	};

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	RingBuffer_InitBuffer(&Tx_Buffer);

	sei();

	for (;;)
	{
		/* Echo bytes from the host to the target via the hardware USART */
		int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		if (!(ReceivedByte < 0) && (UCSR1A & (1 << UDRE1)))
		{
			UDR1 = ReceivedByte;

			LEDs_TurnOnLEDs(LEDMASK_TX);
			PulseMSRemaining.TxLEDPulse = TX_RX_LED_PULSE_MS;
		}

		/* Check if the millisecond timer has elapsed */
		if (TIFR0 & (1 << OCF0A))
		{
			/* Check if the reset pulse period has elapsed, if so tristate the target reset line */
			if (PulseMSRemaining.ResetPulse && !(--PulseMSRemaining.ResetPulse))
			{
				LEDs_TurnOffLEDs(LEDMASK_BUSY);
				AVR_RESET_LINE_DDR &= ~AVR_RESET_LINE_MASK;
			}

			/* Check if the LEDs should be ping-ponging (during enumeration) */
			if (PulseMSRemaining.PingPongLEDPulse && !(--PulseMSRemaining.PingPongLEDPulse))
			{
				LEDs_ToggleLEDs(LEDMASK_TX | LEDMASK_RX);
				PulseMSRemaining.PingPongLEDPulse = PING_PONG_LED_PULSE_MS;
			}

			/* Turn off TX LED(s) once the TX pulse period has elapsed */
			if (PulseMSRemaining.TxLEDPulse && !(--PulseMSRemaining.TxLEDPulse))
			  LEDs_TurnOffLEDs(LEDMASK_TX);

			/* Turn off RX LED(s) once the RX pulse period has elapsed */
			if (PulseMSRemaining.RxLEDPulse && !(--PulseMSRemaining.RxLEDPulse))
			  LEDs_TurnOffLEDs(LEDMASK_RX);

			/* Check if the receive buffer flush period has expired */
			RingBuff_Count_t BufferCount = RingBuffer_GetCount(&Tx_Buffer);
			if (!(--FlushPeriodRemaining) || (BufferCount > 200))
			{
				/* Echo bytes from the target to the host via the virtual serial port */
				if (BufferCount)
				{
					while (BufferCount--)
					  CDC_Device_SendByte(&VirtualSerial_CDC_Interface, RingBuffer_Remove(&Tx_Buffer));

					LEDs_TurnOnLEDs(LEDMASK_RX);
					PulseMSRemaining.RxLEDPulse = TX_RX_LED_PULSE_MS;
				}

				FlushPeriodRemaining = RECEIVE_BUFFER_FLUSH_MS;
			}

			/* Clear the millisecond timer CTC flag (cleared by writing logic one to the register) */
			TIFR0 |= (1 << OCF0A);
		}

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();

	/* Millisecond Timer Interrupt */
	OCR0A  = (F_CPU / 64 / 1000);
	TCCR0A = (1 << WGM01);
	TCCR0B = ((1 << CS01) | (1 << CS00));

	/* Tristate target /RESET Line */
	AVR_RESET_LINE_PORT &= ~AVR_RESET_LINE_MASK;
	AVR_RESET_LINE_DDR  &= ~AVR_RESET_LINE_MASK;
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	PulseMSRemaining.PingPongLEDPulse = PING_PONG_LED_PULSE_MS;
	LEDs_SetAllLEDs(LEDMASK_TX);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	PulseMSRemaining.PingPongLEDPulse = 0;
	LEDs_SetAllLEDs(LEDS_NO_LEDS);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	PulseMSRemaining.PingPongLEDPulse = 0;

	LEDs_SetAllLEDs(ConfigSuccess ? LEDS_NO_LEDS : LEDMASK_ERROR);
}

/** Event handler for the library USB Unhandled Control Request event. */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** Event handler for the CDC Class driver Line Encoding Changed event.
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
	uint8_t ConfigMask = 0;

	switch (CDCInterfaceInfo->State.LineEncoding.ParityType)
	{
		case CDC_PARITY_Odd:
			ConfigMask = ((1 << UPM11) | (1 << UPM10));
			break;
		case CDC_PARITY_Even:
			ConfigMask = (1 << UPM11);
			break;
	}

	if (CDCInterfaceInfo->State.LineEncoding.CharFormat == CDC_LINEENCODING_TwoStopBits)
	  ConfigMask |= (1 << USBS1);

	switch (CDCInterfaceInfo->State.LineEncoding.DataBits)
	{
		case 6:
			ConfigMask |= (1 << UCSZ10);
			break;
		case 7:
			ConfigMask |= (1 << UCSZ11);
			break;
		case 8:
			ConfigMask |= ((1 << UCSZ11) | (1 << UCSZ10));
			break;
	}

	/* Must turn off USART before reconfiguring it, otherwise incorrect operation may occur */
	UCSR1B = 0;
	UCSR1A = 0;
	UCSR1C = 0;

	/* Set the new baud rate before configuring the USART */
	UBRR1  = SERIAL_2X_UBBRVAL(CDCInterfaceInfo->State.LineEncoding.BaudRateBPS);

	/* Reconfigure the USART in double speed mode for a wider baud rate range at the expense of accuracy */
	UCSR1C = ConfigMask;
	UCSR1A = (1 << U2X1);
	UCSR1B = ((1 << RXCIE1) | (1 << TXEN1) | (1 << RXEN1));
}

/** ISR to manage the reception of data from the serial port, placing received bytes into a circular buffer
 *  for later transmission to the host.
 */
ISR(USART1_RX_vect, ISR_BLOCK)
{
	uint8_t ReceivedByte = UDR1;

	if (USB_DeviceState == DEVICE_STATE_Configured)
	  RingBuffer_Insert(&Tx_Buffer, ReceivedByte);
}

/** Event handler for the CDC Class driver Host-to-Device Line Encoding Changed event.
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
	bool CurrentDTRState = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR);

	/* Check if the DTR line has been asserted - if so, start the target AVR's reset pulse */
	if (!(PreviousDTRState) && CurrentDTRState)
	{
		LEDs_SetAllLEDs(LEDMASK_BUSY);

		AVR_RESET_LINE_DDR |= AVR_RESET_LINE_MASK;
		PulseMSRemaining.ResetPulse = AVR_RESET_PULSE_MS;
	}

	PreviousDTRState = CurrentDTRState;
}

