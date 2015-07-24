/**
  ******************************************************************************
  * @file    SPIRIT_Qi.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT QI.
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
#include "SPIRIT_Qi.h"
#include "MCU_Interface.h"



/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_Qi
 * @{
 */


/**
 * @defgroup Qi_Private_TypesDefinitions        QI Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Qi_Private_Defines                 QI Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Qi_Private_Macros                  QI Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Qi_Private_Variables               QI Private Variables
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Qi_Private_FunctionPrototypes      QI Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Qi_Private_Functions               QI Private Functions
 * @{
 */

/**
 * @brief  Enables/Disables the PQI Preamble Quality Indicator check. The running peak PQI is
 *         compared to a threshold value and the preamble valid IRQ is asserted as soon as the threshold is passed.
 * @param  xNewState new state for PQI check.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritQiPqiCheck(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the QI register value */
  g_xStatus = SpiritSpiReadRegisters(QI_BASE, 1, &tempRegValue);

  /* Enables or disables the PQI Check bit on the QI_BASE register */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= QI_PQI_MASK;
  }
  else
  {
    tempRegValue &= ~QI_PQI_MASK;
  }

  /* Writes value on the QI register */
  g_xStatus = SpiritSpiWriteRegisters(QI_BASE, 1, &tempRegValue);

}


/**
 * @brief  Enables/Disables the Synchronization Quality Indicator check. The running peak SQI is
 *         compared to a threshold value and the sync valid IRQ is asserted as soon as the threshold is passed.
 * @param  xNewState new state for SQI check.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritQiSqiCheck(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the QI register value */
  g_xStatus = SpiritSpiReadRegisters(QI_BASE, 1, &tempRegValue);

  /* Enables or disables the SQI Check bit on the QI_BASE register */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= QI_SQI_MASK;
  }
  else
  {
    tempRegValue &= ~QI_SQI_MASK;
  }

  /* Writes value on the QI register */
  g_xStatus = SpiritSpiWriteRegisters(QI_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets the PQI threshold. The preamble quality threshold is 4*PQI_TH (PQI_TH = 0..15).
 * @param  xPqiThr parameter of the formula above.
 * 	   This variable is a @ref PqiThreshold.
 * @retval None.
 */
void SpiritQiSetPqiThreshold(PqiThreshold xPqiThr)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_PQI_THR(xPqiThr));

  /* Reads the QI register value */
  g_xStatus = SpiritSpiReadRegisters(QI_BASE, 1, &tempRegValue);

  /* Build the PQI threshold value to be written */
  tempRegValue &= 0xC3;
  tempRegValue |= ((uint8_t)xPqiThr);

  /* Writes value on the QI register */
  g_xStatus = SpiritSpiWriteRegisters(QI_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the PQI threshold. The preamble quality threshold is 4*PQI_TH (PQI_TH = 0..15).
 * @param  None.
 * @retval PqiThreshold PQI threshold (PQI_TH of the formula above).
 */
PqiThreshold SpiritQiGetPqiThreshold(void)
{
  uint8_t tempRegValue;

  /* Reads the QI register value */
  g_xStatus = SpiritSpiReadRegisters(QI_BASE, 1, &tempRegValue);

  /* Rebuild and return the PQI threshold value */
  return (PqiThreshold)(tempRegValue & 0x3C);

}


/**
 * @brief  Sets the SQI threshold. The synchronization quality
 *         threshold is equal to 8 * SYNC_LEN - 2 * SQI_TH with SQI_TH = 0..3. When SQI_TH is 0 perfect match is required; when
 *         SQI_TH = 1, 2, 3 then 1, 2, or 3 bit errors are respectively accepted. It is recommended that the SQI check is always
 *         enabled.
 * @param  xSqiThr parameter of the formula above.
 * 	   This parameter is a @ref SqiThreshold.
 * @retval None.
 */
void SpiritQiSetSqiThreshold(SqiThreshold xSqiThr)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SQI_THR(xSqiThr));

  /* Reads the QI register value */
  g_xStatus = SpiritSpiReadRegisters(QI_BASE, 1, &tempRegValue);

  /* Build the SQI threshold value to be written */
  tempRegValue &= 0x3F;
  tempRegValue |= ((uint8_t)xSqiThr);

  /* Writes the new value on the QI register */
  g_xStatus = SpiritSpiWriteRegisters(QI_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the SQI threshold. The synchronization quality threshold is equal to 8 * SYNC_LEN - 2 * SQI_TH with SQI_TH = 0..3.
 * @param  None.
 * @retval SqiThreshold SQI threshold (SQI_TH of the formula above).
 */
SqiThreshold SpiritQiGetSqiThreshold(void)
{
  uint8_t tempRegValue;

  /* Reads the QI register value */
  g_xStatus = SpiritSpiReadRegisters(QI_BASE, 1, &tempRegValue);

  /* Rebuild and return the SQI threshold value */
  return (SqiThreshold)(tempRegValue & 0xC0);

}


/**
 * @brief  Returns the PQI value.
 * @param  None.
 * @retval uint8_t PQI value.
 */
uint8_t SpiritQiGetPqi(void)
{
  uint8_t tempRegValue;

  /* Reads the LINK_QUALIF2 register value */
  g_xStatus = SpiritSpiReadRegisters(LINK_QUALIF2_BASE, 1, &tempRegValue);

  /* Returns the PQI value */
  return tempRegValue;

}


/**
 * @brief  Returns the SQI value.
 * @param  None.
 * @retval uint8_t SQI value.
 */
uint8_t SpiritQiGetSqi(void)
{
  uint8_t tempRegValue;

  /* Reads the register LINK_QUALIF1 value */
  g_xStatus = SpiritSpiReadRegisters(LINK_QUALIF1_BASE, 1, &tempRegValue);

  /* Rebuild and return the SQI value */
  return (tempRegValue & 0x7F);

}


/**
 * @brief  Returns the LQI value.
 * @param  None.
 * @retval uint8_t LQI value.
 */
uint8_t SpiritQiGetLqi(void)
{
  uint8_t tempRegValue;

  /* Reads the LINK_QUALIF0 register value */
  g_xStatus = SpiritSpiReadRegisters(LINK_QUALIF0_BASE, 1, &tempRegValue);

  /* Rebuild and return the LQI value */
  return ((tempRegValue & 0xF0)>> 4);

}


/**
 * @brief  Returns the CS status.
 * @param  None.
 * @retval SpiritFlagStatus CS value (S_SET or S_RESET).
 */
SpiritFlagStatus SpiritQiGetCs(void)
{
  uint8_t tempRegValue;

  /* Reads the LINK_QUALIF1 register value */
  g_xStatus = SpiritSpiReadRegisters(LINK_QUALIF1_BASE, 1, &tempRegValue);

  /* Rebuild and returns the CS status value */
  if((tempRegValue & 0x80) == 0)
  {
    return S_RESET;
  }
  else
  {
    return S_SET;
  }

}


/**
 * @brief  Returns the RSSI value. The measured power is reported in steps of half a dB from 0 to 255 and is offset in such a way that -120 dBm corresponds
 *         to 20.
 * @param  None.
 * @retval uint8_t RSSI value.
 */
uint8_t SpiritQiGetRssi(void)
{
  uint8_t tempRegValue;

  /* Reads the RSSI_LEVEL register value */
  g_xStatus = SpiritSpiReadRegisters(RSSI_LEVEL_BASE, 1, &tempRegValue);

  /* Returns the RSSI value */
  return tempRegValue;

}


/**
 * @brief  Sets the RSSI threshold.
 * @param  cRssiThr RSSI threshold reported in steps of half a dBm with a -130 dBm offset.
 *         This parameter must be a uint8_t.
 * @retval None.
 */
void SpiritQiSetRssiThreshold(uint8_t cRssiThr)
{
  /* Writes the new value on the RSSI_TH register */
  g_xStatus = SpiritSpiWriteRegisters(RSSI_TH_BASE, 1, &cRssiThr);

}


/**
 * @brief  Returns the RSSI threshold.
 * @param  None.
 * @retval uint8_t RSSI threshold.
 */
uint8_t SpiritQiGetRssiThreshold(void)
{
  uint8_t tempRegValue;

  /* Reads the RSSI_TH register value */
  g_xStatus = SpiritSpiReadRegisters(RSSI_TH_BASE, 1, &tempRegValue);

  /* Returns RSSI threshold */
  return tempRegValue;

}


/**
 * @brief  Computes the RSSI threshold from its dBm value according to the formula: (RSSI[Dbm] + 130)/0.5
 * @param  nDbmValue RSSI threshold reported in dBm.
 *         This parameter must be a sint16_t.
 * @retval uint8_t RSSI threshold corresponding to dBm value.
 */
uint8_t SpiritQiComputeRssiThreshold(int nDbmValue)
{
  /* Check the parameters */
  s_assert_param(IS_RSSI_THR_DBM(nDbmValue));

  /* Computes the RSSI threshold for register */
  return 2*(nDbmValue+130);

}

/**
 * @brief  Sets the RSSI threshold from its dBm value according to the formula: (RSSI[Dbm] + 130)/0.5.
 * @param  nDbmValue RSSI threshold reported in dBm.
 *         This parameter must be a sint16_t.
 * @retval None.
 */
void SpiritQiSetRssiThresholddBm(int nDbmValue)
{
  uint8_t tempRegValue=2*(nDbmValue+130);

  /* Check the parameters */
  s_assert_param(IS_RSSI_THR_DBM(nDbmValue));

  /* Writes the new value on the RSSI_TH register */
  g_xStatus = SpiritSpiWriteRegisters(RSSI_TH_BASE, 1, &tempRegValue);

}

/**
 * @brief  Sets the RSSI filter gain. This parameter sets the bandwidth of a low pass IIR filter (RSSI_FLT register, allowed values 0..15), a
 *         lower values gives a faster settling of the measurements but lower precision. The recommended value for such parameter is 14.
 * @param  xRssiFg RSSI filter gain value.
 *         This parameter can be any value of @ref RssiFilterGain.
 * @retval None.
 */
void SpiritQiSetRssiFilterGain(RssiFilterGain xRssiFg)
{
  uint8_t tempRegValue;

   /* Check the parameters */
  s_assert_param(IS_RSSI_FILTER_GAIN(xRssiFg));

  /* Reads the RSSI_FLT register */
  g_xStatus = SpiritSpiReadRegisters(RSSI_FLT_BASE, 1, &tempRegValue);

  /* Sets the specified filter gain */
  tempRegValue &= 0x0F;
  tempRegValue |= ((uint8_t)xRssiFg);

  /* Writes the new value on the RSSI_FLT register */
  g_xStatus = SpiritSpiWriteRegisters(RSSI_FLT_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the RSSI filter gain.
 * @param  None.
 * @retval RssiFilterGain RSSI filter gain.
 */
RssiFilterGain SpiritQiGetRssiFilterGain(void)
{
  uint8_t tempRegValue;

  /* Reads the RSSI_FLT register */
  g_xStatus = SpiritSpiReadRegisters(RSSI_FLT_BASE, 1, &tempRegValue);

  /* Rebuild and returns the filter gain value */
  return (RssiFilterGain)(tempRegValue & 0xF0);

}


/**
 * @brief  Sets the CS Mode. When static carrier sensing is used (cs_mode = 0), the carrier sense signal is asserted when the measured RSSI is above the
 *         value specified in the RSSI_TH register and is deasserted when the RSSI falls 3 dB below the same threshold.
 *         When dynamic carrier sense is used (cs_mode = 1, 2, 3), the carrier sense signal is asserted if the signal is above the
 *         threshold and a fast power increase of 6, 12 or 18 dB is detected; it is deasserted if a power fall of the same amplitude is
 *         detected.
 * @param  xCsMode CS mode selector.
 *         This parameter can be any value of @ref CSMode.
 * @retval None.
 */
void SpiritQiSetCsMode(CSMode xCsMode)
{
  uint8_t tempRegValue;

   /* Check the parameters */
  s_assert_param(IS_CS_MODE(xCsMode));

  /* Reads the RSSI_FLT register */
  g_xStatus = SpiritSpiReadRegisters(RSSI_FLT_BASE, 1, &tempRegValue);

  /* Sets bit to select the CS mode */
  tempRegValue &= ~0x0C;
  tempRegValue |= ((uint8_t)xCsMode);

  /* Writes the new value on the RSSI_FLT register */
  g_xStatus = SpiritSpiWriteRegisters(RSSI_FLT_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the CS Mode.
 * @param  None.
 * @retval CSMode CS mode.
 */
CSMode SpiritQiGetCsMode(void)
{
  uint8_t tempRegValue;

  /* Reads the RSSI_FLT register */
  g_xStatus = SpiritSpiReadRegisters(RSSI_FLT_BASE, 1, &tempRegValue);

  /* Rebuild and returns the CS mode value */
  return (CSMode)(tempRegValue & 0x0C);

}

/**
 * @brief  Enables/Disables the CS Timeout Mask. If enabled CS value contributes to timeout disabling.
 * @param  xNewState new state for CS Timeout Mask.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritQiCsTimeoutMask(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PROTOCOL2 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

  /* Enables or disables the CS timeout mask */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PROTOCOL2_CS_TIMEOUT_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL2_CS_TIMEOUT_MASK;
  }

  /* Writes the new value on the PROTOCOL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Enables/Disables the PQI Timeout Mask. If enabled PQI value contributes to timeout disabling.
 * @param  xNewState new state for PQI Timeout Mask.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritQiPqiTimeoutMask(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PROTOCOL2 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

  /* Enables or disables the PQI timeout mask */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PROTOCOL2_PQI_TIMEOUT_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL2_PQI_TIMEOUT_MASK;
  }

  /* Writes the new value on the PROTOCOL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Enables/Disables the SQI Timeout Mask. If enabled SQI value contributes to timeout disabling.
 * @param  xNewState new state for SQI Timeout Mask.
 *         This parameter can be S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritQiSqiTimeoutMask(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PROTOCOL2 register */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

  /* Enables or disables the SQI timeout mask */
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= PROTOCOL2_SQI_TIMEOUT_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL2_SQI_TIMEOUT_MASK;
  }

  /* Writes the new value on the PROTOCOL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

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
