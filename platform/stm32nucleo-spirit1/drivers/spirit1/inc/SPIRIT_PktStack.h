/**
  ******************************************************************************
 * @file    SPIRIT_PktStack.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT STack packets.
  * 
 * @details
 *
 * This module can be used to manage the configuration of Spirit STack
 * packets, and it is quite similar to the Basic packets one since the
 * STack packets can be considered an extension of Basic.
 * The user can obtain a packet configuration filling the structure
 * <i>@ref PktStackInit</i>, defining in it some general parameters
 * for the Spirit STack packet format.
 * Another structure the user can fill is <i>@ref PktStackAddressesInit</i>
 * to define the addresses which will be used during the communication.
 * The structure <i>@ref PktStackLlpInit</i> is provided in order to configure
 * the link layer protocol features like autoack, autoretransmission
 * or piggybacking.
 * Moreover, functions to set the payload length and the destination address
 * are provided.
 *
 * <b>Example:</b>
 * @code
 *
 * PktStackInit stackInit={
 *   PKT_PREAMBLE_LENGTH_08BYTES,       // preamble length in bytes
 *   PKT_SYNC_LENGTH_4BYTES,            // sync word length in bytes
 *   0x1A2635A8,                        // sync word
 *   PKT_LENGTH_VAR,                    // variable or fixed payload length
 *   7,                                 // length field width in bits (used only for variable length)
 *   PKT_NO_CRC,                        // CRC mode
 *   PKT_CONTROL_LENGTH_0BYTES,         // control field length
 *   S_DISABLE,                         // FEC
 *   S_ENABLE                           // whitening
 * };
 *
 * PktStackAddressesInit addressInit={
 *   S_ENABLE,                          // enable/disable filtering on my address
 *   0x34,                              // my address (address of the current node)
 *   S_DISABLE,                         // enable/disable filtering on multicast address
 *   0xEE,                              // multicast address
 *   S_DISABLE,                         // enable/disable filtering on broadcast address
 *   0xFF                               // broadcast address
 * };
 *
 * PktStackLlpInit stackLLPInit ={
 *   S_DISABLE,                         // enable/disable the autoack feature
 *   S_DISABLE,                         // enable/disable the piggybacking feature
 *   PKT_DISABLE_RETX                   // set the max number of retransmissions or disable them
 * };
 * ...
 *
 * SpiritPktStackInit(&stackInit);
 * SpiritPktStackAddressesInit(&addressInit);
 * SpiritPktStackLlpInit(&stackLLPInit);
 *
 * ...
 *
 * SpiritPktStackSetPayloadLength(20);
 * SpiritPktStackSetDestinationAddress(0x44);
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
#ifndef __SPIRIT_PKT_STACK_H
#define __SPIRIT_PKT_STACK_H

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
 * @defgroup SPIRIT_PktStack    Pkt STack
 * @brief Configuration and management of SPIRIT STack packets.
 * @details See the file <i>@ref SPIRIT_PktStack.h</i> for more details.
 * @{
 */

/**
 * @defgroup PktStack_Exported_Types    Pkt STack Exported Types
 * @{
 */

/**
 * @brief  Preamble length in bytes enumeration.
 */
typedef PktPreambleLength                  StackPreambleLength;

#define IS_STACK_PREAMBLE_LENGTH           IS_PKT_PREAMBLE_LENGTH

/**
 * @brief  Sync length in bytes enumeration.
 */
typedef PktSyncLength                      StackSyncLength;

#define IS_STACK_SYNC_LENGTH               IS_PKT_SYNC_LENGTH



/**
 * @brief  CRC length in bytes enumeration.
 */
typedef PktCrcMode                         StackCrcMode;

#define IS_STACK_CRC_MODE                  IS_PKT_CRC_MODE


/**
 * @brief  Fixed or variable payload length enumeration.
 */
typedef PktFixVarLength                    StackFixVarLength;

#define IS_STACK_FIX_VAR_LENGTH            IS_PKT_FIX_VAR_LENGTH

/**
 * @brief  Control length in bytes enumeration for SPIRIT.
 */
typedef PktControlLength                   StackControlLength;

#define IS_STACK_CONTROL_LENGTH            IS_PKT_CONTROL_LENGTH

/**
 * @brief  Sync words enumeration for SPIRIT.
 */
typedef PktSyncX                           StackSyncX;

#define IS_STACK_SYNCx                     IS_PKT_SYNCx

/**
 * @brief  Max retransmission number enumeration for SPIRIT.
 */
typedef PktNMaxReTx                        StackNMaxReTx;

#define IS_STACK_NMAX_RETX                 IS_PKT_NMAX_RETX


/**
 * @brief  SPIRIT STack Packet Init structure definition. This structure allows users to set the main options
 *         for the STack packet.
 */
typedef struct
{

  StackPreambleLength           xPreambleLength;         /*!< Specifies the preamble length of packet.
                                                              This parameter can be any value of @ref StackPreambleLength */
  StackSyncLength               xSyncLength;             /*!< Specifies the sync word length of packet.
                                                              This parameter can be any value of @ref StackSyncLength */
  uint32_t                      lSyncWords;	         /*!< Specifies the sync words.
                                                              This parameter is a uint32_t word with format: 0x|SYNC1|SYNC2|SYNC3|SYNC4| */
  StackFixVarLength             xFixVarLength;           /*!< Specifies if a fixed length of packet has to be used.
                                                              This parameter can be any value of @ref StackFixVarLength */
  uint8_t            	        cPktLengthWidth;         /*!< Specifies the size of the length of packet in bits. This field is useful only if
                                                              the field xFixVarLength is set to STACK_LENGTH_VAR. For STack packets the length width
                                                               is log2( max payload length + control length (0 to 4) + address length (always 2)).
                                                              This parameter is an uint8_t */
  StackCrcMode                	xCrcMode;              	 /*!< Specifies the CRC word length of packet.
                                                              This parameter can be any value of @ref StackCrcMode */
  StackControlLength            xControlLength;          /*!< Specifies the length of a control field to be sent.
                                                              This parameter can be any value of @ref StackControlLength */
  SpiritFunctionalState         xFec;                    /*!< Specifies if FEC has to be enabled.
                                                              This parameter can be any value of @ref SpiritFunctionalState */
  SpiritFunctionalState         xDataWhitening;          /*!< Specifies if data whitening has to be enabled.
                                                              This parameter can be any value of @ref SpiritFunctionalState */

}PktStackInit;


/**
 * @brief  SPIRIT STack packet address structure definition. This structure allows users to specify
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
}PktStackAddressesInit;


/**
 * @brief  SPIRIT STack packet LLP structure definition. This structure allows users to configure
 *         all the LLP options for STack packets.
 */
typedef struct
{

  SpiritFunctionalState         xAutoAck;                /*!< Specifies if the auto ACK feature is used or not.
                                                              This parameter can be a value of @ref SpiritFunctionalState */
  SpiritFunctionalState         xPiggybacking;           /*!< Specifies if the piggybacking feature is used or not.
                                                              This parameter can be a value of @ref SpiritFunctionalState */
  StackNMaxReTx                 xNMaxRetx;               /*!< Specifies the number of MAX-Retransmissions.
                                                              This parameter can be a value of @ref StackNMaxReTx */
}PktStackLlpInit;



/**
 *@}
 */


/**
 * @defgroup PktStack_Exported_Constants        Pkt STack Exported Constants
 * @{
 */

#define IS_STACK_LENGTH_WIDTH_BITS                      IS_PKT_LENGTH_WIDTH_BITS

/**
 *@}
 */


/**
 * @defgroup PktStack_Exported_Macros   Pkt STack Exported Macros
 * @{
 */

/**
 * @brief  Macro used to compute the lower part of the packet length
 *         for Spirit STack packets, to write in the PCKTLEN0 register.
 * @param  nLength length of the packet payload.
 *         This parameter is an uint16_t.
 * @retval None.
 */
#define STACK_BUILD_PCKTLEN0(nLength)                                    BUILD_PCKTLEN0(nLength)


/**
 * @brief  Macro used to compute the upper part of the packet length
 *         for Spirit STack packets, to write the PCKTLEN1 register.
 * @param  nLength length of the packet payload.
 *         This parameter is an uint16_t.
 * @retval None.
 */
#define STACK_BUILD_PCKTLEN1(nLength)                                    BUILD_PCKTLEN1(nLength)


/**
 * @brief  Sets the CONTROL length for SPIRIT STack packets.
 * @param  xControlLength length of CONTROL field in bytes.
 *         This parameter can be any value of @ref StackControlLength.
 * @retval None.
 */
#define SpiritPktStackSetControlLength(xControlLength)                          SpiritPktCommonSetControlLength(xControlLength)


/**
 * @brief  Returns the CONTROL length for SPIRIT STack packets.
 * @param  None.
 * @retval Control length.
 */
#define SpiritPktStackGetControlLength()                                       SpiritPktCommonGetControlLength()


/**
 * @brief  Sets the PREAMBLE Length mode for SPIRIT STack packets.
 * @param  xPreambleLength length of PREAMBLE field in bytes.
 *         This parameter can be any value of @ref StackPreambleLength.
 * @retval None.
 */
#define SpiritPktStackSetPreambleLength(xPreambleLength)                        SpiritPktCommonSetPreambleLength((PktPreambleLength)xPreambleLength)


/**
 * @brief  Returns the PREAMBLE Length mode for SPIRIT STack packets.
 * @param  None.
 * @retval uint8_t Preamble length in bytes.
 */
#define SpiritPktStackGetPreambleLength()                                      SpiritPktCommonGetPreambleLength()


/**
 * @brief  Sets the SYNC Length for SPIRIT STack packets.
 * @param  xSyncLength length of SYNC field in bytes.
 *         This parameter can be any value of @ref StackSyncLength.
 * @retval None.
 */
#define SpiritPktStackSetSyncLength(xSyncLength)                                SpiritPktCommonSetSyncLength((PktSyncLength)xSyncLength)


/**
 * @brief  Returns the SYNC Length for SPIRIT STack packets.
 * @param  None.
 * @retval uint8_t Sync length in bytes.
 */
#define SpiritPktStackGetSyncLength()                                           SpiritPktCommonGetSyncLength()


/**
 * @brief  Sets fixed or variable payload length mode for SPIRIT STack packets.
 * @param  xFixVarLength variable or fixed length.
 *         PKT_FIXED_LENGTH_VAR -> variable (the length is extracted from the received packet).
 *         PKT_FIXED_LENGTH_FIX -> fix (the length is set by PCKTLEN0 and PCKTLEN1).
 * @retval None.
 */
#define SpiritPktStackSetFixVarLength(xFixVarLength)                            SpiritPktCommonSetFixVarLength((PktFixVarLength)xFixVarLength)


/**
 * @brief  Enables or Disables the CRC filtering.
 * @param  xNewState new state for CRC_CHECK.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackFilterOnCrc(xNewState)                                    SpiritPktCommonFilterOnCrc(xNewState)


/**
 * @brief  Returns the CRC filtering bit.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktStackGetFilterOnCrc()                                          SpiritPktCommonGetFilterOnCrc()


/**
 * @brief  Sets the CRC mode for SPIRIT STack packets.
 * @param  xCrcMode CRC mode.
 *         This parameter can be any value of @ref StackCrcMode.
 * @retval None.
 */
#define SpiritPktStackSetCrcMode(xCrcMode)                                      SpiritPktCommonSetCrcMode((PktCrcMode)xCrcMode)


/**
 * @brief  Returns the CRC mode for SPIRIT packets.
 * @param  None.
 * @retval StackCrcMode Crc mode.
 */
#define SpiritPktStackGetCrcMode()                                             (StackCrcMode)SpiritPktCommonGetCrcMode()


/**
 * @brief  Enables or Disables WHITENING for SPIRIT STack packets.
 * @param  xNewState new state for WHITENING mode.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackWhitening(xNewState)                                     SpiritPktCommonWhitening(xNewState)


/**
 * @brief  Enables or Disables FEC for SPIRIT STack packets.
 * @param  xNewState new state for FEC mode.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackFec(xNewState)                                            SpiritPktCommonFec(xNewState)


/**
 * @brief  Sets a specific SYNC word for SPIRIT STack packets.
 * @param  xSyncX SYNC word number to be set.
 *         This parameter can be any value of @ref StackSyncX.
 * @param  cSyncWord SYNC word.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackSetSyncxWord(xSyncX, cSyncWord)                          SpiritPktCommonSetSyncxWord((PktSyncX)xSyncX,cSyncWord)


/**
 * @brief  Returns a specific SYNC word for SPIRIT STack packets.
 * @param  xSyncX SYNC word number to be get.
 *         This parameter can be any value of @ref StackSyncX.
 * @retval uint8_t Sync word x.
 */
#define SpiritPktStackGetSyncxWord(xSyncX)                                     SpiritPktCommonGetSyncxWord(xSyncX)


/**
 * @brief  Sets multiple SYNC words for SPIRIT STack packets.
 * @param  lSyncWords SYNC words to be set with format: 0x|SYNC1|SYNC2|SYNC3|SYNC4|.
 *         This parameter is a uint32_t.
 * @param  xSyncLength SYNC length in bytes. The 32bit word passed will be stored in the SYNCx registers from the MSb
 *         until the number of bytes in xSyncLength has been stored.
 *         This parameter is a @ref StackSyncLength.
 * @retval None.
 */
#define SpiritPktStackSetSyncWords(lSyncWords, xSyncLength)                    SpiritPktCommonSetSyncWords(lSyncWords,(PktSyncLength)xSyncLength)


/**
 * @brief  Returns multiple SYNC words for SPIRIT packets.
 * @param  xSyncLength SYNC length in bytes. The 32bit word passed will be stored in the SYNCx registers from the MSb
 *         until the number of bytes in xSyncLength has been stored.
 *         This parameter is a pointer to @ref StackSyncLength.
 * @retval uint32_t Sync words. The format of the read 32 bit word is 0x|SYNC1|SYNC2|SYNC3|SYNC4|.
 */
#define SpiritPktStackGetSyncWords(xSyncLength)                                 SpiritPktCommonGetSyncWords((PktSyncLength)xSyncLength)


/**
 * @brief  Returns the SPIRIT variable length width (in number of bits).
 * @param  None.
 * @retval uint8_t Variable length width in bits.
 */
#define SpiritPktStackGetVarLengthWidth()                                       SpiritPktCommonGetVarLengthWidth()


/**
 * @brief  Sets the destination address for the Tx packet.
 * @param  cAddress destination address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackSetDestinationAddress(cAddress)                           SpiritPktCommonSetDestinationAddress(cAddress)


/**
 * @brief  Sets the Rx packet reference source address. The source address extracted from the received packet is masked
 *         with the source reference mask and then compared to the masked reference value.
 * @param  cAddress Reference source address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackSetSourceReferenceAddress(cAddress)                       SpiritPktCommonSetDestinationAddress(cAddress)


/**
 * @brief  Returns the Rx packet reference source address. The source address extracted from the received packet is masked
 *         with the source reference mask and then compared to the masked reference value.
 * @param  cAddress Reference source address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackGetSourceReferenceAddress()                               SpiritPktCommonGetTransmittedDestAddress()


/**
 * @brief  Returns the settled destination address.
 * @param  None.
 * @retval uint8_t Transmitted destination address.
 */
#define SpiritPktStackGetTransmittedDestAddress()                               SpiritPktCommonGetTransmittedDestAddress()


/**
 * @brief  Sets the node address. When the filtering on my address is on, if the destination address extracted from the received packet is equal to the content of the
 *         my address, then the packet is accepted (this is the address of the node).
 * @param  cAddress Address of the present node.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackSetMyAddress(cAddress)                                    SpiritPktCommonSetMyAddress(cAddress)


/**
 * @brief  Returns the address of the present node.
 * @param  None.
 * @retval uint8_t My address (address of this node).
 */
#define SpiritPktStackGetMyAddress()                                            SpiritPktCommonGetMyAddress()


/**
 * @brief  Sets the broadcast address. When the broadcast filtering is on, if the destination address extracted from the received packet is equal to the content of the
 *         BROADCAST_ADDR register, then the packet is accepted.
 * @param  cAddress Broadcast address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackSetBroadcastAddress(cAddress)                             SpiritPktCommonSetBroadcastAddress(cAddress)


/**
 * @brief  Returns the broadcast address.
 * @param  None.
 * @retval uint8_t Broadcast address.
 */
#define SpiritPktStackGetBroadcastAddress()                                     SpiritPktCommonGetBroadcastAddress()


/**
 * @brief  Sets the multicast address. When the multicast filtering is on, if the destination address extracted from the received packet is equal to the content of the
 *         MULTICAST_ADDR register, then the packet is accepted.
 * @param  cAddress Multicast address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
#define SpiritPktStackSetMulticastAddress(cAddress)                             SpiritPktCommonSetMulticastAddress(cAddress)


/**
 * @brief  Returns the multicast address.
 * @param  None.
 * @retval uint8_t Multicast address.
 */
#define SpiritPktStackGetMulticastAddress()                                     SpiritPktCommonGetMulticastAddress()


/**
 * @brief  Sets the control mask. The 1 bits of the CONTROL_MASK indicate the
 *         bits to be used in filtering. (All 0s no filtering)
 * @param  lMask Control mask.
 *         This parameter is an uint32_t.
 * @retval None.
 */
#define SpiritPktStackSetCtrlMask(lMask)                                        SpiritPktCommonSetCtrlMask(lMask)


/**
 * @brief  Returns the control mask. The 1 bits of the CONTROL_MASK indicate the
 *         bits to be used in filtering. (All 0s no filtering)
 * @param  None.
 * @retval uint32_t Control mask.
 */
#define SpiritPktStackGetCtrlMask()                                             SpiritPktCommonGetCtrlMask()


/**
 * @brief  Sets the control field reference. If the bits enabled by the
 *         CONTROL_MASK match the ones of the control fields extracted from the received packet
 *         then the packet is accepted.
 * @param  lReference Control reference.
 *         This parameter is an uint32_t.
 * @retval None.
 */
#define SpiritPktStackSetCtrlReference(lReference)                              SpiritPktCommonSetCtrlReference(lReference)


/**
 * @brief  Returns the control field reference.
 * @param  None.
 * @retval uint32_t Control reference.
 */
#define SpiritPktStackGetCtrlReference()                                        SpiritPktCommonGetCtrlReference()


/**
 * @brief  Sets the TX control field.
 * @param  lField TX CONTROL FIELD.
 *         This parameter is an uint32_t.
 * @retval None.
 */
#define SpiritPktStackSetTransmittedCtrlField(lField)                           SpiritPktCommonSetTransmittedCtrlField(lField)


/**
 * @brief  Returns the TX control field.
 * @param  None.
 * @retval uint32_t Control field of the transmitted packet.
 */
#define SpiritPktStackGetTransmittedCtrlField()                                 SpiritPktCommonGetTransmittedCtrlField()


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with TX_SOURCE_ADDRESS.
 * @param  xNewState new state for DEST_VS_SOURCE_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackFilterOnMyAddress(xNewState)                               SpiritPktCommonFilterOnMyAddress(xNewState)


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with MULTICAST_ADDRESS.
 * @param  xNewState new state for DEST_VS_MULTICAST_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackFilterOnMulticastAddress(xNewState)                        SpiritPktCommonFilterOnMulticastAddress(xNewState)


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with BROADCAST_ADDRESS.
 * @param  xNewState new state for DEST_VS_BROADCAST_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackFilterOnBroadcastAddress(xNewState)                        SpiritPktCommonFilterOnBroadcastAddress(xNewState)


/**
 * @brief  Returns the enable bit of the my address filtering.
 * @param  None.
 * @retval SpiritFunctionalStateThis parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktStackGetFilterOnMyAddress()                                    SpiritPktCommonGetFilterOnMyAddress();


/**
 * @brief  Returns the enable bit of the multicast address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktStackGetFilterOnMulticastAddress()                             SpiritPktCommonGetFilterOnMulticastAddress();


/**
 * @brief  Returns the enable bit of the broadcast address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktStackGetFilterOnBroadcastAddress()                             SpiritPktCommonGetFilterOnBroadcastAddress();


/**
 * @brief  Returns the control field of the received packet.
 * @param  None.
 * @retval uint32_t Received control field.
 */
#define SpiritPktStackGetReceivedCtrlField()                                     SpiritPktCommonGetReceivedCtrlField()


/**
 * @brief  Returns the CRC field of the received packet.
 * @param  cCrcFieldVect array in which the CRC field has to be stored.
 *         This parameter is an uint8_t array of 3 elements.
 * @retval None.
 */
#define SpiritPktStackGetReceivedCrcField(cCrcFieldVect)                         SpiritPktCommonGetReceivedCrcField(cCrcFieldVect)

/**
 * @brief  Sets the AUTO ACKNOLEDGEMENT mechanism on the receiver. When the feature is enabled and
 *         a data packet has been correctly received, then an acknowledgement packet is sent back to the originator of the received
 *         packet. If the PIGGYBACKING bit is also set, payload data will be read from the FIFO; otherwise an empty packet is sent
 *         only containing the source and destination addresses and the sequence number of the packet being acknowledged.
 * @param  xAutoAck new state for autoack.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @param  xPiggybacking new state for autoack.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackAutoAck(xAutoAck, xPiggybacking)                          SpiritPktCommonAutoAck(xAutoAck, xPiggybacking)


/**
 * @brief  Sets the AUTO ACKNOLEDGEMENT mechanism on the transmitter. On the transmitter side, the NACK_TX field can be used to require or not an acknowledgment for each individual packet: if
 *         NACK_TX is set to "1" then acknowledgment will not be required; if NACK_TX is set to "0" then acknowledgment will be
 *         required.
 * @param  xNewState new state for TX_AUTOACK.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackRequireAck(xNewState)                                     SpiritPktCommonRequireAck(xNewState)


/**
 * @brief  Sets the TX sequence number to be used to start counting.
 * @param  cSeqNumberReload new value for Tx seq number reload.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
#define SpiritPktStackSetTransmittedSeqNumberReload(cSeqNumberReload)           SpiritPktCommonSetTransmittedSeqNumberReload(cSeqNumberReload)


/**
 * @brief  Sets the max number of automatic retransmission.
 * @param  xNMaxReTx max number of retransmission.
 *         This parameter can be any value of @ref PktNMaxReTx.
 * @retval None.
 */
#define SpiritPktStackSetNMaxReTx(xNMaxReTx)                                    SpiritPktCommonSetNMaxReTx((PktNMaxReTx)xNMaxReTx)


/**
 * @brief  Returns the max number of automatic retransmission.
 * @param  None.
 * @retval uint8_t Max number of retransmissions.
 */
#define SpiritPktStackGetNMaxReTx()                                             SpiritPktCommonGetNMaxReTx()


/**
 * @brief  Returns the TX ACK request.
 * @param  None.
 * @retval SpiritFunctionalState.
 */
#define SpiritPktStackGetGetTxAckRequest()                                       SpiritPktCommonGetTxAckRequest()

/**
 * @brief  Returns the destination address of the received packet.
 * @param  None.
 * @retval uint8_t Destination address of the received packet.
 */
#define SpiritPktStackGetReceivedDestAddress()                                  SpiritPktCommonGetReceivedDestAddress()


/**
 * @brief  Returns the source address of the received packet.
 * @param  None.
 * @retval uint8_t Source address of the received packet.
 */
#define SpiritPktStackGetReceivedSourceAddress()                                SpiritPktCommonGetReceivedSourceAddress()


/**
 * @brief  Returns the sequence number of the received packet.
 * @param  None.
 * @retval uint8_t Received Sequence number.
 */
#define SpiritPktStackGetReceivedSeqNumber()                                    SpiritPktCommonGetReceivedSeqNumber()


/**
 * @brief  Returns the Nack bit of the received packet
 * @param  None.
 * @retval uint8_t Value of the NAck bit.
 */
#define SpiritPktStackGetReceivedNackRx()                                       SpiritPktCommonGetReceivedNackRx()


/**
 * @brief  Returns the sequence number of the transmitted packet.
 * @param  None.
 * @retval uint8_t Sequence number of the transmitted packet.
 */
#define SpiritPktStackGetTransmittedSeqNumber()                                 SpiritPktCommonGetTransmittedSeqNumber()


/**
 * @brief  Returns the number of retransmission done on the transmitted packet.
 * @param  None.
 * @retval uint8_t Number of retransmissions done until now.
 */
#define SpiritPktStackGetNReTx()                                                SpiritPktCommonGetNReTx()


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
#define SpiritPktStackFilterOnControlField(xNewState)                           SpiritPktCommonFilterOnControlField(xNewState)


/**
 * @brief  Returns the enable bit of the control field filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
#define SpiritPktStackGetFilterOnControlField()                                 SpiritPktCommonGetFilterOnControlField();


/**
 *@}
 */


/**
 * @defgroup PktStack_Exported_Functions        Pkt STack Exported Functions
 * @{
 */

void SpiritPktStackInit(PktStackInit* pxPktStackInit);
void SpiritPktStackGetInfo(PktStackInit* pxPktStackInit);
void SpiritPktStackAddressesInit(PktStackAddressesInit* pxPktStackAddresses);
void SpiritPktStackGetAddressesInfo(PktStackAddressesInit* pxPktStackAddresses);
void SpiritPktStackLlpInit(PktStackLlpInit* pxPktStackLlpInit);
void SpiritPktStackLlpGetInfo(PktStackLlpInit* pxPktStackLlpInit);
void SpiritPktStackSetFormat(void);
void SpiritPktStackSetPayloadLength(uint16_t nPayloadLength);
uint16_t SpiritPktStackGetPayloadLength(void);
void SpiritPktStackSetVarLengthWidth(uint16_t nMaxPayloadLength, StackControlLength xControlLength);
void SpiritPktStackSetRxSourceMask(uint8_t cMask);
uint8_t SpiritPktStackGetRxSourceMask(void);
uint16_t SpiritPktStackGetReceivedPktLength(void);
void SpiritPktStackFilterOnSourceAddress(SpiritFunctionalState xNewState);
void SpiritPktStackSetAddressLength(void);

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
