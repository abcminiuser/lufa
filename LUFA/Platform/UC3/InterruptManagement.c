/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#include "../../Common/Common.h"
#if (ARCH == ARCH_UC3)

#define  __INCLUDE_FROM_INTMANAGEMENT_C
#include "InterruptManagement.h"

#if defined(__GNUC__) || defined(__DOXYGEN__)
extern const void EVBA_Table;

/** Interrupt vector table, containing the ISR to call for each interrupt group */
InterruptHandlerPtr_t InterruptHandlers[AVR32_INTC_NUM_INT_GRPS];

/** Retrieves the associated interrupt handler for the interrupt group currently being fired. This
 *  is called directly from the exception handler routine before dispatching to the ISR.
 */
InterruptHandlerPtr_t INTC_GetInterruptHandler(const uint_reg_t InterruptLevel)
{
	return InterruptHandlers[AVR32_INTC.icr[AVR32_INTC_INT3 - InterruptLevel]];
}
#endif

/** ISR for unhandled interrupt groups */
#if defined(__GNUC__) || defined(__DOXYGEN__)
ISR(__unhandled_interrupt)
#elif defined(__ICCAVR32__)
__interrupt void __unhandled_interrupt(void)
#endif
{
	for (;;);
}

/** Initializes the interrupt controller ready to handle interrupts. This must be called at the
 *  start of the user program before any interrupts are registered or enabled.
 */
void INTC_Init(void)
{
	#if defined(__GNUC__)
	for (uint8_t InterruptGroup = 0; InterruptGroup < AVR32_INTC_NUM_INT_GRPS; InterruptGroup++)
	{
		InterruptHandlers[InterruptGroup] = __unhandled_interrupt;
		AVR32_INTC.ipr[InterruptGroup]    = Autovector_Table[AVR32_INTC_INT0];
	}

	WRITE_SYS_REGISTER(AVR32_EVBA, (uintptr_t)&EVBA_Table);
	#endif
}

#endif
