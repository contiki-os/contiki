/**
  ******************************************************************************
 * @file    SPIRIT_Irq.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT IRQs.
  * 
 * @details
 *
 * On the Spirit side specific IRQs can be enabled by setting a specific bitmask.
 * The Spirit libraries allow the user to do this in two different ways:
 * <ul>
 *
 * <li>The first enables the IRQs one by one, i.e. using an SPI transaction for each
 * IRQ to enable.
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritIrqDeInit(NULL);                // this call is used to reset the IRQ mask registers
 *  SpiritIrq(RX_DATA_READY , S_ENABLE);
 *  SpiritIrq(VALID_SYNC , S_ENABLE);
 *  SpiritIrq(RX_TIMEOUT , S_ENABLE);
 *
 * @endcode
 *
 * </li>
 *
 * <li>The second strategy is to set the IRQ bitfields structure. So, during the initialization the user
 * has to fill the @ref SpiritIrqs structure setting to one the single field related to the IRQ he
 * wants to enable, and to zero the single field related to all the IRQs he wants to disable.
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritIrqs irqMask;
 *
 *  ...
 *
 *  SpiritIrqDeInit(&irqMask);                // this call is used to reset the IRQ mask registers
 *                                            // and to set to 0x00000000 the irq mask in order to disable
 *                                            // all IRQs (disabled by default on startup)
 *  irqMask.IRQ_RX_DATA_READY = 1;
 *  irqMask.IRQ_VALID_SYNC = 1;
 *  irqMask.IRQ_RX_TIMEOUT = 1;
 *
 *  ...
 * @endcode
 * </li>
 * </ul>
 *
 * The most applications will require a Spirit IRQ notification on an microcontroller EXTI line.
 * Then, the user can check which IRQ has been raised using two different ways.
 *
 * On the ISR of the EXTI line phisically linked to the Spirit pin configured for IRQ:
 *
 * <ul>
 * <li> Check <b>only one</b> Spirit IRQ (because the Spirit IRQ status register automatically blanks itself
 * after an SPI reading) into the ISR.
 *
 * <b>Example:</b>
 * @code
 *
 *  if(SpiritIrqCheckFlag(RX_DATA_READY))
 *  {
 *          // do something...
 *  }
 *
 * @endcode
 * </li>
 *
 * <li> Check more than one Spirit IRQ status by storing the entire IRQ status registers into a bitfields <i>@ref SpiritIrqs</i> structure
 * and then check the interested bits.
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritIrqGetStatus(&irqStatus);
 *
 *  if(irqStatus.IRQ_RX_DATA_READY)
 *  {
 *          // do something...
 *  }
 *  if(irqStatus.IRQ_VALID_SYNC)
 *  {
 *         // do something...
 *  }
 *  if(irqStatus.RX_TIMEOUT)
 *  {
 *         // do something...
 *  }
 *
 * @endcode
 * </li>
 * </ul>
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
#ifndef __SPIRIT1_IRQ_H
#define __SPIRIT1_IRQ_H


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
 * @defgroup SPIRIT_Irq IRQ
 * @brief Configuration and management of SPIRIT IRQs.
 * @details See the file <i>@ref SPIRIT_Irq.h</i> for more details.
 * @{
 */

/**
 * @defgroup Irq_Exported_Types IRQ Exported Types
 * @{
 */


/**
 * @brief IRQ bitfield structure for SPIRIT. This structure is used to read or write the single IRQ bit.
 *        During the initialization the user has to fill this structure setting to one the single field related
 *        to the IRQ he wants to enable, and to zero the single field related to all the IRQs he wants to disable.
 *        The same structure can be used to retrieve all the IRQ events from the IRQ registers IRQ_STATUS[3:0],
 *        and read if one or more specific IRQ raised.
 * @note  The fields order in the structure depends on used endianness (little or big
 *        endian). The actual definition is valid ONLY for LITTLE ENDIAN mode. Be sure to
 *        change opportunely the fields order when use a different endianness.
 */
typedef struct
{
  SpiritFlagStatus  IRQ_RX_DATA_READY:1;            /*!< IRQ: RX data ready */
  SpiritFlagStatus  IRQ_RX_DATA_DISC:1;             /*!< IRQ: RX data discarded (upon filtering) */
  SpiritFlagStatus  IRQ_TX_DATA_SENT:1;             /*!< IRQ: TX data sent */
  SpiritFlagStatus  IRQ_MAX_RE_TX_REACH:1;          /*!< IRQ: Max re-TX reached */
  SpiritFlagStatus  IRQ_CRC_ERROR:1;                /*!< IRQ: CRC error */
  SpiritFlagStatus  IRQ_TX_FIFO_ERROR:1;            /*!< IRQ: TX FIFO underflow/overflow error */
  SpiritFlagStatus  IRQ_RX_FIFO_ERROR:1;            /*!< IRQ: RX FIFO underflow/overflow error */
  SpiritFlagStatus  IRQ_TX_FIFO_ALMOST_FULL:1;      /*!< IRQ: TX FIFO almost full */

  SpiritFlagStatus  IRQ_TX_FIFO_ALMOST_EMPTY:1;     /*!< IRQ: TX FIFO almost empty */
  SpiritFlagStatus  IRQ_RX_FIFO_ALMOST_FULL:1;      /*!< IRQ: RX FIFO almost full */
  SpiritFlagStatus  IRQ_RX_FIFO_ALMOST_EMPTY:1;     /*!< IRQ: RX FIFO almost empty  */
  SpiritFlagStatus  IRQ_MAX_BO_CCA_REACH:1;         /*!< IRQ: Max number of back-off during CCA */
  SpiritFlagStatus  IRQ_VALID_PREAMBLE:1;           /*!< IRQ: Valid preamble detected */
  SpiritFlagStatus  IRQ_VALID_SYNC:1;               /*!< IRQ: Sync word detected */
  SpiritFlagStatus  IRQ_RSSI_ABOVE_TH:1;            /*!< IRQ: RSSI above threshold */
  SpiritFlagStatus  IRQ_WKUP_TOUT_LDC:1;            /*!< IRQ: Wake-up timeout in LDC mode */

  SpiritFlagStatus  IRQ_READY:1;                    /*!< IRQ: READY state */
  SpiritFlagStatus  IRQ_STANDBY_DELAYED:1;          /*!< IRQ: STANDBY state after MCU_CK_CONF_CLOCK_TAIL_X clock cycles */
  SpiritFlagStatus  IRQ_LOW_BATT_LVL:1;             /*!< IRQ: Battery level below threshold*/
  SpiritFlagStatus  IRQ_POR:1;                      /*!< IRQ: Power On Reset */
  SpiritFlagStatus  IRQ_BOR:1;                      /*!< IRQ: Brown out event (both accurate and inaccurate)*/
  SpiritFlagStatus  IRQ_LOCK:1;                     /*!< IRQ: LOCK state */
  SpiritFlagStatus  IRQ_PM_COUNT_EXPIRED:1;         /*!< IRQ: only for debug; Power Management startup timer expiration (see reg PM_START_COUNTER, 0xB5) */
  SpiritFlagStatus  IRQ_XO_COUNT_EXPIRED:1;         /*!< IRQ: only for debug; Crystal oscillator settling time counter expired */

  SpiritFlagStatus  IRQ_SYNTH_LOCK_TIMEOUT:1;       /*!< IRQ: only for debug; LOCK state timeout */
  SpiritFlagStatus  IRQ_SYNTH_LOCK_STARTUP:1;       /*!< IRQ: only for debug; see CALIBR_START_COUNTER */
  SpiritFlagStatus  IRQ_SYNTH_CAL_TIMEOUT:1;        /*!< IRQ: only for debug; SYNTH calibration timeout */
  SpiritFlagStatus  IRQ_TX_START_TIME:1;            /*!< IRQ: only for debug; TX circuitry startup time; see TX_START_COUNTER */
  SpiritFlagStatus  IRQ_RX_START_TIME:1;            /*!< IRQ: only for debug; RX circuitry startup time; see TX_START_COUNTER */
  SpiritFlagStatus  IRQ_RX_TIMEOUT:1;               /*!< IRQ: RX operation timeout */
  SpiritFlagStatus  IRQ_AES_END:1;                  /*!< IRQ: AES End of operation */
  SpiritFlagStatus  :1;                             /*!< Reserved bit */

} SpiritIrqs;


/**
 * @brief  IRQ list enumeration for SPIRIT. This enumeration type can be used to address a
 *         specific IRQ.
 */
typedef enum
{
  RX_DATA_READY = 0x00000001,           /*!< IRQ: RX data ready */
  RX_DATA_DISC = 0x00000002,            /*!< IRQ: RX data discarded (upon filtering) */
  TX_DATA_SENT = 0x00000004,            /*!< IRQ: TX data sent */
  MAX_RE_TX_REACH = 0x00000008,         /*!< IRQ: Max re-TX reached */
  CRC_ERROR = 0x00000010,               /*!< IRQ: CRC error */
  TX_FIFO_ERROR = 0x00000020,           /*!< IRQ: TX FIFO underflow/overflow error */
  RX_FIFO_ERROR = 0x00000040,           /*!< IRQ: RX FIFO underflow/overflow error */
  TX_FIFO_ALMOST_FULL = 0x00000080,     /*!< IRQ: TX FIFO almost full */
  TX_FIFO_ALMOST_EMPTY = 0x00000100,    /*!< IRQ: TX FIFO almost empty */
  RX_FIFO_ALMOST_FULL = 0x00000200,     /*!< IRQ: RX FIFO almost full */
  RX_FIFO_ALMOST_EMPTY = 0x00000400,    /*!< IRQ: RX FIFO almost empty  */
  MAX_BO_CCA_REACH = 0x00000800,        /*!< IRQ: Max number of back-off during CCA */
  VALID_PREAMBLE = 0x00001000,          /*!< IRQ: Valid preamble detected */
  VALID_SYNC = 0x00002000,              /*!< IRQ: Sync word detected */
  RSSI_ABOVE_TH = 0x00004000,           /*!< IRQ: RSSI above threshold */
  WKUP_TOUT_LDC = 0x00008000,           /*!< IRQ: Wake-up timeout in LDC mode */
  READY = 0x00010000,                   /*!< IRQ: READY state */
  STANDBY_DELAYED = 0x00020000,         /*!< IRQ: STANDBY state after MCU_CK_CONF_CLOCK_TAIL_X clock cycles */
  LOW_BATT_LVL = 0x00040000,            /*!< IRQ: Battery level below threshold*/
  POR = 0x00080000,                     /*!< IRQ: Power On Reset */
  BOR = 0x00100000,                     /*!< IRQ: Brown out event (both accurate and inaccurate)*/
  LOCK = 0x00200000,                    /*!< IRQ: LOCK state */
  PM_COUNT_EXPIRED = 0x00400000,        /*!< IRQ: only for debug; Power Management startup timer expiration (see reg PM_START_COUNTER, 0xB5) */
  XO_COUNT_EXPIRED = 0x00800000,        /*!< IRQ: only for debug; Crystal oscillator settling time counter expired */
  SYNTH_LOCK_TIMEOUT = 0x01000000,      /*!< IRQ: only for debug; LOCK state timeout */
  SYNTH_LOCK_STARTUP = 0x02000000,      /*!< IRQ: only for debug; see CALIBR_START_COUNTER */
  SYNTH_CAL_TIMEOUT = 0x04000000,       /*!< IRQ: only for debug; SYNTH calibration timeout */
  TX_START_TIME = 0x08000000,	        /*!< IRQ: only for debug; TX circuitry startup time; see TX_START_COUNTER */
  RX_START_TIME = 0x10000000,	        /*!< IRQ: only for debug; RX circuitry startup time; see TX_START_COUNTER */
  RX_TIMEOUT = 0x20000000,	        /*!< IRQ: RX operation timeout */
  AES_END = 0x40000000,                 /*!< IRQ: AES End of operation */
  ALL_IRQ = 0x7FFFFFFF			/*!< All the above mentioned IRQs */

} IrqList;

#define IS_SPIRIT_IRQ_LIST(VALUE)   ((VALUE == RX_DATA_READY) || \
                                     (VALUE == RX_DATA_DISC)  || \
                                     (VALUE == TX_DATA_SENT)  || \
                                     (VALUE == MAX_RE_TX_REACH)  || \
                                     (VALUE == CRC_ERROR)  || \
                                     (VALUE == TX_FIFO_ERROR)  || \
                                     (VALUE == RX_FIFO_ERROR)  || \
                                     (VALUE == TX_FIFO_ALMOST_FULL)  || \
                                     (VALUE == TX_FIFO_ALMOST_EMPTY)  || \
                                     (VALUE == RX_FIFO_ALMOST_FULL)  || \
                                     (VALUE == RX_FIFO_ALMOST_EMPTY)  || \
                                     (VALUE == MAX_BO_CCA_REACH)  || \
                                     (VALUE == VALID_PREAMBLE)  || \
                                     (VALUE == VALID_SYNC)  || \
                                     (VALUE == RSSI_ABOVE_TH)  || \
                                     (VALUE == WKUP_TOUT_LDC)  || \
                                     (VALUE == READY)  || \
                                     (VALUE == STANDBY_DELAYED)  || \
                                     (VALUE == LOW_BATT_LVL)  || \
                                     (VALUE == POR)  || \
                                     (VALUE == BOR)  || \
                                     (VALUE == LOCK)  || \
                                     (VALUE == PM_COUNT_EXPIRED)  || \
                                     (VALUE == XO_COUNT_EXPIRED)  || \
                                     (VALUE == SYNTH_LOCK_TIMEOUT)  || \
                                     (VALUE == SYNTH_LOCK_STARTUP)  || \
                                     (VALUE == SYNTH_CAL_TIMEOUT)  || \
                                     (VALUE == TX_START_TIME)  || \
                                     (VALUE == RX_START_TIME)  || \
                                     (VALUE == RX_TIMEOUT)  || \
                                     (VALUE == AES_END)   || \
				     (VALUE == ALL_IRQ ))


/**
 * @}
 */


/**
 * @defgroup Irq_Exported_Constants     IRQ Exported Constants
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup Irq_Exported_Macros        IRQ Exported Macros
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup Irq_Exported_Functions     IRQ Exported Functions
 * @{
 */

void SpiritIrqDeInit(SpiritIrqs* pxIrqInit);
void SpiritIrqInit(SpiritIrqs* pxIrqInit);
void SpiritIrq(IrqList xIrq, SpiritFunctionalState xNewState);
void SpiritIrqGetMask(SpiritIrqs* pxIrqMask);
void SpiritIrqGetStatus(SpiritIrqs* pxIrqStatus);
void SpiritIrqClearStatus(void);
SpiritBool SpiritIrqCheckFlag(IrqList xFlag);

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
