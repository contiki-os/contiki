/**
  ******************************************************************************
  * @file    SPIRIT_PktMbus.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT MBUS packets.
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
#include "SPIRIT_PktMbus.h"
#include "SPIRIT_Radio.h"
#include "MCU_Interface.h"

/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_PktMbus
 * @{
 */


/**
 * @defgroup PktMbus_Private_TypesDefinitions           Pkt MBUS Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktMbus_Private_Defines                    Pkt MBUS Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktMbus_Private_Macros                     Pkt MBUS Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktMbus_Private_Variables                  Pkt MBUS Private Variables
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktMbus_Private_FunctionPrototypes         Pkt MBUS Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup PktMbus_Private_Functions                  Pkt MBUS Private Functions
 * @{
 */


/**
 * @brief  Initializes the SPIRIT MBUS packet according to the specified parameters in the PktMbusInit struct.
 * @param  pxPktMbusInit pointer to a PktMbusInit structure that contains the configuration information for the specified SPIRIT MBUS PACKET FORMAT.
 *         This parameter is a pointer to @ref PktMbusInit.
 * @retval None.
 */
void SpiritPktMbusInit(PktMbusInit* pxPktMbusInit)
{
  uint8_t tempRegValue[3];

  /* Check the parameters */
  s_assert_param(IS_MBUS_SUBMODE(pxPktMbusInit->xMbusSubmode));

  /* Packet format config */
  SpiritPktMbusSetFormat();
  SpiritPktCommonFilterOnCrc(S_DISABLE);
  SpiritRadioCsBlanking(S_ENABLE);
  
  /* Preamble, postamble and submode config */
  tempRegValue[0] = pxPktMbusInit->cPreambleLength;
  tempRegValue[1] = pxPktMbusInit->cPostambleLength;
  tempRegValue[2] = (uint8_t) pxPktMbusInit->xMbusSubmode;

  /* Writes the new values on the MBUS_PRMBL registers */
  g_xStatus = SpiritSpiWriteRegisters(MBUS_PRMBL_BASE, 3, tempRegValue);

}

/**
 * @brief  Returns the SPIRIT MBUS packet structure according to the specified parameters in the registers.
 * @param  pxPktMbusInit MBUS packet init structure.
 *         This parameter is a pointer to @ref PktMbusInit.
 * @retval None.
 */
void SpiritPktMbusGetInfo(PktMbusInit* pxPktMbusInit)
{
  uint8_t tempRegValue[3];

  /* Reads the MBUS regs value */
  g_xStatus = SpiritSpiReadRegisters(MBUS_PRMBL_BASE, 3, tempRegValue);

  /* Fit the structure */
  pxPktMbusInit->cPreambleLength = tempRegValue[0];
  pxPktMbusInit->cPostambleLength = tempRegValue[1];
  pxPktMbusInit->xMbusSubmode = (MbusSubmode) (tempRegValue[2]&0x0E);

}


/**
 * @brief  Configures the MBUS packet format as the one used by SPIRIT.
 * @param  None.
 * @retval None.
 */
void SpiritPktMbusSetFormat(void)
{
  uint8_t tempRegValue;

  /* Reads the PCKTCTRL3 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Sets format bits. Also set to 0 the direct RX mode bits */
  tempRegValue &= 0x0F;
  tempRegValue |= ((uint8_t)PCKTCTRL3_PCKT_FRMT_MBUS);

  /* Writes value on the PCKTCTRL3 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Reads the PCKTCTRL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build the new value. Set to 0 the direct TX mode bits */
  tempRegValue &= 0xF3;

  /* Writes the value on the PCKTCTRL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Reads the PROTOCOL1 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Mask a reserved bit */
  tempRegValue &= ~0x20;

  /* Writes the value on the PROTOCOL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets how many chip sequence “01” shall be added in the preamble
 *         respect to the minimum value as defined according to the specified sub-mode.
 * @param  cPreamble the number of chip sequence.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktMbusSetPreamble(uint8_t cPreamble)
{
  /* Modifies the MBUS_PRMBL register value */
  g_xStatus = SpiritSpiWriteRegisters(MBUS_PRMBL_BASE, 1, &cPreamble);

}


/**
 * @brief  Returns how many chip sequence "01" are added in the preamble
 *         respect to the minimum value as defined according to the specified sub-mode.
 * @param  None.
 * @retval uint8_t Preable in number of "01" chip sequences.
 */
uint8_t SpiritPktMbusGetPreamble(void)
{
  uint8_t tempRegValue;

  /* Modifies the MBUS_PRMBL register value */
  g_xStatus = SpiritSpiReadRegisters(MBUS_PRMBL_BASE, 1, &tempRegValue);

  /* Return value */
  return tempRegValue;

}


/**
 * @brief  Sets how many chip sequence “01” will be used in postamble
 * @param  cPostamble the number of chip sequence.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritPktMbusSetPostamble(uint8_t cPostamble)
{
  /* Modifies the MBUS_PSTMBL register value */
  g_xStatus = SpiritSpiWriteRegisters(MBUS_PSTMBL_BASE, 1, &cPostamble);

}


/**
 * @brief  Returns how many chip sequence "01" are used in the postamble
 * @param  None.
 * @retval uint8_t Postamble in number of "01" chip sequences.
 */
uint8_t SpiritPktMbusGetPostamble(void)
{
  uint8_t tempRegValue;

  /* Reads the MBUS_PSTMBL register */
  g_xStatus = SpiritSpiReadRegisters(MBUS_PSTMBL_BASE, 1, &tempRegValue);

  /* Returns value */
  return tempRegValue;

}


/**
 * @brief  Sets the MBUS submode used.
 * @param  xMbusSubmode the submode used.
 *         This parameter can be any value of @ref MbusSubmode.
 * @retval None.
 */
void SpiritPktMbusSetSubmode(MbusSubmode xMbusSubmode)
{
  /* Modifies the MBUS_CTRL register value */
  g_xStatus = SpiritSpiWriteRegisters(MBUS_CTRL_BASE, 1, (uint8_t*)xMbusSubmode);

}


/**
 * @brief  Returns the MBUS submode used.
 * @param  None.
 * @retval MbusSubmode MBUS submode.
 */
MbusSubmode SpiritPktMbusGetSubmode(void)
{
  uint8_t tempRegValue;

  /* Reads the MBUS_CTRL register value */
  g_xStatus = SpiritSpiReadRegisters(MBUS_CTRL_BASE, 1, &tempRegValue);

  /* Returns value */
  return (MbusSubmode) tempRegValue;

}


/**
 * @brief  Sets the payload length for SPIRIT MBUS packets.
 * @param  nPayloadLength payload length in bytes.
 *         This parameter is an uint16_t.
 * @retval None.
 */
void SpiritPktMbusSetPayloadLength(uint16_t nPayloadLength)
{
  uint8_t tempRegValue[2];

  /* Computes PCKTLEN0 value from nPayloadLength */
  tempRegValue[1]=BUILD_PCKTLEN0(nPayloadLength);//(uint8_t)nPayloadLength;
  /* Computes PCKTLEN1 value from nPayloadLength */
  tempRegValue[0]=BUILD_PCKTLEN1(nPayloadLength);//(uint8_t)(nPayloadLength>>8);

  /* Writes data on the PCKTLEN1/0 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTLEN1_BASE, 2, tempRegValue);

}


/**
 * @brief  Returns the payload length for SPIRIT MBUS packets.
 * @param  None.
 * @retval uint16_t Payload length in bytes.
 */
uint16_t SpiritPktMbusGetPayloadLength(void)
{
  uint8_t tempRegValue[2];

  /* Reads the packet length registers */
  g_xStatus = SpiritSpiReadRegisters(PCKTLEN1_BASE, 2, tempRegValue);

  /* Returns the packet length */
  return ((((uint16_t)tempRegValue[0])<<8) + (uint16_t) tempRegValue[1]);

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
