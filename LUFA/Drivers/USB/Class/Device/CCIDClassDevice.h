/*
             LUFA Library
     Copyright (C) Dean Camera, 2018.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)
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
 *  \brief Device mode driver for the library USB CCID Class driver.
 *
 *  Device mode driver for the library USB CCID Class driver.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB module driver
 *        dispatch header located in LUFA/Drivers/USB.h.
 */

/** \ingroup Group_USBClassCCID
 *  \defgroup Group_USBClassCCIDDevice CCID Class Device Mode Driver
 *
 *  \section Sec_USBClassCCIDDevice_Dependencies Module Source Dependencies
 *  The following files must be built with any user project that uses this module:
 *    - LUFA/Drivers/USB/Class/Device/CCIDClassDevice.c <i>(Makefile source module name: LUFA_SRC_USBCLASS)</i>
 *
 *  \section Sec_USBClassCCIDDevice_ModDescription Module Description
 *  Device Mode USB Class driver framework interface, for the CCID USB Class driver.
 *
 *  @{
 */

#ifndef _CCID_CLASS_DEVICE_H_
#define _CCID_CLASS_DEVICE_H_

	/* Includes: */
		#include "../../USB.h"
		#include "../Common/CCIDClassCommon.h"

	/* Public Interface - May be used in end-application: */
		/* Type Defines: */
			/** \brief CCID Class Device Mode Configuration and State Structure.
			 *
			 *	Class state structure. An instance of this structure should be made for each CCID interface
			 *	within the user application, and passed to each of the CCID class driver functions as the
			 *	CCIDInterfaceInfo parameter. This stores each CCID interface's configuration and state information.
			 */
			typedef struct
			{
				struct
				{
					uint8_t InterfaceNumber; /**< Interface number of the CCID interface within the device. */
					uint8_t TotalSlots; /**< Total of slots no this device. */
					USB_Endpoint_Table_t DataINEndpoint; /**< Data IN endpoint configuration table. */
					USB_Endpoint_Table_t DataOUTEndpoint; /**< Data OUT endpoint configuration table. */
				} Config; /**< Config data for the USB class interface within the device. All elements in this section
				           *   <b>must</b> be set or the interface will fail to enumerate and operate correctly.
				           */
				struct
				{
					bool    Aborted; //< Set if host has started an abort process
					uint8_t AbortedSeq; //< Sequence number for the current abort process
				} State; /**< State data for the USB class interface within the device. All elements in this section
				          *   are reset to their defaults when the interface is enumerated.
				          */
			} USB_ClassInfo_CCID_Device_t;

	/* Function Prototypes: */
			/** Configures the endpoints of a given CCID interface, ready for use. This should be linked to the library
			 *	\ref EVENT_USB_Device_ConfigurationChanged() event so that the endpoints are configured when the configuration containing
			 *	the given CCID interface is selected.
			 *
			 *	\param[in,out] CCIDInterfaceInfo	Pointer to a structure containing a CCID Class configuration.
			 *
			 *	\return Boolean \c true if the endpoints were successfully configured, \c false otherwise.
			 */
			bool CCID_Device_ConfigureEndpoints(USB_ClassInfo_CCID_Device_t* const CCIDInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** Processes incoming control requests from the host, that are directed to the given CCID class interface. This should be
			 *	linked to the library \ref EVENT_USB_Device_ControlRequest() event.
			 *
			 *	\param[in,out] CCIDInterfaceInfo	Pointer to a structure containing a CCID Class configuration and state.
			 */
			void CCID_Device_ProcessControlRequest(USB_ClassInfo_CCID_Device_t* const CCIDInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** General management task for a given CCID class interface, required for the correct operation of the interface. This should
			 *	be called frequently in the main program loop, before the master USB management task \ref USB_USBTask().
			 *
			 *	\param[in,out] CCIDInterfaceInfo	Pointer to a structure containing a CCID Class configuration and state.
			 */
			void CCID_Device_USBTask(USB_ClassInfo_CCID_Device_t* const CCIDInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			uint8_t CALLBACK_CCID_IccPowerOn(uint8_t slot, uint8_t*  atr, uint8_t*  atrSize, uint8_t* error);

			uint8_t CALLBACK_CCID_IccPowerOff(uint8_t slot, uint8_t* error);

			uint8_t CALLBACK_CCID_GetSlotStatus(uint8_t slot, uint8_t* error);

			uint8_t CALLBACK_CCID_XfrBlock(uint8_t slot, uint8_t* error, uint8_t* receivedBuffer, uint8_t receivedBufferSize, uint8_t* sendBuffer, uint8_t* sentBufferSize);

			uint8_t CALLBACK_CCID_Abort(uint8_t slot, uint8_t seq, uint8_t* error);


#endif

/** @} */
