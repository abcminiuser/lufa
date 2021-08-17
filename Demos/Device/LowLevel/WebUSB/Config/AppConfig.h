/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

	#define GENERIC_REPORT_SIZE       8

	#define WEBUSB_VENDOR_CODE 0x42
	#define WEBUSB_LANDING_PAGE_INDEX 0

	#define MS_OS_20_VENDOR_CODE 0x41     // Must be different than WEBUSB_VENDOR_CODE
	#define MS_OS_20_DESCRIPTOR_SET_TOTAL_LENGTH 158
	// python -c "import uuid;print('u\"{' + str(uuid.uuid4()) + '}\"')"
	#define MS_OS_20_DEVICE_GUID_STRING u"{94e78d93-4cbb-481f-b542-a74740d3a713}"    // FIXME: Generate a unique GUID for your device.

#endif
