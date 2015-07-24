/**
  ******************************************************************************
  * @file    SPIRIT_PktBasic.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT Basic packets.
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
#include "SPIRIT_PktBasic.h"
#include "MCU_Interface.h"


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_PktBasic
 * @{
 */


/**
 * @defgroup PktBasic_Private_TypesDefinitions  Pkt Basic Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktBasic_Private_Defines           Pkt Basic Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktBasic_Private_Macros            Pkt Basic Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktBasic_Private_Variables          Pkt Basic Private Variables
 * @{
 */

/**
 *@}
 */



/**
 * @defgroup PktBasic_Private_FunctionPrototypes        Pkt Basic Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktBasic_Private_Functions                 Pkt Basic Private Functions
 * @{
 */

/**
 * @brief  Initializes the SPIRIT Basic packet according to the specified parameters in the PktBasicInit struct.
 *         Notice that this function sets the autofiltering option on CRC if it is set to any value different from BASIC_NO_CRC.
 * @param  pxPktBasicInit Basic packet init structure.
 *         This parameter is a pointer to @ref PktBasicInit.
 * @retval None.
 */
void SpiritPktBasicInit(PktBasicInit* pxPktBasicInit)
{
  uint8_t tempRegValue[4], i;

  /* Check the parameters */
  s_assert_param(IS_BASIC_PREAMBLE_LENGTH(pxPktBasicInit->xPreambleLength));
  s_assert_param(IS_BASIC_SYNC_LENGTH(pxPktBasicInit->xSyncLength));
  s_assert_param(IS_BASIC_CRC_MODE(pxPktBasicInit->xCrcMode));
  s_assert_param(IS_BASIC_LENGTH_WIDTH_BITS(pxPktBasicInit->cPktLengthWidth));
  s_assert_param(IS_BASIC_FIX_VAR_LENGTH(pxPktBasicInit->xFixVarLength));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktBasicInit->xAddressField));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktBasicInit->xFec));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktBasicInit->xDataWhitening));
  s_assert_param(IS_BASIC_CONTROL_LENGTH(pxPktBasicInit->xControlLength));

  /* Reads the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue[0]);

  /* Mask a reserved bit */
  tempRegValue[0] &= ~0x20;

  /* Always set the automatic packet filtering */
  tempRegValue[0] |= PROTOCOL1_AUTO_PCKT_FLT_MASK;

  /* Writes the value on register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue[0]);

  /* Reads the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);

  /* Always reset the control and source filtering (also if it is not present in basic) */
  tempRegValue[0] &= ~(PCKT_FLT_OPTIONS_SOURCE_FILTERING_MASK | PCKT_FLT_OPTIONS_CONTROL_FILTERING_MASK);

  /* Writes the value on register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);

  if(pxPktBasicInit->xAddressField == S_ENABLE)
  {
    tempRegValue[0]=0x08;
  }
  else
  {
    tempRegValue[0]=0x00;
  }
  /* Address and control length setting */
  tempRegValue[0] |= ((uint8_t) pxPktBasicInit->xControlLength);

  /* Packet format and width length setting */
  pxPktBasicInit->cPktLengthWidth == 0 ? pxPktBasicInit->cPktLengthWidth=1 : pxPktBasicInit->cPktLengthWidth;
  tempRegValue[1] = ((uint8_t) PCKTCTRL3_PCKT_FRMT_BASIC) | ((uint8_t)(pxPktBasicInit->cPktLengthWidth-1));

  /* Preamble, sync and fixed or variable length setting */
  tempRegValue[2] = ((uint8_t) pxPktBasicInit->xPreambleLength) | ((uint8_t) pxPktBasicInit->xSyncLength) |
                    ((uint8_t) pxPktBasicInit->xFixVarLength);

  /* CRC length, whitening and FEC setting */
  tempRegValue[3] = (uint8_t) pxPktBasicInit->xCrcMode;

  if(pxPktBasicInit->xDataWhitening == S_ENABLE)
  {
     tempRegValue[3] |= PCKTCTRL1_WHIT_MASK;
  }

  if(pxPktBasicInit->xFec == S_ENABLE)
  {
     tempRegValue[3] |= PCKTCTRL1_FEC_MASK;
  }

  /* Writes registers */
  SpiritSpiWriteRegisters(PCKTCTRL4_BASE, 4, tempRegValue);

  /* Sync words setting */
  for(i=0;i<4;i++)
  {
    if(i<3-(pxPktBasicInit->xSyncLength >>1))
    {
      tempRegValue[i]=0;
    }
    else
    {
      tempRegValue[i] = (uint8_t)(pxPktBasicInit->lSyncWords>>(8*i));
    }
  }

  /* Sets CRC check bit */
  if(pxPktBasicInit->xCrcMode == PKT_NO_CRC)
  {
    SpiritPktBasicFilterOnCrc(S_DISABLE);
  }
  else
  {
    SpiritPktBasicFilterOnCrc(S_ENABLE);
  }

  
  g_xStatus = SpiritSpiWriteRegisters(SYNC4_BASE, 4, tempRegValue);

}


/**
 * @brief  Returns the SPIRIT Basic packet structure according to the specified parameters in the registers.
 * @param  pxPktBasicInit Basic packet init structure.
 *         This parameter is a pointer to @ref PktBasicInit.
 * @retval None.
 */
void SpiritPktBasicGetInfo(PktBasicInit* pxPktBasicInit)
{
  uint8_t tempRegValue[10];

  /* Reads registers */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 10, tempRegValue);

  /* Length width */
  pxPktBasicInit->cPktLengthWidth=(tempRegValue[1] & 0x0F)+1;

  /* Address field */
  pxPktBasicInit->xAddressField=(SpiritFunctionalState)((tempRegValue[0]>>3) & 0x01);

  /* Control length */
  pxPktBasicInit->xControlLength=(BasicControlLength)(tempRegValue[0] & 0x07);

  /* CRC mode */
  pxPktBasicInit->xCrcMode=(BasicCrcMode)(tempRegValue[3] & 0xE0);

  /* Whitening */
  pxPktBasicInit->xDataWhitening=(SpiritFunctionalState)((tempRegValue[3] >> 4) & 0x01);

  /* FEC */
  pxPktBasicInit->xFec=(SpiritFunctionalState)(tempRegValue[3] & 0x01);

  /* FIX or VAR bit */
  pxPktBasicInit->xFixVarLength=(BasicFixVarLength)(tempRegValue[2] & 0x01);

  /* Preamble length */
  pxPktBasicInit->xPreambleLength=(BasicPreambleLength)(tempRegValue[2] & 0xF8);

  /* Sync length */
  pxPktBasicInit->xSyncLength=(BasicSyncLength)(tempRegValue[2] & 0x06);

  /* sync Words */
  pxPktBasicInit->lSyncWords=0;
  for(uint8_t i=0 ; i<4 ; i++)
  {
      if(i>2-(((uint8_t)pxPktBasicInit->xSyncLength) >>1))
      {
        pxPktBasicInit->lSyncWords |= (uint32_t)(tempRegValue[i+6])<<(8*i);
      }
  }

}


/**
 * @brief  Initializes the SPIRIT Basic packet addresses according to the specified
 *         parameters in the PktBasicAddressesInit struct.
 * @param  pxPktBasicAddresses Basic packet addresses init structure.
 *         This parameter is a pointer to @ref PktBasicAddresses.
 * @retval None.
 */
void SpiritPktBasicAddressesInit(PktBasicAddressesInit* pxPktBasicAddresses)
{
  uint8_t tempRegValue[3];

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktBasicAddresses->xFilterOnMyAddress));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktBasicAddresses->xFilterOnMulticastAddress));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktBasicAddresses->xFilterOnBroadcastAddress));


  /* Reads the PCKT_FLT_OPTIONS ragister */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);
  
  /* Enables or disables filtering on my address */
  if(pxPktBasicAddresses->xFilterOnMyAddress == S_ENABLE)
  {
    tempRegValue[0] |= PCKT_FLT_OPTIONS_DEST_VS_TX_ADDR_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PCKT_FLT_OPTIONS_DEST_VS_TX_ADDR_MASK;
  }
  
  /* Enables or disables filtering on multicast address */
  if(pxPktBasicAddresses->xFilterOnMulticastAddress == S_ENABLE)
  {
    tempRegValue[0] |= PCKT_FLT_OPTIONS_DEST_VS_MULTICAST_ADDR_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PCKT_FLT_OPTIONS_DEST_VS_MULTICAST_ADDR_MASK;
  }
  
  /* Enables or disables filtering on broadcast address */
  if(pxPktBasicAddresses->xFilterOnBroadcastAddress == S_ENABLE)
  {
    tempRegValue[0] |= PCKT_FLT_OPTIONS_DEST_VS_BROADCAST_ADDR_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PCKT_FLT_OPTIONS_DEST_VS_BROADCAST_ADDR_MASK;
  }
  
  /* Writes the new value on the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);
  
  /* Fills the array with the addresses passed in the structure */
  tempRegValue[0] = pxPktBasicAddresses->cBroadcastAddress;
  tempRegValue[1] = pxPktBasicAddresses->cMulticastAddress;
  tempRegValue[2] = pxPktBasicAddresses->cMyAddress;
  
  /* Writes values on the PCKT_FLT_GOALS registers */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_BROADCAST_BASE, 3, tempRegValue);
  
  
}


/**
 * @brief  Returns the SPIRIT Basic packet addresses structure according to the specified
 *         parameters in the registers.
 * @param  pxPktBasicAddresses Basic packet addresses init structure.
 *         This parameter is a pointer to @ref PktBasicAddresses.
 * @retval None.
 */
void SpiritPktBasicGetAddressesInfo(PktBasicAddressesInit* pxPktBasicAddresses)
{
  uint8_t tempRegValue[3];

  /* Reads values on the PCKT_FLT_GOALS registers */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_BROADCAST_BASE, 3, tempRegValue);

  /* Fit the structure with the read addresses */
  pxPktBasicAddresses->cBroadcastAddress = tempRegValue[0];
  pxPktBasicAddresses->cMulticastAddress = tempRegValue[1];
  pxPktBasicAddresses->cMyAddress = tempRegValue[2];

  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);

  /* Fit the structure with the read filtering bits */
  pxPktBasicAddresses->xFilterOnBroadcastAddress = (SpiritFunctionalState)((tempRegValue[0] >> 1) & 0x01);
  pxPktBasicAddresses->xFilterOnMulticastAddress = (SpiritFunctionalState)((tempRegValue[0] >> 2) & 0x01);
  pxPktBasicAddresses->xFilterOnMyAddress = (SpiritFunctionalState)((tempRegValue[0] >> 3) & 0x01);

}


/**
 * @brief  Configures the Basic packet format as packet used by SPIRIT.
 * @param  None.
 * @retval None.
 */
void SpiritPktBasicSetFormat(void)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Build the new value. Also set to 0 the direct RX mode bits */
  tempRegValue &= 0x0F;
  tempRegValue |= (uint8_t)PCKTCTRL3_PCKT_FRMT_BASIC;

  /* Writes the  value on the PCKTCTRL3 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Reads the PCKTCTRL1_BASE register */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build the new value. Set to 0 the direct TX mode bits */
  tempRegValue &= 0xF3;

  /* Writes the value on the PCKTCTRL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Reads the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Mask a reserved bit */
  tempRegValue &= ~0x20;

  /* Writes the value on register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);
}


/**
 * @brief  Sets the address length for SPIRIT Basic packets.
 * @param  xAddressField length of ADDRESS in bytes.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritPktBasicAddressField(SpiritFunctionalState xAddressField)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xAddressField));

  /* Reads the PCKTCTRL4 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

  /* Build the address length for the register */
  if(xAddressField==S_ENABLE)
  {
    tempRegValue |= 0x08;
  }
  else
  {
    tempRegValue &= 0x07;
  }

  /* Writes the new value on the PCKTCTRL4 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

}


/**
 * @brief  Specifies if the Address field for SPIRIT Basic packets is enabled or disabled.
 * @param  None.
 * @retval SpiritFunctionalState Notifies if the address field is enabled or disabled.
 */
SpiritFunctionalState SpiritPktBasicGetAddressField(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL4 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

  /* Returns the address field value */
  if(tempRegValue & PCKTCTRL4_ADDRESS_LEN_MASK)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}


/**
 * @brief  Sets the payload length for SPIRIT Basic packets. Since the packet length
 *         depends from the address and the control field size, this
 *         function reads the correspondent registers in order to determine
 *         the correct packet length to be written.
 * @param  nPayloadLength payload length in bytes.
 *         This parameter is an uint16_t.
 * @retval None.
 */
void SpiritPktBasicSetPayloadLength(uint16_t nPayloadLength)
{
  uint8_t tempRegValue[2];
  uint16_t overSize=0;

  /* Computes the oversize (address + control) size */
  if(SpiritPktBasicGetAddressField())
  {
    overSize=1;
  }
  overSize += (uint16_t) SpiritPktBasicGetControlLength();

  /* Computes PCKTLEN0 value from nPayloadLength */
  tempRegValue[1]=BASIC_BUILD_PCKTLEN0(nPayloadLength+overSize);
  /* Computes PCKTLEN1 value from nPayloadLength */
  tempRegValue[0]=BASIC_BUILD_PCKTLEN1(nPayloadLength+overSize);

  /* Writes data on the PCKTLEN1/0 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTLEN1_BASE, 2, tempRegValue);

}


/**
 * @brief  Returns the payload length for SPIRIT Basic packets. Since the
 *         packet length depends from the address and the control
 *         field size, this function reads the correspondent
 *         registers in order to determine the correct payload length
 *         to be returned.
 * @param  None.
 * @retval uint16_t Payload length in bytes.
 */
uint16_t SpiritPktBasicGetPayloadLength(void)
{
  uint8_t tempRegValue[2];
  uint16_t overSize=0;

  /* Computes the oversize (address + control) size */
  if(SpiritPktBasicGetAddressField())
  {
    overSize=1;
  }
  overSize += (uint16_t) SpiritPktBasicGetControlLength();

  /* Reads the packet length registers */
  g_xStatus = SpiritSpiReadRegisters(PCKTLEN1_BASE, 2, tempRegValue);

  /* Returns the packet length */
  return ((((uint16_t)tempRegValue[0])<<8) + (uint16_t) tempRegValue[1]) - overSize;

}

/**
 * @brief  Returns the packet length field of the received packet.
 * @param  None.
 * @retval uint16_t Packet length.
 */
uint16_t SpiritPktBasicGetReceivedPktLength(void)
{
  uint8_t tempRegValue[2];
  uint16_t overSize=0;

  /* Computes the oversize (address + control) size */
  if(SpiritPktBasicGetAddressField())
  {
    overSize=1;
  }
  overSize += (uint16_t) SpiritPktBasicGetControlLength();
  
  /* Reads the RX_PCKT_LENx registers value */
  g_xStatus = SpiritSpiReadRegisters(RX_PCKT_LEN1_BASE, 2, tempRegValue);

  /* Rebuild and return the the length field */
  return ((((uint16_t) tempRegValue[0]) << 8) + (uint16_t) tempRegValue[1] - overSize);

}

/**
 * @brief  Computes and sets the variable payload length for SPIRIT Basic packets.
 * @param  nMaxPayloadLength payload length in bytes.
 *         This parameter is an uint16_t.
 * @param  xAddressField Enable or Disable Address Field.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @param  xControlLength Control length in bytes.
 *         This parameter can be any value of @ref BasicControlLength.
 * @retval None.
 */
void SpiritPktBasicSetVarLengthWidth(uint16_t nMaxPayloadLength, SpiritFunctionalState xAddressField, BasicControlLength xControlLength)
{
  uint8_t tempRegValue,
          addressLength,
          i;
  uint32_t packetLength;

  /* Sets the address length according to xAddressField */
  if(xAddressField == S_ENABLE)
  {
    addressLength=1;
  }
  else
  {
    addressLength=0;
  }

  /* packet length = payload length + address length + control length */
  packetLength=nMaxPayloadLength+addressLength+xControlLength;

  /* Computes the number of bits */
  for(i=0;i<16;i++)
  {
    if(packetLength == 0) break;
    {
    packetLength >>= 1;
    }
  }
  i==0 ? i=1 : i;

  /* Reads the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Build value for the length width */
  tempRegValue &= ~PCKTCTRL3_LEN_WID_MASK;
  tempRegValue |= (uint8_t)(i-1);

  /* Writes the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

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
