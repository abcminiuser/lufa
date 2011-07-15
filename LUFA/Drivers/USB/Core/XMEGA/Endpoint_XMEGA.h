/*
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

/** \file
 *  \brief USB Endpoint definitions for the AVR XMEGA microcontrollers.
 *  \copydetails Group_EndpointManagement_XMEGA
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB driver
 *        dispatch header located in LUFA/Drivers/USB/USB.h.
 */

/** \ingroup Group_EndpointRW
 *  \defgroup Group_EndpointRW_XMEGA Endpoint Data Reading and Writing (XMEGA)
 *  \brief Endpoint data read/write definitions for the Atmel AVR XMEGA architecture.
 *
 *  Functions, macros, variables, enums and types related to data reading and writing from and to endpoints.
 */

/** \ingroup Group_EndpointPrimitiveRW
 *  \defgroup Group_EndpointPrimitiveRW_XMEGA Read/Write of Primitive Data Types (XMEGA)
 *  \brief Endpoint primitive read/write definitions for the Atmel AVR XMEGA architecture.
 *
 *  Functions, macros, variables, enums and types related to data reading and writing of primitive data types
 *  from and to endpoints.
 */

/** \ingroup Group_EndpointPacketManagement
 *  \defgroup Group_EndpointPacketManagement_XMEGA Endpoint Packet Management (XMEGA)
 *  \brief Endpoint packet management definitions for the Atmel AVR XMEGA architecture.
 *
 *  Functions, macros, variables, enums and types related to packet management of endpoints.
 */

/** \ingroup Group_EndpointManagement
 *  \defgroup Group_EndpointManagement_XMEGA Endpoint Management (XMEGA)
 *  \brief Endpoint management definitions for the Atmel AVR XMEGA architecture.
 *
 *  Functions, macros and enums related to endpoint management when in USB Device mode. This
 *  module contains the endpoint management macros, as well as endpoint interrupt and data
 *  send/receive functions for various data types.
 *
 *  @{
 */

#ifndef __ENDPOINT_XMEGA_H__
#define __ENDPOINT_XMEGA_H__

	/* Includes: */
		#include "../../../../Common/Common.h"
		#include "../USBTask.h"
		#include "../USBInterrupt.h"

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB/USB.h instead.
		#endif

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Macros: */
			#define _ENDPOINT_GET_MAXSIZE(EPIndex)         1023
			#define _ENDPOINT_GET_BANKS(EPIndex)           2

			#define ENDPOINT_DETAILS_MAXEP                 16

		/* Inline Functions: */
			static inline uint8_t Endpoint_BytesToEPSizeMask(const uint16_t Bytes) ATTR_WARN_UNUSED_RESULT ATTR_CONST
			                                                                       ATTR_ALWAYS_INLINE;
			static inline uint8_t Endpoint_BytesToEPSizeMask(const uint16_t Bytes)
			{
				uint8_t  MaskVal    = 0;
				uint16_t CheckBytes = 8;

				while (CheckBytes < Bytes)
				{
					MaskVal++;
					CheckBytes <<= 1;
				}

				return (MaskVal << USB_EP_BUFSIZE_gp);
			}

		/* Function Prototypes: */
			void Endpoint_ClearEndpoints(void);
			bool Endpoint_ConfigureEndpoint_Prv(const uint8_t Number,
			                                    const uint8_t UECFG0XData,
			                                    const uint8_t UECFG1XData);

	#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** \name Endpoint Direction Masks */
			//@{
			/** Endpoint data direction mask for \ref Endpoint_ConfigureEndpoint(). This indicates that the endpoint
			 *  should be initialized in the OUT direction - i.e. data flows from host to device.
			 */
			#define ENDPOINT_DIR_OUT                        false

			/** Endpoint data direction mask for \ref Endpoint_ConfigureEndpoint(). This indicates that the endpoint
			 *  should be initialized in the IN direction - i.e. data flows from device to host.
			 */
			#define ENDPOINT_DIR_IN                         true
			//@}
			
			/** \name Endpoint Bank Mode Masks */
			//@{
			/** Mask for the bank mode selection for the \ref Endpoint_ConfigureEndpoint() macro. This indicates
			 *  that the endpoint should have one single bank, which requires less USB FIFO memory but results
			 *  in slower transfers as only one USB device (the AVR or the host) can access the endpoint's
			 *  bank at the one time.
			 */
			#define ENDPOINT_BANK_SINGLE                    0

			/** Mask for the bank mode selection for the \ref Endpoint_ConfigureEndpoint() macro. This indicates
			 *  that the endpoint should have two banks, which requires more USB FIFO memory but results
			 *  in faster transfers as one USB device (the AVR or the host) can access one bank while the other
			 *  accesses the second bank.
			 */
			#define ENDPOINT_BANK_DOUBLE                    USB_EP_BANK_bm
			//@}

			#if (!defined(FIXED_CONTROL_ENDPOINT_SIZE) || defined(__DOXYGEN__))
				/** Default size of the default control endpoint's bank, until altered by the control endpoint bank size
				 *  value in the device descriptor. Not available if the \c FIXED_CONTROL_ENDPOINT_SIZE token is defined.
				 */
				#define ENDPOINT_CONTROLEP_DEFAULT_SIZE     8
			#endif

			/** Retrieves the maximum bank size in bytes of a given endpoint.
			 *
			 *  \note This macro will only work correctly on endpoint indexes that are compile-time constants
			 *        defined by the preprocessor.
			 *
			 *  \param[in] EPIndex  Endpoint number, a value between 0 and (\ref ENDPOINT_TOTAL_ENDPOINTS - 1)
			 */
			#define ENDPOINT_MAX_SIZE(EPIndex)              _ENDPOINT_GET_MAXSIZE(EPIndex)

			/** Retrieves the total number of banks supported by the given endpoint.
			 *
			 *  \note This macro will only work correctly on endpoint indexes that are compile-time constants
			 *        defined by the preprocessor.
			 *
			 *  \param[in] EPIndex  Endpoint number, a value between 0 and (\ref ENDPOINT_TOTAL_ENDPOINTS - 1)
			 */
			#define ENDPOINT_BANKS_SUPPORTED(EPIndex)       _ENDPOINT_GET_BANKS(EPIndex)

			#if !defined(CONTROL_ONLY_DEVICE) || defined(__DOXYGEN__)
				/** Total number of endpoints (including the default control endpoint at address 0) which may
				 *  be used in the device. Different USB AVR models support different amounts of endpoints,
				 *  this value reflects the maximum number of endpoints for the currently selected AVR model.
				 */
				#define ENDPOINT_TOTAL_ENDPOINTS            ENDPOINT_DETAILS_MAXEP
			#else
				#define ENDPOINT_TOTAL_ENDPOINTS            1
			#endif

		/* Enums: */
			/** Enum for the possible error return codes of the \ref Endpoint_WaitUntilReady() function.
			 *
			 *  \ingroup Group_EndpointRW_XMEGA
			 */
			enum Endpoint_WaitUntilReady_ErrorCodes_t
			{
				ENDPOINT_READYWAIT_NoError                 = 0, /**< Endpoint is ready for next packet, no error. */
				ENDPOINT_READYWAIT_EndpointStalled         = 1, /**< The endpoint was stalled during the stream
				                                                 *   transfer by the host or device.
				                                                 */
				ENDPOINT_READYWAIT_DeviceDisconnected      = 2,	/**< Device was disconnected from the host while
				                                                 *   waiting for the endpoint to become ready.
				                                                 */
				ENDPOINT_READYWAIT_BusSuspended            = 3, /**< The USB bus has been suspended by the host and
				                                                 *   no USB endpoint traffic can occur until the bus
				                                                 *   has resumed.
				                                                 */
				ENDPOINT_READYWAIT_Timeout                 = 4, /**< The host failed to accept or send the next packet
				                                                 *   within the software timeout period set by the
				                                                 *   \ref USB_STREAM_TIMEOUT_MS macro.
				                                                 */
			};

		/* Inline Functions: */
			/** Configures the specified endpoint number with the given endpoint type, direction, bank size
			 *  and banking mode. Once configured, the endpoint may be read from or written to, depending
			 *  on its direction.
			 *
			 *  \param[in] Number     Endpoint number to configure. This must be more than 0 and less than
			 *                        \ref ENDPOINT_TOTAL_ENDPOINTS.
			 *
			 *  \param[in] Type       Type of endpoint to configure, a \c EP_TYPE_* mask. Not all endpoint types
			 *                        are available on Low Speed USB devices - refer to the USB 2.0 specification.
			 *
			 *  \param[in] Direction  Endpoint data direction, either \ref ENDPOINT_DIR_OUT or \ref ENDPOINT_DIR_IN.
			 *                        All endpoints (except Control type) are unidirectional - data may only be read
			 *                        from or written to the endpoint bank based on its direction, not both.
			 *
			 *  \param[in] Size       Size of the endpoint's bank, where packets are stored before they are transmitted
			 *                        to the USB host, or after they have been received from the USB host (depending on
			 *                        the endpoint's data direction). The bank size must indicate the maximum packet size
			 *                        that the endpoint can handle.
			 *
			 *  \param[in] Banks      Number of banks to use for the endpoint being configured, an \c ENDPOINT_BANK_* mask.
			 *                        More banks uses more USB DPRAM, but offers better performance. Isochronous type
			 *                        endpoints <b>must</b> have at least two banks.
			 *
			 *  \note When the \c ORDERED_EP_CONFIG compile time option is used, Endpoints <b>must</b> be configured in
			 *        ascending order, or bank corruption will occur.
			 *        \n\n
			 *
			 *  \note Different endpoints may have different maximum packet sizes based on the endpoint's index - refer to
			 *        the chosen microcontroller model's datasheet to determine the maximum bank size for each endpoint.
			 *        \n\n
			 *
			 *  \note The default control endpoint should not be manually configured by the user application, as
			 *        it is automatically configured by the library internally.
			 *        \n\n
			 *
			 *  \note This routine will automatically select the specified endpoint upon success. Upon failure, the endpoint
			 *        which failed to reconfigure correctly will be selected.
			 *
			 *  \return Boolean \c true if the configuration succeeded, \c false otherwise.
			 */
			static inline bool Endpoint_ConfigureEndpoint(const uint8_t Number,
			                                              const uint8_t Type,
			                                              const uint8_t Direction,
			                                              const uint16_t Size,
			                                              const uint8_t Banks) ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_ConfigureEndpoint(const uint8_t Number,
			                                              const uint8_t Type,
			                                              const uint8_t Direction,
			                                              const uint16_t Size,
			                                              const uint8_t Banks)
			{
				return 0; // TODO
			}

			/** Indicates the number of bytes currently stored in the current endpoint's selected bank.
			 *
			 *  \note The return width of this function may differ, depending on the maximum endpoint bank size
			 *        of the selected AVR model.
			 *
			 *  \ingroup Group_EndpointRW_XMEGA
			 *
			 *  \return Total number of bytes in the currently selected Endpoint's FIFO buffer.
			 */
			static inline uint16_t Endpoint_BytesInEndpoint(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint16_t Endpoint_BytesInEndpoint(void)
			{
				return 0; // TODO
			}

			/** Get the endpoint address of the currently selected endpoint. This is typically used to save
			 *  the currently selected endpoint number so that it can be restored after another endpoint has
			 *  been manipulated.
			 *
			 *  \return Index of the currently selected endpoint.
			 */
			static inline uint8_t Endpoint_GetCurrentEndpoint(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint8_t Endpoint_GetCurrentEndpoint(void)
			{
				return 0; // TODO
			}

			/** Selects the given endpoint number. If the address from the device descriptors is used, the
			 *  value should be masked with the \ref ENDPOINT_EPNUM_MASK constant to extract only the endpoint
			 *  number (and discarding the endpoint direction bit).
			 *
			 *  Any endpoint operations which do not require the endpoint number to be indicated will operate on
			 *  the currently selected endpoint.
			 *
			 *  \param[in] EndpointNumber Endpoint number to select.
			 */
			static inline void Endpoint_SelectEndpoint(const uint8_t EndpointNumber) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_SelectEndpoint(const uint8_t EndpointNumber)
			{
				// TODO
			}

			/** Resets the endpoint bank FIFO. This clears all the endpoint banks and resets the USB controller's
			 *  data In and Out pointers to the bank's contents.
			 *
			 *  \param[in] EndpointNumber Endpoint number whose FIFO buffers are to be reset.
			 */
			static inline void Endpoint_ResetEndpoint(const uint8_t EndpointNumber) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_ResetEndpoint(const uint8_t EndpointNumber)
			{
				// TODO
			}

			/** Enables the currently selected endpoint so that data can be sent and received through it to
			 *  and from a host.
			 *
			 *  \note Endpoints must first be configured properly via \ref Endpoint_ConfigureEndpoint().
			 */
			static inline void Endpoint_EnableEndpoint(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_EnableEndpoint(void)
			{
				// TODO
			}

			/** Disables the currently selected endpoint so that data cannot be sent and received through it
			 *  to and from a host.
			 */
			static inline void Endpoint_DisableEndpoint(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_DisableEndpoint(void)
			{
				// TODO
			}

			/** Determines if the currently selected endpoint is enabled, but not necessarily configured.
			 *
			 * \return Boolean \c true if the currently selected endpoint is enabled, \c false otherwise.
			 */
			static inline bool Endpoint_IsEnabled(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsEnabled(void)
			{
				return 0; // TODO
			}

			/** Retrieves the number of busy banks in the currently selected endpoint, which have been queued for
			 *  transmission via the \ref Endpoint_ClearIN() command, or are awaiting acknowledgement via the
			 *  \ref Endpoint_ClearOUT() command.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \return Total number of busy banks in the selected endpoint.
			 */
			static inline uint8_t Endpoint_GetBusyBanks(void) ATTR_ALWAYS_INLINE ATTR_WARN_UNUSED_RESULT;
			static inline uint8_t Endpoint_GetBusyBanks(void)
			{
				return 0; // TODO
			}

			/** Aborts all pending IN transactions on the currently selected endpoint, once the bank
			 *  has been queued for transmission to the host via \ref Endpoint_ClearIN(). This function
			 *  will terminate all queued transactions, resetting the endpoint banks ready for a new
			 *  packet.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 */
			static inline void Endpoint_AbortPendingIN(void)
			{
				// TODO
			}

			/** Determines if the currently selected endpoint may be read from (if data is waiting in the endpoint
			 *  bank and the endpoint is an OUT direction, or if the bank is not yet full if the endpoint is an IN
			 *  direction). This function will return false if an error has occurred in the endpoint, if the endpoint
			 *  is an OUT direction and no packet (or an empty packet) has been received, or if the endpoint is an IN
			 *  direction and the endpoint bank is full.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \return Boolean \c true if the currently selected endpoint may be read from or written to, depending
			 *          on its direction.
			 */
			static inline bool Endpoint_IsReadWriteAllowed(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsReadWriteAllowed(void)
			{
				return 0; // TODO
			}

			/** Determines if the currently selected endpoint is configured.
			 *
			 *  \return Boolean \c true if the currently selected endpoint has been configured, \c false otherwise.
			 */
			static inline bool Endpoint_IsConfigured(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsConfigured(void)
			{
				return 0; // TODO
			}

			/** Returns a mask indicating which INTERRUPT type endpoints have interrupted - i.e. their
			 *  interrupt duration has elapsed. Which endpoints have interrupted can be determined by
			 *  masking the return value against <tt>(1 << <i>{Endpoint Number}</i>)</tt>.
			 *
			 *  \return Mask whose bits indicate which endpoints have interrupted.
			 */
			static inline uint8_t Endpoint_GetEndpointInterrupts(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint8_t Endpoint_GetEndpointInterrupts(void)
			{
				return 0; // TODO
			}

			/** Determines if the specified endpoint number has interrupted (valid only for INTERRUPT type
			 *  endpoints).
			 *
			 *  \param[in] EndpointNumber  Index of the endpoint whose interrupt flag should be tested.
			 *
			 *  \return Boolean \c true if the specified endpoint has interrupted, \c false otherwise.
			 */
			static inline bool Endpoint_HasEndpointInterrupted(const uint8_t EndpointNumber) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_HasEndpointInterrupted(const uint8_t EndpointNumber)
			{
				return 0; // TODO
			}

			/** Determines if the selected IN endpoint is ready for a new packet to be sent to the host.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \return Boolean \c true if the current endpoint is ready for an IN packet, \c false otherwise.
			 */
			static inline bool Endpoint_IsINReady(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsINReady(void)
			{
				return 0; // TODO
			}

			/** Determines if the selected OUT endpoint has received new packet from the host.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \return Boolean \c true if current endpoint is has received an OUT packet, \c false otherwise.
			 */
			static inline bool Endpoint_IsOUTReceived(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsOUTReceived(void)
			{
				return 0; // TODO
			}

			/** Determines if the current CONTROL type endpoint has received a SETUP packet.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \return Boolean \c true if the selected endpoint has received a SETUP packet, \c false otherwise.
			 */
			static inline bool Endpoint_IsSETUPReceived(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsSETUPReceived(void)
			{
				return 0; // TODO
			}

			/** Clears a received SETUP packet on the currently selected CONTROL type endpoint, freeing up the
			 *  endpoint for the next packet.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \note This is not applicable for non CONTROL type endpoints.
			 */
			static inline void Endpoint_ClearSETUP(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_ClearSETUP(void)
			{
				// TODO
			}

			/** Sends an IN packet to the host on the currently selected endpoint, freeing up the endpoint for the
			 *  next packet and switching to the alternative endpoint bank if double banked.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 */
			static inline void Endpoint_ClearIN(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_ClearIN(void)
			{
				// TODO
			}

			/** Acknowledges an OUT packet to the host on the currently selected endpoint, freeing up the endpoint
			 *  for the next packet and switching to the alternative endpoint bank if double banked.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 */
			static inline void Endpoint_ClearOUT(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_ClearOUT(void)
			{
				// TODO
			}

			/** Stalls the current endpoint, indicating to the host that a logical problem occurred with the
			 *  indicated endpoint and that the current transfer sequence should be aborted. This provides a
			 *  way for devices to indicate invalid commands to the host so that the current transfer can be
			 *  aborted and the host can begin its own recovery sequence.
			 *
			 *  The currently selected endpoint remains stalled until either the \ref Endpoint_ClearStall() macro
			 *  is called, or the host issues a CLEAR FEATURE request to the device for the currently selected
			 *  endpoint.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 */
			static inline void Endpoint_StallTransaction(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_StallTransaction(void)
			{
				// TODO
			}

			/** Clears the STALL condition on the currently selected endpoint.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 */
			static inline void Endpoint_ClearStall(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_ClearStall(void)
			{
				// TODO
			}

			/** Determines if the currently selected endpoint is stalled, false otherwise.
			 *
			 *  \ingroup Group_EndpointPacketManagement_XMEGA
			 *
			 *  \return Boolean \c true if the currently selected endpoint is stalled, \c false otherwise.
			 */
			static inline bool Endpoint_IsStalled(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline bool Endpoint_IsStalled(void)
			{
				return 0; // TODO
			}

			/** Resets the data toggle of the currently selected endpoint. */
			static inline void Endpoint_ResetDataToggle(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_ResetDataToggle(void)
			{
				// TODO
			}

			/** Determines the currently selected endpoint's direction.
			 *
			 *  \return The currently selected endpoint's direction, as a \c ENDPOINT_DIR_* mask.
			 */
			static inline uint8_t Endpoint_GetEndpointDirection(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint8_t Endpoint_GetEndpointDirection(void)
			{
				return 0; // TODO
			}

			/** Sets the direction of the currently selected endpoint.
			 *
			 *  \param[in] DirectionMask  New endpoint direction, as a \c ENDPOINT_DIR_* mask.
			 */
			static inline void Endpoint_SetEndpointDirection(const uint8_t DirectionMask) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_SetEndpointDirection(const uint8_t DirectionMask)
			{
				// TODO
			}

			/** Reads one byte from the currently selected endpoint's bank, for OUT direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \return Next byte in the currently selected endpoint's FIFO buffer.
			 */
			static inline uint8_t Endpoint_Read_8(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint8_t Endpoint_Read_8(void)
			{
				return 0; // TODO
			}

			/** Writes one byte to the currently selected endpoint's bank, for IN direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \param[in] Data  Data to write into the the currently selected endpoint's FIFO buffer.
			 */
			static inline void Endpoint_Write_8(const uint8_t Data) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Write_8(const uint8_t Data)
			{
				// TODO
			}

			/** Discards one byte from the currently selected endpoint's bank, for OUT direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 */
			static inline void Endpoint_Discard_8(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Discard_8(void)
			{
				// TODO
			}

			/** Reads two bytes from the currently selected endpoint's bank in little endian format, for OUT
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \return Next two bytes in the currently selected endpoint's FIFO buffer.
			 */
			static inline uint16_t Endpoint_Read_16_LE(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint16_t Endpoint_Read_16_LE(void)
			{
				return 0; // TODO
			}

			/** Reads two bytes from the currently selected endpoint's bank in big endian format, for OUT
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \return Next two bytes in the currently selected endpoint's FIFO buffer.
			 */
			static inline uint16_t Endpoint_Read_16_BE(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint16_t Endpoint_Read_16_BE(void)
			{
				return 0; // TODO
			}

			/** Writes two bytes to the currently selected endpoint's bank in little endian format, for IN
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \param[in] Data  Data to write to the currently selected endpoint's FIFO buffer.
			 */
			static inline void Endpoint_Write_16_LE(const uint16_t Data) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Write_16_LE(const uint16_t Data)
			{
				// TODO
			}

			/** Writes two bytes to the currently selected endpoint's bank in big endian format, for IN
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \param[in] Data  Data to write to the currently selected endpoint's FIFO buffer.
			 */
			static inline void Endpoint_Write_16_BE(const uint16_t Data) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Write_16_BE(const uint16_t Data)
			{
				// TODO
			}

			/** Discards two bytes from the currently selected endpoint's bank, for OUT direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 */
			static inline void Endpoint_Discard_16(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Discard_16(void)
			{
				// TODO
			}

			/** Reads four bytes from the currently selected endpoint's bank in little endian format, for OUT
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \return Next four bytes in the currently selected endpoint's FIFO buffer.
			 */
			static inline uint32_t Endpoint_Read_32_LE(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint32_t Endpoint_Read_32_LE(void)
			{
				return 0; // TODO
			}

			/** Reads four bytes from the currently selected endpoint's bank in big endian format, for OUT
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \return Next four bytes in the currently selected endpoint's FIFO buffer.
			 */
			static inline uint32_t Endpoint_Read_32_BE(void) ATTR_WARN_UNUSED_RESULT ATTR_ALWAYS_INLINE;
			static inline uint32_t Endpoint_Read_32_BE(void)
			{
				return 0; // TODO
			}

			/** Writes four bytes to the currently selected endpoint's bank in little endian format, for IN
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \param[in] Data  Data to write to the currently selected endpoint's FIFO buffer.
			 */
			static inline void Endpoint_Write_32_LE(const uint32_t Data) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Write_32_LE(const uint32_t Data)
			{
				// TODO
			}

			/** Writes four bytes to the currently selected endpoint's bank in big endian format, for IN
			 *  direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 *
			 *  \param[in] Data  Data to write to the currently selected endpoint's FIFO buffer.
			 */
			static inline void Endpoint_Write_32_BE(const uint32_t Data) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Write_32_BE(const uint32_t Data)
			{
				// TODO
			}

			/** Discards four bytes from the currently selected endpoint's bank, for OUT direction endpoints.
			 *
			 *  \ingroup Group_EndpointPrimitiveRW_XMEGA
			 */
			static inline void Endpoint_Discard_32(void) ATTR_ALWAYS_INLINE;
			static inline void Endpoint_Discard_32(void)
			{
				// TODO
			}

		/* External Variables: */
			/** Global indicating the maximum packet size of the default control endpoint located at address
			 *  0 in the device. This value is set to the value indicated in the device descriptor in the user
			 *  project once the USB interface is initialized into device mode.
			 *
			 *  If space is an issue, it is possible to fix this to a static value by defining the control
			 *  endpoint size in the \c FIXED_CONTROL_ENDPOINT_SIZE token passed to the compiler in the makefile
			 *  via the -D switch. When a fixed control endpoint size is used, the size is no longer dynamically
			 *  read from the descriptors at runtime and instead fixed to the given value. When used, it is
			 *  important that the descriptor control endpoint size value matches the size given as the
			 *  \c FIXED_CONTROL_ENDPOINT_SIZE token - it is recommended that the \c FIXED_CONTROL_ENDPOINT_SIZE token
			 *  be used in the device descriptors to ensure this.
			 *
			 *  \note This variable should be treated as read-only in the user application, and never manually
			 *        changed in value.
			 */
			#if (!defined(FIXED_CONTROL_ENDPOINT_SIZE) || defined(__DOXYGEN__))
				extern uint8_t USB_Device_ControlEndpointSize;
			#else
				#define USB_Device_ControlEndpointSize FIXED_CONTROL_ENDPOINT_SIZE
			#endif

		/* Function Prototypes: */
			/** Completes the status stage of a control transfer on a CONTROL type endpoint automatically,
			 *  with respect to the data direction. This is a convenience function which can be used to
			 *  simplify user control request handling.
			 */
			void Endpoint_ClearStatusStage(void);

			/** Spin-loops until the currently selected non-control endpoint is ready for the next packet of data
			 *  to be read or written to it.
			 *
			 *  \note This routine should not be called on CONTROL type endpoints.
			 *
			 *  \ingroup Group_EndpointRW_XMEGA
			 *
			 *  \return A value from the \ref Endpoint_WaitUntilReady_ErrorCodes_t enum.
			 */
			uint8_t Endpoint_WaitUntilReady(void);

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

