/**
  ******************************************************************************
 * @file    SPIRIT_LinearFifo.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT Fifo.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT_LINEAR_FIFO_H
#define __SPIRIT_LINEAR_FIFO_H


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
 * @defgroup SPIRIT_LinearFifo          Linear FIFO
 * @brief Configuration and management of SPIRIT FIFO.
 * @details See the file <i>@ref SPIRIT_LinearFifo.h</i> for more details.
 * @{
 */

/**
 * @defgroup LinearFifo_Exported_Types  Linear FIFO Exported Types
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup LinearFifo_Exported_Constants      Linear FIFO Exported Constants
 * @{
 */
#define IS_FIFO_THR(VAL)  (VAL<=96)

/**
 * @}
 */


/**
 * @defgroup LinearFifo_Exported_Macros         Linear FIFO Exported Macros
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup LinearFifo_Exported_Functions                      Linear FIFO Exported Functions
 * @{
 */

uint8_t SpiritLinearFifoReadNumElementsRxFifo(void);
uint8_t SpiritLinearFifoReadNumElementsTxFifo(void);
void SpiritLinearFifoSetAlmostFullThresholdRx(uint8_t cThrRxFifo);
uint8_t SpiritLinearFifoGetAlmostFullThresholdRx(void);
void SpiritLinearFifoSetAlmostEmptyThresholdRx(uint8_t cThrRxFifo);
uint8_t SpiritLinearFifoGetAlmostEmptyThresholdRx(void);
void SpiritLinearFifoSetAlmostFullThresholdTx(uint8_t cThrTxFifo);
uint8_t SpiritLinearFifoGetAlmostFullThresholdTx(void);
void SpiritLinearFifoSetAlmostEmptyThresholdTx(uint8_t cThrTxFifo);
uint8_t SpiritLinearFifoGetAlmostEmptyThresholdTx(void);

/**
 * @}
 */

/**
 * @}
 */


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
