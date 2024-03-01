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
 *  \brief Module Clock Driver for the AVR Dx USB microcontrollers.
 *
 *  Clock management driver for the AVR Dx USB microcontrollers. This driver allows for the configuration
 *  of the various clocks within the device to clock the various peripherals.
 */

/** \ingroup Group_PlatformDrivers_AVRDX
 *  \defgroup Group_PlatformDrivers_AVRDXClocks Clock Management Driver - LUFA/Platform/AVR_Dx/ClockManagement.h
 *  \brief Module Clock Driver for the AVR Dx USB microcontrollers.
 *
 *  \section Sec_PlatformDrivers_AVRDXClocks_Dependencies Module Source Dependencies
 *  The following files must be built with any user project that uses this module:
 *    - None
 *
 *  \section Sec_PlatformDrivers_AVRDXClocks_ModDescription Module Description
 *  Clock management driver for the AVR Dx USB microcontrollers. This driver allows for the configuration
 *  of the various clocks within the device to clock the various peripherals.
 *
 *  Usage Example:
 *  \code
 *   	#include <LUFA/Platform/AVR_Dx/ClockManagement.h>
 *
 *   	void main(void)
 *   	{
 *   		// Start the PLL to multiply the 2MHz RC oscillator to F_CPU and switch the CPU core to run from it
 *   		AVRDXCLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
 *   		AVRDXCLK_SetCPUClockSource(CLOCK_SRC_PLL);
 *
 *   		// Start the 32MHz internal RC oscillator and start the DFLL to increase it to F_USB using the USB SOF as a reference
 *   		AVRDXCLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
 *   		AVRDXCLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);
 *   	}
 *  \endcode
 *
 *  @{
 */

#ifndef _AVRDX_CLOCK_MANAGEMENT_H_
#define _AVRDX_CLOCK_MANAGEMENT_H_

	/* Includes: */
		#include "../../Common/Common.h"

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Enum for the possible external oscillator frequency ranges. */
			enum AVRDX_Extern_OSC_ClockFrequency_t
			{
				EXOSC_FREQ_8MHZ_MAX      = CLKCTRL_FRQRANGE_8M_gc,  /**< External crystal oscillator equal to or slower than 8MHz. */
				EXOSC_FREQ_16MHZ_MAX     = CLKCTRL_FRQRANGE_16M_gc, /**< External crystal oscillator equal to or slower than 16MHz. */
				EXOSC_FREQ_24MHZ_MAX     = CLKCTRL_FRQRANGE_24M_gc, /**< External crystal oscillator equal to or slower than 24MHz. */
				EXOSC_FREQ_32MHZ_MAX     = CLKCTRL_FRQRANGE_32M_gc, /**< External crystal oscillator equal to or slower than 32MHz. */
				EXCLK     				 = CLKCTRL_SELHF_bm,        /**< External clock on XTALHF1 pin. */
			};

			/** Enum for the possible high-frequency external oscillator startup times. */
			enum AVRDX_Extern_OSC_ClockStartup_t
			{
				EXOSC_START_256CLK       = CLKCTRL_CSUTHF_256_gc,  /**< Wait 256 clock cycles before startup. */
				EXOSC_START_1KCLK        = CLKCTRL_CSUTHF_1K_gc,  /**< Wait 1K clock cycles before startup. */
				EXOSC_START_4KCLK        = CLKCTRL_CSUTHF_4K_gc,  /**< Wait 4K clock cycles before startup. */
			};

			/** Enum for the possible module clock sources. */
			enum AVRDX_System_ClockSource_t
			{
				CLOCK_SRC_INT_OSCHF    = 0, /**< Clock sourced from the Internal High-Frequency Oscillator clock. */
				CLOCK_SRC_INT_32KHZ    = 1, /**< Clock sourced from the Internal 32KHz Oscillator clock. */
				CLOCK_SRC_EXT_CLK    = 2, 	/**< Clock sourced from an external clock. */
				CLOCK_SRC_EXT_OSC    = 3, 	/**< Clock sourced from an extental high-frequency oscillator. */
				CLOCK_SRC_EXT_32KHZ    = 4, /**< Clock sourced from an extental 32KHz oscillator. */
			};

			/** Enum for the possible OSCHF Autotune sources. */
			enum AVRDX_System_OSCHFAutotune_t
			{
				AUTOTUNE_DISABLED   = CLKCTRL_AUTOTUNE_OFF_gc, /**< OSCHF Autotune disabled. */
				AUTOTUNE_XOSC32K   	= CLKCTRL_AUTOTUNE_32K_gc, /**< OSCHF Autotune from the External 32KHz RC Oscillator clock . */
				AUTOTUNE_SOF_BIN    = (CLKCTRL_AUTOTUNE_SOF_gc | CLKCTRL_ALGSEL_BIN_gc), /**< OSCHF Autotune with binary search from the USB Start Of Frame packets. */
				AUTOTUNE_SOF_INCR   = (CLKCTRL_AUTOTUNE_SOF_gc | CLKCTRL_ALGSEL_INCR_gc), /**< OSCHF Autotune with incremental search from the USB Start Of Frame packets. */
			};

		/* Inline Functions: */
			/** Write a value to a location protected by the AVR Dx CCP protection mechanism. This function uses inline assembly to ensure that
			 *  the protected address is written to within four clock cycles of the CCP key being written.
			 *
			 *  \param[in] Address  Address to write to, a memory address protected by the CCP mechanism
			 *  \param[in] Value    Value to write to the protected location
			 */
			ATTR_ALWAYS_INLINE
			static inline void AVRDXCLK_CCP_Write(volatile void* Address, const uint8_t Value)
			{
				__asm__ __volatile__ (
					"out %0, __zero_reg__" "\n\t" /* Zero RAMPZ using fixed zero value register */
					"movw r30, %1"         "\n\t" /* Copy address to Z register pair */
					"out %2, %3"           "\n\t" /* Write key to CCP register */
					"st Z, %4"             "\n\t" /* Indirectly write value to address */
					: /* No output operands */
					: /* Input operands: */ "m" (RAMPZ), "e" (Address), "m" (CCP), "r" (CCP_IOREG_gc), "r" (Value)
					: /* Clobbered registers: */ "r30", "r31"
				);
			}

			/** Starts the external oscillator of the AVR Dx microcontroller, with the given options. This routine blocks until
			 *  the oscillator is ready for use.
			 *
			 *  \param[in] FreqRange  Frequency range of the external oscillator, a value from \ref AVRDX_Extern_OSC_ClockFrequency_t.
			 *  \param[in] Startup    Startup time of the external oscillator, a value from \ref AVRDX_Extern_OSC_ClockStartup_t.
			 *
			 *  \return Boolean \c true if the external oscillator was successfully started, \c false if invalid parameters specified.
			 */
			static inline bool AVRDXCLK_StartExternalOscillator(const uint8_t FreqRange,
			                                                    const uint8_t Startup) ATTR_ALWAYS_INLINE;
			static inline bool AVRDXCLK_StartExternalOscillator(const uint8_t FreqRange,
			                                                    const uint8_t Startup)
			{
				AVRDXCLK_CCP_Write(&CLKCTRL.XOSCHFCTRLA, FreqRange | Startup | CLKCTRL_ENABLE_bm);

				while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
				return true;
			}

			/** Stops the external oscillator of the AVR Dx microcontroller. */
			ATTR_ALWAYS_INLINE
			static inline void AVRDXCLK_StopExternalOscillator(void)
			{
				AVRDXCLK_CCP_Write(&CLKCTRL.XOSCHFCTRLA, 0);
			}

			/** Configures the OSCHF of the AVR Dx microcontroller, with the given options. This routine blocks until the OSCHF is ready for use.
			 *
			 *  \attention The output frequency must be equal to or greater than 12 MHz.
			 *
			 *  \param[in] Frequency    Target frequency of the OSCHF's output.
			 *  \param[in] Autotune     Autotune source for the OSCHF, a value from \ref AVRDX_System_OSCHFAutotune_t.
			 *
			 *  \return Boolean \c true if the OSCHF was successfully started, \c false if invalid parameters specified.
			 */
			static inline bool AVRDXCLK_ConfigureOSCHF(const uint32_t Frequency, const uint8_t Autotune) ATTR_ALWAYS_INLINE;
			static inline bool AVRDXCLK_ConfigureOSCHF(const uint32_t Frequency, const uint8_t Autotune)
			{
				uint8_t ClockConfigMask = Autotune;
				switch (Frequency)
				{
					case 12000000ul:
						ClockConfigMask |= CLKCTRL_FRQSEL_12M_gc;
						break;
					case 16000000ul:
						ClockConfigMask |= CLKCTRL_FRQSEL_16M_gc;
						break;
					case 20000000ul:
						ClockConfigMask |= CLKCTRL_FRQSEL_20M_gc;
						break;
					case 24000000ul:
						ClockConfigMask |= CLKCTRL_FRQSEL_24M_gc;
						break;
					default:
						return false;
				}

				AVRDXCLK_CCP_Write(&CLKCTRL.OSCHFCTRLA, ClockConfigMask);

				while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSCHFS_bm));
				return true;
			}


			/** Sets the clock source for the main microcontroller core. The given clock source should be configured
			 *  and ready for use before this function is called.
			 *
			 *  \param[in] Source      Clock source for the CPU core, a value from \ref AVRDX_System_ClockSource_t.
			 *
			 *  \return Boolean \c true if the CPU core clock was successfully altered, \c false if invalid parameters specified.
			 */
			ATTR_ALWAYS_INLINE
			static inline bool AVRDXCLK_SetCPUClockSource(const uint8_t Source)
			{
				uint8_t ClockSourceMask = 0;

				switch (Source)
				{
					case CLOCK_SRC_INT_OSCHF:
						ClockSourceMask = CLKCTRL_CLKSEL_OSCHF_gc;
						break;
					case CLOCK_SRC_INT_32KHZ:
						ClockSourceMask = CLKCTRL_CLKSEL_OSC32K_gc;
						break;
					case CLOCK_SRC_EXT_CLK:
					case CLOCK_SRC_EXT_OSC:
						ClockSourceMask = CLKCTRL_CLKSEL_EXTCLK_gc;
						break;
					case CLOCK_SRC_EXT_32KHZ:
						ClockSourceMask = CLKCTRL_CLKSEL_XOSC32K_gc;
						break;
					default:
						return false;
				}

				uint_reg_t CurrentGlobalInt = GetGlobalInterruptMask();
				GlobalInterruptDisable();

				AVRDXCLK_CCP_Write(&CLKCTRL.MCLKCTRLA, ClockSourceMask);

				SetGlobalInterruptMask(CurrentGlobalInt);

				while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm);
				return ((CLKCTRL.MCLKCTRLA & CLKCTRL_CLKSEL_gm) == ClockSourceMask);
			}

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

