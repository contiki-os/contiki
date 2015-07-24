/**
  ******************************************************************************
  * @file    SPIRIT_PktCommon.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of the common features of SPIRIT packets.
  * @details
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

/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_PktCommon.h"
#include "MCU_Interface.h"


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_PktCommon
 * @{
 */


/**
 * @defgroup PktCommon_Private_TypesDefinitions         Pkt Common Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktCommon_Private_Defines                  Pkt Common Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktCommon_Private_Macros                   Pkt Common Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktCommon_Private_Variables                Pkt Common Private Variables
 * @{
 */

/**
 *@}
 */



/**
 * @defgroup PktCommon_Private_FunctionPrototypes       Pkt Common Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktCommon_Private_Functions                Pkt Common Private Functions
 * @{
 */

/**
 * @brief  Sets the CONTROL field length for SPIRIT packets.
 * @param  xControlLength length of CONTROL field in bytes.
 *         This parameter can be any value of @ref PktControlLength.
 * @retval None.
 */
void SpiritPktCommonSetControlLength(PktControlLength xControlLength)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_CONTROL_LENGTH(xControlLength));

  /* Reads the PCKTCTRL4 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

  /* Set the control length */
  tempRegValue &= ~PCKTCTRL4_CONTROL_LEN_MASK;
  tempRegValue |= (uint8_t)xControlLength;

  /* Writes the new value on the PCKTCTRL4 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the CONTROL field length for SPIRIT packets.
 * @param  None.
 * @retval uint8_t Control field length.
 */
uint8_t SpiritPktCommonGetControlLength(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL4 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

  /* Rebuild and return value */
  return (tempRegValue & PCKTCTRL4_CONTROL_LEN_MASK);

}


/**
 * @brief  Sets the PREAMBLE field Length mode for SPIRIT packets.
 * @param  xPreambleLength length of PREAMBLE field in bytes.
 *         This parameter can be any value of @ref PktPreambleLength.
 * @retval None.
 */
void SpiritPktCommonSetPreambleLength(PktPreambleLength xPreambleLength)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_PREAMBLE_LENGTH(xPreambleLength));

  /* Reads the PCKTCTRL2 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

  /* Set the preamble length */
  tempRegValue &= ~PCKTCTRL2_PREAMBLE_LENGTH_MASK;
  tempRegValue |= (uint8_t)xPreambleLength;

  /* Writes the new value on the PCKTCTRL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the PREAMBLE field Length mode for SPIRIT packets.
 * @param  None.
 * @retval uint8_t Preamble field length in bytes.
 */
uint8_t SpiritPktCommonGetPreambleLength(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL2 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

  /* Rebuild and return value */
  return ((tempRegValue & PCKTCTRL2_PREAMBLE_LENGTH_MASK)>>3) + 1;

}


/**
 * @brief  Sets the SYNC field Length for SPIRIT packets.
 * @param  xSyncLength length of SYNC field in bytes.
 *         This parameter can be any value of @ref PktSyncLength.
 * @retval None.
 */
void SpiritPktCommonSetSyncLength(PktSyncLength xSyncLength)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_SYNC_LENGTH(xSyncLength));

  /* Reads the PCKTCTRL2 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

  /* Set the sync length */
  tempRegValue &= ~PCKTCTRL2_SYNC_LENGTH_MASK;
  tempRegValue |= (uint8_t)xSyncLength;

  /* Writes the new value on the PCKTCTRL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the SYNC field Length for SPIRIT packets.
 * @param  None.
 * @retval uint8_t Sync field length in bytes.
 */
uint8_t SpiritPktCommonGetSyncLength(void)
{
  uint8_t tempRetValue;

  /* Reads the PCKTCTRL2 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL2_BASE, 1, &tempRetValue);

  /* Rebuild and return value */
  return ((tempRetValue & PCKTCTRL2_SYNC_LENGTH_MASK)>>1) + 1;

}


/**
 * @brief  Sets fixed or variable payload length mode for SPIRIT packets.
 * @param  xFixVarLength variable or fixed length.
 *         PKT_FIXED_LENGTH_VAR -> variable (the length is extracted from the received packet).
 *         PKT_FIXED_LENGTH_FIX -> fix (the length is set by PCKTLEN0 and PCKTLEN1).
 * @retval None.
 */
void SpiritPktCommonSetFixVarLength(PktFixVarLength xFixVarLength)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_FIX_VAR_LENGTH(xFixVarLength));

  /* Reads the PCKTCTRL2 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

  /* Set fixed or variable address mode */
  tempRegValue &= ~PCKTCTRL2_FIX_VAR_LEN_MASK;
  tempRegValue |= (uint8_t)xFixVarLength;

  /* Writes the new value on the PCKTCTRL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Enables or Disables the filtering on CRC.
 * @param  xNewState new state for CRC_CHECK.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonFilterOnCrc(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PCKT_FLT_OPTIONS register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Modify the register value: enable or disable the CRC filtering */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKT_FLT_OPTIONS_CRC_CHECK_MASK;
  }
  else
  {
    tempRegValue &= ~PCKT_FLT_OPTIONS_CRC_CHECK_MASK;
  }

  /* Writes the PCKT_FLT_OPTIONS register value */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the CRC filtering enable bit.
 * @param  None.
 * @retval SpiritFunctionalState CRC filtering.
 */
SpiritFunctionalState SpiritPktCommonGetFilterOnCrc(void)
{
  uint8_t tempRegValue;


  /* Reads the PCKT_FLT_OPTIONS register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Check the CRC filtering bit */
  if(tempRegValue & PCKT_FLT_OPTIONS_CRC_CHECK_MASK)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}


/**
 * @brief  Sets the CRC mode for SPIRIT packets.
 * @param  xCrcMode length of CRC field in bytes.
 *         This parameter can be any value of @ref PktCrcMode.
 * @retval None.
 */
void SpiritPktCommonSetCrcMode(PktCrcMode xCrcMode)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_CRC_MODE(xCrcMode));

  /* Reads the PCKTCTRL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build data to write setting the CRC mode */
  tempRegValue &= ~PCKTCTRL1_CRC_MODE_MASK;
  tempRegValue |= (uint8_t)xCrcMode;

  /* Writes the new value on the PCKTCTRL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the CRC mode for SPIRIT packets.
 * @param  None.
 * @retval PktCrcMode Crc mode.
 */
PktCrcMode SpiritPktCommonGetCrcMode(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL1 register */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Rebuild and return value */
  return (PktCrcMode)(tempRegValue & 0xE0);

}


/**
 * @brief  Enables or Disables WHITENING for SPIRIT packets.
 * @param  xNewState new state for WHITENING mode.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonWhitening(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PCKTCTRL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build data to write: set or reset the whitening enable bit */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKTCTRL1_WHIT_MASK;
  }
  else
  {
    tempRegValue &= ~PCKTCTRL1_WHIT_MASK;
  }

  /* Writes the new value on the PCKTCTRL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Enables or Disables FEC for SPIRIT packets.
 * @param  xNewState new state for FEC mode.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonFec(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PCKTCTRL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build data to write: set or reset the FEC enable bit */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKTCTRL1_FEC_MASK;
  }
  else
  {
    tempRegValue &= ~PCKTCTRL1_FEC_MASK;
  }

  /* Writes data on the PCKTCTRL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets a specific SYNC word for SPIRIT packets.
 * @param  xSyncX SYNC word number to be set.
 *         This parameter can be any value of @ref PktSyncX.
 * @param  cSyncWord SYNC word.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktCommonSetSyncxWord(PktSyncX xSyncX ,  uint8_t cSyncWord)
{
  uint8_t tempRegAddress;

  /* Check the parameters */
  s_assert_param(IS_PKT_SYNCx(xSyncX));

  /* Set the specified address */
  switch(xSyncX)
  {
    case PKT_SYNC_WORD_1:
      tempRegAddress=SYNC1_BASE;
      break;
    case PKT_SYNC_WORD_2:
      tempRegAddress=SYNC2_BASE;
      break;
    case PKT_SYNC_WORD_3:
      tempRegAddress=SYNC3_BASE;
      break;
    case PKT_SYNC_WORD_4:
      tempRegAddress=SYNC4_BASE;
      break;
  }

  /* Writes value on the selected register */
  g_xStatus = SpiritSpiWriteRegisters(tempRegAddress, 1, &cSyncWord);

}


/**
 * @brief  Returns a specific SYNC word for SPIRIT packets.
 * @param  xSyncX SYNC word number to be get.
 *         This parameter can be any value of @ref PktSyncX.
 * @retval uint8_t Sync word x.
 */
uint8_t SpiritPktCommonGetSyncxWord(PktSyncX xSyncX)
{
  uint8_t tempRegAddress, tempRegValue;

  /* Set the specified address */
  switch(xSyncX)
  {
    case PKT_SYNC_WORD_1:
      tempRegAddress=SYNC1_BASE;
      break;
    case PKT_SYNC_WORD_2:
      tempRegAddress=SYNC2_BASE;
      break;
    case PKT_SYNC_WORD_3:
      tempRegAddress=SYNC3_BASE;
      break;
    case PKT_SYNC_WORD_4:
      tempRegAddress=SYNC4_BASE;
      break;
  }

  /* Reads the selected register value */
  g_xStatus = SpiritSpiReadRegisters(tempRegAddress, 1, &tempRegValue);

  /* Returns the read value */
  return tempRegValue;

}


/**
 * @brief  Sets multiple SYNC words for SPIRIT packets.
 * @param  lSyncWords SYNC words to be set with format: 0x|SYNC1|SYNC2|SYNC3|SYNC4|.
 *         This parameter is a uint32_t.
 * @param  xSyncLength SYNC length in bytes. The 32bit word passed will be stored in the SYNCx registers from the MSb
 *         until the number of bytes in xSyncLength has been stored.
 *         This parameter is a @ref PktSyncLength.
 * @retval None.
 */
void SpiritPktCommonSetSyncWords(uint32_t lSyncWords, PktSyncLength xSyncLength)
{
  uint8_t tempRegValue[4];

  /* Split the 32-bit value in 4 8-bit values */
  for(uint8_t i=0 ; i<4 ; i++)
  {
    if(i<3-xSyncLength>>1)
    {
      tempRegValue[i]=0;
    }
    else
    {
      tempRegValue[i]=(uint8_t)(lSyncWords>>(8*i));
    }
  }

  /* Writes SYNC value on the SYNCx registers */
  g_xStatus = SpiritSpiWriteRegisters(SYNC4_BASE, 4, tempRegValue);

}


/**
 * @brief  Returns multiple SYNC words for SPIRIT packets.
 * @param  xSyncLength SYNC length in bytes. The 32bit word passed will be stored in the SYNCx registers from the MSb
 *         until the number of bytes in xSyncLength has been stored.
 *         This parameter is a pointer to @ref PktSyncLength.
 * @retval uint32_t Sync words. The format of the read 32 bit word is 0x|SYNC1|SYNC2|SYNC3|SYNC4|.
 */
uint32_t SpiritPktCommonGetSyncWords(PktSyncLength xSyncLength)
{
  uint8_t tempRegValue[4];
  uint32_t tempRetValue=0;

  /* Reads the SYNCx registers value */
  g_xStatus = SpiritSpiReadRegisters(SYNC4_BASE, 4, tempRegValue);

  /* Rebuild the SYNC words */
  for(uint8_t i=0 ; i<4 ; i++)
  {
    if(i>2-(xSyncLength >>1))
    {
      tempRetValue |= tempRegValue[i]<<(8*i);
    }
  }

  /* Return SYNC words */
  return tempRetValue;

}


/**
 * @brief  Returns the variable length width (in number of bits).
 * @param  None.
 * @retval uint8_t Variable length width in bits.
 */
uint8_t SpiritPktCommonGetVarLengthWidth(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Rebuild and return value */
  return (tempRegValue & PCKTCTRL3_LEN_WID_MASK)+1;

}


/**
 * @brief  Sets the destination address for the Tx packet.
 * @param  cAddress Destination address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktCommonSetDestinationAddress(uint8_t cAddress)
{
  /* Writes value on PCKT_FLT_GOALS_SOURCE_ADDR register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_SOURCE_ADDR_BASE, 1, &cAddress);

}


/**
 * @brief  Returns the settled destination address.
 * @param  None.
 * @retval uint8_t Transmitted destination address.
 */
uint8_t SpiritPktCommonGetTransmittedDestAddress(void)
{
  uint8_t tempRegValue;

  /* Reads value on the PCKT_FLT_GOALS_SOURCE_ADDR register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_SOURCE_ADDR_BASE, 1, &tempRegValue);

  /* Return value */
  return tempRegValue;

}


/**
 * @brief  Sets the node my address. When the filtering on my address is on, if the destination address extracted from the received packet is equal to the content of the
 *         my address, then the packet is accepted (this is the address of the node).
 * @param  cAddress Address of the present node.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktCommonSetMyAddress(uint8_t cAddress)
{
  /* Writes value on the PCKT_FLT_GOALS_TX_ADDR register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_TX_ADDR_BASE, 1, &cAddress);

}


/**
 * @brief  Returns the address of the present node.
 * @param  None.
 * @retval uint8_t My address (address of this node).
 */
uint8_t SpiritPktCommonGetMyAddress(void)
{
  uint8_t tempRegValue;

  /* Reads value on the PCKT_FLT_GOALS_TX_ADDR register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_TX_ADDR_BASE, 1, &tempRegValue);

  /* Return value */
  return tempRegValue;

}


/**
 * @brief  Sets the broadcast address. If the destination address extracted from the received packet is equal to the content of the
 *         BROADCAST_ADDR register, then the packet is accepted.
 * @param  cAddress Broadcast address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktCommonSetBroadcastAddress(uint8_t cAddress)
{
  /* Writes value on the PCKT_FLT_GOALS_BROADCAST register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_BROADCAST_BASE, 1, &cAddress);

}


/**
 * @brief  Returns the broadcast address.
 * @param  None.
 * @retval uint8_t Broadcast address.
 */
uint8_t SpiritPktCommonGetBroadcastAddress(void)
{
  uint8_t tempRegValue;

  /* Reads value on the PCKT_FLT_GOALS_BROADCAST register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_BROADCAST_BASE, 1, &tempRegValue);

  /* Return value */
  return tempRegValue;

}


/**
 * @brief  Sets the multicast address. When the multicast filtering is on, if the destination address extracted from the received packet is equal to the content of the
 *         MULTICAST_ADDR register, then the packet is accepted.
 * @param  cAddress Multicast address.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktCommonSetMulticastAddress(uint8_t cAddress)
{
  /* Writes value on the PCKT_FLT_GOALS_MULTICAST register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_MULTICAST_BASE, 1, &cAddress);

}


/**
 * @brief  Returns the multicast address.
 * @param  None.
 * @retval uint8_t Multicast address.
 */
uint8_t SpiritPktCommonGetMulticastAddress(void)
{
  uint8_t tempRegValue;

  /* Reads value on the PCKT_FLT_GOALS_MULTICAST register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_MULTICAST_BASE, 1, &tempRegValue);

  /* Return value */
  return tempRegValue;

}


/**
 * @brief  Sets the control mask. The 1 bits of the CONTROL_MASK indicate the
 *         bits to be used in filtering. (All 0s no filtering)
 * @param  lMask Control mask.
 *         This parameter is an uint32_t.
 * @retval None.
 */
void SpiritPktCommonSetCtrlMask(uint32_t lMask)
{
  uint8_t tempRegValue[4];

  /* Split the 32-bit value in 4 8-bit values */
  tempRegValue[0] = (uint8_t) lMask;
  tempRegValue[1] = (uint8_t)(lMask >> 8);
  tempRegValue[2] = (uint8_t)(lMask >> 16);
  tempRegValue[3] = (uint8_t)(lMask >> 24);

  /* Writes values on the CKT_FLT_GOALS_CONTROLx_MASK registers */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_CONTROL0_MASK_BASE, 4, tempRegValue);

}


/**
 * @brief  Returns the control mask. The 1 bits of the CONTROL_MASK indicate the
 *         bits to be used in filtering. (All 0s no filtering)
 * @param  None.
 * @retval uint32_t Control mask.
 */
uint32_t SpiritPktCommonGetCtrlMask(void)
{
  uint8_t tempRegValue[4];
  uint32_t tempRetValue=0;

  /* Reads the PCKT_FLT_GOALS_CONTROLx_MASK registers */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_CONTROL0_MASK_BASE, 4, tempRegValue);

  /* Rebuild the control mask value on a 32-bit integer variable */
  for(uint8_t i=0 ; i<4 ; i++)
  {
    tempRetValue |= ((uint32_t)tempRegValue[i])<<(8*i);
  }

  /* Return value */
  return tempRetValue;
}

/**
 * @brief  Sets the control field reference. If the bits enabled by the CONTROL_MASK
 *         match the ones of the control fields extracted from the received packet
 *         then the packet is accepted.
 * @param  lReference Control reference.
 *         This parameter is an uint32_t.
 * @retval None.
 */
void SpiritPktCommonSetCtrlReference(uint32_t lReference)
{
  uint8_t tempRegValue[4];

  /* Split the 32-bit value in 4 8-bit values */
  tempRegValue[0] = (uint8_t) lReference;
  tempRegValue[1] = (uint8_t)(lReference >> 8);
  tempRegValue[2] = (uint8_t)(lReference >> 16);
  tempRegValue[3] = (uint8_t)(lReference >> 24);

  /* Writes values on the CKT_FLT_GOALS_CONTROLx_FIELD registers */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_CONTROL0_FIELD_BASE, 4, tempRegValue);

}


/**
 * @brief  Returns the control field reference.
 * @param  None.
 * @retval uint32_t Control reference.
 */
uint32_t SpiritPktCommonGetCtrlReference(void)
{
  uint8_t tempRegValue[4];
  uint32_t tempRetValue=0;

  /* Reads the PCKT_FLT_GOALS_CONTROLx_FIELD registers */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_CONTROL0_FIELD_BASE, 4, tempRegValue);

  /* Rebuild the control mask value on a 32-bit integer variable */
  for(uint8_t i=0 ; i<4 ; i++)
  {
    tempRetValue |= ((uint32_t)tempRegValue[i])<<(8*i);
  }

  /* Return value */
  return tempRetValue;
}


/**
 * @brief  Sets the TX control field.
 * @param  lField Tx contro field.
 *         This parameter is an uint32_t.
 * @retval None.
 */
void SpiritPktCommonSetTransmittedCtrlField(uint32_t lField)
{
  uint8_t tempRegValue[4];

  /* Split the 32-bit value in 4 8-bit values */
  tempRegValue[3] = (uint8_t) lField;
  tempRegValue[2] = (uint8_t)(lField >> 8);
  tempRegValue[1] = (uint8_t)(lField >> 16);
  tempRegValue[0] = (uint8_t)(lField >> 24);

  /* Writes value on the TX_CTRL_FIELDx register */
  g_xStatus = SpiritSpiWriteRegisters(TX_CTRL_FIELD3_BASE, 4, tempRegValue);

}


/**
 * @brief  Returns the Tx control field.
 * @param  None.
 * @retval uint32_t Control field of the transmitted packet.
 */
uint32_t SpiritPktCommonGetTransmittedCtrlField(void)
{
  uint8_t tempRegValue[4];
  uint32_t tempRetValue=0;

  /* Reads the TX_CTRL_FIELDx registers */
  g_xStatus = SpiritSpiReadRegisters(TX_CTRL_FIELD3_BASE, 4, tempRegValue);

  /* Rebuild value: build a 32-bit value from the read bytes */
  for(uint8_t i=0 ; i<4 ; i++)
  {
    tempRetValue |= ((uint32_t)tempRegValue[i])<<(8*(3-i));
  }

  /* Return value */
  return tempRetValue;

}


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with My address.
 * @param  xNewState new state for DEST_VS_SOURCE_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonFilterOnMyAddress(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

   /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));


  /* Modify the register value: set or reset the TX source address control */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Set or reset the DESTINATION vs TX enabling bit */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKT_FLT_OPTIONS_DEST_VS_TX_ADDR_MASK;
  }
  else
  {
    tempRegValue &= ~PCKT_FLT_OPTIONS_DEST_VS_TX_ADDR_MASK;
  }

  /* Writes the new value on the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

}


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with multicast address.
 * @param  xNewState new state for DEST_VS_MULTICAST_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonFilterOnMulticastAddress(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PCKT_FLT_OPTIONS register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Enable or disable the filtering option */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKT_FLT_OPTIONS_DEST_VS_MULTICAST_ADDR_MASK;
  }
  else
  {
    tempRegValue &= ~PCKT_FLT_OPTIONS_DEST_VS_MULTICAST_ADDR_MASK;
  }

  /* Writes the new value on the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

}


/**
 * @brief  If enabled RX packet is accepted if its destination address matches with broadcast address.
 * @param  xNewState new state for DEST_VS_BROADCAST_ADDRESS.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonFilterOnBroadcastAddress(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Enable or disable the filtering option */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKT_FLT_OPTIONS_DEST_VS_BROADCAST_ADDR_MASK;
  }
  else
  {
    tempRegValue &= ~PCKT_FLT_OPTIONS_DEST_VS_BROADCAST_ADDR_MASK;
  }

  /* Writes the new value on the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the enable bit of the my address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
SpiritFunctionalState SpiritPktCommonGetFilterOnMyAddress(void)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Gets the enable/disable bit in form of SpiritFunctionalState type */
  if(tempRegValue & 0x08)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}

/**
 * @brief  Returns the enable bit of the multicast address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
SpiritFunctionalState SpiritPktCommonGetFilterOnMulticastAddress(void)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Get the enable/disable bit in form of SpiritFunctionalState type */
  if(tempRegValue & 0x04)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}

/**
 * @brief  Returns the enable bit of the broadcast address filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
SpiritFunctionalState SpiritPktCommonGetFilterOnBroadcastAddress(void)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Get the enable/disable bit in form of SpiritFunctionalState type */
  if(tempRegValue & 0x02)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}


/**
 * @brief  Returns the destination address of the received packet.
 * @param  None.
 * @retval uint8_t Destination address of the received address.
 */
uint8_t SpiritPktCommonGetReceivedDestAddress(void)
{
  uint8_t tempRegValue;

  /* Reads the RX_ADDR_FIELD0 register value */
  g_xStatus = SpiritSpiReadRegisters(RX_ADDR_FIELD0_BASE, 1, &tempRegValue);

  /* Return value */
  return tempRegValue;

}


/**
 * @brief  Returns the control field of the received packet.
 * @param  None.
 * @retval uint32_t Received control field.
 */
uint32_t SpiritPktCommonGetReceivedCtrlField(void)
{
  uint8_t tempRegValue[4];
  uint32_t tempRetValue=0;

  /* Reads the PCKT_FLT_GOALS_CONTROLx_MASK registers */
  g_xStatus = SpiritSpiReadRegisters(RX_CTRL_FIELD0_BASE, 4, tempRegValue);

  /* Rebuild the control mask value on a 32-bit integer variable */
  for(uint8_t i=0 ; i<4 ; i++)
  {
    tempRetValue |= ((uint32_t)tempRegValue[i])<<(8*i);
  }

  /* Returns value */
  return tempRetValue;
}


/**
 * @brief  Returns the CRC field of the received packet.
 * @param  cCrcFieldVect array in which the CRC field has to be stored.
 *         This parameter is an uint8_t array of 3 elements.
 * @retval None.
 */
void SpiritPktCommonGetReceivedCrcField(uint8_t* cCrcFieldVect)
{
  uint8_t tempRegValue[3],crcLength;
  PktCrcMode crcMode;
  
  /* Gets the CRC mode in PktCrcMode enum */
  crcMode=SpiritPktCommonGetCrcMode();
  
  /* Cast to uint8_t */
  crcLength = (uint8_t)crcMode;
  
  /* Obtains the real length: see the @ref PktCrcMode enumeration */
  crcLength >>= 5;
  if(crcLength>=3) crcLength--;
  
  /* Reads the CRC_FIELDx registers value */
  g_xStatus = SpiritSpiReadRegisters(CRC_FIELD2_BASE, 3,tempRegValue);
  
  /* Sets the array to be returned */
  for(uint8_t i=0 ; i<3 ; i++)
  {
    if(i<crcLength) 
    {
      cCrcFieldVect[i]=tempRegValue[2-i];
    }
    else 
    {
      cCrcFieldVect[i]=0;
    }
  }
  
}


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
void SpiritPktCommonAutoAck(SpiritFunctionalState xAutoAck , SpiritFunctionalState xPiggybacking)
{
  uint8_t tempRegValue[2];

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xAutoAck));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xPiggybacking));
  /* Check if piggybacking is enabled and autoack is disabled */
  s_assert_param(!(xPiggybacking==S_ENABLE && xAutoAck==S_DISABLE));

  /* Reads the PROTOCOL[1:0] registers value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 2, tempRegValue);

  /* Sets the specified LLP option */
  /* Autoack setting */
  if(xAutoAck == S_ENABLE)
  {
    tempRegValue[1] |= PROTOCOL0_AUTO_ACK_MASK;
  }
  else
  {
    tempRegValue[1] &= (~PROTOCOL0_AUTO_ACK_MASK);
  }

  /* Piggybacking setting */
  if(xPiggybacking == S_ENABLE)
  {
    tempRegValue[0] |= PROTOCOL1_PIGGYBACKING_MASK;
  }
  else
  {
    tempRegValue[0] &= (~PROTOCOL1_PIGGYBACKING_MASK);
  }

  /* Writes data on the PROTOCOL[1:0] registers */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the AUTO ACKNOLEDGEMENT mechanism on the transmitter. On the transmitter side, the NACK_TX field can be used to require or not an acknowledgment for each individual packet: if
 *         NACK_TX is set to "1" then acknowledgment will not be required; if NACK_TX is set to "0" then acknowledgment will be
 *         required.
 * @param  xNewState new state for TX_AUTOACK.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonRequireAck(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads value on the PROTOCOL0 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL0_BASE, 1, &tempRegValue);

  /* Enables or disables the ack requirement option */
  if(xNewState == S_DISABLE)
  {
    tempRegValue |= PROTOCOL0_NACK_TX_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL0_NACK_TX_MASK;
  }

  /* Writes value on the PROTOCOL0 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL0_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets the TX sequence number to be used to start counting.
 * @param  cSeqNumberReload new value for Tx seq number reload.
 * @retval None.
 */
void SpiritPktCommonSetTransmittedSeqNumberReload(uint8_t cSeqNumberReload){
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_SEQ_NUMBER_RELOAD(cSeqNumberReload));

  /* Reads value on the PROTOCOL2 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

  tempRegValue &= 0xE7;
  tempRegValue |= (cSeqNumberReload << 3);

  /* Writes value on the PROTOCOL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets the TX sequence number to be used to start counting.
 * @param  cSeqNumberReload new value for Tx seq number reload.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktCommonSetNMaxReTx(PktNMaxReTx xNMaxReTx)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PKT_NMAX_RETX(xNMaxReTx));

  /* Reads the PROTOCOL0 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL0_BASE, 1, &tempRegValue);

  /* Build the value to be written */
  tempRegValue &= ~PROTOCOL0_NMAX_RETX_MASK;
  tempRegValue |= xNMaxReTx;

  /* Writes value on the PROTOCOL0 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL0_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the max number of automatic retransmission.
 * @param  None.
 * @retval uint8_t Max number of retransmissions.
 *         This parameter is an uint8_t.
 */
uint8_t SpiritPktCommonGetNMaxReTx(void)
{
  uint8_t tempRegValue;

  /* Reads the PROTOCOL0 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL0_BASE, 1, &tempRegValue);

  /* Build the value to be written */
  return ((tempRegValue & PROTOCOL0_NMAX_RETX_MASK)>>4);

}

/**
 * @brief  Returns the TX ACK request
 * @param  None.
 * @retval uint8_t Max number of retransmissions.
 *         This parameter is an uint8_t.
 */
SpiritFunctionalState SpiritPktCommonGetTxAckRequest(void)
{
  uint8_t tempRegValue;

  /* Reads the PROTOCOL0 register value */
  g_xStatus = SpiritSpiReadRegisters(RX_PCKT_INFO_BASE, 1, &tempRegValue);

  /* Build the value to be written */
  return (SpiritFunctionalState)((tempRegValue & TX_PCKT_INFO_NACK_RX)>>2);

}
   
   
/**
 * @brief  Returns the source address of the received packet.
 * @param  None.
 * @retval uint8_t Source address of the received packet.
 */
uint8_t SpiritPktCommonGetReceivedSourceAddress(void)
{
  uint8_t tempRegValue;

  /* Reads the RX_ADDR_FIELD1 register value */
  g_xStatus = SpiritSpiReadRegisters(RX_ADDR_FIELD1_BASE, 1, &tempRegValue);

  /* Returns value */
  return tempRegValue;

}


/**
 * @brief  Returns the sequence number of the received packet.
 * @param  None.
 * @retval uint8_t Received Sequence number.
 */
uint8_t SpiritPktCommonGetReceivedSeqNumber(void)
{
  uint8_t tempRegValue;

  /* Reads the RX_PCKT_INFO register value */
  g_xStatus = SpiritSpiReadRegisters(RX_PCKT_INFO_BASE, 1, &tempRegValue);

  /* Obtains and returns the sequence number */
  return tempRegValue & 0x03;

}


/**
 * @brief  Returns the Nack bit of the received packet
 * @param  None.
 * @retval uint8_t Value of the Nack bit.
 */
uint8_t SpiritPktCommonGetReceivedNackRx(void)
{
  uint8_t tempRegValue;

  /* Reads the RX_PCKT_INFO register value */
  g_xStatus = SpiritSpiReadRegisters(RX_PCKT_INFO_BASE, 1, &tempRegValue);

  /* Obtains and returns the RX nack bit */
  return (tempRegValue >> 2) & 0x01;

}


/**
 * @brief  Returns the sequence number of the transmitted packet.
 * @param  None.
 * @retval uint8_t Sequence number of the transmitted packet.
 */
uint8_t SpiritPktCommonGetTransmittedSeqNumber(void)
{
  uint8_t tempRegValue;

  /* Reads the TX_PCKT_INFO register value */
  g_xStatus = SpiritSpiReadRegisters(TX_PCKT_INFO_BASE, 1, &tempRegValue);

  /* Obtains and returns the TX sequence number */
  return (tempRegValue >> 4) & 0x07;

}


/**
 * @brief  Returns the number of retransmission done on the transmitted packet.
 * @param  None.
 * @retval uint8_t Number of retransmissions done until now.
 */
uint8_t SpiritPktCommonGetNReTx(void)
{
  uint8_t tempRetValue;

  /* Reads the TX_PCKT_INFO register value */
  g_xStatus = SpiritSpiReadRegisters(TX_PCKT_INFO_BASE, 1, &tempRetValue);

  /* Obtains and returns the number of retransmission done */
  return (tempRetValue & 0x0F);

}


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
void SpiritPktCommonFilterOnControlField(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

   /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));


  /* Modify the register value: set or reset the control bit filtering */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Set or reset the CONTROL filtering enabling bit */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKT_FLT_OPTIONS_CONTROL_FILTERING_MASK;
  }
  else
  {
    tempRegValue &= ~PCKT_FLT_OPTIONS_CONTROL_FILTERING_MASK;
  }

  /* Writes the new value on the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the enable bit of the control field filtering.
 * @param  None.
 * @retval SpiritFunctionalState This parameter can be S_ENABLE or S_DISABLE.
 */
SpiritFunctionalState SpiritPktCommonGetFilterOnControlField(void)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Gets the enable/disable bit in form of SpiritFunctionalState type */
  if(tempRegValue & PCKT_FLT_OPTIONS_CONTROL_FILTERING_MASK)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}


/**
 *@}
 */

/**
 *@}
 */


/**
 *@}
 */


/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
