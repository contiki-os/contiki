/**
  ******************************************************************************
  * @file    SPIRIT_PktStack.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT STack packets.
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
#include "SPIRIT_PktStack.h"
#include "MCU_Interface.h"


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_PktStack
 * @{
 */


/**
 * @defgroup PktStack_Private_TypesDefinitions          Pkt STack Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktStack_Private_Defines                   Pkt STack Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktStack_Private_Macros                    Pkt STack Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktStack_Private_Variables                 Pkt STack Private Variables
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktStack_Private_FunctionPrototypes        Pkt STack Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktStack_Private_Functions                 Pkt STack Private Functions
 * @{
 */


/**
 * @brief  Initializes the SPIRIT STack packet according to the specified
 *         parameters in the PktStackInit.
 * @param  pxPktStackInit STack packet init structure.
 *         This parameter is a pointer to @ref PktStackInit.
 * @retval None.
 */
void SpiritPktStackInit(PktStackInit* pxPktStackInit)
{
  uint8_t tempRegValue[4], i;

  /* Check the parameters */
  s_assert_param(IS_STACK_PREAMBLE_LENGTH(pxPktStackInit->xPreambleLength));
  s_assert_param(IS_STACK_SYNC_LENGTH(pxPktStackInit->xSyncLength));
  s_assert_param(IS_STACK_CRC_MODE(pxPktStackInit->xCrcMode));
  s_assert_param(IS_STACK_LENGTH_WIDTH_BITS(pxPktStackInit->cPktLengthWidth));
  s_assert_param(IS_STACK_FIX_VAR_LENGTH(pxPktStackInit->xFixVarLength));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackInit->xFec));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackInit->xDataWhitening));
  s_assert_param(IS_STACK_CONTROL_LENGTH(pxPktStackInit->xControlLength));


  /* Reads the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue[0]);

  /* Mask a reserved bit */
  tempRegValue[0] &= ~0x20;

  /* Always (!) set the automatic packet filtering */
  tempRegValue[0] |= PROTOCOL1_AUTO_PCKT_FLT_MASK;

  /* Writes the value on register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue[0]);

  /* Reads the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);

  /* Always reset the control and source filtering */
  tempRegValue[0] &= ~(PCKT_FLT_OPTIONS_SOURCE_FILTERING_MASK | PCKT_FLT_OPTIONS_CONTROL_FILTERING_MASK);

  /* Writes the value on register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);


  /* Address and control length setting: source and destination address are always present so ADDRESS_LENGTH=2 */
  tempRegValue[0] = 0x10 | ((uint8_t) pxPktStackInit->xControlLength);


  /* Packet format and width length setting */
  pxPktStackInit->cPktLengthWidth == 0 ? pxPktStackInit->cPktLengthWidth=1 : pxPktStackInit->cPktLengthWidth;
  tempRegValue[1] = ((uint8_t) PCKTCTRL3_PCKT_FRMT_STACK) | ((uint8_t)(pxPktStackInit->cPktLengthWidth-1));

  /* Preamble, sync and fixed or variable length setting */
  tempRegValue[2] = ((uint8_t) pxPktStackInit->xPreambleLength) | ((uint8_t) pxPktStackInit->xSyncLength) |
                    ((uint8_t) pxPktStackInit->xFixVarLength);

  /* CRC length, whitening and FEC setting */
  tempRegValue[3] = (uint8_t) pxPktStackInit->xCrcMode;

  if(pxPktStackInit->xDataWhitening == S_ENABLE)
  {
     tempRegValue[3] |= PCKTCTRL1_WHIT_MASK;
  }

  if(pxPktStackInit->xFec == S_ENABLE)
  {
     tempRegValue[3] |= PCKTCTRL1_FEC_MASK;
  }
  
  /* Writes registers */
  SpiritSpiWriteRegisters(PCKTCTRL4_BASE, 4, tempRegValue);

  /* Sync words setting */
  for(i=0;i<4;i++)
  {
    if(i<3-(pxPktStackInit->xSyncLength >>1))
    {
      tempRegValue[i]=0;
    }
    else
    {
      tempRegValue[i] = (uint8_t)(pxPktStackInit->lSyncWords>>(8*i));
    }
  }

  /* Enables or disables the CRC check */
  if(pxPktStackInit->xCrcMode == PKT_NO_CRC)
  {
    SpiritPktStackFilterOnCrc(S_DISABLE);
  }
  else
  {
    SpiritPktStackFilterOnCrc(S_ENABLE);
  }

  /* Writes registers */
  g_xStatus = SpiritSpiWriteRegisters(SYNC4_BASE, 4, tempRegValue);

}


/**
 * @brief  Returns the SPIRIT STack packet structure according to the specified parameters in the registers.
 * @param  pxPktStackInit STack packet init structure.
 *         This parameter is a pointer to @ref PktStackInit.
 * @retval None.
 */
void SpiritPktStackGetInfo(PktStackInit* pxPktStackInit)
{
  uint8_t tempRegValue[10];

  /* Reads registers */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 10, tempRegValue);

  /* Length width */
  pxPktStackInit->cPktLengthWidth=(tempRegValue[1] & 0x0F)+1;

  /* Control length */
  pxPktStackInit->xControlLength=(StackControlLength)(tempRegValue[0] & 0x07);

  /* CRC mode */
  pxPktStackInit->xCrcMode=(StackCrcMode)(tempRegValue[3] & 0xE0);

  /* Whitening */
  pxPktStackInit->xDataWhitening=(SpiritFunctionalState)((tempRegValue[3] >> 4) & 0x01);

  /* FEC */
  pxPktStackInit->xFec=(SpiritFunctionalState)(tempRegValue[3] & 0x01);

  /* FIX or VAR bit */
  pxPktStackInit->xFixVarLength=(StackFixVarLength)(tempRegValue[2] & 0x01);

  /* Preamble length */
  pxPktStackInit->xPreambleLength=(StackPreambleLength)(tempRegValue[2] & 0xF8);

  /* Sync length */
  pxPktStackInit->xSyncLength=(StackSyncLength)(tempRegValue[2] & 0x06);

  /* sync Words */
  pxPktStackInit->lSyncWords=0;
  for(uint8_t i=0 ; i<4 ; i++)
  {
      if(i>2-(pxPktStackInit->xSyncLength >>1))
      {
        pxPktStackInit->lSyncWords |= tempRegValue[i+6]<<(8*i);
      }
  }

}


/**
 * @brief  Initializes the SPIRIT STack packet addresses according to the specified
 *         parameters in the PktStackAddresses struct.
 * @param  pxPktStackAddresses STack packet addresses init structure.
 *         This parameter is a pointer to @ref PktStackAddressesInit .
 * @retval None.
 */
void SpiritPktStackAddressesInit(PktStackAddressesInit* pxPktStackAddresses)
{
  uint8_t tempRegValue[3];

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackAddresses->xFilterOnMyAddress));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackAddresses->xFilterOnMulticastAddress));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackAddresses->xFilterOnBroadcastAddress));
  
  /* Reads the filtering options ragister */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);
  
  /* Enables or disables filtering on my address */
  if(pxPktStackAddresses->xFilterOnMyAddress == S_ENABLE)
  {
    tempRegValue[0] |= PCKT_FLT_OPTIONS_DEST_VS_TX_ADDR_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PCKT_FLT_OPTIONS_DEST_VS_TX_ADDR_MASK;
  }
  
  /* Enables or disables filtering on multicast address */
  if(pxPktStackAddresses->xFilterOnMulticastAddress == S_ENABLE)
  {
    tempRegValue[0] |= PCKT_FLT_OPTIONS_DEST_VS_MULTICAST_ADDR_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PCKT_FLT_OPTIONS_DEST_VS_MULTICAST_ADDR_MASK;
  }
  
  /* Enables or disables filtering on broadcast address */
  if(pxPktStackAddresses->xFilterOnBroadcastAddress == S_ENABLE)
  {
    tempRegValue[0] |= PCKT_FLT_OPTIONS_DEST_VS_BROADCAST_ADDR_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PCKT_FLT_OPTIONS_DEST_VS_BROADCAST_ADDR_MASK;
  }
  
  /* Writes value on the register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);
  
  /* Fills array with the addresses passed in the structure */
  tempRegValue[0] = pxPktStackAddresses->cBroadcastAddress;
  tempRegValue[1] = pxPktStackAddresses->cMulticastAddress;
  tempRegValue[2] = pxPktStackAddresses->cMyAddress;
  
  /* Writes them on the addresses registers */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_BROADCAST_BASE, 3, tempRegValue);
  
}


/**
* @brief  Returns the SPIRIT STack packet addresses structure according to the specified
*         parameters in the registers.
* @param  pxPktStackAddresses STack packet addresses init structure.
*         This parameter is a pointer to @ref PktStackAddresses.
* @retval None.
*/
void SpiritPktStackGetAddressesInfo(PktStackAddressesInit* pxPktStackAddresses)
{
  uint8_t tempRegValue[3];
  
  /* Reads values on the PCKT_FLT_GOALS registers */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_BROADCAST_BASE, 3, tempRegValue);
  
  /* Fit the structure with the read addresses */
  pxPktStackAddresses->cBroadcastAddress = tempRegValue[0];
  pxPktStackAddresses->cMulticastAddress = tempRegValue[1];
  pxPktStackAddresses->cMyAddress = tempRegValue[2];
  
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue[0]);
  
  /* Fit the structure with the read filtering bits */
  pxPktStackAddresses->xFilterOnBroadcastAddress = (SpiritFunctionalState)((tempRegValue[0] >> 1) & 0x01);
  pxPktStackAddresses->xFilterOnMulticastAddress = (SpiritFunctionalState)((tempRegValue[0] >> 2) & 0x01);
  pxPktStackAddresses->xFilterOnMyAddress = (SpiritFunctionalState)((tempRegValue[0] >> 3) & 0x01);
  
}


/**
* @brief  Initializes the SPIRIT STack packet LLP options according to the specified
*         parameters in the PktStackLlpInit struct.
* @param  pxPktStackLlpInit STack packet LLP init structure.
*         This parameter is a pointer to @ref PktStackLlpInit.
* @retval None.
*/
void SpiritPktStackLlpInit(PktStackLlpInit* pxPktStackLlpInit)
{
  uint8_t tempRegValue[2];

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackLlpInit->xPiggybacking));
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxPktStackLlpInit->xAutoAck));
  s_assert_param(IS_STACK_NMAX_RETX(pxPktStackLlpInit->xNMaxRetx));
  /* check if piggybacking is enabled and autoack is disabled */
  s_assert_param(!(pxPktStackLlpInit->xPiggybacking==S_ENABLE && pxPktStackLlpInit->xAutoAck==S_DISABLE));

  /* Piggybacking mechanism setting on the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 2, tempRegValue);
  if(pxPktStackLlpInit->xPiggybacking == S_ENABLE)
  {
    tempRegValue[0] |= PROTOCOL1_PIGGYBACKING_MASK;
  }
  else
  {
    tempRegValue[0] &= ~PROTOCOL1_PIGGYBACKING_MASK;
  }

  /* RX and TX autoack mechanisms setting on the PROTOCOL0 register */
  if(pxPktStackLlpInit->xAutoAck == S_ENABLE)
  {
    tempRegValue[1] |= PROTOCOL0_AUTO_ACK_MASK;
  }
  else
  {
    tempRegValue[1] &= ~PROTOCOL0_AUTO_ACK_MASK;
  }

  /* Max number of retransmission setting */
  tempRegValue[1] &= ~PROTOCOL0_NMAX_RETX_MASK;
  tempRegValue[1] |= pxPktStackLlpInit->xNMaxRetx;

  /* Writes registers */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 2, tempRegValue);

}


/**
 * @brief  Returns the SPIRIT STack packet LLP options according to the specified
 *         values in the registers.
 * @param  pxPktStackLlpInit STack packet LLP structure.
 *         This parameter is a pointer to @ref PktStackLlpInit.
 * @retval None.
 */
void SpiritPktStackLlpGetInfo(PktStackLlpInit* pxPktStackLlpInit)
{
  uint8_t tempRegValue[2];

  /* Piggybacking mechanism setting on the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 2, tempRegValue);

  /* Fit the structure with the read values */
  pxPktStackLlpInit->xPiggybacking = (SpiritFunctionalState)((tempRegValue[0] >> 6) & 0x01);
  pxPktStackLlpInit->xAutoAck = (SpiritFunctionalState)((tempRegValue[1] >> 2) & 0x01);
  pxPktStackLlpInit->xNMaxRetx = (StackNMaxReTx)(tempRegValue[1] & PROTOCOL0_NMAX_RETX_MASK);

}


/**
 * @brief  Configures the STack packet format for SPIRIT.
 * @param  None.
 * @retval None.
 */
void SpiritPktStackSetFormat(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Build value to be written. Also set to 0 the direct RX mode bits */
  tempRegValue &= 0x0F;
  tempRegValue |= ((uint8_t)PCKTCTRL3_PCKT_FRMT_STACK);

  /* Writes the value on the PCKTCTRL3 register. */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Reads the PCKTCTRL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build the new value. Set to 0 the direct TX mode bits */
  tempRegValue &= 0xF3;

  /* Writes the PCKTCTRL1 value on register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Reads the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Mask a reserved bit */
  tempRegValue &= ~0x20;

  /* Writes the value on the PROTOCOL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets the address length for SPIRIT STack packets (always 2).
 * @param  None.
 * @retval None.
 */
void SpiritPktStackSetAddressLength(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL4 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

  /* Build the new value */
  tempRegValue &= ~PCKTCTRL4_ADDRESS_LEN_MASK;
  tempRegValue |= ((uint8_t)0x10);

  /* Writes the value on the PCKTCTRL4 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL4_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets the payload length for SPIRIT STack packets. Since the packet length
 *         depends from the address (always 2 for this packet format)
 *         and the control field size, this function reads the control length register
 *         content in order to determine the correct packet length to be written.
 * @param  nPayloadLength payload length in bytes.
 *         This parameter can be any value of uint16_t.
 * @retval None.
 */
void SpiritPktStackSetPayloadLength(uint16_t nPayloadLength)
{
  uint8_t tempRegValue[2];

  /* Computes the oversize (address + control) size */
  uint16_t overSize = 2 + (uint16_t) SpiritPktStackGetControlLength();

  /* Computes PCKTLEN0 value from lPayloadLength */
  tempRegValue[1]=STACK_BUILD_PCKTLEN0(nPayloadLength+overSize);
  /* Computes PCKTLEN1 value from lPayloadLength */
  tempRegValue[0]=STACK_BUILD_PCKTLEN1(nPayloadLength+overSize);

  /* Writes the value on the PCKTLENx registers */
  g_xStatus = SpiritSpiWriteRegisters(PCKTLEN1_BASE, 2, tempRegValue);

}


/**
 * @brief  Returns the payload length for SPIRIT STack packets. Since the
 *         packet length depends from the address and the control
 *         field size, this function reads the correspondent
 *         registers in order to determine the correct payload length
 *         to be returned.
 * @param  None.
 * @retval uint16_t Payload length.
 */
uint16_t SpiritPktStackGetPayloadLength(void)
{
  uint8_t tempRegValue[2];
  /* Computes the oversize (address + control) size */
  uint16_t overSize = 2 + (uint16_t) SpiritPktStackGetControlLength();

  /* Reads the PCKTLEN1 registers value */
  g_xStatus = SpiritSpiReadRegisters(PCKTLEN1_BASE, 2, tempRegValue);

  /* Rebuild and return the payload length value */
  return (((uint16_t) tempRegValue[1])<<8 + (uint16_t) tempRegValue[0] - overSize);

}


/**
 * @brief  Computes and sets the variable payload length for SPIRIT STack packets.
 * @param  nMaxPayloadLength payload length in bytes.
 *         This parameter is an uint16_t.
 * @param  xControlLength control length in bytes.
 *         This parameter can be any value of @ref StackControlLength.
 * @retval None.
 */
void SpiritPktStackSetVarLengthWidth(uint16_t nMaxPayloadLength, StackControlLength xControlLength)
{
  uint8_t tempRegValue,
          i;
  uint32_t packetLength;


  /* packet length = payload length + address length (2) + control length */
  packetLength=nMaxPayloadLength+2+xControlLength;

  /* Computes the number of bits */
  for(i=0;i<16;i++)
  {
    if(packetLength == 0) 
    {
      break;
    }
    packetLength >>= 1;
  }
  i==0 ? i=1 : i;

  /* Reads the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Build the register value */
  tempRegValue &= ~PCKTCTRL3_LEN_WID_MASK;
  tempRegValue |= ((uint8_t)(i-1));

  /* Writes the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

}


/**
 * @brief  Rx packet source mask. Used to mask the address of the accepted packets. If 0 -> no filtering.
 * @param  cMask Rx source mask.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktStackSetRxSourceMask(uint8_t cMask)
{
  /* Writes value on the register PCKT_FLT_GOALS_SOURCE_MASK */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_GOALS_SOURCE_MASK_BASE, 1, &cMask);

}


/**
 * @brief  Returns the Rx packet source mask. Used to mask the address of the accepted packets. If 0 -> no filtering.
 * @param  None.
 * @retval uint8_t Rx source mask.
 */
uint8_t SpiritPktStackGetRxSourceMask(void)
{
  uint8_t tempRegValue;

  /* Writes value on the PCKT_FLT_GOALS_SOURCE_MASK register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_GOALS_SOURCE_MASK_BASE, 1, &tempRegValue);

  /* Return the read value */
  return tempRegValue;

}

/**
 * @brief  Returns the packet length field of the received packet.
 * @param  None.
 * @retval uint16_t Packet length.
 */
uint16_t SpiritPktStackGetReceivedPktLength(void)
{
  uint8_t tempRegValue[2];
  uint16_t tempLength;
  
  /* Reads the RX_PCKT_LENx registers value */
  g_xStatus = SpiritSpiReadRegisters(RX_PCKT_LEN1_BASE, 2, tempRegValue);

  /* Rebuild and return the the length field */
  tempLength = ((((uint16_t) tempRegValue[0]) << 8) + (uint16_t) tempRegValue[1]);
  
  /* Computes the oversize (address + control) size */
  tempLength -= 2 + (uint16_t) SpiritPktStackGetControlLength();
  
  return tempLength;

}


/**
 * @brief  If enabled RX packet is accepted only if the masked source address field matches the
 *         masked source address field reference (SOURCE_MASK & SOURCE_FIELD_REF == SOURCE_MASK & RX_SOURCE_FIELD).
 * @param  xNewState new state for Source address filtering enable bit.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 * @note   This filtering control is enabled by default but the source address mask is by default set to 0.
 *         As a matter of fact the user has to enable the source filtering bit after the packet initialization
 *         because the PktInit routine disables it.
 */
void SpiritPktStackFilterOnSourceAddress(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

   /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));


  /* Modify the register value: set or reset the source bit filtering */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

  /* Set or reset the SOURCE ADDRESS filtering enabling bit */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PCKT_FLT_OPTIONS_SOURCE_FILTERING_MASK;
  }
  else
  {
    tempRegValue &= ~PCKT_FLT_OPTIONS_SOURCE_FILTERING_MASK;
  }

  /* Writes the new value on the PCKT_FLT_OPTIONS register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 1, &tempRegValue);

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
