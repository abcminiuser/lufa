/*
             LUFA Library
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
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
 *  Main low level USB driver. This module manages the low level initialization and shut down of the USB AVR's
 *  USB interface in either device or (if supported) host mode.
 */

#ifndef __USBLOWLEVEL_H__
#define __USBLOWLEVEL_H__

	/* Includes: */
		#include "../../../Common/Common.h"
		#include "../HighLevel/USBMode.h"
		#include "../HighLevel/Events.h"
		#include "../HighLevel/USBTask.h"
		#include "../HighLevel/USBInterrupt.h"
		
		#if defined(USB_CAN_BE_HOST) || defined(__DOXYGEN__)
			#include "Host.h"
			#include "Pipe.h"
			#include "OTG.h"
		#endif
		
		#if defined(USB_CAN_BE_DEVICE) || defined(__DOXYGEN__)
			#include "Device.h"
			#include "Endpoint.h"
			#include "DevChapter9.h"
		#endif
		
		#if (MCU_ARCHITECTURE == ARCH_AVR8)
			#include "AVR8/LowLevel.h"
		#elif (MCU_ARCHITECTURE == ARCH_AVR32)
			#include "AVR32/LowLevel.h"
		#endif
		
#endif
