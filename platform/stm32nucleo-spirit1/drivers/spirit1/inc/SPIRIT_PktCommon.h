/**
  ******************************************************************************
 * @file    SPIRIT_PktCommon.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of the common features of SPIRIT packets.
  * 
 * @details
 *
 * This module provides all the common functions and definitions used by the
 * packets modules.
 * Here are also defined all the generic enumeration types that are redefined
 * in the specific packets modules, but every enumeration value is referred
 * to this module. So the user who wants to configure the preamble of a Basic,
 * or a STack packet has to use the enumeration values defined here.
 *
 * <b>Example:</b>
 * @code
 *
 *   ...
 *
 *   SpiritPktBasicSetPreambleLength(PKT_PREAMBLE_LENGTH_18BYTES);
 *
 *   ...
 *
 * @endcode
 *
 * @note Is recommended for the user to not use these API directly
 * importing this module in his application.
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
#ifndef __SPIRIT_PKT_COMMON_H
#define __SPIRIT_PKT_COMMON_H

/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Regs.h"
#include "SPIRIT_Types.h"


#ifdef __cplusplus
 extern "C" {
#endif



/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @defgroup SPIRIT_PktCommon           Pkt Common
 * @brief Configuration and management of the common features of SPIRIT packets.
 * @details See the file <i>@ref SPIRIT_PktCommon.h</i> for more details.
 * @{
 */

/**
 * @defgroup PktCommon_Exported_Types   Pkt Common Exported Types
 * @{
 */


/**
 * @brief  Preamble length in bytes enumeration.
 */
typedef enum
{
  PKT_PREAMBLE_LENGTH_01BYTE            = 0x00, /*!< Preamble length 1 byte*/
  PKT_PREAMBLE_LENGTH_02BYTES           = 0x08, /*!< Preamble length 2 bytes */
  PKT_PREAMBLE_LENGTH_03BYTES           = 0x10, /*!< Preamble length 3 bytes */
  PKT_PREAMBLE_LENGTH_04BYTES           = 0x18, /*!< Preamble length 4 bytes */
  PKT_PREAMBLE_LENGTH_05BYTES           = 0x20, /*!< Preamble length 5 bytes */
  PKT_PREAMBLE_LENGTH_06BYTES           = 0x28, /*!< Preamble length 6 bytes */
  PKT_PREAMBLE_LENGTH_07BYTES           = 0x30, /*!< Preamble length 7 bytes */
  PKT_PREAMBLE_LENGTH_08BYTES           = 0x38, /*!< Preamble length 8 bytes */
  PKT_PREAMBLE_LENGTH_09BYTES           = 0x40, /*!< Preamble length 9 bytes */
  PKT_PREAMBLE_LENGTH_10BYTES           = 0x48, /*!< Preamble length 10 bytes */
  PKT_PREAMBLE_LENGTH_11BYTES           = 0x50, /*!< Preamble length 11 bytes */
  PKT_PREAMBLE_LENGTH_12BYTES           = 0x58, /*!< Preamble length 12 bytes */
  PKT_PREAMBLE_LENGTH_13BYTES           = 0x60, /*!< Preamble length 13 bytes */
  PKT_PREAMBLE_LENGTH_14BYTES           = 0x68, /*!< Preamble length 14 bytes */
  PKT_PREAMBLE_LENGTH_15BYTES           = 0x70, /*!< Preamble length 15 bytes */
  PKT_PREAMBLE_LENGTH_16BYTES           = 0x78, /*!< Preamble length 16 bytes */
  PKT_PREAMBLE_LENGTH_17BYTES           = 0x80, /*!< Preamble length 17 bytes */
  PKT_PREAMBLE_LENGTH_18BYTES           = 0x88, /*!< Preamble length 18 bytes */
  PKT_PREAMBLE_LENGTH_19BYTES           = 0x90, /*!< Preamble length 19 bytes */
  PKT_PREAMBLE_LENGTH_20BYTES           = 0x98, /*!< Preamble length 20 bytes */
  PKT_PREAMBLE_LENGTH_21BYTES           = 0xA0, /*!< Preamble length 21 bytes */
  PKT_PREAMBLE_LENGTH_22BYTES           = 0xA8, /*!< Preamble length 22 bytes */
  PKT_PREAMBLE_LENGTH_23BYTES           = 0xB0, /*!< Preamble length 23 bytes */
  PKT_PREAMBLE_LENGTH_24BYTES           = 0xB8, /*!< Preamble length 24 bytes */
  PKT_PREAMBLE_LENGTH_25BYTES           = 0xC0, /*!< Preamble length 25 bytes */
  PKT_PREAMBLE_LENGTH_26BYTES           = 0xC8, /*!< Preamble length 26 bytes */
  PKT_PREAMBLE_LENGTH_27BYTES           = 0xD0, /*!< Preamble length 27 bytes */
  PKT_PREAMBLE_LENGTH_28BYTES           = 0xD8, /*!< Preamble length 28 bytes */
  PKT_PREAMBLE_LENGTH_29BYTES           = 0xE0, /*!< Preamble length 29 bytes */
  PKT_PREAMBLE_LENGTH_30BYTES           = 0xE8, /*!< Preamble length 30 bytes */
  PKT_PREAMBLE_LENGTH_31BYTES           = 0xF0, /*!< Preamble length 31 bytes */
  PKT_PREAMBLE_LENGTH_32BYTES           = 0xF8  /*!< Preamble length 32 bytes */

}PktPreambleLength;

#define IS_PKT_PREAMBLE_LENGTH(LENGTH)  ((LENGTH == PKT_PREAMBLE_LENGTH_01BYTE) || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_02BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_03BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_04BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_05BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_06BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_07BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_08BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_09BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_10BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_11BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_12BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_13BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_14BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_15BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_16BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_17BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_18BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_19BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_20BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_21BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_22BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_23BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_24BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_25BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_26BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_27BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_28BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_29BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_30BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_31BYTES)  || \
                                           (LENGTH == PKT_PREAMBLE_LENGTH_32BYTES))



/**
 * @brief  Sync length in bytes enumeration.
 */
typedef enum
{
  PKT_SYNC_LENGTH_1BYTE            = 0x00, /*!< Sync length 1 byte*/
  PKT_SYNC_LENGTH_2BYTES           = 0x02, /*!< Sync length 2 bytes*/
  PKT_SYNC_LENGTH_3BYTES           = 0x04, /*!< Sync length 3 bytes */
  PKT_SYNC_LENGTH_4BYTES           = 0x06 , /*!< Sync length 4 bytes */

}PktSyncLength;

#define IS_PKT_SYNC_LENGTH(LENGTH)     ((LENGTH == PKT_SYNC_LENGTH_1BYTE) || \
                                          (LENGTH == PKT_SYNC_LENGTH_2BYTES)|| \
                                          (LENGTH == PKT_SYNC_LENGTH_3BYTES)|| \
                                          (LENGTH == PKT_SYNC_LENGTH_4BYTES))



/**
 * @brief  CRC length in bytes enumeration.
 */
typedef enum
{
  PKT_NO_CRC               = 0x00, /*!< No CRC                              */
  PKT_CRC_MODE_8BITS       = 0x20, /*!< CRC length 8 bits  - poly: 0x07     */
  PKT_CRC_MODE_16BITS_1    = 0x40, /*!< CRC length 16 bits - poly: 0x8005   */
  PKT_CRC_MODE_16BITS_2    = 0x60, /*!< CRC length 16 bits - poly: 0x1021   */
  PKT_CRC_MODE_24BITS      = 0x80, /*!< CRC length 24 bits - poly: 0x864CFB */

}PktCrcMode;

#define IS_PKT_CRC_MODE(MODE)   ((MODE == PKT_NO_CRC) || \
                                   (MODE == PKT_CRC_MODE_8BITS)  || \
                                   (MODE == PKT_CRC_MODE_16BITS_1)  || \
                                   (MODE == PKT_CRC_MODE_16BITS_2) || \
                                   (MODE == PKT_CRC_MODE_24BITS))



/**
 * @brief  Fixed or variable payload length enumeration.
 */
typedef enum
{
  PKT_LENGTH_FIX  = 0x00,    /*!< Fixed payload length     */
  PKT_LENGTH_VAR  = 0x01     /*!< Variable payload length  */

}PktFixVarLength;

#define IS_PKT_FIX_VAR_LENGTH(LENGTH)   ((LENGTH == PKT_LENGTH_FIX) || \
                                           (LENGTH == PKT_LENGTH_VAR))


/**
 * @brief  Control length in bytes enumeration for SPIRIT packets.
 */
typedef enum
{
  PKT_CONTROL_LENGTH_0BYTES = 0x00,     /*!< Control length 0 byte*/
  PKT_CONTROL_LENGTH_1BYTE,             /*!< Control length 1 byte*/
  PKT_CONTROL_LENGTH_2BYTES,            /*!< Control length 2 bytes*/
  PKT_CONTROL_LENGTH_3BYTES,            /*!< Control length 3 bytes*/
  PKT_CONTROL_LENGTH_4BYTES             /*!< Control length 4 bytes*/

}PktControlLength;

#define IS_PKT_CONTROL_LENGTH(LENGTH) ((LENGTH == PKT_CONTROL_LENGTH_0BYTES) || \
                                         (LENGTH == PKT_CONTROL_LENGTH_1BYTE)   || \
                                         (LENGTH == PKT_CONTROL_LENGTH_2BYTES)  || \
                                         (LENGTH == PKT_CONTROL_LENGTH_3BYTES)  || \
                                         (LENGTH == PKT_CONTROL_LENGTH_4BYTES))

/**
 * @brief  Sync words enumeration for SPIRIT packets.
 */
typedef enum
{
  PKT_SYNC_WORD_1=0x01,  /*!< Index of the 1st sync word*/
  PKT_SYNC_WORD_2,       /*!< Index of the 2nd sync word*/
  PKT_SYNC_WORD_3,       /*!< Index of the 3rd sync word*/
  PKT_SYNC_WORD_4        /*!< Index of the 4th sync word*/

}PktSyncX;

#define IS_PKT_SYNCx(WORD)    ((WORD == PKT_SYNC_WORD_1) || \
                                 (WORD == PKT_SYNC_WORD_2) || \
                                 (WORD == PKT_SYNC_WORD_3) || \
                                 (WORD == PKT_SYNC_WORD_4))



/**
 * @brief  Max retransmissions number enumeration for SPIRIT packets.
 */
typedef enum
{
  PKT_DISABLE_RETX    = 0x00,   /*!< No retrasmissions*/
  PKT_N_RETX_1        = 0x10,   /*!< Max retrasmissions 1*/
  PKT_N_RETX_2        = 0x20,   /*!< Max retrasmissions 2*/
  PKT_N_RETX_3        = 0x30,   /*!< Max retrasmissions 3*/
  PKT_N_RETX_4        = 0x40,   /*!< Max retrasmissions 4*/
  PKT_N_RETX_5        = 0x50,   /*!< Max retrasmissions 5*/
  PKT_N_RETX_6        = 0x60,   /*!< Max retrasmissions 6*/
  PKT_N_RETX_7        = 0x70,   /*!< Max retrasmissions 7*/
  PKT_N_RETX_8        = 0x80,   /*!< Max retrasmissions 8*/
  PKT_N_RETX_9        = 0x90,   /*!< Max retrasmissions 9*/
  PKT_N_RETX_10       = 0xA0,   /*!< Max retrasmissions 10*/
  PKT_N_RETX_11       = 0xB0,   /*!< Max retrasmissions 11*/
  PKT_N_RETX_12       = 0xC0,   /*!< Max retrasmissions 12*/
  PKT_N_RETX_13       = 0xD0,   /*!< Max retrasmissions 13*/
  PKT_N_RETX_14       = 0xE0,   /*!< Max retrasmissions 14*/
  PKT_N_RETX_15       = 0xF0    /*!< Max retrasmissions 15*/

}PktNMaxReTx;

#define IS_PKT_NMAX_RETX(N_RETX)    ((N_RETX == PKT_DISABLE_RETX) || \
                                       (N_RETX == PKT_N_RETX_1) || \
                                       (N_RETX == PKT_N_RETX_2) || \
                                       (N_RETX == PKT_N_RETX_3) || \
                                       (N_RETX == PKT_N_RETX_4) || \
                                       (N_RETX == PKT_N_RETX_5) || \
                                       (N_RETX == PKT_N_RETX_6) || \
                                       (N_RETX == PKT_N_RETX_7) || \
                                       (N_RETX == PKT_N_RETX_8) || \
                                       (N_RETX == PKT_N_RETX_9) || \
                                       (N_RETX == PKT_N_RETX_10) || \
                                       (N_RETX == PKT_N_RETX_11) || \
                                       (N_RETX == PKT_N_RETX_12) || \
                                       (N_RETX == PKT_N_RETX_13) || \
                                       (N_RETX == PKT_N_RETX_14) || \
                                       (N_RETX == PKT_N_RETX_15))


/**
 *@}
 */


/**
 * @defgroup PktCommon_Exported_Constants               Pkt Common Exported Constants
 * @{
 */

#define IS_PKT_LENGTH_WIDTH_BITS(BITS)                (BITS<=16)
#define IS_PKT_SEQ_NUMBER_RELOAD(SEQN)                (SEQN<=3)

/**
 *@}
 */


/**
 * @defgroup PktCommon_Exported_Macros                  Pkt Common Exported Macros
 * @{
 */


/**
 * @brief  Macro used to compute the lower part of the packet length, to write in the PCKTLEN0 register
 * @param  nLength Length of the packet payload.
 *         This parameter is an uint16_t.
 * @retval None.
 */
#define BUILD_PCKTLEN0(nLength) (nLength & 0xFF)


/**
 * @brief  Macro used to compute the upper part of the packet length, to write the PCKTLEN1 register
 * @param  nLength Length of the packet payload.
 *         This parameter is an uint16_t.
 * @retval None.
 */
#define BUILD_PCKTLEN1(nLength) (nLength >> 8)

/**
 *@}
 */


/**
 * @defgroup PktCommon_Exported_Functions               Pkt Common Exported Functions
 * @{
 */

void SpiritPktCommonSetControlLength(PktControlLength xControlLength);
uint8_t SpiritPktCommonGetControlLength(void);
void SpiritPktCommonSetPreambleLength(PktPreambleLength xPreambleLength);
uint8_t SpiritPktCommonGetPreambleLength(void);
void SpiritPktCommonSetSyncLength(PktSyncLength xSyncLength);
uint8_t SpiritPktCommonGetSyncLength(void);
void SpiritPktCommonSetFixVarLength(PktFixVarLength xFixVarLength);
void SpiritPktCommonFilterOnCrc(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritPktCommonGetFilterOnCrc(void);
void SpiritPktCommonSetCrcMode(PktCrcMode xCrcLength);
PktCrcMode SpiritPktCommonGetCrcMode(void);
void SpiritPktCommonWhitening(SpiritFunctionalState xNewState);
void SpiritPktCommonFec(SpiritFunctionalState xNewState);
void SpiritPktCommonSetSyncxWord(PktSyncX xSyncX,  uint8_t cSyncWord);
uint8_t SpiritPktCommonGetSyncxWord(PktSyncX xSyncX);
void SpiritPktCommonSetSyncWords(uint32_t lSyncWords, PktSyncLength xSyncLength);
uint32_t SpiritPktCommonGetSyncWords(PktSyncLength xSyncLength);
uint8_t SpiritPktCommonGetVarLengthWidth(void);
void SpiritPktCommonSetDestinationAddress(uint8_t cAddress);
uint8_t SpiritPktCommonGetTransmittedDestAddress(void);
void SpiritPktCommonSetMyAddress(uint8_t cAddress);
uint8_t SpiritPktCommonGetMyAddress(void);
void SpiritPktCommonSetBroadcastAddress(uint8_t cAddress);
uint8_t SpiritPktCommonGetBroadcastAddress(void);
SpiritFunctionalState SpiritPktCommonGetTxAckRequest(void);
void SpiritPktCommonSetMulticastAddress(uint8_t cAddress);
uint8_t SpiritPktCommonGetMulticastAddress(void);
void SpiritPktCommonSetCtrlMask(uint32_t lMask);
uint32_t SpiritPktCommonGetCtrlMask(void);
void SpiritPktCommonSetCtrlReference(uint32_t lReference);
uint32_t SpiritPktCommonGetCtrlReference(void);
void SpiritPktCommonSetTransmittedCtrlField(uint32_t lField);
uint32_t SpiritPktCommonGetTransmittedCtrlField(void);
void SpiritPktCommonFilterOnMyAddress(SpiritFunctionalState xNewState);
void SpiritPktCommonFilterOnMulticastAddress(SpiritFunctionalState xNewState);
void SpiritPktCommonFilterOnBroadcastAddress(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritPktCommonGetFilterOnMyAddress(void);
SpiritFunctionalState SpiritPktCommonGetFilterOnMulticastAddress(void);
SpiritFunctionalState SpiritPktCommonGetFilterOnBroadcastAddress(void);
uint8_t SpiritPktCommonGetReceivedDestAddress(void);
uint32_t SpiritPktCommonGetReceivedCtrlField(void);
void SpiritPktCommonGetReceivedCrcField(uint8_t* cCrcFieldVect);
void SpiritPktCommonAutoAck(SpiritFunctionalState xAutoAck,SpiritFunctionalState xPiggybacking);
void SpiritPktCommonRequireAck(SpiritFunctionalState xRequireAck);
void SpiritPktCommonSetTransmittedSeqNumberReload(uint8_t cSeqNumberReload);
void SpiritPktCommonSetNMaxReTx(PktNMaxReTx xNMaxReTx);
uint8_t SpiritPktCommonGetNMaxReTx(void);
uint8_t SpiritPktCommonGetReceivedDestAddress(void);
uint8_t SpiritPktCommonGetReceivedSourceAddress(void);
uint8_t SpiritPktCommonGetReceivedSeqNumber(void);
uint8_t SpiritPktCommonGetReceivedNackRx(void);
uint8_t SpiritPktCommonGetTransmittedSeqNumber(void);
uint8_t SpiritPktCommonGetNReTx(void);
void SpiritPktCommonFilterOnControlField(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritPktCommonGetFilterOnControlField(void);

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
