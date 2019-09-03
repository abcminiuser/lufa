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

#ifndef _MS_OS_20_WEBUSB_DEVICE_H
#define _MS_OS_20_WEBUSB_DEVICE_H

	/* Macros */
		#ifndef WORD_TO_BYTES_LE
		#define WORD_TO_BYTES_LE(n) n % 256, (n / 256) % 256
		#endif
		#ifndef LONG_TO_BYTES_LE
		#define LONG_TO_BYTES_LE(n) n % 256, (n / 256) % 256, (n / 65536) % 256, (n / 16777216) % 256
		#endif

		/* $ python -c "import uuid;print(', '.join(map(hex, uuid.UUID('D8DD60DF-4589-4CC7-9CD2-659D9E648A9F').bytes_le)))" */
		#define MS_OS_20_PLATFORM_UUID 0xdf, 0x60, 0xdd, 0xd8, 0x89, 0x45, 0xc7, 0x4c, 0x9c, 0xd2, 0x65, 0x9d, 0x9e, 0x64, 0x8a, 0x9f

		#define MS_OS_20_WINDOWS_VERSION_8_1 0x06030000 // Windows version (8.1)

        #ifndef MS_OS_20_ALTERNATE_ENUMERATION_CODE
		#define MS_OS_20_ALTERNATE_ENUMERATION_CODE 0 /**< Set to non-zero to enable Windows to allow device to return alternate USB descriptors. */
        #endif

		/** \brief Convenience macro to easily create device capability platform descriptors for the MS OS 2.0 platform.
		 *
		 *  \note This macro is designed to be wrapped in parentheses and included in a sequence to the \ref BOS_DESCRIPTOR macro.
		 *
		 * 	\param[in] VendorCode  Vendor Code that all control requests coming from Windows must use.
		 *
		 * 	\param[in] TotalLength  The length, in bytes, of the MS OS 2.0 descriptor set to be retrieved by Windows.
		 */
		#define MS_OS_20_PLATFORM_DESCRIPTOR(VendorCode, TotalLength) \
			/* Total size of this descriptor */ 28, \
			DTYPE_DeviceCapability,\
			DCTYPE_Platform, \
			/* Reserved */ 0, \
			MS_OS_20_PLATFORM_UUID, \
			LONG_TO_BYTES_LE(MS_OS_20_WINDOWS_VERSION_8_1), \
			WORD_TO_BYTES_LE(TotalLength), \
			VendorCode, \
			MS_OS_20_ALTERNATE_ENUMERATION_CODE

	/* MS OS 2.0 Descriptors Data Structures */
		enum MS_OS_20_wIndex_t
		{
			MS_OS_20_DESCRIPTOR_INDEX = 0x07, /**< Indicates the device should return MS OS 2.0 Descriptor Set. */
			MS_OS_20_SET_ALT_ENUMERATION = 0x08, /**< Indicates the device may "subsequently return alternate USB descriptors when Windows requests the information." */
		};

		enum MS_OS_20_Descriptor_Types
		{
			MS_OS_20_SET_HEADER_DESCRIPTOR = 0x00,
			MS_OS_20_SUBSET_HEADER_CONFIGURATION = 0x01,
			MS_OS_20_SUBSET_HEADER_FUNCTION = 0x02,
			MS_OS_20_FEATURE_COMPATBLE_ID = 0x03,
			MS_OS_20_FEATURE_REG_PROPERTY = 0x04,
//			MS_OS_20_FEATURE_MIN_RESUME_TIME = 0x05,
//			MS_OS_20_FEATURE_MODEL_ID = 0x06,
			MS_OS_20_FEATURE_CCGP_DEVICE =0x07,
		};

		/** \brief Microsoft OS 2.0 Descriptor Set Header (LUFA naming convention).
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			uint16_t Length; /**< The length, in bytes, of this header. Shall be set to 10. */
			uint16_t DescriptorType; /**< Shall be set to MS_OS_20_SET_HEADER_DESCRIPTOR */
			uint32_t WindowsVersion;
			uint16_t TotalLength; /**< The size of entire MS OS 2.0 descriptor set. The value shall match the value in the descriptor set information structure. */
		} ATTR_PACKED MS_OS_20_Descriptor_Set_Header_t;

		/** \brief Microsoft OS 2.0 configuration subset header.
		 *
		 */
		typedef struct
		{
			uint16_t Length; /**< The length, in bytes, of this subset header. Shall be set to 8. */
			uint16_t DescriptorType; /**< MS_OS_20_SUBSET_HEADER_CONFIGURATION */
			uint8_t ConfigurationValue; /**< The configuration value for the USB configuration to which this subset applies. */
			uint8_t Reserved; /**< Shall be set to 0. */
			uint16_t TotalLength; /**< The size of entire configuration subset including this header. */
		} ATTR_PACKED MS_OS_20_Configuration_Subset_Header;

		/** \brief Microsoft OS 2.0 Function subset header.
		 *
		 */
		typedef struct
		{
			uint16_t Length; /**< The length, in bytes, of this subset header. Shall be set to 8. */
			uint16_t DescriptorType; /**< MS_OS_20_SUBSET_HEADER_FUNCTION */
			uint8_t FirstInterface; /**< The interface number for the first interface of the function to which this subset applies. */
			uint8_t Reserved; /**< Shall be set to 0. */
			uint16_t SubsetLength; /**< The size of entire function subset including this header. */
		} ATTR_PACKED MS_OS_20_Function_Subset_Header;

		/** \brief Microsoft OS 2.0 Feature Descriptor for CompatibleID.
		 *
		 *  These values are used by Windows to locate the appropriate driver for the device.
		 *
		 *  For WebUSB in Chrome, the CompatibleID needs to be WINUSB, and the SubCompatibleID is null.
		 *
		 *  \note ID values must be 8 bytes long and contain only the ASCII values for uppercase letters, numbers, underscores, and the NULL character. No other characters are allowed, and the last byte in the ID must be the NULL 0x00.
		 */
		typedef struct
		{
			uint16_t Length; /**< The length, bytes, of the compatible ID descriptor including value descriptors. Shall be set to 20. */
			uint16_t DescriptorType; /**< MS_OS_20_FEATURE_COMPATIBLE_ID */
			uint8_t CompatibleID[8]; /**< Compatible ID ASCII String */
			uint8_t SubCompatibleID[8]; /**< Sub-compatible ID ASCII String */
		} ATTR_PACKED MS_OS_20_CompatibleID_Descriptor;

		/** \brief Property Data Type values for the Microsoft OS 2.0 Registry Property Descriptor.
		 *
		 */
		enum MS_OS_20_Property_Data_Types
		{
			MS_OS_20_REG_SZ = 1, /**< A NULL-terminated Unicode String */
			MS_OS_20_REG_EXPAND_SZ = 2, /**< A NULL-terminated Unicode String that includes environment variables */
			MS_OS_20_REG_BINARY = 3, /**< Free-form binary */
			MS_OS_20_REG_DWORD_LITTLE_ENDIAN = 4, /**< A little-endian 32-bit integer */
			MS_OS_20_REG_DWORD_BIG_ENDIAN = 5, /**< A big-endian 32-bit integer */
			MS_OS_20_REG_LINK = 6, /**< A NULL-terminated Unicode string that contains a symbolic link */
			MS_OS_20_REG_MULTI_SZ = 7 /**< Multiple NULL-terminated Unicode strings */
		};

		/** \brief Microsoft OS 2.0 Registry Property Descriptor.
		 *
		 *  This descriptor is used to add per-device or per-function registry values that is read by the Windows USB driver stack or the device’s function driver.
		 */
		typedef struct {
			uint16_t Length; /**< The length in bytes of is descriptor. */
			uint16_t DescriptorType; /**< MS_OS_20_FEATURE_REG_PROPERTY */
			uint16_t PropertyDataType; /**< MS_OS_20_Property_Data_types, MS_OS_20_REG_MULTI_SZ even for single interface because libusb. */
			uint16_t PropertyNameLength; /**< The length of the property name. */
			// FIXME: Only works for 16-bit architectures.
			wchar_t PropertyName[sizeof(MS_OS_20_REGISTRY_KEY) / sizeof(wchar_t)]; /**< The name of registry property as NULL-terminated UTF-16 LE string. */
			uint16_t PropertyDataLength; /**< The length of property data. */
			// FIXME: Only handles REG_SZ & REG_MULTI_SZ types (strings)
			wchar_t PropertyData[]; /**< Property Data. */
		} ATTR_PACKED MS_OS_20_Registry_Property_Descriptor;

        /** \brief Microsoft OS 2.0 Feature Descriptor for CCGP Devices.
         *
         *  This descriptor indicates that the device should be treated as a composite device by Windows regardless of
         *  the number of interfaces, configuration, or class, subclass, and protocol codes, the device reports.
         *
         *  \note The CCGP device descriptor must be applied to the entire device.
         */
        typedef struct
        {
            uint16_t Length; /**< The length, bytes, of the compatible ID descriptor including value descriptors. Shall be set to 4. */
            uint16_t DescriptorType; /**< MS_OS_20_FEATURE_CCGP_DEVICE */
        } ATTR_PACKED MS_OS_20_CCGP_Device_Descriptor;

#endif //_MS_OS_20_WEBUSB_DEVICE_H
