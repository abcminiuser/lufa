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
 *  Macros for embedded USB hosts with dual role On The Go capabilities, for managing role exchange. OTG
 *  is a way for two USB dual role devices to talk to one another directly without fixed device/host roles.
 *
 *  \note These macros are only for AVRs which support the OTG protocol, and do not exist for device-only AVRs.
 */

#ifndef __USBOTG_H__
#define __USBOTG_H__

	/* Includes: */
		#include "../../../Common/Common.h"
		
		#if (MCU_ARCHITECTURE == ARCH_AVR8)
			#include "AVR8/OTG.h"
		#endif
		
#endif
