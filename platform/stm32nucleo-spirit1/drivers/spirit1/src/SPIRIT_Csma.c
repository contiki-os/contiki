/**
  ******************************************************************************
  * @file    SPIRIT_Csma.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT CSMA.
  * @details
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
#include "SPIRIT_Csma.h"
#include "MCU_Interface.h"


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_Csma
 * @{
 */


/**
 * @defgroup Csma_Private_TypesDefinitions      CSMA Private TypesDefinitions
 * @{
 */


/**
 *@}
 */


/**
 * @defgroup Csma_Private_Defines               CSMA Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Csma_Private_Macros               CSMA Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Csma_Private_Variables             CSMA Private Variables
 * @{
 */

/**
 *@}
 */



/**
 * @defgroup Csma_Private_FunctionPrototypes    CSMA Private FunctionPrototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Csma_Private_Functions             CSMA Private Functions
 * @{
 */


/**
 * @brief  Initializes the SPIRIT CSMA according to the specified parameters in the CsmaInit.
 * @param  pxCsmaInit Csma init structure.
 *         This parameter is a pointer to @ref CsmaInit.
 * @retval None.
 */
void SpiritCsmaInit(CsmaInit* pxCsmaInit)
{
  uint8_t tempRegValue[5];

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(pxCsmaInit->xCsmaPersistentMode));
  s_assert_param(IS_CCA_PERIOD(pxCsmaInit->xMultiplierTbit));
  s_assert_param(IS_CSMA_LENGTH(pxCsmaInit->xCcaLength));
  s_assert_param(IS_BU_COUNTER_SEED(pxCsmaInit->nBuCounterSeed));
  s_assert_param(IS_BU_PRESCALER(pxCsmaInit->cBuPrescaler));
  s_assert_param(IS_CMAX_NB(pxCsmaInit->cMaxNb));

  /* CSMA BU counter seed (MSB) config */
  tempRegValue[0] = (uint8_t)(pxCsmaInit->nBuCounterSeed >> 8);

  /* CSMA BU counter seed (LSB) config */
  tempRegValue[1] = (uint8_t) pxCsmaInit->nBuCounterSeed;

  /* CSMA BU prescaler config and CCA period config */
  tempRegValue[2] = (pxCsmaInit->cBuPrescaler << 2) | pxCsmaInit->xMultiplierTbit;

  /* CSMA CCA length config and max number of back-off */
  tempRegValue[3] = (pxCsmaInit->xCcaLength | pxCsmaInit->cMaxNb);

  /* Reads the PROTOCOL1_BASE register value, to write the SEED_RELOAD and CSMA_PERS_ON fields */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue[4]);

  /* Writes the new value for persistent mode */
  if(pxCsmaInit->xCsmaPersistentMode==S_ENABLE)
  {
    tempRegValue[4] |= PROTOCOL1_CSMA_PERS_ON_MASK;
  }
  else
  {
    tempRegValue[4] &= ~PROTOCOL1_CSMA_PERS_ON_MASK;
  }

  /* Writes PROTOCOL1_BASE register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue[4]);

  /* Writes CSMA_CONFIGx_BASE registers */
  g_xStatus = SpiritSpiWriteRegisters(CSMA_CONFIG3_BASE, 4, tempRegValue);

}


 /**
  * @brief  Returns the fitted structure CsmaInit starting from the registers values.
  * @param  pxCsmaInit Csma structure to be fitted.
  *         This parameter is a pointer to @ref CsmaInit.
  * @retval None.
  */
void SpiritCsmaGetInfo(CsmaInit* pxCsmaInit)
{
   uint8_t tempRegValue[5];

   /* Reads PROTOCOL1_BASE register */
   g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue[4]);

   /* Reads CSMA_CONFIGx_BASE registers */
   g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG3_BASE, 4, tempRegValue);

   /* Reads the bu counter seed */
   pxCsmaInit->nBuCounterSeed = (uint16_t)tempRegValue[1] | ((uint16_t)(tempRegValue[0] << 8));

   /* Reads the bu prescaler */
   pxCsmaInit->cBuPrescaler = tempRegValue[2]>>2;

   /* Reads the Cca period */
   pxCsmaInit->xMultiplierTbit = (CcaPeriod)(tempRegValue[2] & 0x03);

   /* Reads the Cca length */
   pxCsmaInit->xCcaLength = (CsmaLength)(tempRegValue[3]&0xF0);

   /* Reads the max number of back off */
   pxCsmaInit->cMaxNb = tempRegValue[3] & 0x07;

   /* Reads the persistent mode enable bit */
   pxCsmaInit->xCsmaPersistentMode = (SpiritFunctionalState)((tempRegValue[4]>>1) & 0x01);

}


/**
 * @brief  Enables or Disables the CSMA.
 * @param  xNewState the state of the CSMA mode.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritCsma(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PROTOCOL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Sets or resets the CSMA enable bit */
  if(xNewState==S_ENABLE)
  {
    tempRegValue |= PROTOCOL1_CSMA_ON_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL1_CSMA_ON_MASK;
  }

  /* Writes the new value on the PROTOCOL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

}

/**
 * @brief  Gets the CSMA mode. Says if it is enabled or disabled.
 * @param  None.
 * @retval SpiritFunctionalState: CSMA mode.
 */
SpiritFunctionalState SpiritCsmaGetCsma(void)
{
  uint8_t tempRegValue;

  /* Reads the PROTOCOL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Return if set or reset */
  if(tempRegValue & PROTOCOL1_CSMA_ON_MASK)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}

/**
 * @brief  Enables or Disables the persistent CSMA mode.
 * @param  xNewState the state of the persistent CSMA mode.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritCsmaPersistentMode(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PROTOCOL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Enables/disables the CSMA persistent mode */
  if(xNewState==S_ENABLE)
  {
    tempRegValue |= PROTOCOL1_CSMA_PERS_ON_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL1_CSMA_PERS_ON_MASK;
  }

  /* Writes the new vaue on the PROTOCOL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Gets the persistent CSMA mode.
 * @param  None.
 * @retval SpiritFunctionalState: CSMA persistent mode.
 */
SpiritFunctionalState SpiritCsmaGetPersistentMode(void)
{
  uint8_t tempRegValue;

  /* Reads the PROTOCOL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Return if set or reset */
  if(tempRegValue & PROTOCOL1_CSMA_PERS_ON_MASK)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }

}


/**
 * @brief  Enables or Disables the seed reload mode (if enabled it reloads the back-off generator seed using the value written in the BU_COUNTER_SEED register).
 * @param  xNewState the state of the seed reload mode.
 *	   This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritCsmaSeedReloadMode(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the PROTOCOL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Enables/disables the seed reload mode */
  if(xNewState==S_ENABLE)
  {
    tempRegValue |= PROTOCOL1_SEED_RELOAD_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL1_SEED_RELOAD_MASK;
  }

  /* Writes the new value on the PROTOCOL1 register */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Gets the seed reload mode.
 * @param  None.
 * @retval SpiritFunctionalState: CSMA seed reload mode.
 */
SpiritFunctionalState SpiritCsmaGetSeedReloadMode(void)
{
  uint8_t tempRegValue;

  /* Reads the PROTOCOL1 register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Return if set or reset */
  if(tempRegValue & PROTOCOL1_SEED_RELOAD_MASK)
  {
    return S_ENABLE;
  }
  else
  {
    return S_DISABLE;
  }
}


/**
 * @brief  Sets the BU counter seed (BU_COUNTER_SEED register). The CSMA back off time is given by the formula: BO = rand(2^NB)*BU.
 * @param  nBuCounterSeed seed of the random number generator used to apply the BBE algorithm.
 *	   This parameter is an uint16_t.
 * @retval None.
 */
void SpiritCsmaSetBuCounterSeed(uint16_t nBuCounterSeed)
{
  uint8_t tempRegValue[2];

  /* Check parameters */
  s_assert_param(IS_BU_COUNTER_SEED(nBuCounterSeed));

  /* Build value (MSB)*/
  tempRegValue[0]=(uint8_t)(nBuCounterSeed>>8);
  /* Build value (LSB) */
  tempRegValue[1]=(uint8_t)nBuCounterSeed;

  /* Writes the CSMA_CONFIG3 registers */
  g_xStatus = SpiritSpiWriteRegisters(CSMA_CONFIG3_BASE, 2, tempRegValue);

}

/**
 * @brief  Returns the BU counter seed (BU_COUNTER_SEED register).
 * @param  None.
 * @retval uint16_t Seed of the random number generator used to apply the BBE algorithm.
 */
uint16_t SpiritCsmaGetBuCounterSeed(void)
{
  uint8_t tempRegValue[2];

  /* Reads the CSMA_CONFIGx registers value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG3_BASE, 2, tempRegValue);

  /* Build the counter seed and return it */
  return ((uint16_t)tempRegValue[1] + (((uint16_t)tempRegValue[0])<<8));

}


/**
 * @brief  Sets the BU prescaler. The CSMA back off time is given by the formula: BO = rand(2^NB)*BU.
 * @param  cBuPrescaler used to program the back-off unit BU.
 * 	   This parameter is an uint8_t.
 * @retval None.
 */
void SpiritCsmaSetBuPrescaler(uint8_t cBuPrescaler)
{
  uint8_t tempRegValue;

  /* Check parameters */
  s_assert_param(IS_BU_PRESCALER(cBuPrescaler));

  /* Reads the CSMA_CONFIG1 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG1_BASE, 1, &tempRegValue);

  /* Build the new value for the BU prescaler */
  tempRegValue &= 0x03;
  tempRegValue |= (cBuPrescaler<<2);

  /* Writes the new value on the CSMA_CONFIG1_BASE register */
  g_xStatus = SpiritSpiWriteRegisters(CSMA_CONFIG1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the BU prescaler.
 * @param  None.
 * @retval uint8_t Value back-off unit (BU).
 */
uint8_t SpiritCsmaGetBuPrescaler(void)
{
  uint8_t tempRegValue;

  /* Reads the CSMA_CONFIG1 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG1_BASE, 1, &tempRegValue);

  /* Build and return the BU prescaler value */
  return (tempRegValue >> 2);

}


/**
 * @brief  Sets the CCA period.
 * @param  xMultiplierTbit value of CCA period to store.
 * 	   This parameter can be a value of @ref CcaPeriod.
 * @retval None.
 */
void SpiritCsmaSetCcaPeriod(CcaPeriod xMultiplierTbit)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_CCA_PERIOD(xMultiplierTbit));

  /* Reads the CSMA_CONFIG1 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG1_BASE, 1, &tempRegValue);

  /* Build the new value setting the the CCA period */
  tempRegValue &= 0xFC;
  tempRegValue |= xMultiplierTbit;

  /* Writes the new value on the CSMA_CONFIG1 register */
  g_xStatus = SpiritSpiWriteRegisters(CSMA_CONFIG1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the CCA period.
 * @param  None.
 * @retval CcaPeriod CCA period.
 */
CcaPeriod SpiritCsmaGetCcaPeriod(void)
{
  uint8_t tempRegValue;

  /* Reads the CSMA_CONFIG1 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG1_BASE, 1, &tempRegValue);

  /* Build and return the CCA period value */
  return (CcaPeriod)(tempRegValue & 0x03);

}


/**
 * @brief  Sets the CCA length.
 * @param  xCcaLength the CCA length (a value between 1 and 15 that multiplies the CCA period).
 *	   This parameter can be any value of @ref CsmaLength.
 * @retval None.
 */
void SpiritCsmaSetCcaLength(CsmaLength xCcaLength)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_CSMA_LENGTH(xCcaLength));

  /* Reads the CSMA_CONFIG0 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG0_BASE, 1, &tempRegValue);

  /* Build the value of CCA length to be set */
  tempRegValue &= 0x0F;
  tempRegValue |= xCcaLength;

  /* Writes the new value on the CSMA_CONFIG0 register */
  g_xStatus = SpiritSpiWriteRegisters(CSMA_CONFIG0_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the CCA length.
 * @param  None.
 * @retval uint8_t CCA length.
 */
uint8_t SpiritCsmaGetCcaLength(void)
{
  uint8_t tempRegValue;

  /* Reads the CSMA_CONFIG0 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG0_BASE, 1, &tempRegValue);

  /* Build and return the CCA length */
  return tempRegValue >> 4;

}


/**
 * @brief  Sets the max number of back-off. If reached Spirit stops the transmission.
 * @param  cMaxNb the max number of back-off.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SpiritCsmaSetMaxNumberBackoff(uint8_t cMaxNb)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_CMAX_NB(cMaxNb));

  /* Reads the CSMA_CONFIG0 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG0_BASE, 1, &tempRegValue);

  /* Build the value of max back off to be set */
  tempRegValue &= 0xF8;
  tempRegValue |= cMaxNb;

  /* Writes the new value on the CSMA_CONFIG0 register */
  g_xStatus = SpiritSpiWriteRegisters(CSMA_CONFIG0_BASE, 1, &tempRegValue);
}

/**
 * @brief  Returns the max number of back-off.
 * @param  None.
 * @retval uint8_t Max number of back-off.
 */
uint8_t SpiritCsmaGetMaxNumberBackoff(void)
{
  uint8_t tempRegValue;

  /* Reads the CSMA_CONFIG0 register value */
  g_xStatus = SpiritSpiReadRegisters(CSMA_CONFIG0_BASE, 1, &tempRegValue);

  /* Build and return the max number of back-off */
  return (tempRegValue & 0x07);

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
