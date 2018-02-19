/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2018  Filipe Rodrigues (filipepazrodrigues [at] gmail [dot] com)

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
 *  \brief Common definitions and declarations for the library USB CCID Class driver.
 *
 *  Common definitions and declarations for the library USB CCID Class driver.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB module driver
 *        dispatch header located in LUFA/Drivers/USB.h.
 */

/** \ingroup Group_USBClassCCID
 *  \defgroup Group_USBClassCCIDCommon  Common Class Definitions
 *
 *  \section Sec_USBClassCCIDCommon_ModDescription Module Description
 *  Constants, Types and Enum definitions that are common to both Device and Host modes for the USB
 *  CCID Class.
 *
 *  @{
 */

#ifndef _CCID_CLASS_COMMON_H_
#define _CCID_CLASS_COMMON_H_


	/* Includes: */
		#include "../../Core/StdDescriptors.h"

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_CCID_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB.h instead.
		#endif

	/* Macros: */
		#define CCID_CURRENT_SPEC_RELEASE_NUMBER			0x0110
		#define CCID_VOLTAGESUPPORT_5V						0
		#define CCID_VOLTAGESUPPORT_3V						(1 << 0)
		#define CCID_VOLTAGESUPPORT_1V8						(1 << 1)

		#define CCID_PROTOCOL_T0							0
		#define CCID_PROTOCOL_T1							(1 << 0)

		#define CCID_ICCSTATUS_PRESENTANDACTIVE				0
		#define CCID_ICCSTATUS_PRESENTANDINACTIVE			(1 << 0)
		#define CCID_ICCSTATUS_NOICCPRESENT					(1 << 1)

		#define CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR	0
		#define CCID_COMMANDSTATUS_FAILED					(1 << 6)
		#define CCID_COMMANDSTATUS_TIMEEXTENSIONREQUESTED	(1 << 7)

		#define CCID_ERROR_NOERROR			0
		#define CCID_ERROR_SLOTNOTFOUND		5


	/* Enums: */
		/** Enum for possible Class, Subclass and Protocol values of device and interface descriptors relating to the CCID
		 *  device class.
		 */
		enum CCID_Descriptor_ClassSubclassProtocol_t
		{
			CCID_CSCP_CCIDClass             = 0x0b, /**< Descriptor Class value indicating that the device or interface
			                                         *   belongs to the CCID class.
			                                         */
			CCID_CSCP_NoSpecificSubclass     = 0x00, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to no specific subclass of the CCID class.
			                                         */
			CCID_CSCP_NoSpecificProtocol     = 0x00, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to no specific protocol of the CCID class.
			                                         */
		};

		/** Enum for possible bulk messages between PC and Reader */
		enum CCID_BulkOutMessages_t
		{
			CCID_PC_to_RDR_IccPowerOn  			= 0x62,
			CCID_PC_to_RDR_IccPowerOff 			= 0x63,
			CCID_PC_to_RDR_GetSlotStatus  		= 0x65,
			CCID_PC_to_RDR_XfrBlock			= 0x6f,
			CCID_PC_to_RDR_GetParameters			= 0x6c,
			CCID_PC_to_RDR_ResetParameters		= 0x6d,
			CCID_PC_to_RDR_SetParameters         	= 0x61,
			CCID_PC_to_RDR_Escape			= 0x6b,
			
			CCID_RDR_to_PC_DataBlock   			= 0x80,
			CCID_RDR_to_PC_SlotStatus  			= 0x81

		};

		/** Enum for the Mass Storage class specific control requests that can be issued by the USB bus host. */
		enum CCID_ClassRequests_t
		{
			CCID_ABORT                  	= 0x1,
			CCID_GET_CLOCK_FREQUENCIES      = 0x2,
			CCID_GET_DATA_RATES 			= 0x3
		};

	/* Type Defines: */
		typedef struct
		{
			USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */

			uint16_t                CCID;
			uint8_t					MaxSlotIndex;
			uint8_t					VoltageSupport;
			uint32_t				Protocols;
			uint32_t				DefaultClock;
			uint32_t				MaximumClock;
			uint8_t					NumClockSupported;
			uint32_t				DataRate;
			uint32_t				MaxDataRate;
			uint8_t					NumDataRatesSupported;
			uint32_t				MaxIFSD;
			uint32_t				SynchProtocols;
			uint32_t				Mechanical;
			uint32_t				Features;
			uint32_t				MaxCCIDMessageLength;
			uint8_t					ClassGetResponse;
			uint8_t					ClassEnvelope;
			uint16_t				LcdLayout;
			uint8_t					PINSupport;
			uint8_t					MaxCCIDBusySlots;

		} ATTR_PACKED USB_CCID_Descriptor_t;

		/** Enum for a common bulk message header. */
		typedef struct
		{
			uint8_t MessageType;
			uint32_t Length;
			uint8_t Slot;
			uint8_t Seq;

		} ATTR_PACKED USB_CCID_BulkMessage_Header_t;
		
		typedef struct
		{
			USB_CCID_BulkMessage_Header_t CCIDHeader;

		} ATTR_PACKED USB_CCID_PC_to_RDR_GetSlotStatus_t;

		typedef struct
		{
			USB_CCID_BulkMessage_Header_t CCIDHeader;
			uint8_t Status;
			uint8_t Error;
			uint8_t ChainParam;
			uint8_t Data[0];
		} ATTR_PACKED USB_CCID_RDR_to_PC_DataBlock_t;

		typedef struct
		{
			USB_CCID_BulkMessage_Header_t CCIDHeader;
			uint8_t Status;
			uint8_t Error;
			uint8_t ClockStatus;
		} ATTR_PACKED USB_CCID_RDR_to_PC_SlotStatus_t;

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

