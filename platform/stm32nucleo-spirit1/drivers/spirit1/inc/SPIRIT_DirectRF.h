/**
  ******************************************************************************
 * @file    SPIRIT_DirectRF.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT direct transmission / receive modes.
 * @details
 *
 * This module contains functions to manage the direct Tx/Rx mode.
 * The user can choose the way to send data to Spirit through the
 * enumerative types <i>@ref DirectTx</i>/<i>@ref DirectRx</i>.
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
#ifndef __SPIRIT1_DIRECT_RF_H
#define __SPIRIT1_DIRECT_RF_H

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
 * @defgroup SPIRIT_DirectRf    Direct RF
 * @brief Configuration and management of SPIRIT direct transmission / receive modes.
 * @details See the file <i>@ref SPIRIT_DirectRF.h</i> for more details.
 * @{
 */

/**
 * @defgroup DirectRf_Exported_Types    Direct RF Exported Types
 * @{
 */

/**
 * @brief  Direct transmission mode enumeration for SPIRIT.
 */
typedef enum
{
  NORMAL_TX_MODE = 0x00,          /*!< Normal mode, no direct transmission is used */
  DIRECT_TX_FIFO_MODE = 0x04,     /*!< Source is FIFO: payload bits are continuously read from the TX FIFO */
  DIRECT_TX_GPIO_MODE = 0x08,     /*!< Source is GPIO: payload bits are continuously read from one of the GPIO ports and transmitted without any processing */
  PN9_TX_MODE = 0x0C              /*!< A pseudorandom binary sequence is generated internally */
}DirectTx;

#define IS_DIRECT_TX(MODE)  (((MODE) == NORMAL_TX_MODE) || \
			     ((MODE) == DIRECT_TX_FIFO_MODE) || \
                             ((MODE) == DIRECT_TX_GPIO_MODE)  || \
                             ((MODE) == PN9_TX_MODE))

/**
 * @brief  Direct receive mode enumeration for SPIRIT.
 */
typedef enum
{
  NORMAL_RX_MODE = 0x00,          /*!< Normal mode, no direct reception is used */
  DIRECT_RX_FIFO_MODE = 0x10,     /*!< Destination is FIFO: payload bits are continuously written to the RX FIFO and not subjected to any processing*/
  DIRECT_RX_GPIO_MODE = 0x20      /*!< Destination is GPIO: payload bits are continuously written to one of the GPIO ports and not subjected to any processing*/
}DirectRx;

#define IS_DIRECT_RX(MODE)  (((MODE) == NORMAL_RX_MODE) || \
		             ((MODE) == DIRECT_RX_FIFO_MODE) || \
		             ((MODE) == DIRECT_RX_GPIO_MODE))


/**
 *@}
 */


/**
 * @defgroup DirectRf_Exported_Constants        Direct RF Exported Constants
 * @{
 */


/**
 *@}
 */


/**
 * @defgroup DirectRf_Exported_Macros           Direct RF Exported Macros
 * @{
 */


/**
 *@}
 */


/**
 * @defgroup DirectRf_Exported_Functions        Direct RF Exported Functions
 * @{
 */

void SpiritDirectRfSetRxMode(DirectRx xDirectRx);
DirectRx SpiritDirectRfGetRxMode(void);
void SpiritDirectRfSetTxMode(DirectTx xDirectTx);
DirectTx SpiritDirectRfGetTxMode(void);

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
