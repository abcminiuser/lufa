/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Benjamin Riggs (https://github.com/riggs)

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

#ifndef _BOS_DESCRIPTORS_H
#define _BOS_DESCRIPTORS_H

	/** Type define for the device BOS descriptor structure. This must be defined in the
	 *  application code, as the BOS descriptor may contain multiple sub-descriptors which
	 *  vary between devices, and which describe the device's usage to the host.
	 */
	typedef struct
	{
		USB_Descriptor_BOS_Header_t                         BOS_Header;
		USB_Descriptor_DeviceCapability_Platform_t const    *CapabilityDescriptors[];
	} USB_Descriptor_BOS_t;

#endif //_BOS_DESCRIPTORS_H
