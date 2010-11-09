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

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

	/* Includes: */
		#include <LUFA/Drivers/USB/USB.h>

		#include <avr/pgmspace.h>

		#include "Sideshow.h"

	/* Macros: */
		#define SIDESHOW_IN_EPNUM          3
		#define SIDESHOW_OUT_EPNUM         4
		#define SIDESHOW_IO_EPSIZE         64

	/* Type Defines: */
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t Config;
			USB_Descriptor_Interface_t            SSHOW_Interface;
			USB_Descriptor_Endpoint_t             SSHOW_DataInEndpoint;
			USB_Descriptor_Endpoint_t             SSHOW_DataOutEndpoint;
		} USB_Descriptor_Configuration_t;

		typedef struct
		{
			USB_Descriptor_Header_t                Header;

			int                                    Signature[7];
			uint16_t                               VendorCode;
		} USB_OSDescriptor_t;

		typedef struct
		{
			uint8_t                                FirstInterfaceNumber;

			uint8_t                                Reserved;

			uint8_t                                CompatibleID[8];
			uint8_t                                SubCompatibleID[8];

			uint8_t                                Reserved2[6];
		} USB_OSCompatibleSection_t;

		typedef struct
		{
			uint32_t                               TotalLength;
			uint16_t                               Version;
			uint16_t                               Index;
			uint8_t                                TotalSections;

			uint8_t                                Reserved[7];

			USB_OSCompatibleSection_t              SideshowCompatID;
		} USB_OSCompatibleIDDescriptor_t;

	/* Function Prototypes: */
		uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
		                                    const uint8_t wIndex,
		                                    const void** const DescriptorAddress)
		                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

		uint16_t USB_GetOSFeatureDescriptor(const uint16_t wValue,
		                                    const uint8_t wIndex,
		                                    const void** const DescriptorAddress)
		                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif

