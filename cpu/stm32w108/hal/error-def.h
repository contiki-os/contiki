/**
 * @file error-def.h
 * @brief Return-code definitions for API functions.
 * 
 * See @ref status_codes for documentation.
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
*/

/**
 * @addtogroup stm32w-cpu
 * @{ */

/**
 * @defgroup status_codes Status Codes
 *
 * Many StZNet API functions return an ::StStatus value to indicate
 * the success or failure of the call.
 * Return codes are one byte long.
 * This page documents the possible status codes and their meanings.
 *
 * See error-def.h for source code.
 *
 * See also error.h for information on how the values for the return codes are
 * built up from these definitions.
 * The file error-def.h is separated from error.h because utilities will use this file
 * to parse the return codes.
 *
 * @note Do not include error-def.h directly. It is included by
 * error.h inside an enum typedef, which is in turn included by st.h.
 *
 * @{
 */

/**
 * @name Generic Messages
 * These messages are system wide.
 *
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The generic "no error" message.
 */
#define ST_SUCCESS(0x00)
#else
DEFINE_ERROR(SUCCESS, 0)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The generic "fatal error" message.
 */
#define ST_ERR_FATAL(0x01)
#else
DEFINE_ERROR(ERR_FATAL, 0x01)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief An invalid value was passed as an argument to a function.
 */
#define ST_BAD_ARGUMENT(0x02)
#else
DEFINE_ERROR(BAD_ARGUMENT, 0x02)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The manufacturing and stack token format in non-volatile memory
 * is different than what the stack expects (returned at initialization).
 */
#define ST_EEPROM_MFG_STACK_VERSION_MISMATCH(0x04)
#else
DEFINE_ERROR(EEPROM_MFG_STACK_VERSION_MISMATCH, 0x04)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The static memory definitions in st-static-memory.h
 * are incompatible with this stack version.
 */
#define ST_INCOMPATIBLE_STATIC_MEMORY_DEFINITIONS(0x05)
#else
DEFINE_ERROR(INCOMPATIBLE_STATIC_MEMORY_DEFINITIONS, 0x05)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The manufacturing token format in non-volatile memory is
 * different than what the stack expects (returned at initialization).
 */
#define ST_EEPROM_MFG_VERSION_MISMATCH(0x06)
#else
DEFINE_ERROR(EEPROM_MFG_VERSION_MISMATCH, 0x06)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The stack token format in non-volatile memory is different 
 * than what the stack expects (returned at initialization).
 */
#define ST_EEPROM_STACK_VERSION_MISMATCH(0x07)
#else
DEFINE_ERROR(EEPROM_STACK_VERSION_MISMATCH, 0x07)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // END Generic Messages


/**
 * @name Packet Buffer Module Errors
 *
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief There are no more buffers.
 */
#define ST_NO_BUFFERS(0x18)
#else
DEFINE_ERROR(NO_BUFFERS, 0x18)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // END Packet Buffer Module Errors

/**
 * @name Serial Manager Errors
 *
 * @{ 
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Specified an invalid baud rate.
 */
#define ST_SERIAL_INVALID_BAUD_RATE(0x20)
#else
DEFINE_ERROR(SERIAL_INVALID_BAUD_RATE, 0x20)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Specified an invalid serial port.
 */
#define ST_SERIAL_INVALID_PORT(0x21)
#else
DEFINE_ERROR(SERIAL_INVALID_PORT, 0x21)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Tried to send too much data.
 */
#define ST_SERIAL_TX_OVERFLOW(0x22)
#else
DEFINE_ERROR(SERIAL_TX_OVERFLOW, 0x22)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief There was not enough space to store a received character
 * and the character was dropped.
 */
#define ST_SERIAL_RX_OVERFLOW(0x23)
#else
DEFINE_ERROR(SERIAL_RX_OVERFLOW, 0x23)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Detected a UART framing error.
 */
#define ST_SERIAL_RX_FRAME_ERROR(0x24)
#else
DEFINE_ERROR(SERIAL_RX_FRAME_ERROR, 0x24)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Detected a UART parity error.
 */
#define ST_SERIAL_RX_PARITY_ERROR(0x25)
#else
DEFINE_ERROR(SERIAL_RX_PARITY_ERROR, 0x25)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief There is no received data to process.
 */
#define ST_SERIAL_RX_EMPTY(0x26)
#else
DEFINE_ERROR(SERIAL_RX_EMPTY, 0x26)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The receive interrupt was not handled in time, and a 
 * character was dropped.
 */
#define ST_SERIAL_RX_OVERRUN_ERROR(0x27)
#else
DEFINE_ERROR(SERIAL_RX_OVERRUN_ERROR, 0x27)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */

/**
 * @name MAC Errors
 *  
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The MAC transmit queue is full.
 */
#define ST_MAC_TRANSMIT_QUEUE_FULL(0x39)
#else
// Internal
DEFINE_ERROR(MAC_TRANSMIT_QUEUE_FULL, 0x39)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief MAC header FCF error on receive.
 */
#define ST_MAC_UNKNOWN_HEADER_TYPE(0x3A)
#else
DEFINE_ERROR(MAC_UNKNOWN_HEADER_TYPE, 0x3A)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief MAC ACK header received.
 */
#define ST_MAC_ACK_HEADER_TYPE(0x3B)
#else
DEFINE_ERROR(MAC_ACK_HEADER_TYPE,  0x3B)
#endif //DOXYGEN_SHOULD_SKIP_THIS



#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The MAC can't complete this task because it is scanning.
 */
#define ST_MAC_SCANNING(0x3D)
#else
DEFINE_ERROR(MAC_SCANNING, 0x3D)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief No pending data exists for device doing a data poll.
 */
#define ST_MAC_NO_DATA(0x31)
#else
DEFINE_ERROR(MAC_NO_DATA, 0x31)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Attempt to scan when we are joined to a network.
 */
#define ST_MAC_JOINED_NETWORK(0x32)
#else
DEFINE_ERROR(MAC_JOINED_NETWORK, 0x32)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Scan duration must be 0 to 14 inclusive.  Attempt was made to
 * scan with an incorrect duration value.
 */
#define ST_MAC_BAD_SCAN_DURATION(0x33)
#else
DEFINE_ERROR(MAC_BAD_SCAN_DURATION, 0x33)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief stStartScan was called with an incorrect scan type.
 */
#define ST_MAC_INCORRECT_SCAN_TYPE(0x34)
#else
DEFINE_ERROR(MAC_INCORRECT_SCAN_TYPE, 0x34)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief stStartScan was called with an invalid channel mask.
 */
#define ST_MAC_INVALID_CHANNEL_MASK(0x35)
#else
DEFINE_ERROR(MAC_INVALID_CHANNEL_MASK, 0x35)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Failed to scan current channel because we were unable to
 * transmit the relevent MAC command.
 */
#define ST_MAC_COMMAND_TRANSMIT_FAILURE(0x36)
#else
DEFINE_ERROR(MAC_COMMAND_TRANSMIT_FAILURE, 0x36)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief We expected to receive an ACK following the transmission, but
 * the MAC level ACK was never received.
 */
#define ST_MAC_NO_ACK_RECEIVED(0x40)
#else
DEFINE_ERROR(MAC_NO_ACK_RECEIVED, 0x40)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief Indirect data message timed out before polled.
 */
#define ST_MAC_INDIRECT_TIMEOUT(0x42)
#else
DEFINE_ERROR(MAC_INDIRECT_TIMEOUT, 0x42)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */


/**
 * @name  Simulated EEPROM Errors
 *
 * @{
 */


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief The Simulated EEPROM is telling the application that there
 * is at least one flash page to be erased.  The GREEN status means the
 * current page has not filled above the ERASE_CRITICAL_THRESHOLD.
 * 
 * The application should call the function halSimEepromErasePage() when it can
 * to erase a page.
 */
#define ST_SIM_EEPROM_ERASE_PAGE_GREEN(0x43)
#else
DEFINE_ERROR(SIM_EEPROM_ERASE_PAGE_GREEN, 0x43)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief The Simulated EEPROM is telling the application that there
 * is at least one flash page to be erased.  The RED status means the
 * current page has filled above the ERASE_CRITICAL_THRESHOLD.
 * 
 * Due to the shrinking availablity of write space, there is a danger of
 * data loss.  The application must call the function halSimEepromErasePage()
 * as soon as possible to erase a page.
 */
#define ST_SIM_EEPROM_ERASE_PAGE_RED(0x44)
#else
DEFINE_ERROR(SIM_EEPROM_ERASE_PAGE_RED, 0x44)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief The Simulated EEPROM has run out of room to write any new data
 * and the data trying to be set has been lost.  This error code is the
 * result of ignoring the ::ST_SIM_EEPROM_ERASE_PAGE_RED error code.
 *
 * The application must call the function halSimEepromErasePage() to make room for
 * any further calls to set a token.
 */
#define ST_SIM_EEPROM_FULL(0x45)
#else
DEFINE_ERROR(SIM_EEPROM_FULL, 0x45)
#endif //DOXYGEN_SHOULD_SKIP_THIS


//  Errors 46 and 47 are now defined below in the 
//    flash error block (was attempting to prevent renumbering)


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief Attempt 1 to initialize the Simulated EEPROM has failed.
 *
 * This failure means the information already stored in Flash (or a lack
 * thereof), is fatally incompatible with the token information compiled
 * into the code image being run.
 */
#define ST_SIM_EEPROM_INIT_1_FAILED(0x48)
#else
DEFINE_ERROR(SIM_EEPROM_INIT_1_FAILED, 0x48)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief Attempt 2 to initialize the Simulated EEPROM has failed.
 *
 * This failure means Attempt 1 failed, and the token system failed to
 * properly reload default tokens and reset the Simulated EEPROM.
 */
#define ST_SIM_EEPROM_INIT_2_FAILED(0x49)
#else
DEFINE_ERROR(SIM_EEPROM_INIT_2_FAILED, 0x49)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief Attempt 3 to initialize the Simulated EEPROM has failed.
 *
 * This failure means one or both of the tokens TOKEN_MFG_NVDATA_VERSION or
 * TOKEN_STACK_NVDATA_VERSION were incorrect and the token system failed to
 * properly reload default tokens and reset the Simulated EEPROM.
 */
#define ST_SIM_EEPROM_INIT_3_FAILED(0x4A)
#else
DEFINE_ERROR(SIM_EEPROM_INIT_3_FAILED, 0x4A)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */


/**
 * @name  Flash Errors
 *
 * @{ */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief A fatal error has occured while trying to write data to the
 * Flash.  The target memory attempting to be programmed is already programmed.
 * The flash write routines were asked to flip a bit from a 0 to 1, which is
 * physically impossible and the write was therefore inhibited.  The data in
 * the flash cannot be trusted after this error.
 */
#define ST_ERR_FLASH_WRITE_INHIBITED(0x46)
#else
DEFINE_ERROR(ERR_FLASH_WRITE_INHIBITED, 0x46)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief A fatal error has occured while trying to write data to the
 * Flash and the write verification has failed.  The data in the flash
 * cannot be trusted after this error, and it is possible this error is the
 * result of exceeding the life cycles of the flash.
 */
#define ST_ERR_FLASH_VERIFY_FAILED(0x47)
#else
DEFINE_ERROR(ERR_FLASH_VERIFY_FAILED, 0x47)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief A fatal error has occured while trying to write data to the
 * flash, possibly due to write protection or an invalid address.  The data in
 * the flash cannot be trusted after this error, and it is possible this error
 * is the result of exceeding the life cycles of the flash.
 */
#define ST_ERR_FLASH_PROG_FAIL(0x4B)
#else
DEFINE_ERROR(ERR_FLASH_PROG_FAIL, 0x4B)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief A fatal error has occured while trying to erase flash, possibly
 * due to write protection.  The data in the flash cannot be trusted after
 * this error, and it is possible this error is the result of exceeding the
 * life cycles of the flash.
 */
#define ST_ERR_FLASH_ERASE_FAIL(0x4C)
#else
DEFINE_ERROR(ERR_FLASH_ERASE_FAIL, 0x4C)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */


/**
 * @name  Bootloader Errors
 *
 * @{
 */


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The bootloader received an invalid message (failed attempt 
 * to go into bootloader).
 */
#define ST_ERR_BOOTLOADER_TRAP_TABLE_BAD(0x58)
#else
DEFINE_ERROR(ERR_BOOTLOADER_TRAP_TABLE_BAD, 0x58)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Bootloader received an invalid message (failed attempt to go 
 * into bootloader).
 */
#define ST_ERR_BOOTLOADER_TRAP_UNKNOWN(0x59)
#else
DEFINE_ERROR(ERR_BOOTLOADER_TRAP_UNKNOWN, 0x59)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The bootloader cannot complete the bootload operation because
 * either an image was not found or the image exceeded memory bounds.
 */
#define ST_ERR_BOOTLOADER_NO_IMAGE(0x05A)
#else
DEFINE_ERROR(ERR_BOOTLOADER_NO_IMAGE, 0x5A)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */


/**
 * @name  Transport Errors
 *
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The APS layer attempted to send or deliver a message, but
 * it failed.
 */
#define ST_DELIVERY_FAILED(0x66)
#else
DEFINE_ERROR(DELIVERY_FAILED, 0x66)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief This binding index is out of range for the current binding table.
 */
#define ST_BINDING_INDEX_OUT_OF_RANGE(0x69)
#else
DEFINE_ERROR(BINDING_INDEX_OUT_OF_RANGE, 0x69)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief This address table index is out of range for the current 
 * address table.
 */
#define ST_ADDRESS_TABLE_INDEX_OUT_OF_RANGE(0x6A)
#else
DEFINE_ERROR(ADDRESS_TABLE_INDEX_OUT_OF_RANGE, 0x6A)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief An invalid binding table index was given to a function.
 */
#define ST_INVALID_BINDING_INDEX(0x6C)
#else
DEFINE_ERROR(INVALID_BINDING_INDEX, 0x6C)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The API call is not allowed given the current state of the 
 * stack.
 */
#define ST_INVALID_CALL(0x70)
#else
DEFINE_ERROR(INVALID_CALL, 0x70)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The link cost to a node is not known.
 */
#define ST_COST_NOT_KNOWN(0x71)
#else
DEFINE_ERROR(COST_NOT_KNOWN, 0x71)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The maximum number of in-flight messages (i.e. 
 * ST_APS_UNICAST_MESSAGE_COUNT)  has been reached.
 */
#define ST_MAX_MESSAGE_LIMIT_REACHED(0x72)
#else
DEFINE_ERROR(MAX_MESSAGE_LIMIT_REACHED, 0x72)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The message to be transmitted is too big to fit into a
 * single over-the-air packet.
 */
#define ST_MESSAGE_TOO_LONG(0x74)
#else
DEFINE_ERROR(MESSAGE_TOO_LONG, 0x74)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The application is trying to delete or overwrite a binding
 * that is in use.
 */
#define ST_BINDING_IS_ACTIVE(0x75)
#else
DEFINE_ERROR(BINDING_IS_ACTIVE, 0x75)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The application is trying to overwrite an address table entry
 * that is in use.
 */
#define ST_ADDRESS_TABLE_ENTRY_IS_ACTIVE(0x76)
#else
DEFINE_ERROR(ADDRESS_TABLE_ENTRY_IS_ACTIVE, 0x76)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */

/**
 * @name  HAL Module Errors
 *
 * @{
 */
  

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Conversion is complete.
 */
#define ST_ADC_CONVERSION_DONE(0x80)
#else
DEFINE_ERROR(ADC_CONVERSION_DONE, 0x80)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Conversion cannot be done because a request is being 
 * processed.
 */
#define ST_ADC_CONVERSION_BUSY(0x81)
#else
DEFINE_ERROR(ADC_CONVERSION_BUSY, 0x81)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Conversion is deferred until the current request has been 
 * processed.
 */
#define ST_ADC_CONVERSION_DEFERRED(0x82)
#else
DEFINE_ERROR(ADC_CONVERSION_DEFERRED, 0x82)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief No results are pending.
 */
#define ST_ADC_NO_CONVERSION_PENDING(0x84)
#else
DEFINE_ERROR(ADC_NO_CONVERSION_PENDING, 0x84)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Sleeping (for a duration) has been abnormally interrupted
 * and exited prematurely.
 */
#define ST_SLEEP_INTERRUPTED(0x85)
#else
DEFINE_ERROR(SLEEP_INTERRUPTED, 0x85)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */

/**
 * @name  PHY Errors
 *
 * @{
 */


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The transmit hardware buffer underflowed.
 */
#define ST_PHY_TX_UNDERFLOW(0x88)
#else
DEFINE_ERROR(PHY_TX_UNDERFLOW, 0x88)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The transmit hardware did not finish transmitting a packet.
 */
#define ST_PHY_TX_INCOMPLETE(0x89)
#else
DEFINE_ERROR(PHY_TX_INCOMPLETE, 0x89)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief An unsupported channel setting was specified.
 */
#define ST_PHY_INVALID_CHANNEL(0x8A)
#else
DEFINE_ERROR(PHY_INVALID_CHANNEL, 0x8A)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief An unsupported power setting was specified.
 */
#define ST_PHY_INVALID_POWER(0x8B)
#else
DEFINE_ERROR(PHY_INVALID_POWER, 0x8B)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The requested operation cannot be completed because the radio
 * is currently busy, either transmitting a packet or performing calibration.
 */
#define ST_PHY_TX_BUSY(0x8C)
#else
DEFINE_ERROR(PHY_TX_BUSY, 0x8C)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The transmit attempt failed because all CCA attempts indicated that
 * the channel was busy.
 */
#define ST_PHY_TX_CCA_FAIL(0x8D)
#else
DEFINE_ERROR(PHY_TX_CCA_FAIL, 0x8D)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The software installed on the hardware doesn't recognize the
 * hardware radio type.
 */
#define ST_PHY_OSCILLATOR_CHECK_FAILED(0x8E)
#else
DEFINE_ERROR(PHY_OSCILLATOR_CHECK_FAILED, 0x8E)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The expected ACK was received after the last transmission.
 */
#define ST_PHY_ACK_RECEIVED(0x8F)
#else
DEFINE_ERROR(PHY_ACK_RECEIVED, 0x8F)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */

/**
 * @name  Return Codes Passed to stStackStatusHandler()
 * See also stStackStatusHandler().
 *
 * @{
 */


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The stack software has completed initialization and is ready
 * to send and receive packets over the air.
 */
#define ST_NETWORK_UP(0x90)
#else
DEFINE_ERROR(NETWORK_UP, 0x90)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The network is not operating.
 */
#define ST_NETWORK_DOWN(0x91)
#else
DEFINE_ERROR(NETWORK_DOWN, 0x91)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief An attempt to join a network failed.
 */
#define ST_JOIN_FAILED(0x94)
#else
DEFINE_ERROR(JOIN_FAILED, 0x94)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief After moving, a mobile node's attempt to re-establish contact
  * with the network failed.
 */
#define ST_MOVE_FAILED(0x96)
#else
DEFINE_ERROR(MOVE_FAILED, 0x96)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief An attempt to join as a router failed due to a Zigbee
 * versus Zigbee Pro incompatibility.  Zigbee devices joining Zigbee Pro networks
 * (or vica versa) must join as End Devices, not Routers.
 */
#define ST_CANNOT_JOIN_AS_ROUTER(0x98)
#else
DEFINE_ERROR(CANNOT_JOIN_AS_ROUTER, 0x98)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief The local node ID has changed. The application can obtain the new
 *  node ID by calling stGetNodeId().
 */
#define ST_NODE_ID_CHANGED(0x99)
#else
DEFINE_ERROR(NODE_ID_CHANGED, 0x99)
#endif


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief The local PAN ID has changed. The application can obtain the new PAN
 *  ID by calling stGetPanId().
 */
#define ST_PAN_ID_CHANGED(0x9A)
#else
DEFINE_ERROR(PAN_ID_CHANGED, 0x9A)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief The channel has changed.
 */
#define ST_CHANNEL_CHANGED(0x9B)
#else
DEFINE_ERROR(CHANNEL_CHANGED, 0x9B)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief An attempt to join or rejoin the network failed because 
 *  no router beacons could be heard by the joining node.
 */
#define ST_NO_BEACONS(0xAB)
#else 
DEFINE_ERROR(NO_BEACONS, 0xAB)
#endif


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief An attempt was made to join a Secured Network using a pre-configured
 *  key, but the Trust Center sent back a Network Key in-the-clear when
 *  an encrypted Network Key was required. (ST_REQUIRE_ENCRYPTED_KEY).
 */
#define ST_RECEIVED_KEY_IN_THE_CLEAR(0xAC)
#else
DEFINE_ERROR(RECEIVED_KEY_IN_THE_CLEAR, 0xAC)
#endif


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief An attempt was made to join a Secured Network, but the device did
 *  not receive a Network Key.
 */
#define ST_NO_NETWORK_KEY_RECEIVED(0xAD)
#else
DEFINE_ERROR(NO_NETWORK_KEY_RECEIVED, 0xAD)
#endif


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief After a device joined a Secured Network, a Link Key was requested 
 *  (ST_GET_LINK_KEY_WHEN_JOINING) but no response was ever received.
 */
#define ST_NO_LINK_KEY_RECEIVED(0xAE)
#else
DEFINE_ERROR(NO_LINK_KEY_RECEIVED, 0xAE)
#endif


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief An attempt was made to join a Secured Network without a
 *  pre-configured key, but the Trust Center sent encrypted data using a
 *  pre-configured key.
 */
#define ST_PRECONFIGURED_KEY_REQUIRED(0xAF)
#else
DEFINE_ERROR(PRECONFIGURED_KEY_REQUIRED, 0xAF)
#endif


/** @} */

/**
 * @name  Security Errors
 *
 * @{
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief The passed key data is not valid.  A key of all zeros or
 *   all F's are reserved values and cannot be used.
 */
#define ST_KEY_INVALID(0xB2)
#else
DEFINE_ERROR(KEY_INVALID, 0xB2)
#endif // DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The chosen security level (the value of ST_SECURITY_LEVEL)
 *  is not supported by the stack.
 */
#define ST_INVALID_SECURITY_LEVEL(0x95)
#else
DEFINE_ERROR(INVALID_SECURITY_LEVEL, 0x95)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief There was an error in trying to encrypt at the APS Level.
 *
 * This could result from either an inability to determine the long address
 * of the recipient from the short address (no entry in the binding table)
 * or there is no link key entry in the table associated with the destination,
 * or there was a failure to load the correct key into the encryption core.
 */
#define ST_APS_ENCRYPTION_ERROR(0xA6)
#else
     DEFINE_ERROR(APS_ENCRYPTION_ERROR, 0xA6)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief There was an attempt to form a network using High
 * security without setting the Trust Center master key first.
 */
#define ST_TRUST_CENTER_MASTER_KEY_NOT_SET(0xA7)
#else
     DEFINE_ERROR(TRUST_CENTER_MASTER_KEY_NOT_SET, 0xA7)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief There was an attempt to form or join a network with security
 * without calling stSetInitialSecurityState() first.
 */
#define ST_SECURITY_STATE_NOT_SET(0xA8)
#else
     DEFINE_ERROR(SECURITY_STATE_NOT_SET, 0xA8)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief There was an attempt to set an entry in the key table using
 *   an invalid long address.  An entry cannot be set using either
 *   the local device's or Trust Center's IEEE address.  Or an entry
 *   already exists in the table with the same IEEE address.
 *   An Address of all zeros or all F's are not valid
 *   addresses in 802.15.4.
 */
#define ST_KEY_TABLE_INVALID_ADDRESS(0xB3)
#else
DEFINE_ERROR(KEY_TABLE_INVALID_ADDRESS, 0xB3)
#endif //DOYXGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief There was an attempt to set a security configuration that
 *   is not valid given the other security settings.
 */
#define ST_SECURITY_CONFIGURATION_INVALID(0xB7)
#else
DEFINE_ERROR(SECURITY_CONFIGURATION_INVALID, 0xB7)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief There was an attempt to broadcast a key switch too quickly after
 *    broadcasting the next network key.  The Trust Center must wait at
 *    least a period equal to the broadcast timeout so that all routers
 *    have a chance to receive the broadcast of the new network key.
 */
#define ST_TOO_SOON_FOR_SWITCH_KEY(0xB8)
#else
     DEFINE_ERROR(TOO_SOON_FOR_SWITCH_KEY, 0xB8)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief The received signature corresponding to the message that was passed
    to the CBKE Library failed verification, it is not valid.
*/
#define ST_SIGNATURE_VERIFY_FAILURE(0xB9)
#else
     DEFINE_ERROR(SIGNATURE_VERIFY_FAILURE, 0xB9)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief The message could not be sent because the link key corresponding
    to the destination is not authorized for use in APS data messages.  
    APS Commands (sent by the stack) are allowed.   To use it for encryption
    of APS data messages it must be authorized using a key agreement protocol
    (such as CBKE). 
*/
#define ST_KEY_NOT_AUTHORIZED(0xBB)
#else
     DEFINE_ERROR(KEY_NOT_AUTHORIZED, 0xBB)
#endif


/** @} */


/**
 * @name  Miscellaneous Network Errors
 *
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The node has not joined a network.
 */
#define ST_NOT_JOINED(0x93)
#else
DEFINE_ERROR(NOT_JOINED, 0x93)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief A message cannot be sent because the network is currently
 *  overloaded.
 */
#define ST_NETWORK_BUSY(0xA1)
#else
DEFINE_ERROR(NETWORK_BUSY, 0xA1)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The application tried to send a message using an 
 * endpoint that it has not defined.
 */
#define ST_INVALID_ENDPOINT(0xA3)
#else
DEFINE_ERROR(INVALID_ENDPOINT, 0xA3)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The application tried to use a binding that has been remotely
 * modified and the change has not yet been reported to the application.
 */
#define ST_BINDING_HAS_CHANGED(0xA4)
#else
DEFINE_ERROR(BINDING_HAS_CHANGED, 0xA4)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief An attempt to generate random bytes failed because of 
 * insufficient random data from the radio.
 */
#define ST_INSUFFICIENT_RANDOM_DATA(0xA5)
#else
     DEFINE_ERROR(INSUFFICIENT_RANDOM_DATA, 0xA5)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** A ZigBee route error command frame was received indicating
 * that a source routed message from this node failed en route.
 */
#define ST_SOURCE_ROUTE_FAILURE(0xA9)
#else
     DEFINE_ERROR(SOURCE_ROUTE_FAILURE, 0xA9)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** A ZigBee route error command frame was received indicating
 * that a message sent to this node along a many-to-one route
 * failed en route.  The route error frame was delivered by
 * an ad-hoc search for a functioning route.
 */
#define ST_MANY_TO_ONE_ROUTE_FAILURE(0xAA)
#else
     DEFINE_ERROR(MANY_TO_ONE_ROUTE_FAILURE, 0xAA)
#endif


/** @} */

/**
 * @name  Miscellaneous Utility Errors
 *
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief A critical and fatal error indicating that the version of the
 * stack trying to run does not match with the chip it is running on. The
 * software (stack) on the chip must be replaced with software that is
 * compatible with the chip.
 */
#define ST_STACK_AND_HARDWARE_MISMATCH(0xB0)
#else
DEFINE_ERROR(STACK_AND_HARDWARE_MISMATCH, 0xB0)
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** 
 * @brief An index was passed into the function that was larger
 * than the valid range.
 */
#define ST_INDEX_OUT_OF_RANGE(0xB1)
#else
DEFINE_ERROR(INDEX_OUT_OF_RANGE, 0xB1)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief There are no empty entries left in the table.
 */
#define ST_TABLE_FULL(0xB4)
#else
DEFINE_ERROR(TABLE_FULL, 0xB4)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The requested table entry has been erased and contains
 *   no valid data.
 */
#define ST_TABLE_ENTRY_ERASED(0xB6)
#else
DEFINE_ERROR(TABLE_ENTRY_ERASED, 0xB6)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The requested function cannot be executed because
 *   the library that contains the necessary functionality is not present.
 */
#define ST_LIBRARY_NOT_PRESENT(0xB5)
#else
DEFINE_ERROR(LIBRARY_NOT_PRESENT, 0xB5)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief The stack accepted the command and is currently processing
 * the request.  The results will be returned via an appropriate handler.
 */
#define ST_OPERATION_IN_PROGRESS(0xBA)
#else
DEFINE_ERROR(OPERATION_IN_PROGRESS, 0xBA)
#endif

/** @} */

/**
 * @name  Application Errors
 * These error codes are available for application use.
 *
 * @{
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief This error is reserved for customer application use.
 *  This will never be returned from any portion of the network stack or HAL.
 */
#define ST_APPLICATION_ERROR_0(0xF0)
#define ST_APPLICATION_ERROR_1(0xF1)
#define ST_APPLICATION_ERROR_2(0xF2)
#define ST_APPLICATION_ERROR_3(0xF3)
#define ST_APPLICATION_ERROR_4(0xF4)
#define ST_APPLICATION_ERROR_5(0xF5)
#define ST_APPLICATION_ERROR_6(0xF6)
#define ST_APPLICATION_ERROR_7(0xF7)
#define ST_APPLICATION_ERROR_8(0xF8)
#define ST_APPLICATION_ERROR_9(0xF9)
#define ST_APPLICATION_ERROR_10(0xFA)
#define ST_APPLICATION_ERROR_11(0xFB)
#define ST_APPLICATION_ERROR_12(0xFC)
#define ST_APPLICATION_ERROR_13(0xFD)
#define ST_APPLICATION_ERROR_14(0xFE)
#define ST_APPLICATION_ERROR_15(0xFF)
#else
DEFINE_ERROR( APPLICATION_ERROR_0, 0xF0)
DEFINE_ERROR( APPLICATION_ERROR_1, 0xF1)
DEFINE_ERROR( APPLICATION_ERROR_2, 0xF2)
DEFINE_ERROR( APPLICATION_ERROR_3, 0xF3)
DEFINE_ERROR( APPLICATION_ERROR_4, 0xF4)
DEFINE_ERROR( APPLICATION_ERROR_5, 0xF5)
DEFINE_ERROR( APPLICATION_ERROR_6, 0xF6)
DEFINE_ERROR( APPLICATION_ERROR_7, 0xF7)
DEFINE_ERROR( APPLICATION_ERROR_8, 0xF8)
DEFINE_ERROR( APPLICATION_ERROR_9, 0xF9)
DEFINE_ERROR( APPLICATION_ERROR_10, 0xFA)
DEFINE_ERROR( APPLICATION_ERROR_11, 0xFB)
DEFINE_ERROR( APPLICATION_ERROR_12, 0xFC)
DEFINE_ERROR( APPLICATION_ERROR_13, 0xFD)
DEFINE_ERROR( APPLICATION_ERROR_14, 0xFE)
DEFINE_ERROR( APPLICATION_ERROR_15, 0xFF)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */

/** @} END defgroup */

/** @} END addtogroup */

/**
 * <!-- HIDDEN
 * @page 2p5_to_3p0
 * <hr>
 * All error status codes defined in error-def.h 
 * (and described in @ref status_codes) now have an \c ST_ prefix. 
 * Status code changes also include the following:
 * <ul>
 * <li> <b>New items</b>
 *   - ::ST_ERR_FLASH_WRITE_INHIBITED
 *   - ::ST_ERR_FLASH_VERIFY_FAILED
 *   - ::ST_ADDRESS_TABLE_INDEX_OUT_OF_RANGE
 *   - ::ST_ADDRESS_TABLE_ENTRY_IS_ACTIVE
 *   - ::ST_CANNOT_JOIN_AS_ROUTER
 *   - ::ST_INSUFFICIENT_RANDOM_DATA
 *   - ::ST_APS_ENCRYPTION_ERROR
 *   - ::ST_TRUST_CENTER_MASTER_KEY_NOT_SET
 *   - ::ST_SECURITY_STATE_NOT_SET
 *   - ::ST_SOURCE_ROUTE_FAILURE
 *   - ::ST_MANY_TO_ONE_ROUTE_FAILURE
 *   .
 * <li> <b>Changed items</b>
 *   -
 *   -
 *   .
 * <li> <b>Removed items</b>
 *   - ::SIM_EEPROM_FLASH_WRITE_FAILED
 *   - ::TOO_MANY_CONNECTIONS
 *   - ::CONNECTION_OPEN
 *   - ::CONNECTION_FAILED
 *   - ::CONNECTION_CLOSED
 *   - ::CONNECTION_CLOSING
 *   - ::CONNECTION_NOT_YET_OPEN
 *   - ::INCOMING_SEQUENCED_MESSAGES_LOST
 *   - ::NODEID_INVALID
 *   .
 * </ul>
 * HIDDEN -->
 */

/**
 * <!-- HIDDEN
 * @page 3p0_to_350
 * <hr>
 * <ul>
 * <li> <b>Removed items</b>
 *   - ::ST_ERR_TOKEN_UNKNOWN
 *   - ::ST_ERR_TOKEN_EXISTS
 *   - ::ST_ERR_TOKEN_INVALID_SIZE
 *   - ::ST_ERR_TOKEN_READ_ONLY
 *   .
 * </ul>
 * HIDDEN -->
 */

