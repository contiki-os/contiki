/**
  ******************************************************************************
 * @file    SPIRIT_PktBasic.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT Basic packets.
  * 
 * @details
 *
 * This module can be used to manage the configuration of Spirit Basic
 * packets.
 * The user can obtain a packet configuration filling the structure
 * <i>@ref PktBasicInit</i>, defining in it some general parameters
 * for the Spirit Basic packet format.
 * Another structure the user can fill is <i>@ref PktBasicAddressesInit</i>
 * to define the addresses which will be used during the communication.
 * Moreover, functions to set the payload length and the destination address
 * are provided.
 *
 * <b>Example:</b>
 * @code
 *
 * PktBasicInit basicInit={
 *   PKT_PREAMBLE_LENGTH_08BYTES,       // preamble length in bytes
 *   PKT_SYNC_LENGTH_4BYTES,            // sync word length in bytes
 *   0x1A2635A8,                        // sync word
 *   PKT_LENGTH_VAR,                    // variable or fixed payload length
 *   7,                                 // length field width in bits (used only for variable length)
 *   PKT_NO_CRC,                        // CRC mode
 *   PKT_CONTROL_LENGTH_0BYTES,         // control field length
 *   S_ENABLE,                          // address field
 *   S_DISABLE,                         // FEC
 *   S_ENABLE                           // whitening
 * };
 *
 * PktBasicAddressesInit addressInit={
 *   S_ENABLE,                          // enable/disable filtering on my address
 *   0x34,                              // my address (address of the current node)
 *   S_DISABLE,                         // enable/disable filtering on multicast address
 *   0xEE,                              // multicast address
 *   S_DISABLE,                         // enable/disable filtering on broadcast address
 *   0xFF                               // broadcast address
 * };
 *
 * ...
 *
 * SpiritPktBasicInit(&basicInit);
 * SpiritPktBasicAddressesInit(&addressInit);
 *
 * ...
 *
 * SpiritPktBasicSetPayloadLength(20);
 * SpiritPktBasicSetDestinationAddress(0x44);
 *
 * ...
 *
 * @endcode
 *
 * The module provides some other functions that can be used to modify
 * or read only some configuration parameters.
 *
 *
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
  ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT_PKT_BASIC_H
#define __SPIRIT_PKT_BASIC_H

/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Regs.h"
#include "SPIRIT_Types.h"
#include "SPIRIT_PktCommon.h"

#ifdef __cplusplus
 extern "C" {
#endif



/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @defgroup SPIRIT_PktBasic    Pkt Basic
 * @brief Configuration and management of SPIRIT Basic packets.
 * @details See the file <i>@ref SPIRIT_PktBasic.h</i> for more details.
 * @{
 */

/**
 * @defgroup PktBasic_Exported_Types    Pkt Basic Exported Types
 * @{
 */


/**
 * @brief  Preamble length in bytes enumeration.
 */
typedef PktPreambleLength                  BasicPreambleLength;

#define IS_BASIC_PREAMBLE_LENGTH           IS_PKT_PREAMBLE_LENGTH

/**
 * @brief  Sync length in bytes enumeration.
 */
typedef PktSyncLength                      BasicSyncLength;

#define IS_BASIC_SYNC_LENGTH               IS_PKT_SYNC_LENGTH



/**
 * @brief  CRC length in bytes enumeration.
 */
typedef PktCrcMode                         BasicCrcMode;

#define IS_BASIC_CRC_MODE                  IS_PKT_CRC_MODE


/**
 * @brief  Fixed or variable payload length enumeration.
 */
typedef PktFixVarLength                    BasicFixVarLength;

#define IS_BASIC_FIX_VAR_LENGTH            IS_PKT_FIX_VAR_LENGTH

/**
 * @brief  Control length in bytes enumeration.
 */
typedef PktControlLength                   BasicControlLength;

#define IS_BASIC_CONTROL_LENGTH            IS_PKT_CONTROL_LENGTH

/**
 * @brief  Sync words enumeration.
 */
typedef PktSyncX                           BasicSyncX;

#define IS_BASIC_SYNCx                     IS_PKT_SYNCx


/**
 * @brief  SPIRIT Basic Packet Init structure definition. This structure allows users to set the main options
 *         for the Basic packet.
 */
typedef struct
{

  BasicPreambleLength           xPreambleLength;        /*!< Specifies the preamble length.
                                                             This parameter can be any value of @ref BasicPreambleLength */
  BasicSyncLength               xSyncLength;            /*!< Specifies the sync word length. The 32bit word passed (lSyncWords) will be stored in the SYNCx registers from the MSb
                                                             until the number of bytes in xSyncLength has been stored.
                                                             This parameter can be any value of @ref BasicSyncLength */
  uint32_t                      lSyncWords;             /*!< Specifies the sync words.
                                                             This parameter is a uint32_t word with format: 0x|SYNC1|SYNC2|SYNC3|SYNC4| */
  BasicFixVarLength             xFixVarLength;          /*!< Specifies if a fixed length of packet has to be used.
                                                             This parameter can be any value of @ref BasicFixVarLength */
  uint8_t                       cPktLengthWidth;        /*!< Specifies the size of the length of packet in bits. This field is useful only if
                                                             the field xFixVarLength is set to BASIC_LENGTH_VAR. For Basic packets the length width
                                                             is log2( max payload length + control length (0 to 4) + address length (0 or 1)).
                                                             This parameter is an uint8_t */
  BasicCrcMode                  xCrcMode;               /*!< Specifies the CRC word length of packet.
                                                             This parameter can be any value of @ref BasicCrcMode */
  BasicControlLength            xControlLength;         /*!< Specifies the length of a control field to be sent.
                                                             This parameter can be any value of @ref BasicControlLength */
  SpiritFunctionalState         xAddressField;          /*!< Specifies if the destination address has to be sent.
                                                             This parameter can be S_ENABLE or S_DISABLE */
  SpiritFunctionalState         xFec;                   /*!< Specifies if FEC has to be enabled.
                                                             This parameter can be S_ENABLE or S_DISABLE */
  SpiritFunctionalState         xDataWhitening;         /*!< Specifies if data whitening has to be enabled.
                                                             This parameter can be S_ENABLE or S_DISABLE */
}PktBasicInit;


/**
 * @brief  SPIRIT Basic Packet address structure definition. This structure allows users to specify
 *         the node/multicast/broadcast addresses and the correspondent filtering options.
 */
typedef struct
{

  SpiritFunctionalState         xFilterOnMyAddress;             /*!< If set RX packet is accepted if its destination address matches with cMyAddress.
                                                                     This parameter can be S_ENABLE or S_DISABLE */
  uint8_t                       cMyAddress;                     /*!< Specifies the TX packet source address (address of this node).
                                                                     This parameter is an uint8_t */
  SpiritFunctionalState         xFilterOnMulticastAddress;      /*!< If set RX packet is accepted if its destination address matches with cMulticastAddress.
                                                                     This parameter can be S_ENABLE or S_DISABLE */
  uint8_t                       cMulticastAddress;              /*!< Specifies the Multicast group address for this node.
                                                                     This parameter is an uint8_t */
  SpiritFunctionalState         xFilterOnBroadcastAddress;      /*!< If set RX packet is accepted if its destination address matches with cBroadcastAddress.
                                                                     This parameter can be S_ENABLE or S_DISABLE */
  uint8_t                       cBroadcastAddress;              /*!< Specifies the Broadcast address for this node.
                                                                     This parameter is an uint8_t */
}PktBasicAddressesInit;

/**
 *@}
 */


/**
 * @defgroup PktBasic_Exported_Constants        Pkt Basic Exported Constants
 * @{
 */

#define IS_BASIC_LENGTH_WIDTH_BITS                      IS_PKT_LENGTH_WIDTH_BITS


/**
 *@}
 */


/**
 * @defgroup PktBasic_Exported_Macros   Pkt Basic Exported Macros
 * @{
 */

/**
 * @brief  Macro used to compute per lower part of the packet length
 *         for Spirit Basic packets, to write in the PCKTLEN0 register.
 * @param  nLength Length of the packet payload.
 *         This parameter is an uint16_t.
 * @retval None.
 */
#define BASIC_BUILD_PCKTLEN0(nLength)                                            BUILD_PCKTLEN0(nLength)


/**
 * @brief  Macro used to compute per upper part of the packet length
 *         for Spirit Basic packets, to write the PCKTLEN1 register.
 * @param  nLengthLength of the packet payload.
 *         This parameter is an uint16_t.
 * @retval None.
 */
#define BASIC_BUILD_PCKTLEN1(nLength)                                            BUILD_PCKTLEN1(nLength)

/**
 * @brief  Sets the CONTROL field length for SPIRIT Basic packets.
 * @param  xControlLength length of CONTROL field in bytes.
 *         This parameter can be any value of @ref PktControlLength.
 * @retval None.
 */
#define SpiritPktBasicSetControlLength(xControlLength)                  SpiritPktCommonSetControlLength(xControlLength)


/**
 * @brief  Returns the CONTROL field length for SPIRIT Basic packets.
 * @param  None.
 * @retval uint8_t Control field length.
 */
#define SpiritPktBasicGetControlLength()                                SpiritPktCommonGetControlLength()


/**
 * @brief  Sets the PREAMBLE field length for SPIRIT Basic packets.
 * @param  xPreambleLength length of PREAMBLE field in bytes.
 *         This parameter can be any value of @ref BasicPreambleLength.
 * @retval None.
 */
#define SpiritPktBasicSetPreambleLength(xPreambleLength)                SpiritPktCommonSetPreambleLength((PktPreambleLength)xPreambleLength)


/**
 * @brief  Returns the PREAMBLE field length mode for SPIRIT Basic packets.
 * @param  None.
 * @retval uint8_t Preamble field length in bytes.
 */
#define SpiritPktBasicGetPreambleLength()                               SpiritPktCommonGetPreambleLength()


/**
 * @brief  Sets the SYNC field length for SPIRIT Basic packets.
 * @param  xSyncLength length of SYNC field in bytes.
 *         This parameter can be any value of @ref BasicSyncLength.
 * @retval None.
 */
#define SpiritPktBasicSetSyncLength(xSyncLength)                         SpiritPktCommonSetSyncLength((PktSyncLength)xSyncLength)


/**
 * @brief  Returns the SYNC field length for SPIRIT Basic packets.
 * @param  None.
 * @retval uint8_t SYNC field length in bytes.
 */
#define SpiritPktBasicGetSyncLength()                                   SpiritPktCommonGetSyncLength()


/**
 * @brief  Sets fixed or variable payload length mode for SPIRIT packets.
 * @param  xFixVarLength variable or fixed length.
 *         BASIC_FIXED_LENGTH_VAR -> variable (the length is extracted from the received packet).
 *         BASIC_FIXED_LENGTH_FIX -> fix (the length is set by PCKTLEN0 and PCKTLEN1).
 * @retval None.
 */
#define SpiritPktBasicSetFixVarLength(xFixVarLength)                    SpiritPktCommonSetFixVarLength((PktFixVarLength)xFixVarLength)


/**
 * @brief  Enables or Disables the CRC filtering.
 * @param  xNewState new state for CRC_CHECK.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktBasicFilterOnCrc(xNewState)                            SpiritPktCommonFilterOnCrc(xNewState)


/**
 * @brief  Returns the CRC filtering bit.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktBasicGetFilterOnCrc()                                  SpiritPktCommonGetFilterOnCrc()


/**
 * @brief  Sets the CRC mode for SPIRIT Basic packets.
 * @param  xCrcMode CRC mode.
 *         This parameter can be any value of @ref BasicCrcMode.
 * @retval None.
 */
#define SpiritPktBasicSetCrcMode(xCrcMode)                              SpiritPktCommonSetCrcMode((PktCrcMode)xCrcMode)


/**
 * @brief  Returns the CRC mode for SPIRIT Basic packets.
 * @param  None.
 * @retval BasicCrcMode Crc mode.
 */
#define SpiritPktBasicGetCrcMode()                                      (BasicCrcMode)SpiritPktCommonGetCrcMode()


/**
 * @brief  Enables or Disables WHITENING for SPIRIT packets.
 * @param  xNewState new state for WHITENING mode.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktBasicWhitening(xNewState)                               SpiritPktCommonWhitening(xNewState)


/**
 * @brief  Enables or Disables FEC for SPIRIT Basic packets.
 * @param  xNewState new state for FEC mode.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktBasicFec(xNewState)                                     SpiritPktCommonFec(xNewState)


/**
 * @brief  Sets a specific SYNC word for SPIRIT Basic packets.
 * @param  xSyncX SYNC word number to be set.
 *         This parameter can be any value of @ref BasicSyncX.
 * @param  cSyncWord SYNC word.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktBasicSetSyncxWord(xSyncX, cSyncWord)                   SpiritPktCommonSetSyncxWord((PktSyncX)xSyncX, cSyncWord)


/**
 * @brief  Returns a specific SYNC words for SPIRIT Basic packets.
 * @param  xSyncX SYNC word number to be get.
 *         This parameter can be any value of @ref BasicSyncX.
 * @retval uint8_t Sync word x.
 */
#define SpiritPktBasicGetSyncxWord(xSyncX)                              SpiritPktCommonGetSyncxWord(xSyncX)


/**
 * @brief  Sets multiple SYNC words for SPIRIT Basic packets.
 * @param  lSyncWords SYNC words to be set with format: 0x|SYNC1|SYNC2|SYNC3|SYNC4|.
 *         This parameter is a uint32_t.
 * @param  xSyncLength SYNC length in bytes. The 32bit word passed will be stored in the SYNCx registers from the MSb
 *         until the number of bytes in xSyncLength has been stored.
 *         This parameter is a @ref BasicSyncLength.
 * @retval None.
 */
#define SpiritPktBasicSetSyncWords(lSyncWords, xSyncLength)              SpiritPktCommonSetSyncWords(lSyncWords, (PktSyncLength)xSyncLength)


/**
 * @brief  Returns multiple SYNC words for SPIRIT Basic packets.
 * @param  xSyncLength SYNC length in bytes. The 32bit word passed will be stored in the SYNCx registers from the MSb
 *         until the number of bytes in xSyncLength has been stored.
 *         This parameter is a pointer to @ref BasicSyncLength.
 * @retval uint32_t Sync words. The format of the read 32 bit word is 0x|SYNC1|SYNC2|SYNC3|SYNC4|.
 */
#define SpiritPktBasicGetSyncWords(xSyncLength)                         SpiritPktCommonGetSyncWords((PktSyncLength)xSyncLength)


/**
 * @brief  Returns the SPIRIT variable length width (in number of bits).
 * @param  None.
 * @retval Variable length width in bits.
 */
#define SpiritPktBasicGetVarLengthWidth()                               SpiritPktCommonGetVarLengthWidth()


/**
 * @brief  Sets the destination address for the Tx packet.
 * @param  cAddress destination address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktBasicSetDestinationAddress(cAddress)                   SpiritPktCommonSetDestinationAddress(cAddress)


/**
 * @brief  Returns the settled destination address.
 * @param  None.
 * @retval uint8_t Transmitted destination address.
 */
#define SpiritPktBasicGetTransmittedDestAddress()                       SpiritPktCommonGetTransmittedDestAddress()


/**
 * @brief  Sets the node address. When the filtering on my address is on, if the destination address extracted from the received packet is equal to the content of the
 *         my address, then the packet is accepted (this is the address of the node).
 * @param  cAddress Address of the present node.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktBasicSetMyAddress(cAddress)                            SpiritPktCommonSetMyAddress(cAddress)


/**
 * @brief  Returns the address of the present node.
 * @param  None.
 * @retval uint8_t My address (address of this node).
 */
#define SpiritPktBasicGetMyAddress()                                    SpiritPktCommonGetMyAddress()


/**
 * @brief  Sets the broadcast address. When the broadcast filtering is on, if the destination address extracted from the received packet is equal to the content of the
 *         BROADCAST_ADDR register, then the packet is accepted.
 * @param  cAddress Broadcast address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktBasicSetBroadcastAddress(cAddress)                     SpiritPktCommonSetBroadcastAddress(cAddress)


/**
 * @brief  Returns the broadcast address.
 * @param  None.
 * @retval uint8_t Broadcast address.
 */
#define SpiritPktBasicGetBroadcastAddress()                             SpiritPktCommonGetBroadcastAddress()


/**
 * @brief  Sets the multicast address. When the multicast filtering is on, if the destination address extracted from the received packet is equal to the content of the
 *         MULTICAST_ADDR register, then the packet is accepted.
 * @param  cAddress Multicast address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktBasicSetMulticastAddress(cAddress)                     SpiritPktCommonSetMulticastAddress(cAddress)


/**
 * @brief  Returns the multicast address.
 * @param  None.
 * @retval uint8_t Multicast address.
 */
#define SpiritPktBasicGetMulticastAddress()                             SpiritPktCommonGetMulticastAddress()


/**
 * @brief  Sets the control mask. The 1 bits of the CONTROL_MASK indicate the
 *         bits to be used in filtering. (All 0s no filtering)
 * @param  lMask Control mask.
 *         This parameter is an uint32_t.
 * @retval None.
 */
#define SpiritPktBasicSetCtrlMask(lMask)                                SpiritPktCommonSetCtrlMask(lMask)


/**
 * @brief  Returns the control mask. The 1 bits of the CONTROL_MASK indicate the
 *         bits to be used in filtering. (All 0s no filtering)
 * @param  None.
 * @retval uint32_t Control mask.
 */
#define SpiritPktBasicGetCtrlMask()                                     SpiritPktCommonGetCtrlMask()


/**
 * @brief  Sets the control field reference. If the bits enabled by the
 *         CONTROL_MASK match the ones of the control fields extracted from the received packet
 *         then the packet is accepted.
 * @param  lReference Control reference.
 *         This parameter is an uint32_t.
 * @retval None.
 */
#define SpiritPktBasicSetCtrlReference(lReference)                      SpiritPktCommonSetCtrlReference(lReference)


/**
 * @brief  Returns the control field reference.
 * @param  None.
 * @retval uint32_t Control reference.
 */
#define SpiritPktBasicGetCtrlReference()                                SpiritPktCommonGetCtrlReference()


/**
 * @brief  Sets the TX control field.
 * @param  lField Tx control field.
 *         This parameter is an uint32_t.
 * @retval None.
 */
#define SpiritPktBasicSetTransmittedCtrlField(lField)                   SpiritPktCommonSetTransmittedCtrlField(lField)


/**
 * @brief  Returns the TX control field.
 * @param  None.
 * @retval uint32_t Control field of the transmitted packet.
 */
#define SpiritPktBasicGetTransmittedCtrlField()                         SpiritPktCommonGetTransmittedCtrlField()


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with My address.
 * @param  xNewState new state for DEST_VS_SOURCE_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktBasicFilterOnMyAddress(xNewState)                      SpiritPktCommonFilterOnMyAddress(xNewState)


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with multicast address.
 * @param  xNewState new state for DEST_VS_MULTICAST_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktBasicFilterOnMulticastAddress(xNewState)               SpiritPktCommonFilterOnMulticastAddress(xNewState)


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with broadcast address.
 * @param  xNewState new state for DEST_VS_BROADCAST_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktBasicFilterOnBroadcastAddress(xNewState)               SpiritPktCommonFilterOnBroadcastAddress(xNewState)


/**
 * @brief  Returns the enable bit of the my address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktBasicGetFilterOnMyAddress()                            SpiritPktCommonGetFilterOnMyAddress();


/**
 * @brief  Returns the enable bit of the multicast address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktBasicGetFilterOnMulticastAddress()                     SpiritPktCommonGetFilterOnMulticastAddress();


/**
 * @brief  Returns the enable bit of the broadcast address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktBasicGetFilterOnBroadcastAddress()                     SpiritPktCommonGetFilterOnBroadcastAddress();


/**
 * @brief  Returns the destination address of the received packet.
 * @param  None.
 * @retval uint8_t Destination address of the received packet.
 */
#define SpiritPktBasicGetReceivedDestAddress()                          SpiritPktCommonGetReceivedDestAddress()


/**
 * @brief  Returns the control field of the received packet.
 * @param  None.
 * @retval uint32_t Received control field.
 */
#define SpiritPktBasicGetReceivedCtrlField()                            SpiritPktCommonGetReceivedCtrlField()


/**
 * @brief  Returns the CRC field of the received packet.
 * @param  cCrcFieldVect array in which the CRC field has to be stored.
 *         This parameter is an uint8_t array of 3 elements.
 * @retval None.
 */
#define SpiritPktBasicGetReceivedCrcField(cCrcFieldVect)                SpiritPktCommonGetReceivedCrcField(cCrcFieldVect)


/**
 * @brief  If enabled RX packet is accepted only if the masked control field matches the
 *         masked control field reference (CONTROL_MASK & CONTROL_FIELD_REF == CONTROL_MASK & RX_CONTROL_FIELD).
 * @param  xNewState new state for Control filtering enable bit.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 * @note   This filtering control is enabled by default but the control mask is by default set to 0.
 *         As a matter of fact the user has to enable the control filtering bit after the packet initialization
 *         because the PktInit routine disables it.
 */
#define SpiritPktBasicFilterOnControlField(xNewState)                           SpiritPktCommonFilterOnControlField(xNewState)


/**
 * @brief  Returns the enable bit of the control field filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktBasicGetFilterOnControlField()                                 SpiritPktCommonGetFilterOnControlField();

/**
 *@}
 */


/**
 * @defgroup PktBasic_Exported_Functions        Pkt Basic Exported Functions
 * @{
 */

void SpiritPktBasicInit(PktBasicInit* pxPktBasicInit);
void SpiritPktBasicGetInfo(PktBasicInit* pxPktBasicInit);
void SpiritPktBasicAddressesInit(PktBasicAddressesInit* pxPktBasicAddresses);
void SpiritPktBasicGetAddressesInfo(PktBasicAddressesInit* pxPktBasicAddresses);
void SpiritPktBasicSetFormat(void);
void SpiritPktBasicAddressField(SpiritFunctionalState xAddressField);
SpiritFunctionalState SpiritPktBasicGetAddressField(void);
void SpiritPktBasicSetPayloadLength(uint16_t nPayloadLength);
uint16_t SpiritPktBasicGetPayloadLength(void);
uint16_t SpiritPktBasicGetReceivedPktLength(void);
void SpiritPktBasicSetVarLengthWidth(uint16_t nMaxPayloadLength,SpiritFunctionalState xAddressField, BasicControlLength xControlLength);

/**
 *@}
 */

/**
 *@}
 */


/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
