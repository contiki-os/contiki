/**
  ******************************************************************************
 * @file    SPIRIT_Timer.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT timers.
 * @details
 *
 * This module provides API to configure the Spirit timing mechanisms.
 * They allow the user to set the timer registers using raw values or
 * compute them since the desired timer value is expressed in ms.
 * Moreover the management of the Spirit LDCR mode can be done using
 * these API.
 *
 * <b>Example:</b>
 * @code
 *   ...
 *
 *   SpiritTimerSetRxTimeoutMs(50.0);
 *   SpiritTimerSetWakeUpTimerMs(150.0);
 *
 *   // IRQ configuration for RX_TIMEOUT and WAKEUP_TIMEOUT
 *   ...
 *
 *   SpiritTimerLdcrMode(S_ENABLE);
 *
 *   ...
 *
 * @endcode
 *
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
#ifndef __SPIRIT1_TIMER_H
#define __SPIRIT1_TIMER_H


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
 * @defgroup SPIRIT_Timer               Timer
 * @brief Configuration and management of SPIRIT Timers.
 * @details See the file <i>@ref SPIRIT_Timer.h</i> for more details.
 * @{
 */


/**
 * @defgroup Timer_Exported_Types       Timer Exported Types
 * @{
 */

/**
 * @brief  All the possible RX timeout stop conditions enumeration.
 */
typedef enum{

     NO_TIMEOUT_STOP = 0x00,                /*!< Timeout never stopped */
     TIMEOUT_ALWAYS_STOPPED = 0x08,         /*!< Timeout always stopped (default) */
     RSSI_ABOVE_THRESHOLD = 0x04,           /*!< Timeout stopped on RSSI above threshold */
     SQI_ABOVE_THRESHOLD = 0x02,            /*!< Timeout stopped on SQI above threshold */
     PQI_ABOVE_THRESHOLD = 0x01,            /*!< Timeout stopped on PQI above threshold */
     RSSI_AND_SQI_ABOVE_THRESHOLD = 0x06,   /*!< Timeout stopped on both RSSI and SQI above threshold */
     RSSI_AND_PQI_ABOVE_THRESHOLD = 0x05,   /*!< Timeout stopped on both RSSI and PQI above threshold */
     SQI_AND_PQI_ABOVE_THRESHOLD = 0x03,    /*!< Timeout stopped on both SQI and PQI above threshold */
     ALL_ABOVE_THRESHOLD = 0x07,            /*!< Timeout stopped only if RSSI, SQI and PQI are above threshold */
     RSSI_OR_SQI_ABOVE_THRESHOLD = 0x0E,    /*!< Timeout stopped if one between RSSI or SQI are above threshold */
     RSSI_OR_PQI_ABOVE_THRESHOLD = 0x0D,    /*!< Timeout stopped if one between RSSI or PQI are above threshold */
     SQI_OR_PQI_ABOVE_THRESHOLD = 0x0B,     /*!< Timeout stopped if one between SQI or PQI are above threshold */
     ANY_ABOVE_THRESHOLD = 0x0F             /*!< Timeout stopped if one among RSSI, SQI or SQI are above threshold */

} RxTimeoutStopCondition;


#define IS_RX_TIMEOUT_STOP_CONDITION(COND)  ( COND == NO_TIMEOUT_STOP || \
                                                COND == TIMEOUT_ALWAYS_STOPPED || \
                                                COND == RSSI_ABOVE_THRESHOLD || \
                                                COND == SQI_ABOVE_THRESHOLD || \
                                                COND == PQI_ABOVE_THRESHOLD || \
                                                COND == RSSI_AND_SQI_ABOVE_THRESHOLD || \
                                                COND == RSSI_AND_PQI_ABOVE_THRESHOLD || \
                                                COND == SQI_AND_PQI_ABOVE_THRESHOLD || \
                                                COND == ALL_ABOVE_THRESHOLD || \
                                                COND == RSSI_OR_SQI_ABOVE_THRESHOLD || \
                                                COND == RSSI_OR_PQI_ABOVE_THRESHOLD || \
                                                COND == SQI_OR_PQI_ABOVE_THRESHOLD || \
                                                COND == ANY_ABOVE_THRESHOLD )



/**
 * @}
 */


/**
 * @defgroup Timer_Exported_Constants   Timer Exported Constants
 * @{
 */

/**
 * @brief  It represents the Time Step for RX_Timeout timer in case of 24 MHz Crystal, expressed in us.
 *         It is equal to 1210/(24*10^6). With this time step it is possible to fix the RX_Timeout to
 *         a minimum value of 50.417us to a maximum value of about 3.278 s.
 *         Remember that it is possible to have infinite RX_Timeout writing 0 in the RX_Timeout_Counter and/or RX_Timeout_Prescaler registers.
 */
#define      RX_TCLK_24MHz           50.417f
#define      IS_RX_TIMEOUT_24MHz(TIMEOUT)        (TIMEOUT*1000)>=RX_TCLK_24MHz

/**
 * @brief  It represents the Time Step for RX_Timeout timer in case of 26 MHz Crystal, expressed in us.
 *         It is equal to 1210/(26*10^6). With this time step it is possible to fix the RX_Timeout to
 *         a minimum value of 46.538us to a maximum value of about 3.026 s.
 *         Remember that it is possible to have infinite RX_Timeout writing 0 in the RX_Timeout_Counter register.
 */
#define      RX_TCLK_26MHz           46.538f
#define      IS_RX_TIMEOUT_26MHz(TIMEOUT)        (TIMEOUT*1000)>=RX_TCLK_26MHz

/**
 * @brief  It represents the Time Step for RX_Wakeup timer expressed in us. This timer is based on RCO (about 34.7 kHZ).
 *         With this time step it is possible to fix the Wakeup_Timeout to a minimum value of 28.818us to a maximum
 *         value of about 1.888 s.
 */
#define      WAKEUP_TCLK            28.818f
#define      IS_WKUP_TIMEOUT(TIMEOUT)        (TIMEOUT*1000)>=WAKEUP_TCLK



/**
 * @}
 */


/**
 * @defgroup Timer_Exported_Macros              Timer Exported Macros
 * @{
 */

#define SET_INFINITE_RX_TIMEOUT()     SpiritTimerSetRxTimeoutCounter(0)

/**
 * @}
 */


/**
 * @defgroup Timer_Exported_Functions           Timer Exported Functions
 * @{
 */

void SpiritTimerLdcrMode(SpiritFunctionalState xNewState);
void SpiritTimerLdcrAutoReload(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritTimerLdcrGetAutoReload(void);
void SpiritTimerSetRxTimeout(uint8_t cCounter , uint8_t cPrescaler);
void SpiritTimerSetRxTimeoutMs(float fDesiredMsec);
void SpiritTimerSetRxTimeoutCounter(uint8_t cCounter);
void SpiritTimerSetRxTimeoutPrescaler(uint8_t cPrescaler);
void SpiritTimerGetRxTimeout(float* pfTimeoutMsec, uint8_t* pcCounter , uint8_t* pcPrescaler);
void SpiritTimerSetWakeUpTimer(uint8_t cCounter , uint8_t cPrescaler);
void SpiritTimerSetWakeUpTimerMs(float fDesiredMsec);
void SpiritTimerSetWakeUpTimerCounter(uint8_t cCounter);
void SpiritTimerSetWakeUpTimerPrescaler(uint8_t cPrescaler);
void SpiritTimerSetWakeUpTimerReloadMs(float fDesiredMsec);
void SpiritTimerGetWakeUpTimer(float* pfWakeUpMsec, uint8_t* pcCounter , uint8_t* pcPrescaler);
void SpiritTimerSetWakeUpTimerReload(uint8_t cCounter , uint8_t cPrescaler);
void SpiritTimerSetWakeUpTimerReloadCounter(uint8_t cCounter);
void SpiritTimerSetWakeUpTimerReloadPrescaler(uint8_t cPrescaler);
void SpiritTimerGetWakeUpTimerReload(float* pfWakeUpReloadMsec, uint8_t* pcCounter , uint8_t* pcPrescaler);
void SpiritTimerComputeWakeUpValues(float fDesiredMsec , uint8_t* pcCounter , uint8_t* pcPrescaler);
void SpiritTimerComputeRxTimeoutValues(float fDesiredMsec , uint8_t* pcCounter , uint8_t* pcPrescaler);
void SpiritTimerSetRxTimeoutStopCondition(RxTimeoutStopCondition xStopCondition);
void SpiritTimerReloadStrobe(void);
uint16_t SpiritTimerGetRcoFrequency(void);

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

