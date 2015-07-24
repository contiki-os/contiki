/**
  ******************************************************************************
  * @file    SPIRIT_DirectRF.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT direct transmission / receive modes.
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
#include "SPIRIT_DirectRF.h"
#include "MCU_Interface.h"



/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_DirectRf
 * @{
 */


/**
 * @defgroup DirectRf_Private_TypesDefinitions          Direct RF Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup DirectRf_Private_Defines                   Direct RF Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup DirectRf_Private_Macros                    Direct RF Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup DirectRf_Private_Variables                 Direct RF Private Variables
 * @{
 */

/**
 *@}
 */



/**
 * @defgroup DirectRf_Private_FunctionPrototypes        Direct RF Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup DirectRf_Private_Functions                 Direct RF Private Functions
 * @{
 */

/**
 * @brief  Sets the DirectRF RX mode of SPIRIT.
 * @param  xDirectRx code of the desired mode.
 *         This parameter can be any value of @ref DirectRx.
 * @retval None.
 */
void SpiritDirectRfSetRxMode(DirectRx xDirectRx)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_DIRECT_RX(xDirectRx));

  /* Reads the register value */
  SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Build the value to be stored */
  tempRegValue &= ~PCKTCTRL3_RX_MODE_MASK;
  tempRegValue |= (uint8_t)xDirectRx;

  /* Writes value on register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the DirectRF RX mode of SPIRIT.
 * @param  None.
 * @retval DirectRx Direct Rx mode.
 */
DirectRx SpiritDirectRfGetRxMode(void)
{
  uint8_t tempRegValue;

  /* Reads the register value and mask the RX_Mode field */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL3_BASE, 1, &tempRegValue);

  /* Rebuild and return value */
  return (DirectRx)(tempRegValue & 0x30);

}


/**
 * @brief  Sets the TX mode of SPIRIT.
 * @param  xDirectTx code of the desired source.
 *         This parameter can be any value of @ref DirectTx.
 * @retval None.
 */
void SpiritDirectRfSetTxMode(DirectTx xDirectTx)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_DIRECT_TX(xDirectTx));

  /* Reads the register value */
  SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Build the value to be stored */
  tempRegValue &= ~PCKTCTRL1_TX_SOURCE_MASK;
  tempRegValue |= (uint8_t)xDirectTx;

  /* Writes value on register */
  g_xStatus = SpiritSpiWriteRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the DirectRF TX mode of SPIRIT.
 * @param  None.
 * @retval DirectTx Direct Tx mode.
 */
DirectTx SpiritDirectRfGetTxMode(void)
{
  uint8_t tempRegValue;

  /* Reads the register value and mask the RX_Mode field */
  g_xStatus = SpiritSpiReadRegisters(PCKTCTRL1_BASE, 1, &tempRegValue);

  /* Returns value */
  return (DirectTx)(tempRegValue & 0x0C);

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
