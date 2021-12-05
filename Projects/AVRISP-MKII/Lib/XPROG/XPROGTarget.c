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

/** \file
 *
 *  Target-related functions for the PDI Protocol decoder.
 */

#define  INCLUDE_FROM_XPROGTARGET_C
#include "XPROGTarget.h"

#if defined(ENABLE_XPROG_PROTOCOL) || defined(__DOXYGEN__)

/** Flag to indicate if the USART is currently in Tx or Rx mode. */
static bool IsSending;

/** Enables the target's PDI interface, holding the target in reset until PDI mode is exited. */
void XPROGTarget_EnableTargetPDI(void)
{
	IsSending = false;
#if (ARCH == ARCH_AVR8)
	/* Set Tx and XCK as outputs, Rx as input */
	DDRD |=  (1 << 5) | (1 << 3);
	DDRD &= ~(1 << 2);

	/* Set Tx (PDI CLOCK) high, DATA line low for at least 90ns to disable /RESET functionality */
	PORTD |=  (1 << 5);
	PORTD &= ~(1 << 3);
	_delay_us(100);

	/* Set DATA line high (enables PDI interface after 16 PDI CLK cycles) */
	PORTD |= (1 << 3);
	_delay_us(20);

	/* Set up the synchronous USART for XMEGA communications - 8 data bits, even parity, 2 stop bits */
	UBRR1  = ((F_CPU / 2 / XPROG_HARDWARE_SPEED) - 1);
	UCSR1B = (1 << TXEN1);
	UCSR1C = (1 << UMSEL10) | (1 << UPM11) | (1 << USBS1) | (1 << UCSZ11) | (1 << UCSZ10) | (1 << UCPOL1);


#elif (ARCH == ARCH_XMEGA)
	/* Set Tx and XCK as outputs, Rx as input, Invert the XCK PIN */
	PDI_PORT.DIRSET = PDI_TX_MASK | PDI_XCK_MASK;
	PDI_PORT.DIRCLR = PDI_RX_MASK;
	PDI_PORT.PDI_XCK_CTRL = PORT_INVEN_bm;
	
	/* Set Tx (PDI CLOCK) high (Set low because its inverted), DATA line low for at least 90ns to disable /RESET functionality */
	PDI_PORT.DIRCLR = PDI_RX_MASK | PDI_XCK_MASK;
	_delay_us(100);

	/* Set DATA line high for at least 90ns to disable /RESET functionality */
	PDI_PORT.OUTSET = PDI_TX_MASK;
	_delay_us(20);

	/* Set up the synchronous USART for XMEGA communications - 8 data bits, even parity, 2 stop bits */
	PDI_USART.BAUDCTRLA = ((((uint32_t)F_CPU + XPROG_HARDWARE_SPEED) /(2*XPROG_HARDWARE_SPEED)) - 1) & 0xFF;
	PDI_USART.BAUDCTRLB = ((((uint32_t)F_CPU + XPROG_HARDWARE_SPEED) /(2*XPROG_HARDWARE_SPEED)) - 1) >> 8;
	PDI_USART.CTRLA = 0;
	PDI_USART.CTRLB = USART_TXEN_bm | USART_CLK2X_bm;
	PDI_USART.CTRLC = USART_CMODE_SYNCHRONOUS_gc | USART_PMODE_EVEN_gc | USART_SBMODE_bm | USART_CHSIZE_8BIT_gc;
#endif
	/* Send two IDLEs of 12 bits each to enable PDI interface (need at least 16 idle bits) */
	XPROGTarget_SendIdle();
	XPROGTarget_SendIdle();
}

/** Enables the target's TPI interface, holding the target in reset until TPI mode is exited. */
void XPROGTarget_EnableTargetTPI(void)
{
	IsSending = false;

#if (ARCH == ARCH_AVR8)
	/* Set /RESET line low for at least 400ns to enable TPI functionality */
	AUX_LINE_DDR  |=  AUX_LINE_MASK;
	AUX_LINE_PORT &= ~AUX_LINE_MASK;
	_delay_us(100);

	/* Set Tx and XCK as outputs, Rx as input */
	DDRD |=  (1 << 5) | (1 << 3);
	DDRD &= ~(1 << 2);

	/* Set up the synchronous USART for TPI communications - 8 data bits, even parity, 2 stop bits */
	UBRR1  = ((F_CPU / 2 / XPROG_HARDWARE_SPEED) - 1);
	UCSR1B = (1 << TXEN1);
	UCSR1C = (1 << UMSEL10) | (1 << UPM11) | (1 << USBS1) | (1 << UCSZ11) | (1 << UCSZ10) | (1 << UCPOL1);
#elif (ARCH == ARCH_XMEGA)
	/* Set /RESET line low for at least 400ns to enable TPI functionality */
	AUX_LINE_PORT.DIRSET = AUX_LINE_MASK;
	AUX_LINE_PORT.OUTCLR = AUX_LINE_MASK;
	_delay_us(100);

	/* Set Tx and XCK as outputs, Rx as input, Invert XCK */
	PDI_PORT.DIRSET = PDI_TX_MASK | PDI_XCK_MASK;
	PDI_PORT.DIRCLR = PDI_RX_MASK;
	PDI_PORT.PDI_XCK_CTRL = PORT_INVEN_bm;

	/* Set up the synchronous USART for TPI communications - 8 data bits, even parity, 2 stop bits */
	PDI_USART.BAUDCTRLA = ((((uint32_t)F_CPU + XPROG_HARDWARE_SPEED) /(2*XPROG_HARDWARE_SPEED)) - 1) & 0xFF;
	PDI_USART.BAUDCTRLB = ((((uint32_t)F_CPU + XPROG_HARDWARE_SPEED) /(2*XPROG_HARDWARE_SPEED)) - 1) >> 8;
	PDI_USART.CTRLA = 0;
	PDI_USART.CTRLB = USART_TXEN_bm | USART_CLK2X_bm;
	PDI_USART.CTRLC = USART_CMODE_SYNCHRONOUS_gc | USART_PMODE_EVEN_gc | USART_SBMODE_bm | USART_CHSIZE_8BIT_gc;
#endif
	/* Send two IDLEs of 12 bits each to enable TPI interface (need at least 16 idle bits) */
	XPROGTarget_SendIdle();
	XPROGTarget_SendIdle();
}

/** Disables the target's PDI interface, exits programming mode and starts the target's application. */
void XPROGTarget_DisableTargetPDI(void)
{
	/* Switch to Rx mode to ensure that all pending transmissions are complete */
	if (IsSending)
	  XPROGTarget_SetRxMode();

#if (ARCH == ARCH_AVR8)
	/* Turn off receiver and transmitter of the USART, clear settings */
	UCSR1A  = ((1 << TXC1) | (1 << RXC1));
	UCSR1B  = 0;
	UCSR1C  = 0;

	/* Tristate all pins */
	DDRD  &= ~((1 << 5) | (1 << 3));
	PORTD &= ~((1 << 5) | (1 << 3) | (1 << 2));
#elif (ARCH == ARCH_XMEGA)
	/* Turn off receiver and transmitter of the USART, clear settings */
	PDI_USART.STATUS = USART_RXCIF_bm | USART_TXCIF_bm;
	PDI_USART.CTRLA = 0;
	PDI_USART.CTRLB = 0;
	PDI_USART.CTRLC = 0;

	/* Tristate all pins and disable inversion */
	PDI_PORT.DIRCLR = PDI_TX_MASK | PDI_XCK_MASK;
	
	PDI_PORT.PDI_XCK_CTRL &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc | PORT_INVEN_bm);
	PDI_PORT.PDI_TX_CTRL  &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc);
	PDI_PORT.PDI_RX_CTRL  &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc);
#endif
}

/** Disables the target's TPI interface, exits programming mode and starts the target's application. */
void XPROGTarget_DisableTargetTPI(void)
{
	/* Switch to Rx mode to ensure that all pending transmissions are complete */
	if (IsSending)
	  XPROGTarget_SetRxMode();

#if (ARCH == ARCH_AVR8)
	/* Turn off receiver and transmitter of the USART, clear settings */
	UCSR1A |= (1 << TXC1) | (1 << RXC1);
	UCSR1B  = 0;
	UCSR1C  = 0;

	/* Set all USART lines as inputs, tristate */
	DDRD  &= ~((1 << 5) | (1 << 3));
	PORTD &= ~((1 << 5) | (1 << 3) | (1 << 2));

	/* Tristate target /RESET line */
	AUX_LINE_DDR  &= ~AUX_LINE_MASK;
	AUX_LINE_PORT &= ~AUX_LINE_MASK;
#elif (ARCH == ARCH_XMEGA)
	/* Turn off receiver and transmitter of the USART, clear settings */
	PDI_USART.STATUS = USART_RXCIF_bm | USART_TXCIF_bm;
	PDI_USART.CTRLA = 0;
	PDI_USART.CTRLB = 0;
	PDI_USART.CTRLC = 0;

	/* Tristate all pins */
	PDI_PORT.DIRCLR = PDI_TX_MASK | PDI_XCK_MASK;
	PDI_PORT.PDI_XCK_CTRL &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc | PORT_INVEN_bm);
	PDI_PORT.PDI_TX_CTRL  &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc);
	PDI_PORT.PDI_RX_CTRL  &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc);
	
	AUX_LINE_PORT.DIRCLR = AUX_LINE_MASK;
	AUX_LINE_PORT.OUTCLR = AUX_LINE_MASK;
	AUX_LINE_PORT.AUX_LINE_CTRL &= ~(PORT_OPC_PULLUP_gc | PORT_OPC_PULLDOWN_gc);
#endif
}

/** Sends a byte via the USART.
 *
 *  \param[in] Byte  Byte to send through the USART
 */
void XPROGTarget_SendByte(const uint8_t Byte)
{
	/* Switch to Tx mode if currently in Rx mode */
	if (!(IsSending))
	  XPROGTarget_SetTxMode();

	/* Wait until there is space in the hardware Tx buffer before writing */
#if (ARCH == ARCH_AVR8)
	while (!(UCSR1A & (1 << UDRE1)));
	UCSR1A |= (1 << TXC1);
	UDR1    = Byte;
#elif (ARCH == ARCH_XMEGA)
	while(!(PDI_USART.STATUS & USART_DREIF_bm))
		;
	PDI_USART.STATUS |= USART_TXCIF_bm;
	PDI_USART.DATA = Byte;
#endif
}

/** Receives a byte via the hardware USART, blocking until data is received or timeout expired.
 *
 *  \return Received byte from the USART
 */
uint8_t XPROGTarget_ReceiveByte(void)
{
	/* Switch to Rx mode if currently in Tx mode */
	if (IsSending)
	  XPROGTarget_SetRxMode();

	/* Wait until a byte has been received before reading */
#if (ARCH == ARCH_AVR8)
	while (!(UCSR1A & (1 << RXC1)) && TimeoutTicksRemaining)
		;
	return UDR1;
#elif (ARCH == ARCH_XMEGA)
	while(!(PDI_USART.STATUS & USART_RXCIF_bm) && TimeoutTicksRemaining)
		;
	return PDI_USART.DATA;
#endif
}

/** Sends an IDLE via the USART to the attached target, consisting of a full frame of idle bits. */
void XPROGTarget_SendIdle(void)
{
	/* Switch to Tx mode if currently in Rx mode */
	if (!(IsSending))
	  XPROGTarget_SetTxMode();

	/* Need to do nothing for a full frame to send an IDLE */
	for (uint8_t i = 0; i < BITS_IN_USART_FRAME; i++)
	{
#if (ARCH == ARCH_AVR8)
		/* Wait for a full cycle of the clock */
		while (PIND & (1 << 5));
		while (!(PIND & (1 << 5)));
		while (PIND & (1 << 5));
#elif (ARCH == ARCH_XMEGA)
		/* As the pin inversion is active we need to wait inverted */
		uint32_t timeout = 100000;
		while (timeout && !(PDI_PORT.IN & PDI_XCK_MASK))
			timeout--;
		while (timeout && (PDI_PORT.IN & PDI_XCK_MASK))
			timeout--;
		while (timeout && !(PDI_PORT.IN & PDI_XCK_MASK))
			timeout--;
#endif
	}
}

static void XPROGTarget_SetTxMode(void)
{
	/* Wait for a full cycle of the clock */
#if (ARCH == ARCH_AVR8)
	while (PIND & (1 << 5));
	while (!(PIND & (1 << 5)));
	while (PIND & (1 << 5));

	PORTD  |=  (1 << 3);
	DDRD   |=  (1 << 3);

	UCSR1B &= ~(1 << RXEN1);
	UCSR1B |=  (1 << TXEN1);
#elif (ARCH == ARCH_XMEGA)
	/* As the pin is actived, we need to wait inverted */
	uint32_t timeout = 100000;
	while (timeout && !(PDI_PORT.IN & PDI_XCK_MASK))
	{
		timeout--;
	}
	timeout = 100000;
	while (timeout && (PDI_PORT.IN & PDI_XCK_MASK))
	{
		timeout--;
	}
	timeout = 100000;
	while (timeout && !(PDI_PORT.IN & PDI_XCK_MASK))
	{
		timeout--;
	}

	PDI_PORT.OUTSET = PDI_TX_MASK;
	PDI_PORT.DIRSET = PDI_TX_MASK;

	PDI_USART.CTRLB &= ~USART_RXEN_bm;
	PDI_USART.CTRLB |=  USART_TXEN_bm;
#endif
	IsSending = true;
}

static void XPROGTarget_SetRxMode(void)
{
#if (ARCH == ARCH_AVR8)
	while (!(UCSR1A & (1 << TXC1)));
	UCSR1A |=  (1 << TXC1);

	UCSR1B &= ~(1 << TXEN1);
	UCSR1B |=  (1 << RXEN1);

	DDRD   &= ~(1 << 3);
	PORTD  &= ~(1 << 3);
#elif (ARCH == ARCH_XMEGA)
	while(!(PDI_USART.STATUS & USART_TXCIF_bm))
		;
	PDI_USART.STATUS = USART_TXCIF_bm;

	PDI_USART.CTRLB &= ~USART_TXEN_bm;
	PDI_USART.CTRLB |=  USART_RXEN_bm;

	PDI_PORT.DIRCLR = PDI_TX_MASK;
	PDI_PORT.OUTCLR = PDI_TX_MASK;
#endif
	IsSending = false;
}

#endif

