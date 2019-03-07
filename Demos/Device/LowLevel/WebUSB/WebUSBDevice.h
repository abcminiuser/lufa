/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2019  Benjamin Riggs (https://github.com/riggs)

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

#ifndef _WEBUSB_DEVICE_H
#define _WEBUSB_DEVICE_H

	/* Macros */
		#ifndef WORD_TO_BYTES_LE
		#define WORD_TO_BYTES_LE(n) n % 256, (n / 256) % 256
		#endif

		#define WEBUSB_VERSION VERSION_BCD(1, 0, 0)

		/* $ python -c "import uuid;print(', '.join(map(hex, uuid.UUID('3408b638-09a9-47a0-8bfd-a0768815b665').bytes_le)))" */
		#define WEBUSB_PLATFORM_UUID 0x38, 0xb6, 0x8, 0x34, 0xa9, 0x9, 0xa0, 0x47, 0x8b, 0xfd, 0xa0, 0x76, 0x88, 0x15, 0xb6, 0x65

		/** \brief Convenience macro to easily create device capability platform descriptors for the WebUSB platform.
		 *
		 *  \note This macro is designed to be wrapped in parentheses and included in a sequence to the \ref BOS_DESCRIPTOR macro.
		 *
		 * 	\param[in] VendorCode  Vendor Code that all control requests coming from the browser must use.
		 *
		 * 	\param[in] LandingPageIndex  Index of the URL Descriptor to use as the Landing Page for the device.
		 *
		 */
		#define WEBUSB_PLATFORM_DESCRIPTOR(VendorCode, LandingPageIndex) \
			/* WebUSB Platform Descriptor size */ 24, \
			DTYPE_DeviceCapability, \
			DCTYPE_Platform, \
			/* Reserved */ 0, \
			WEBUSB_PLATFORM_UUID, \
			WORD_TO_BYTES_LE(WEBUSB_VERSION), \
			VendorCode, \
			LandingPageIndex

		/** \brief Convenience macro to easily create \ref WebUSB_URL_Descriptor_t instances from a wide character string.
		 *
		 *  \note This macro is for little-endian systems only.
		 *
		 * 	\param[in] Prefix  0 for "http://", 1 for "https://", 255 for included in the URL string.
		 *
		 *  \param[in] URL  URL string to initialize a URL Descriptor structure with.
		 *
		 * 	\note Prefix String literal with u8 to ensure proper conversion: e.g. WEBUSB_URL_DESCRIPTOR(u8"www.google.com")
		 */
		#define WEBUSB_URL_DESCRIPTOR(Prefix, URL)     { .Header = {.Size = sizeof(WebUSB_URL_Descriptor_t) + (sizeof(URL) - 1), \
																    .Type = WebUSB_DTYPE_URL}, \
														 .Scheme = (Prefix), \
														 .UTF8_URL = (URL) }

	/* WebUSB Protocol Data Structures */
		enum WebUSB_Request_t
		{
			WebUSB_RTYPE_GetURL = 2, /**< Indicates the device should return the indicated WebUSB_URL descriptor. */
		};

		enum WebUSB_Descriptor_t
		{
			WebUSB_DTYPE_URL = 3, /**< Indicates that the descriptor is a URL descriptor. */
		};

		/** \brief WebUSB URL Descriptor (LUFA naming convention).
		 *
		 *  Type define for a WebUSB URL Descriptor. This structure uses LUFA-specific element names
		 *  to make each element's purpose clearer.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t Header; /**< Descriptor header, including type (WebUSB_DTYPE_URL) and size. */

			uint8_t Scheme; /**< URL scheme prefix: 0 means http://, 1 means https://, 255 means included in URL */
			uint8_t UTF8_URL[]; /**< UTF-8 encoded URL (excluding scheme prefix). */
		} ATTR_PACKED WebUSB_URL_Descriptor_t;

#endif //_WEBUSB_DEVICE_H
