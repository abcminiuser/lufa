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
 *  \brief Application Configuration Header File
 *
 *  This is a header file which is be used to configure some of
 *  the application's compile time options, as an alternative to
 *  specifying the compile time constants supplied through a
 *  makefile or build system.
 *
 *  For information on what each token does, refer to the
 *  \ref Sec_Options section of the application documentation.
 */

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_
	#if (ARCH == ARCH_AVR8)
		#define AUX_LINE_PORT              PORTB
		#define AUX_LINE_PIN               PINB
		#define AUX_LINE_DDR               DDRB
		#if (BOARD == BOARD_U2S)
			#define AUX_LINE_MASK          (1 << 0)
		#else
			#define AUX_LINE_MASK          (1 << 4)
		#endif
		#define VTARGET_ADC_CHANNEL        2
		#define VTARGET_REF_VOLTS          5
		#define VTARGET_SCALE_FACTOR       1
//		#define VTARGET_USE_INTERNAL_REF
	#elif (ARCH == ARCH_XMEGA)
		#define SPI_REG                 SPIC
		#define SPI_PORT                PORTC

		#define SPI_SCK_MASK           (1 << 7)
		#define SPI_SCK_CTRL            PIN7CTRL

		#define SPI_MISO_MASK          (1 << 6)
		#define SPI_MISO_CTRL           PIN6CTRL

		#define SPI_MOSI_MASK          (1 << 5)
		#define SPI_MOSI_CTRL           PIN5CTRL

		#define SPI_RST_MASK           (1 << 4)
		#define SPI_RST_CTRL            PIN4CTRL

		#define RESCUE_PORT             PORTD
		#define RESCUE_TIMER            TCD0
		#define RESCUE_PIN_MASK        (1 << 0)
		#define RESCUE_TIMER_CMP_EN     TC0_CCAEN_bm //A if pin 0, B if pin 1...
		#define RESCUE_TIMER_CMP_REG    CCA

		#define PDI_USART               USARTC0
		#define PDI_PORT                PORTC
		#define PDI_RX_MASK            (1 << 2)
		#define PDI_RX_CTRL             PIN1CTRL
		
		#define PDI_TX_MASK            (1 << 3)
		#define PDI_TX_CTRL             PIN3CTRL

		#define PDI_XCK_MASK           (1 << 1)
		#define PDI_XCK_CTRL            PIN1CTRL
		
		#define AUX_LINE_PORT           PORTC
		#define AUX_LINE_MASK          (1 << 0)
		#define AUX_LINE_CTRL           PIN0CTRL


		#define DELAY_TIMER                TCC0
		#define DELAY_TIMER_OVF_vect       TCC0_OVF_vect
		#define SW_SPI_TIMER               TCC1
		#define SW_SPI_TIMER_CCA_vect      TCC1_CCA_vect
		#define SW_SPI_TIMER_OVF_vect      TCC1_OVF_vect
		#define SW_SPI_PIN_IRQ_vect        PORTC_INT0_vect
		#define VTARGET_ADC_CHANNEL        2 //Unimplemented
		#define VTARGET_REF_VOLTS          5 //Unimplemented
		#define VTARGET_SCALE_FACTOR       1 //Unimplemented
	#endif
	#define ENABLE_ISP_PROTOCOL
	#define ENABLE_XPROG_PROTOCOL



	#define NO_VTARGET_DETECT
//	#define XCK_RESCUE_CLOCK_ENABLE
//	#define INVERTED_ISP_MISO

//	#define LIBUSB_DRIVER_COMPAT
//	#define RESET_TOGGLES_LIBUSB_COMPAT
//	#define FIRMWARE_VERSION_MINOR     0x11

#endif
