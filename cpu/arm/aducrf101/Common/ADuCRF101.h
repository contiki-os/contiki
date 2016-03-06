/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/****************************************************************************************************//**
 * @file     ADuCRF101.h
 *
 * @brief    CMSIS Cortex-M3 Core Peripheral Access Layer Header File for
 *           default ADUCRF101 Device Series
 *
 * @version  V1.0
 * @date     Thursday January 10 2013 15:30
 *
 *******************************************************************************************************/



/** @addtogroup ADUCRF101
  * @{
  */

#ifndef __ADUCRF101_H__
#define __ADUCRF101_H__

#ifndef __NO_MMR_STRUCTS__
// The new style CMSIS structure definitions for MMRs clash with
// the old style defs. If the old style are required for compilation
// then set __NO_MMR_STRUCTS__ to 0x1
#define __NO_MMR_STRUCTS__ 0x0
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum {
/* -------------------  Cortex-M3 Processor Exceptions Numbers  ------------------- */
  Reset_IRQn                        = -15,  /*!<   1  Reset Vector, invoked on Power up and warm reset */
  NonMaskableInt_IRQn               = -14,  /*!<   2  Non maskable Interrupt, cannot be stopped or preempted */
  HardFault_IRQn                    = -13,  /*!<   3  Hard Fault, all classes of Fault */
  MemoryManagement_IRQn             = -12,  /*!<   4  Memory Management, MPU mismatch, including Access Violation and No Match */
  BusFault_IRQn                     = -11,  /*!<   5  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory related Fault */
  UsageFault_IRQn                   = -10,  /*!<   6  Usage Fault, i.e. Undef Instruction, Illegal State Transition */
  SVCall_IRQn                       = -5,   /*!<  11  System Service Call via SVC instruction */
  DebugMonitor_IRQn                 = -4,   /*!<  12  Debug Monitor                    */
  PendSV_IRQn                       = -2,   /*!<  14  Pendable request for system service */
  SysTick_IRQn                      = -1,   /*!<  15  System Tick Timer                */
// --------------------------  ADUCRF101 Specific Interrupt Numbers  ------------------------------
  WUT_IRQn                          = 0,    /*!<   0  WUT                              */
  EINT0_IRQn                        = 1,    /*!<   1  EINT0                            */
  EINT1_IRQn                        = 2,    /*!<   2  EINT1                            */
  EINT2_IRQn                        = 3,    /*!<   3  EINT2                            */
  EINT3_IRQn                        = 4,    /*!<   4  EINT3                            */
  EINT4_IRQn                        = 5,    /*!<   5  EINT4                            */
  EINT5_IRQn                        = 6,    /*!<   6  EINT5                            */
  EINT6_IRQn                        = 7,    /*!<   7  EINT6                            */
  EINT7_IRQn                        = 8,    /*!<   8  EINT7                            */
  EINT8_IRQn                        = 9,    /*!<   9  EINT8                            */
  UHFTRX_IRQn                       = 9,    /*!<   9  UHFTRX                           */
  WDT_IRQn                          = 10,   /*!<  10  WDT                              */
  TIMER0_IRQn                       = 12,   /*!<  12  TIMER0                           */
  TIMER1_IRQn                       = 13,   /*!<  13  TIMER1                           */
  ADC0_IRQn                         = 14,   /*!<  14  ADC0                             */
  FLASH_IRQn                        = 15,   /*!<  15  FLASH                            */
  UART_IRQn                         = 16,   /*!<  16  UART                             */
  SPI0_IRQn                         = 17,   /*!<  17  SPI0                             */
  SPI1_IRQn                         = 18,   /*!<  18  SPI1                             */
  I2CS_IRQn                         = 19,   /*!<  19  I2CS                             */
  I2CM_IRQn                         = 20,   /*!<  20  I2CM                             */
  DMA_ERR_IRQn                      = 23,   /*!<  23  DMA_ERR                          */
  DMA_SPI1_TX_IRQn                  = 24,   /*!<  24  DMA_SPI1_TX                      */
  DMA_SPI1_RX_IRQn                  = 25,   /*!<  25  DMA_SPI1_RX                      */
  DMA_UART_TX_IRQn                  = 26,   /*!<  26  DMA_UART_TX                      */
  DMA_UART_RX_IRQn                  = 27,   /*!<  27  DMA_UART_RX                      */
  DMA_I2CS_TX_IRQn                  = 28,   /*!<  28  DMA_I2CS_TX                      */
  DMA_I2CS_RX_IRQn                  = 29,   /*!<  29  DMA_I2CS_RX                      */
  DMA_I2CM_TX_IRQn                  = 30,   /*!<  30  DMA_I2CM_TX                      */
  DMA_I2CM_RX_IRQn                  = 31,   /*!<  31  DMA_I2CM_RX                      */
  DMA_ADC0_IRQn                     = 35,   /*!<  35  DMA_ADC0                         */
  DMA_SPI0_TX_IRQn                  = 36,   /*!<  36  DMA_SPI0_TX                      */
  DMA_SPI0_RX_IRQn                  = 37,   /*!<  37  DMA_SPI0_RX                      */
  PWM_TRIP_IRQn                     = 38,   /*!<  38  PWM_TRIP                         */
  PWM_PAIR0_IRQn                    = 39,   /*!<  39  PWM_PAIR0                        */
  PWM_PAIR1_IRQn                    = 40,   /*!<  40  PWM_PAIR1                        */
  PWM_PAIR2_IRQn                    = 41,   /*!<  41  PWM_PAIR2                        */
  PWM_PAIR3_IRQn                    = 42    /*!<  42  PWM_PAIR3                        */
} IRQn_Type;


/** @addtogroup Configuration_of_CMSIS
  * @{
  */

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* ----------------Configuration of the cm3 Processor and Core Peripherals---------------- */

#define __CM3_REV              0x0200       /*!< Cortex-M3 Core Revision          */
#define __MPU_PRESENT             0         /*!< MPU present or not                    */
#define __NVIC_PRIO_BITS          3         /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used */
/** @} */ /* End of group Configuration_of_CMSIS */

#include <core_cm3.h>                       /*!< Cortex-M3 processor and core peripherals */
#include "system_ADuCRF101.h"               /*!< ADUCRF101 System                      */


/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */


/** @addtogroup Device_Peripheral_Registers
  * @{
  */


/* -------------------  Start of section using anonymous unions  ------------------ */
#if defined(__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined(__ICCARM__)
  #pragma language=extended
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
  #pragma warning 586
#else
  #warning Not supported compiler type
#endif




/* TCON[EVENTEN] - Enable event bit. */
#define TCON_EVENTEN_MSK               (0x1   << 12 )
#define TCON_EVENTEN                   (0x1   << 12 )
#define TCON_EVENTEN_DIS               (0x0   << 12 ) /* DIS. Cleared by user.    */
#define TCON_EVENTEN_EN                (0x1   << 12 ) /* EN. Enable time capture of an event. */

/* TCON[EVENT] - Event select bits. Settings not described are reserved and should not be used. */
#define TCON_EVENT_MSK                 (0xF   << 8  )

/* TCON[RLD] - Reload control bit for periodic mode. */
#define TCON_RLD_MSK                   (0x1   << 7  )
#define TCON_RLD                       (0x1   << 7  )
#define TCON_RLD_DIS                   (0x0   << 7  ) /* DIS. Reload on a time out. */
#define TCON_RLD_EN                    (0x1   << 7  ) /* EN. Reload the counter on a write to T0CLRI. */

/* TCON[CLK] - Clock select. */
#define TCON_CLK_MSK                   (0x3   << 5  )
#define TCON_CLK_UCLK                  (0x0   << 5  ) /* UCLK. Undivided system clock. */
#define TCON_CLK_PCLK                  (0x1   << 5  ) /* PCLK. Peripheral clock.  */
#define TCON_CLK_LFOSC                 (0x2   << 5  ) /* LFOSC. 32 kHz internal oscillator. */
#define TCON_CLK_LFXTAL                (0x3   << 5  ) /* LFXTAL. 32 kHz external crystal. */

/* TCON[ENABLE] - Timer enable bit. */
#define TCON_ENABLE_MSK                (0x1   << 4  )
#define TCON_ENABLE                    (0x1   << 4  )
#define TCON_ENABLE_DIS                (0x0   << 4  ) /* DIS. Disable the timer. Clearing this bit resets the timer, including the T0VAL register. */
#define TCON_ENABLE_EN                 (0x1   << 4  ) /* EN. Enable the timer.  The timer starts counting from its initial value, 0 if count-up mode or 0xFFFF if count-down mode. */

/* TCON[MOD] - Timer mode. */
#define TCON_MOD_MSK                   (0x1   << 3  )
#define TCON_MOD                       (0x1   << 3  )
#define TCON_MOD_FREERUN               (0x0   << 3  ) /* FREERUN. Operate in free running mode. */
#define TCON_MOD_PERIODIC              (0x1   << 3  ) /* PERIODIC. Operate in periodic mode. */

/* TCON[UP] - Count down/up. */
#define TCON_UP_MSK                    (0x1   << 2  )
#define TCON_UP                        (0x1   << 2  )
#define TCON_UP_DIS                    (0x0   << 2  ) /* DIS. Timer to count down. */
#define TCON_UP_EN                     (0x1   << 2  ) /* EN. Timer to count up.   */

/* TCON[PRE] - Prescaler. */
#define TCON_PRE_MSK                   (0x3   << 0  )
#define TCON_PRE_DIV1                  (0x0   << 0  ) /* DIV1. Source clock/1.If the selected clock source is UCLK or PCLK this setting results in a prescaler of 4. */
#define TCON_PRE_DIV16                 (0x1   << 0  ) /* DIV16. Source clock/16.  */
#define TCON_PRE_DIV256                (0x2   << 0  ) /* DIV256. Source clock/256. */
#define TCON_PRE_DIV32768              (0x3   << 0  ) /* DIV32768. Source clock/32768. */

/* TCLRI[CAP] - Clear captured event interrupt. */
#define TCLRI_CAP_MSK                  (0x1   << 1  )
#define TCLRI_CAP                      (0x1   << 1  )
#define TCLRI_CAP_CLR                  (0x1   << 1  ) /* CLR. Clear a captured event interrupt. This bit always reads 0. */

/* TCLRI[TMOUT] - Clear timeout interrupt. */
#define TCLRI_TMOUT_MSK                (0x1   << 0  )
#define TCLRI_TMOUT                    (0x1   << 0  )
#define TCLRI_TMOUT_CLR                (0x1   << 0  ) /* CLR. Clear a timeout interrupt. This bit always reads 0. */

/* TSTA[CLRI] - T0CLRI write sync in progress.. */
#define TSTA_CLRI_MSK                  (0x1   << 7  )
#define TSTA_CLRI                      (0x1   << 7  )
#define TSTA_CLRI_CLR                  (0x0   << 7  ) /* CLR. Cleared when the interrupt is cleared in the timer clock domain. */
#define TSTA_CLRI_SET                  (0x1   << 7  ) /* SET. Set automatically when the T0CLRI value is being updated in the timer clock domain, indicating that the timers configuration is not yet valid. */

/* TSTA[CON] - T0CON write sync in progress. */
#define TSTA_CON_MSK                   (0x1   << 6  )
#define TSTA_CON                       (0x1   << 6  )
#define TSTA_CON_CLR                   (0x0   << 6  ) /* CLR. Timer ready to receive commands to T0CON. The previous change of T0CON has been synchronized in the timer clock domain. */
#define TSTA_CON_SET                   (0x1   << 6  ) /* SET. Timer not ready to receive commands to T0CON. Previous change of the T0CON value has not been synchronized in the timer clock domain. */

/* TSTA[CAP] - Capture event pending. */
#define TSTA_CAP_MSK                   (0x1   << 1  )
#define TSTA_CAP                       (0x1   << 1  )
#define TSTA_CAP_CLR                   (0x0   << 1  ) /* CLR. No capture event is pending. */
#define TSTA_CAP_SET                   (0x1   << 1  ) /* SET. Capture event is pending. */

/* TSTA[TMOUT] - Time out event occurred. */
#define TSTA_TMOUT_MSK                 (0x1   << 0  )
#define TSTA_TMOUT                     (0x1   << 0  )
#define TSTA_TMOUT_CLR                 (0x0   << 0  ) /* CLR. No timeout event has occurred. */
#define TSTA_TMOUT_SET                 (0x1   << 0  ) /* SET. Timeout event has occurred.  For count-up mode, this is when the counter reaches full scale. For count-down mode, this is when the counter reaches 0. */

/* GPCON[CON7] - Configuration bits for Px.7 (not available for port 1). */
#define GPCON_CON7_MSK                 (0x3   << 14 )

/* GPCON[CON6] - Configuration bits for Px.6 (not available for port 1). */
#define GPCON_CON6_MSK                 (0x3   << 12 )

/* GPCON[CON5] - Configuration bits for Px.5. */
#define GPCON_CON5_MSK                 (0x3   << 10 )

/* GPCON[CON4] - Configuration bits for Px.4. */
#define GPCON_CON4_MSK                 (0x3   << 8  )

/* GPCON[CON3] - Configuration bits for Px.3. */
#define GPCON_CON3_MSK                 (0x3   << 6  )

/* GPCON[CON2] - Configuration bits for Px.2. */
#define GPCON_CON2_MSK                 (0x3   << 4  )

/* GPCON[CON1] - Configuration bits for Px.1. */
#define GPCON_CON1_MSK                 (0x3   << 2  )

/* GPCON[CON0] - Configuration bits for Px.0. */
#define GPCON_CON0_MSK                 (0x3   << 0  )

/* GPOEN[OEN7] - Port pin direction. */
#define GPOEN_OEN7_MSK                 (0x1   << 7  )
#define GPOEN_OEN7                     (0x1   << 7  )
#define GPOEN_OEN7_IN                  (0x0   << 7  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN7_OUT                 (0x1   << 7  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN6] - Port pin direction. */
#define GPOEN_OEN6_MSK                 (0x1   << 6  )
#define GPOEN_OEN6                     (0x1   << 6  )
#define GPOEN_OEN6_IN                  (0x0   << 6  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN6_OUT                 (0x1   << 6  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN5] - Port pin direction. */
#define GPOEN_OEN5_MSK                 (0x1   << 5  )
#define GPOEN_OEN5                     (0x1   << 5  )
#define GPOEN_OEN5_IN                  (0x0   << 5  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN5_OUT                 (0x1   << 5  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN4] - Port pin direction. */
#define GPOEN_OEN4_MSK                 (0x1   << 4  )
#define GPOEN_OEN4                     (0x1   << 4  )
#define GPOEN_OEN4_IN                  (0x0   << 4  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN4_OUT                 (0x1   << 4  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN3] - Port pin direction. */
#define GPOEN_OEN3_MSK                 (0x1   << 3  )
#define GPOEN_OEN3                     (0x1   << 3  )
#define GPOEN_OEN3_IN                  (0x0   << 3  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN3_OUT                 (0x1   << 3  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN2] - Port pin direction. */
#define GPOEN_OEN2_MSK                 (0x1   << 2  )
#define GPOEN_OEN2                     (0x1   << 2  )
#define GPOEN_OEN2_IN                  (0x0   << 2  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN2_OUT                 (0x1   << 2  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN1] - Port pin direction. */
#define GPOEN_OEN1_MSK                 (0x1   << 1  )
#define GPOEN_OEN1                     (0x1   << 1  )
#define GPOEN_OEN1_IN                  (0x0   << 1  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN1_OUT                 (0x1   << 1  ) /* OUT. Enables the output on corresponding port pin. */

/* GPOEN[OEN0] - Port pin direction. */
#define GPOEN_OEN0_MSK                 (0x1   << 0  )
#define GPOEN_OEN0                     (0x1   << 0  )
#define GPOEN_OEN0_IN                  (0x0   << 0  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GPOEN_OEN0_OUT                 (0x1   << 0  ) /* OUT. Enables the output on corresponding port pin.. */

/* GPIN[IN7] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN7_MSK                   (0x1   << 7  )
#define GPIN_IN7                       (0x1   << 7  )
#define GPIN_IN7_LOW                   (0x0   << 7  ) /* LOW                      */
#define GPIN_IN7_HIGH                  (0x1   << 7  ) /* HIGH                     */

/* GPIN[IN6] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN6_MSK                   (0x1   << 6  )
#define GPIN_IN6                       (0x1   << 6  )
#define GPIN_IN6_LOW                   (0x0   << 6  ) /* LOW                      */
#define GPIN_IN6_HIGH                  (0x1   << 6  ) /* HIGH                     */

/* GPIN[IN5] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN5_MSK                   (0x1   << 5  )
#define GPIN_IN5                       (0x1   << 5  )
#define GPIN_IN5_LOW                   (0x0   << 5  ) /* LOW                      */
#define GPIN_IN5_HIGH                  (0x1   << 5  ) /* HIGH                     */

/* GPIN[IN4] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN4_MSK                   (0x1   << 4  )
#define GPIN_IN4                       (0x1   << 4  )
#define GPIN_IN4_LOW                   (0x0   << 4  ) /* LOW                      */
#define GPIN_IN4_HIGH                  (0x1   << 4  ) /* HIGH                     */

/* GPIN[IN3] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN3_MSK                   (0x1   << 3  )
#define GPIN_IN3                       (0x1   << 3  )
#define GPIN_IN3_LOW                   (0x0   << 3  ) /* LOW                      */
#define GPIN_IN3_HIGH                  (0x1   << 3  ) /* HIGH                     */

/* GPIN[IN2] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN2_MSK                   (0x1   << 2  )
#define GPIN_IN2                       (0x1   << 2  )
#define GPIN_IN2_LOW                   (0x0   << 2  ) /* LOW                      */
#define GPIN_IN2_HIGH                  (0x1   << 2  ) /* HIGH                     */

/* GPIN[IN1] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN1_MSK                   (0x1   << 1  )
#define GPIN_IN1                       (0x1   << 1  )
#define GPIN_IN1_LOW                   (0x0   << 1  ) /* LOW                      */
#define GPIN_IN1_HIGH                  (0x1   << 1  ) /* HIGH                     */

/* GPIN[IN0] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GPIN_IN0_MSK                   (0x1   << 0  )
#define GPIN_IN0                       (0x1   << 0  )
#define GPIN_IN0_LOW                   (0x0   << 0  ) /* LOW                      */
#define GPIN_IN0_HIGH                  (0x1   << 0  ) /* HIGH                     */

/* GPOUT[OUT7] - Data out register. */
#define GPOUT_OUT7_MSK                 (0x1   << 7  )
#define GPOUT_OUT7                     (0x1   << 7  )
#define GPOUT_OUT7_LOW                 (0x0   << 7  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT7_HIGH                (0x1   << 7  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT6] - Data out register. */
#define GPOUT_OUT6_MSK                 (0x1   << 6  )
#define GPOUT_OUT6                     (0x1   << 6  )
#define GPOUT_OUT6_LOW                 (0x0   << 6  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT6_HIGH                (0x1   << 6  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT5] - Data out register. */
#define GPOUT_OUT5_MSK                 (0x1   << 5  )
#define GPOUT_OUT5                     (0x1   << 5  )
#define GPOUT_OUT5_LOW                 (0x0   << 5  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT5_HIGH                (0x1   << 5  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT4] - Data out register. */
#define GPOUT_OUT4_MSK                 (0x1   << 4  )
#define GPOUT_OUT4                     (0x1   << 4  )
#define GPOUT_OUT4_LOW                 (0x0   << 4  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT4_HIGH                (0x1   << 4  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT3] - Data out register. */
#define GPOUT_OUT3_MSK                 (0x1   << 3  )
#define GPOUT_OUT3                     (0x1   << 3  )
#define GPOUT_OUT3_LOW                 (0x0   << 3  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT3_HIGH                (0x1   << 3  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT2] - Data out register. */
#define GPOUT_OUT2_MSK                 (0x1   << 2  )
#define GPOUT_OUT2                     (0x1   << 2  )
#define GPOUT_OUT2_LOW                 (0x0   << 2  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT2_HIGH                (0x1   << 2  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT1] - Data out register. */
#define GPOUT_OUT1_MSK                 (0x1   << 1  )
#define GPOUT_OUT1                     (0x1   << 1  )
#define GPOUT_OUT1_LOW                 (0x0   << 1  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT1_HIGH                (0x1   << 1  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPOUT[OUT0] - Data out register. */
#define GPOUT_OUT0_MSK                 (0x1   << 0  )
#define GPOUT_OUT0                     (0x1   << 0  )
#define GPOUT_OUT0_LOW                 (0x0   << 0  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GPOUT_OUT0_HIGH                (0x1   << 0  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET7] - Set output high for corresponding port pin. */
#define GPSET_SET7_MSK                 (0x1   << 7  )
#define GPSET_SET7                     (0x1   << 7  )
#define GPSET_SET7_SET                 (0x1   << 7  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET6] - Set output high for corresponding port pin. */
#define GPSET_SET6_MSK                 (0x1   << 6  )
#define GPSET_SET6                     (0x1   << 6  )
#define GPSET_SET6_SET                 (0x1   << 6  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET5] - Set output high for corresponding port pin. */
#define GPSET_SET5_MSK                 (0x1   << 5  )
#define GPSET_SET5                     (0x1   << 5  )
#define GPSET_SET5_SET                 (0x1   << 5  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET4] - Set output high for corresponding port pin. */
#define GPSET_SET4_MSK                 (0x1   << 4  )
#define GPSET_SET4                     (0x1   << 4  )
#define GPSET_SET4_SET                 (0x1   << 4  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET3] - Set output high for corresponding port pin. */
#define GPSET_SET3_MSK                 (0x1   << 3  )
#define GPSET_SET3                     (0x1   << 3  )
#define GPSET_SET3_SET                 (0x1   << 3  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET2] - Set output high for corresponding port pin. */
#define GPSET_SET2_MSK                 (0x1   << 2  )
#define GPSET_SET2                     (0x1   << 2  )
#define GPSET_SET2_SET                 (0x1   << 2  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET1] - Set output high for corresponding port pin. */
#define GPSET_SET1_MSK                 (0x1   << 1  )
#define GPSET_SET1                     (0x1   << 1  )
#define GPSET_SET1_SET                 (0x1   << 1  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPSET[SET0] - Set output high for corresponding port pin. */
#define GPSET_SET0_MSK                 (0x1   << 0  )
#define GPSET_SET0                     (0x1   << 0  )
#define GPSET_SET0_SET                 (0x1   << 0  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GPCLR[CLR7] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR7_MSK                 (0x1   << 7  )
#define GPCLR_CLR7                     (0x1   << 7  )
#define GPCLR_CLR7_CLR                 (0x1   << 7  ) /* CLR                      */

/* GPCLR[CLR6] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR6_MSK                 (0x1   << 6  )
#define GPCLR_CLR6                     (0x1   << 6  )
#define GPCLR_CLR6_CLR                 (0x1   << 6  ) /* CLR                      */

/* GPCLR[CLR5] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR5_MSK                 (0x1   << 5  )
#define GPCLR_CLR5                     (0x1   << 5  )
#define GPCLR_CLR5_CLR                 (0x1   << 5  ) /* CLR                      */

/* GPCLR[CLR4] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR4_MSK                 (0x1   << 4  )
#define GPCLR_CLR4                     (0x1   << 4  )
#define GPCLR_CLR4_CLR                 (0x1   << 4  ) /* CLR                      */

/* GPCLR[CLR3] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR3_MSK                 (0x1   << 3  )
#define GPCLR_CLR3                     (0x1   << 3  )
#define GPCLR_CLR3_CLR                 (0x1   << 3  ) /* CLR                      */

/* GPCLR[CLR2] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR2_MSK                 (0x1   << 2  )
#define GPCLR_CLR2                     (0x1   << 2  )
#define GPCLR_CLR2_CLR                 (0x1   << 2  ) /* CLR                      */

/* GPCLR[CLR1] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR1_MSK                 (0x1   << 1  )
#define GPCLR_CLR1                     (0x1   << 1  )
#define GPCLR_CLR1_CLR                 (0x1   << 1  ) /* CLR                      */

/* GPCLR[CLR0] - Set by user code to drive the corresponding GPIO low. */
#define GPCLR_CLR0_MSK                 (0x1   << 0  )
#define GPCLR_CLR0                     (0x1   << 0  )
#define GPCLR_CLR0_CLR                 (0x1   << 0  ) /* CLR                      */

/* GPTGL[TGL7] - Toggle for corresponding port pin. */
#define GPTGL_TGL7_MSK                 (0x1   << 7  )
#define GPTGL_TGL7                     (0x1   << 7  )
#define GPTGL_TGL7_TGL                 (0x1   << 7  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL6] - Toggle for corresponding port pin. */
#define GPTGL_TGL6_MSK                 (0x1   << 6  )
#define GPTGL_TGL6                     (0x1   << 6  )
#define GPTGL_TGL6_TGL                 (0x1   << 6  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL5] - Toggle for corresponding port pin. */
#define GPTGL_TGL5_MSK                 (0x1   << 5  )
#define GPTGL_TGL5                     (0x1   << 5  )
#define GPTGL_TGL5_TGL                 (0x1   << 5  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL4] - Toggle for corresponding port pin. */
#define GPTGL_TGL4_MSK                 (0x1   << 4  )
#define GPTGL_TGL4                     (0x1   << 4  )
#define GPTGL_TGL4_TGL                 (0x1   << 4  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL3] - Toggle for corresponding port pin. */
#define GPTGL_TGL3_MSK                 (0x1   << 3  )
#define GPTGL_TGL3                     (0x1   << 3  )
#define GPTGL_TGL3_TGL                 (0x1   << 3  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL2] - Toggle for corresponding port pin. */
#define GPTGL_TGL2_MSK                 (0x1   << 2  )
#define GPTGL_TGL2                     (0x1   << 2  )
#define GPTGL_TGL2_TGL                 (0x1   << 2  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL1] - Toggle for corresponding port pin. */
#define GPTGL_TGL1_MSK                 (0x1   << 1  )
#define GPTGL_TGL1                     (0x1   << 1  )
#define GPTGL_TGL1_TGL                 (0x1   << 1  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GPTGL[TGL0] - Toggle for corresponding port pin. */
#define GPTGL_TGL0_MSK                 (0x1   << 0  )
#define GPTGL_TGL0                     (0x1   << 0  )
#define GPTGL_TGL0_TGL                 (0x1   << 0  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* CLK[T1] - T1 clocks enable bit. */
#define CLK_T1_MSK                     (0x1   << 11 )
#define CLK_T1                         (0x1   << 11 )
#define CLK_T1_DIS                     (0x0   << 11 ) /* DIS. Disable T1 clocks.  */
#define CLK_T1_EN                      (0x1   << 11 ) /* EN. Enable T1 clocks.    */

/* CLK[T0] - T0 clocks enable bit. */
#define CLK_T0_MSK                     (0x1   << 10 )
#define CLK_T0                         (0x1   << 10 )
#define CLK_T0_DIS                     (0x0   << 10 ) /* DIS. Disable T0 clocks.  */
#define CLK_T0_EN                      (0x1   << 10 ) /* EN. Enable T0 clocks.    */

/* CLK[PWM] - PWM clocks enable bit. */
#define CLK_PWM_MSK                    (0x1   << 9  )
#define CLK_PWM                        (0x1   << 9  )
#define CLK_PWM_DIS                    (0x0   << 9  ) /* DIS. Disable PWM clocks. */
#define CLK_PWM_EN                     (0x1   << 9  ) /* EN. Enable PWM clocks.   */

/* CLK[I2C] - I2C clocks enable bit. */
#define CLK_I2C_MSK                    (0x1   << 8  )
#define CLK_I2C                        (0x1   << 8  )
#define CLK_I2C_DIS                    (0x0   << 8  ) /* DIS. Disable I2C clocks. */
#define CLK_I2C_EN                     (0x1   << 8  ) /* EN. Enable I2C clocks.   */

/* CLK[COM] - UART clocks enable bit. */
#define CLK_COM_MSK                    (0x1   << 7  )
#define CLK_COM                        (0x1   << 7  )
#define CLK_COM_DIS                    (0x0   << 7  ) /* DIS. Disable UART clocks. */
#define CLK_COM_EN                     (0x1   << 7  ) /* EN. Enable UART clocks.  */

/* CLK[SPI1] - SPI1 clocks enable bit. */
#define CLK_SPI1_MSK                   (0x1   << 6  )
#define CLK_SPI1                       (0x1   << 6  )
#define CLK_SPI1_DIS                   (0x0   << 6  ) /* DIS. Disable SPI1 clocks. */
#define CLK_SPI1_EN                    (0x1   << 6  ) /* EN. Enable SPI1 clocks.  */

/* CLK[SPI0] - SPI0 clocks enable bit. */
#define CLK_SPI0_MSK                   (0x1   << 5  )
#define CLK_SPI0                       (0x1   << 5  )
#define CLK_SPI0_DIS                   (0x0   << 5  ) /* DIS. Disable SPI0 clocks. */
#define CLK_SPI0_EN                    (0x1   << 5  ) /* EN. Enable SPI0 clocks.  */

/* CLK[T2] - T2 clocks enable bit. */
#define CLK_T2_MSK                     (0x1   << 4  )
#define CLK_T2                         (0x1   << 4  )
#define CLK_T2_DIS                     (0x0   << 4  ) /* DIS. Disable T2 clocks.  */
#define CLK_T2_EN                      (0x1   << 4  ) /* EN. Enable T2 clocks.    */

/* CLK[ADC] - ADC clocks enable bit. */
#define CLK_ADC_MSK                    (0x1   << 3  )
#define CLK_ADC                        (0x1   << 3  )
#define CLK_ADC_DIS                    (0x0   << 3  ) /* DIS. Disable ADC clocks. */
#define CLK_ADC_EN                     (0x1   << 3  ) /* EN. Enable ADC clocks.   */

/* CLK[SRAM] - SRAM clocks enable bit. */
#define CLK_SRAM_MSK                   (0x1   << 2  )
#define CLK_SRAM                       (0x1   << 2  )
#define CLK_SRAM_DIS                   (0x0   << 2  ) /* DIS. Disable SRAM memory clocks. */
#define CLK_SRAM_EN                    (0x1   << 2  ) /* EN. Enable SRAM memory clocks. */

/* CLK[FEE] - Flash clocks enable bit. */
#define CLK_FEE_MSK                    (0x1   << 1  )
#define CLK_FEE                        (0x1   << 1  )
#define CLK_FEE_DIS                    (0x0   << 1  ) /* DIS. Disable Flash memory clocks. */
#define CLK_FEE_EN                     (0x1   << 1  ) /* EN. Enable Flash memory clocks. */

/* CLK[DMA] - DMA clock enable bit. */
#define CLK_DMA_MSK                    (0x1   << 0  )
#define CLK_DMA                        (0x1   << 0  )
#define CLK_DMA_DIS                    (0x0   << 0  ) /* DIS. Disable DMA clock.  */
#define CLK_DMA_EN                     (0x1   << 0  ) /* EN. Enable DMA clock.    */

/* SPIDIV[BCRST] - Configures the behavior of SPI communication after an abrupt deassertion of CS.  This bit should be set in slave and master mode. */
#define SPIDIV_BCRST_MSK               (0x1   << 7  )
#define SPIDIV_BCRST                   (0x1   << 7  )
#define SPIDIV_BCRST_DIS               (0x0   << 7  ) /* DIS. Resumes communication from where it stopped when the CS is deasserted. The rest of the bits are then received/ transmitted when CS returns low. User code should ignore the CSERR interrupt. */
#define SPIDIV_BCRST_EN                (0x1   << 7  ) /* EN. Enabled for a clean restart of SPI transfer after a CSERR condition. User code must also clear the SPI enable bit in SPI0CON during the CSERR interrupt. */

/* SPIDIV[DIV] - Factor used to divide UCLK in the generation of the master mode serial clock. */
#define SPIDIV_DIV_MSK                 (0x3F  << 0  )

/* SPICON[MOD] - IRQ mode bits. When TIM is set these bits configure when the Tx/Rx interrupts occur in a transfer. For a DMA Rx transfer, these bits should be 00. */
#define SPICON_MOD_MSK                 (0x3   << 14 )
#define SPICON_MOD_TX1RX1              (0x0   << 14 ) /* TX1RX1. Tx/Rx interrupt occurs when 1 byte has been transmitted/received from/into the FIFO. */
#define SPICON_MOD_TX2RX2              (0x1   << 14 ) /* TX2RX2. Tx/Rx interrupt occurs when 2 bytes have been transmitted/received from/into the FIFO. */
#define SPICON_MOD_TX3RX3              (0x2   << 14 ) /* TX3RX3. Tx/Rx interrupt occurs when 3 bytes have been transmitted/received from/into the FIFO. */
#define SPICON_MOD_TX4RX4              (0x3   << 14 ) /* TX4RX4. Tx/Rx interrupt occurs when 4 bytes have been transmitted/received from/into the FIFO. */

/* SPICON[TFLUSH] - Tx FIFO flush enable bit. */
#define SPICON_TFLUSH_MSK              (0x1   << 13 )
#define SPICON_TFLUSH                  (0x1   << 13 )
#define SPICON_TFLUSH_DIS              (0x0   << 13 ) /* DIS. Disable Tx FIFO flushing. */
#define SPICON_TFLUSH_EN               (0x1   << 13 ) /* EN. Flush the Tx FIFO. This bit does not clear itself and should be toggled if a single flush is required. If this bit is left high, then either the last transmitted value or 0x00 is transmitted depending on the ZEN bit (SPI0CON[7]). Any writes to the Tx FIFO are ignored while this bit is set. */

/* SPICON[RFLUSH] - Rx FIFO flush enable bit. */
#define SPICON_RFLUSH_MSK              (0x1   << 12 )
#define SPICON_RFLUSH                  (0x1   << 12 )
#define SPICON_RFLUSH_DIS              (0x0   << 12 ) /* DIS. Disable Rx FIFO flushing. */
#define SPICON_RFLUSH_EN               (0x1   << 12 ) /* EN. If this bit is set, all incoming data is ignored and no interrupts are generated. If set and TIM = 0  (SPI0CON[6]), a read of the Rx FIFO initiates a transfer. */

/* SPICON[CON] - Continuous transfer enable bit. */
#define SPICON_CON_MSK                 (0x1   << 11 )
#define SPICON_CON                     (0x1   << 11 )
#define SPICON_CON_DIS                 (0x0   << 11 ) /* DIS. Disable continuous transfer. Each transfer consists of a single 8-bit serial transfer. If valid data exists in the SPIxTX register, then a new transfer is initiated after a stall period of one serial clock cycle. The CS line is deactivated for this one serial clock cycle. */
#define SPICON_CON_EN                  (0x1   << 11 ) /* EN. Enable continuous transfer. In master mode, the transfer continues until no valid data is available in the Tx register. CS is asserted and remains asserted for the duration of each 8-bit serial transfer until Tx is empty. */

/* SPICON[LOOPBACK] - Loopback enable bit. */
#define SPICON_LOOPBACK_MSK            (0x1   << 10 )
#define SPICON_LOOPBACK                (0x1   << 10 )
#define SPICON_LOOPBACK_DIS            (0x0   << 10 ) /* DIS. Normal mode.        */
#define SPICON_LOOPBACK_EN             (0x1   << 10 ) /* EN. Connect MISO to MOSI, thus, data transmitted from Tx register is looped back to the Rx register. SPI must be configured in master mode. */

/* SPICON[SOEN] - Slave output enable bit. */
#define SPICON_SOEN_MSK                (0x1   << 9  )
#define SPICON_SOEN                    (0x1   << 9  )
#define SPICON_SOEN_DIS                (0x0   << 9  ) /* DIS. Disable the output driver on the MISO pin. The MISO pin is open-circuit when this bit is clear. */
#define SPICON_SOEN_EN                 (0x1   << 9  ) /* EN. MISO operates as normal. */

/* SPICON[RXOF] - RX overflow overwrite enable bit. */
#define SPICON_RXOF_MSK                (0x1   << 8  )
#define SPICON_RXOF                    (0x1   << 8  )
#define SPICON_RXOF_DIS                (0x0   << 8  ) /* DIS. The new serial byte received is discarded when there is no space left in the FIFO */
#define SPICON_RXOF_EN                 (0x1   << 8  ) /* EN. The valid data in the Rx register is overwritten by the new serial byte received when there is no space left in the FIFO. */

/* SPICON[ZEN] - Transmit underrun: Transmit 0s when the Tx FIFO is empty */
#define SPICON_ZEN_MSK                 (0x1   << 7  )
#define SPICON_ZEN                     (0x1   << 7  )
#define SPICON_ZEN_DIS                 (0x0   << 7  ) /* DIS. The last byte from the previous transmission is shifted out when a transfer is initiated with no valid data in the FIFO. */
#define SPICON_ZEN_EN                  (0x1   << 7  ) /* EN. Transmit 0x00 when a transfer is initiated with no valid data in the FIFO. */

/* SPICON[TIM] - Transfer and interrupt mode bit. */
#define SPICON_TIM_MSK                 (0x1   << 6  )
#define SPICON_TIM                     (0x1   << 6  )
#define SPICON_TIM_TXWR                (0x1   << 6  ) /* TXWR. Initiate transfer with a write to the SPIxTX register.  Interrupt only occurs when Tx is empty. */
#define SPICON_TIM_RXRD                (0x0   << 6  ) /* RXRD. Initiate transfer with a read of the SPIxRX register.  The read must be done while the SPI interface is idle. Interrupt only occurs when Rx is full. */

/* SPICON[LSB] - LSB first transfer enable bit. */
#define SPICON_LSB_MSK                 (0x1   << 5  )
#define SPICON_LSB                     (0x1   << 5  )
#define SPICON_LSB_DIS                 (0x0   << 5  ) /* DIS. MSB is transmitted first. */
#define SPICON_LSB_EN                  (0x1   << 5  ) /* EN. LSB is transmitted first. */

/* SPICON[WOM] - Wired OR enable bit. */
#define SPICON_WOM_MSK                 (0x1   << 4  )
#define SPICON_WOM                     (0x1   << 4  )
#define SPICON_WOM_DIS                 (0x0   << 4  ) /* DIS. Normal driver output operation. */
#define SPICON_WOM_EN                  (0x1   << 4  ) /* EN. Enable open circuit data output for multimaster/multislave configuration. */

/* SPICON[CPOL] - Serial clock polarity mode bit. */
#define SPICON_CPOL_MSK                (0x1   << 3  )
#define SPICON_CPOL                    (0x1   << 3  )
#define SPICON_CPOL_LOW                (0x0   << 3  ) /* LOW. Serial clock idles low. */
#define SPICON_CPOL_HIGH               (0x1   << 3  ) /* HIGH. Serial clock idles high. */

/* SPICON[CPHA] - Serial clock phase mode bit. */
#define SPICON_CPHA_MSK                (0x1   << 2  )
#define SPICON_CPHA                    (0x1   << 2  )
#define SPICON_CPHA_SAMPLELEADING      (0x0   << 2  ) /* SAMPLELEADING. Serial clock pulses at the middle of the first data bit transfer. */
#define SPICON_CPHA_SAMPLETRAILING     (0x1   << 2  ) /* SAMPLETRAILING. Serial clock pulses at the start of the first data bit. */

/* SPICON[MASEN] - Master mode enable bit. */
#define SPICON_MASEN_MSK               (0x1   << 1  )
#define SPICON_MASEN                   (0x1   << 1  )
#define SPICON_MASEN_DIS               (0x0   << 1  ) /* DIS. Configure in slave mode. */
#define SPICON_MASEN_EN                (0x1   << 1  ) /* EN. Configure in master mode. */

/* SPICON[ENABLE] - SPI enable bit. */
#define SPICON_ENABLE_MSK              (0x1   << 0  )
#define SPICON_ENABLE                  (0x1   << 0  )
#define SPICON_ENABLE_DIS              (0x0   << 0  ) /* DIS. Disable the SPI. Clearing this bit will also reset all the FIFO related logic to enable a clean start. */
#define SPICON_ENABLE_EN               (0x1   << 0  ) /* EN. Enable the SPI.      */

/* SPIDMA[IENRXDMA] - Receive DMA request enable bit. */
#define SPIDMA_IENRXDMA_MSK            (0x1   << 2  )
#define SPIDMA_IENRXDMA                (0x1   << 2  )
#define SPIDMA_IENRXDMA_DIS            (0x0   << 2  ) /* DIS. Disable Rx DMA requests. */
#define SPIDMA_IENRXDMA_EN             (0x1   << 2  ) /* EN. Enable Rx DMA requests. */

/* SPIDMA[IENTXDMA] - Transmit DMA request enable bit. */
#define SPIDMA_IENTXDMA_MSK            (0x1   << 1  )
#define SPIDMA_IENTXDMA                (0x1   << 1  )
#define SPIDMA_IENTXDMA_DIS            (0x0   << 1  ) /* DIS. Disable Tx DMA requests. */
#define SPIDMA_IENTXDMA_EN             (0x1   << 1  ) /* EN. Enable Tx DMA requests. */

/* SPIDMA[ENABLE] - DMA data transfer enable bit. */
#define SPIDMA_ENABLE_MSK              (0x1   << 0  )
#define SPIDMA_ENABLE                  (0x1   << 0  )
#define SPIDMA_ENABLE_DIS              (0x0   << 0  ) /* DIS. Disable DMA transfer. This bit needs to be cleared to prevent extra DMA request to the µDMA controller. */
#define SPIDMA_ENABLE_EN               (0x1   << 0  ) /* EN. Enable a DMA transfer. Starts the transfer of a master configured to initiate transfer on transmit. */

/* SPISTA[CSERR] - CS error status bit. This bit generates an interrupt when detecting an abrupt CS desassertion before the full byte of data is transmitted completely. */
#define SPISTA_CSERR_MSK               (0x1   << 12 )
#define SPISTA_CSERR                   (0x1   << 12 )
#define SPISTA_CSERR_CLR               (0x0   << 12 ) /* CLR: Cleared when no CS error is detected. Cleared to 0 on a read of SPI0STA register. */
#define SPISTA_CSERR_SET               (0x1   << 12 ) /* SET: Set when the CS line is deasserted abruptly. */

/* SPISTA[RXS] - Rx FIFO excess bytes present. */
#define SPISTA_RXS_MSK                 (0x1   << 11 )
#define SPISTA_RXS                     (0x1   << 11 )
#define SPISTA_RXS_CLR                 (0x0   << 11 ) /* CLR. When the number of bytes in the FIFO is equal or less than the number in SPI0CON[15:14]. This bit is not cleared on a read of SPI0STA register. */
#define SPISTA_RXS_SET                 (0x1   << 11 ) /* SET. When there are more bytes in the Rx FIFO than configured in MOD (SPI0CON[15:14]). For example if MOD = TX1RX1, RXS is set when there are 2 or more bytes in the Rx FIFO. This bit does not dependent on SPI0CON[6] and does not cause an interrupt. */

/* SPISTA[RXFSTA] - Rx FIFO status bits, indicates how many valid bytes are in the Rx FIFO. */
#define SPISTA_RXFSTA_MSK              (0x7   << 8  )
#define SPISTA_RXFSTA_EMPTY            (0x0   << 8  ) /* EMPTY. When Rx FIFO is empty. */
#define SPISTA_RXFSTA_ONEBYTE          (0x1   << 8  ) /* ONEBYTE. When 1 valid byte is in the FIFO. */
#define SPISTA_RXFSTA_TWOBYTES         (0x2   << 8  ) /* TWOBYTES. When 2 valid bytes are in the FIFO. */
#define SPISTA_RXFSTA_THREEBYTES       (0x3   << 8  ) /* THREEBYTES. When 3 valid bytes are in the FIFO. */
#define SPISTA_RXFSTA_FOURBYTES        (0x4   << 8  ) /* FOURBYTES. When 4 valid bytes are in the FIFO. */

/* SPISTA[RXOF] - Rx FIFO overflow status bit. This bit generates an interrupt. */
#define SPISTA_RXOF_MSK                (0x1   << 7  )
#define SPISTA_RXOF                    (0x1   << 7  )
#define SPISTA_RXOF_CLR                (0x0   << 7  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPISTA_RXOF_SET                (0x1   << 7  ) /* SET. Set when the Rx FIFO is already full when new data is loaded to the FIFO. This bit generates an interrupt except when RFLUSH is set. (SPI0CON[12]). */

/* SPISTA[RX] - Rx interrupt status bit. This bit generates an interrupt, except when DMA transfer is enabled. */
#define SPISTA_RX_MSK                  (0x1   << 6  )
#define SPISTA_RX                      (0x1   << 6  )
#define SPISTA_RX_CLR                  (0x0   << 6  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPISTA_RX_SET                  (0x1   << 6  ) /* SET. Set when a receive interrupt occurs. This bit is set when TIM (SPI0CON[6]) is cleared and the required number of bytes have been received. */

/* SPISTA[TX] - Tx interrupt status bit. This bit generates an interrupt, except when DMA transfer is enabled. */
#define SPISTA_TX_MSK                  (0x1   << 5  )
#define SPISTA_TX                      (0x1   << 5  )
#define SPISTA_TX_CLR                  (0x0   << 5  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPISTA_TX_SET                  (0x1   << 5  ) /* SET. Set when a transmit interrupt occurs. This bit is set when TIM (SPI0CON[6]) set and the required number of bytes have been transmitted. */

/* SPISTA[TXUR] - Tx FIFO Underrun. This bit generates an interrupt. */
#define SPISTA_TXUR_MSK                (0x1   << 4  )
#define SPISTA_TXUR                    (0x1   << 4  )
#define SPISTA_TXUR_CLR                (0x0   << 4  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPISTA_TXUR_SET                (0x1   << 4  ) /* SET. Set when a transmit is initiated without any valid data in the Tx FIFO. This bit generates an interrupt except when TFLUSH is set in SPI0CON. */

/* SPISTA[TXFSTA] - Tx FIFO status bits, indicates how many valid bytes are in the Tx FIFO. */
#define SPISTA_TXFSTA_MSK              (0x7   << 1  )
#define SPISTA_TXFSTA_EMPTY            (0x0   << 1  ) /* EMPTY. Tx FIFO is empty. */
#define SPISTA_TXFSTA_ONEBYTE          (0x1   << 1  ) /* ONEBYTE. 1 valid byte is in the FIFO. */
#define SPISTA_TXFSTA_TWOBYTES         (0x2   << 1  ) /* TWOBYTES. 2 valid bytes are in the FIFO. */
#define SPISTA_TXFSTA_THREEBYTES       (0x3   << 1  ) /* THREEBYTES. 3 valid bytes are in the FIFO. */
#define SPISTA_TXFSTA_FOURBYTES        (0x4   << 1  ) /* FOURBYTES . 4 valid bytes are in the FIFO. */

/* SPISTA[IRQ] - Interrupt status bit. */
#define SPISTA_IRQ_MSK                 (0x1   << 0  )
#define SPISTA_IRQ                     (0x1   << 0  )
#define SPISTA_IRQ_CLR                 (0x0   << 0  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPISTA_IRQ_SET                 (0x1   << 0  ) /* SET. Set to 1 when an SPI0 based interrupt occurs. */

/* SPIDIV[BCRST] - Configures the behavior of SPI communication after an abrupt deassertion of CS.  This bit should be set in slave and master mode. */
#define SPIDIV_BCRST_MSK               (0x1   << 7  )
#define SPIDIV_BCRST                   (0x1   << 7  )
#define SPIDIV_BCRST_DIS               (0x0   << 7  ) /* DIS. Resumes communication from where it stopped when the CS is deasserted. The rest of the bits are then received/ transmitted when CS returns low. User code should ignore the CSERR interrupt. */
#define SPIDIV_BCRST_EN                (0x1   << 7  ) /* EN. Enabled for a clean restart of SPI transfer after a CSERR condition. User code must also clear the SPI enable bit in SPI0CON during the CSERR interrupt. */

/* SPIDIV[DIV] - Factor used to divide UCLK in the generation of the master mode serial clock. */
#define SPIDIV_DIV_MSK                 (0x3F  << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        ADC0                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Analog to Digital Converter (pADI_ADC0)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_ADC0 Structure                    */
  __IO uint16_t  ADCCFG;                    /*!< ADC Configuration Register            */
  __I  uint16_t  RESERVED0;
  __IO uint8_t   ADCCON;                    /*!< ADC Control Register                  */
  __I  uint8_t   RESERVED1[3];
  __IO uint8_t   ADCSTA;                    /*!< ADC Status Register                   */
  __I  uint8_t   RESERVED2[3];
  __IO uint16_t  ADCDAT;                    /*!< ADC Data Register                     */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  ADCGN;                     /*!< ADC Gain Register                     */
  __I  uint16_t  RESERVED4;
  __IO uint16_t  ADCOF;                     /*!< ADC Offset Register                   */
} ADI_ADC_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          ADCCFG                                     (*(volatile unsigned short int *) 0x40050000)
#define          ADCCON                                     (*(volatile unsigned char      *) 0x40050004)
#define          ADCSTA                                     (*(volatile unsigned char      *) 0x40050008)
#define          ADCDAT                                     (*(volatile unsigned short int *) 0x4005000C)
#define          ADCGN                                      (*(volatile unsigned short int *) 0x40050010)
#define          ADCOF                                      (*(volatile unsigned short int *) 0x40050014)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for ADCCFG*/
#define ADCCFG_RVAL                    0xA00

/* ADCCFG[REF] - Reference select */
#define ADCCFG_REF_BBA                 (*(volatile unsigned long *) 0x42A00034)
#define ADCCFG_REF_MSK                 (0x1   << 13 )
#define ADCCFG_REF                     (0x1   << 13 )
#define ADCCFG_REF_INTERNAL125V        (0x0   << 13 ) /* INTERNAL125V. Select the internal 1.25 V reference as the ADC reference. */
#define ADCCFG_REF_LVDD                (0x1   << 13 ) /* LVDD. Select the 1.8V regulator output (LVDD1) as the ADC reference. */

/* ADCCFG[CLK] - ADC clock frequency */
#define ADCCFG_CLK_MSK                 (0x7   << 10 )
#define ADCCFG_CLK_FCORE               (0x0   << 10 ) /* FCORE.                   */
#define ADCCFG_CLK_FCOREDIV2           (0x1   << 10 ) /* FCOREDIV2.               */
#define ADCCFG_CLK_FCOREDIV4           (0x2   << 10 ) /* FCOREDIV4.               */
#define ADCCFG_CLK_FCOREDIV8           (0x3   << 10 ) /* FCOREDIV8.               */
#define ADCCFG_CLK_FCOREDIV16          (0x4   << 10 ) /* FCOREDIV16.              */
#define ADCCFG_CLK_FCOREDIV32          (0x5   << 10 ) /* FCOREDIV32.              */

/* ADCCFG[ACQ] - Acquisition clocks */
#define ADCCFG_ACQ_MSK                 (0x3   << 8  )
#define ADCCFG_ACQ_2                   (0x0   << 8  ) /* 2.                       */
#define ADCCFG_ACQ_4                   (0x1   << 8  ) /* 4.                       */
#define ADCCFG_ACQ_8                   (0x2   << 8  ) /* 8.                       */
#define ADCCFG_ACQ_16                  (0x3   << 8  ) /* 16.                      */

/* ADCCFG[CHSEL] - Channel select */
#define ADCCFG_CHSEL_MSK               (0xF   << 0  )
#define ADCCFG_CHSEL_ADC0              (0x0   << 0  ) /* ADC0. Single ended ADC0 input. */
#define ADCCFG_CHSEL_ADC1              (0x1   << 0  ) /* ADC1. Single ended ADC1 input. */
#define ADCCFG_CHSEL_ADC2              (0x2   << 0  ) /* ADC2. Single ended ADC2 input. */
#define ADCCFG_CHSEL_ADC3              (0x3   << 0  ) /* ADC3. Single ended ADC3 input. */
#define ADCCFG_CHSEL_ADC4              (0x4   << 0  ) /* ADC4. Single ended ADC4 input. */
#define ADCCFG_CHSEL_ADC5              (0x5   << 0  ) /* ADC5. Single ended ADC5 input. */
#define ADCCFG_CHSEL_DIFF0             (0x6   << 0  ) /* DIFF0. Differential ADC0 - ADC1 inputs. */
#define ADCCFG_CHSEL_DIFF1             (0x7   << 0  ) /* DIFF1. Differential ADC2 - ADC3 inputs. */
#define ADCCFG_CHSEL_DIFF2             (0x8   << 0  ) /* DIFF2. Differential ADC4 - ADC5 inputs. */
#define ADCCFG_CHSEL_TEMP              (0x9   << 0  ) /* TEMP. Internal temperature sensor. */
#define ADCCFG_CHSEL_VBATDIV4          (0xA   << 0  ) /* VBATDIV4. Internal supply divided by 4. */
#define ADCCFG_CHSEL_LVDDDIV2          (0xB   << 0  ) /* LVDDDIV2. Internal 1.8V regulator output (LVDD1) divided by 2. */
#define ADCCFG_CHSEL_VREF              (0xC   << 0  ) /* VREF. Internal ADC reference input for gain calibration. */
#define ADCCFG_CHSEL_AGND              (0xD   << 0  ) /* AGND. Internal ADC ground input for offset calibration. */

/* Reset Value for ADCCON*/
#define ADCCON_RVAL                    0x90

/* ADCCON[REFBUF] - Reference buffer enable bit. */
#define ADCCON_REFBUF_BBA              (*(volatile unsigned long *) 0x42A0009C)
#define ADCCON_REFBUF_MSK              (0x1   << 7  )
#define ADCCON_REFBUF                  (0x1   << 7  )
#define ADCCON_REFBUF_EN               (0x0   << 7  ) /* EN. Turn on the reference buffer. The reference buffer takes 5 ms to settle and consumes approximately 210 μA. */
#define ADCCON_REFBUF_DIS              (0x1   << 7  ) /* DIS. Turn off the reference buffer. The internal reference buffer must be turned off if using an external reference. */

/* ADCCON[DMA] - DMA transfer enable bit. */
#define ADCCON_DMA_BBA                 (*(volatile unsigned long *) 0x42A00098)
#define ADCCON_DMA_MSK                 (0x1   << 6  )
#define ADCCON_DMA                     (0x1   << 6  )
#define ADCCON_DMA_DIS                 (0x0   << 6  ) /* DIS. Disable DMA transfer. */
#define ADCCON_DMA_EN                  (0x1   << 6  ) /* EN. Enable DMA transfer. */

/* ADCCON[IEN] - Interrupt enable. */
#define ADCCON_IEN_BBA                 (*(volatile unsigned long *) 0x42A00094)
#define ADCCON_IEN_MSK                 (0x1   << 5  )
#define ADCCON_IEN                     (0x1   << 5  )
#define ADCCON_IEN_DIS                 (0x0   << 5  ) /* DIS. Disable the ADC interrupt. */
#define ADCCON_IEN_EN                  (0x1   << 5  ) /* EN. Enable the ADC interrupt. An interrupt is generated when new data is available. */

/* ADCCON[ENABLE] - ADC enable. */
#define ADCCON_ENABLE_BBA              (*(volatile unsigned long *) 0x42A00090)
#define ADCCON_ENABLE_MSK              (0x1   << 4  )
#define ADCCON_ENABLE                  (0x1   << 4  )
#define ADCCON_ENABLE_EN               (0x0   << 4  ) /* EN. Enable the ADC.      */
#define ADCCON_ENABLE_DIS              (0x1   << 4  ) /* DIS. Disable the ADC.    */

/* ADCCON[MOD] - Conversion mode. */
#define ADCCON_MOD_MSK                 (0x7   << 1  )
#define ADCCON_MOD_SOFT                (0x0   << 1  ) /* SOFT. Software trigger, used in conjunction with the START bit. */
#define ADCCON_MOD_CONT                (0x1   << 1  ) /* CONT. Continuous convert mode. */
#define ADCCON_MOD_T0OVF               (0x3   << 1  ) /* T0OVF. Timer0 overflow.  */
#define ADCCON_MOD_T1OVF               (0x4   << 1  ) /* T1OVF. Timer1 overflow.  */
#define ADCCON_MOD_GPIO                (0x5   << 1  ) /* GPIO. ADC conversion triggered by P0.3 input. */

/* ADCCON[START] - ADC conversion start. */
#define ADCCON_START_BBA               (*(volatile unsigned long *) 0x42A00080)
#define ADCCON_START_MSK               (0x1   << 0  )
#define ADCCON_START                   (0x1   << 0  )
#define ADCCON_START_DIS               (0x0   << 0  ) /* DIS. Has no effect.      */
#define ADCCON_START_EN                (0x1   << 0  ) /* EN. Start conversion when SOFT conversion mode is selected. This bit does not clear after a single software conversion. */

/* Reset Value for ADCSTA*/
#define ADCSTA_RVAL                    0x0

/* ADCSTA[READY] - ADC Ready bit */
#define ADCSTA_READY_BBA               (*(volatile unsigned long *) 0x42A00100)
#define ADCSTA_READY_MSK               (0x1   << 0  )
#define ADCSTA_READY                   (0x1   << 0  )
#define ADCSTA_READY_CLR               (0x0   << 0  ) /* CLR. Cleared automatically when ADCDAT is read. */
#define ADCSTA_READY_EN                (0x1   << 0  ) /* EN. Set by the ADC when a conversion is complete. This bit generates an interrupt if enabled (IEN set in ADCCON). */

/* Reset Value for ADCDAT*/
#define ADCDAT_RVAL                    0x0

/* ADCDAT[VALUE] - ADC result */
#define ADCDAT_VALUE_MSK               (0xFFF << 2  )

/* ADCDAT[Value_Reserved] - ADC result / Reserved */
#define ADCDAT_Value_Reserved_MSK      (0x3   << 0  )

/* Reset Value for ADCGN*/
#define ADCGN_RVAL                     0x0

/* ADCGN[VALUE] - Gain */
#define ADCGN_VALUE_MSK                (0xFFFF << 0  )

/* Reset Value for ADCOF*/
#define ADCOF_RVAL                     0x0

/* ADCOF[VALUE] - Offset */
#define ADCOF_VALUE_MSK                (0xFFFF << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        CLKCTL                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Clock Control (pADI_CLKCTL)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_CLKCTL Structure                  */
  __IO uint16_t  CLKCON;                    /*!< System Clocking Architecture Control Register */
  __I  uint16_t  RESERVED0[519];
  __IO uint8_t   XOSCCON;                   /*!< Crystal Oscillator Control Register   */
  __I  uint8_t   RESERVED1[111];
  __IO uint16_t  CLKACT;                    /*!< Clock in Active Mode Enable Register  */
  __I  uint16_t  RESERVED2;
  __IO uint16_t  CLKPD;                     /*!< Clock in Power-Down Mode Enable Register */
} ADI_CLKCTL_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          CLKCON                                     (*(volatile unsigned short int *) 0x40002000)
#define          XOSCCON                                    (*(volatile unsigned char      *) 0x40002410)
#define          CLKACT                                     (*(volatile unsigned short int *) 0x40002480)
#define          CLKPD                                      (*(volatile unsigned short int *) 0x40002484)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for CLKCON*/
#define CLKCON_RVAL                    0x0

/* CLKCON[CLKOUT] - GPIO output clock multiplexer select bits. */
#define CLKCON_CLKOUT_MSK              (0x7   << 5  )
#define CLKCON_CLKOUT_UCLKCG           (0x0   << 5  ) /* UCLKCG.                  */
#define CLKCON_CLKOUT_UCLK             (0x1   << 5  ) /* UCLK.                    */
#define CLKCON_CLKOUT_PCLK             (0x2   << 5  ) /* PCLK.                    */
#define CLKCON_CLKOUT_HFOSC            (0x5   << 5  ) /* HFOSC.                   */
#define CLKCON_CLKOUT_LFOSC            (0x6   << 5  ) /* LFOSC.                   */
#define CLKCON_CLKOUT_LFXTAL           (0x7   << 5  ) /* LFXTAL.                  */

/* CLKCON[CLKMUX] - Digital subsystem clock source select bits. */
#define CLKCON_CLKMUX_MSK              (0x3   << 3  )
#define CLKCON_CLKMUX_HFOSC            (0x0   << 3  ) /* HFOSC. 16MHz internal oscillator. */
#define CLKCON_CLKMUX_LFXTAL           (0x1   << 3  ) /* LFXTAL. 32.768kHz external crystal. */
#define CLKCON_CLKMUX_LFOSC            (0x2   << 3  ) /* LFOSC. 32.768kHz internal oscillator. */
#define CLKCON_CLKMUX_ECLKIN           (0x3   << 3  ) /* ECLKIN. External clock on P0.5. */

/* CLKCON[CD] - Clock divide bits. */
#define CLKCON_CD_MSK                  (0x7   << 0  )
#define CLKCON_CD_DIV1                 (0x0   << 0  ) /* DIV1.                    */
#define CLKCON_CD_DIV2                 (0x1   << 0  ) /* DIV2.                    */
#define CLKCON_CD_DIV4                 (0x2   << 0  ) /* DIV4.                    */
#define CLKCON_CD_DIV8                 (0x3   << 0  ) /* DIV8.                    */
#define CLKCON_CD_DIV16                (0x4   << 0  ) /* DIV16.                   */
#define CLKCON_CD_DIV32                (0x5   << 0  ) /* DIV32.                   */
#define CLKCON_CD_DIV64                (0x6   << 0  ) /* DIV64.                   */
#define CLKCON_CD_DIV128               (0x7   << 0  ) /* DIV128.                  */

/* Reset Value for XOSCCON*/
#define XOSCCON_RVAL                   0x0

/* XOSCCON[ENABLE] - Crystal oscillator circuit enable bit. */
#define XOSCCON_ENABLE_BBA             (*(volatile unsigned long *) 0x42048200)
#define XOSCCON_ENABLE_MSK             (0x1   << 0  )
#define XOSCCON_ENABLE                 (0x1   << 0  )
#define XOSCCON_ENABLE_DIS             (0x0   << 0  ) /* DIS. Disables the watch crystal circuitry.(LFXTAL) */
#define XOSCCON_ENABLE_EN              (0x1   << 0  ) /* EN. Enables the watch crystal circuitry.(LFXTAL) */

/* Reset Value for CLKACT*/
#define CLKACT_RVAL                    0x3FFF

/* CLKACT[T1] - T1 clocks enable bit. */
#define CLKACT_T1_BBA                  (*(volatile unsigned long *) 0x4204902C)
#define CLKACT_T1_MSK                  (0x1   << 11 )
#define CLKACT_T1                      (0x1   << 11 )
#define CLKACT_T1_DIS                  (0x0   << 11 ) /* DIS. Disable T1 clocks.  */
#define CLKACT_T1_EN                   (0x1   << 11 ) /* EN. Enable T1 clocks.    */

/* CLKACT[T0] - T0 clocks enable bit. */
#define CLKACT_T0_BBA                  (*(volatile unsigned long *) 0x42049028)
#define CLKACT_T0_MSK                  (0x1   << 10 )
#define CLKACT_T0                      (0x1   << 10 )
#define CLKACT_T0_DIS                  (0x0   << 10 ) /* DIS. Disable T0 clocks.  */
#define CLKACT_T0_EN                   (0x1   << 10 ) /* EN. Enable T0 clocks.    */

/* CLKACT[PWM] - PWM clocks enable bit. */
#define CLKACT_PWM_BBA                 (*(volatile unsigned long *) 0x42049024)
#define CLKACT_PWM_MSK                 (0x1   << 9  )
#define CLKACT_PWM                     (0x1   << 9  )
#define CLKACT_PWM_DIS                 (0x0   << 9  ) /* DIS. Disable PWM clocks. */
#define CLKACT_PWM_EN                  (0x1   << 9  ) /* EN. Enable PWM clocks.   */

/* CLKACT[I2C] - I2C clocks enable bit. */
#define CLKACT_I2C_BBA                 (*(volatile unsigned long *) 0x42049020)
#define CLKACT_I2C_MSK                 (0x1   << 8  )
#define CLKACT_I2C                     (0x1   << 8  )
#define CLKACT_I2C_DIS                 (0x0   << 8  ) /* DIS. Disable I2C clocks. */
#define CLKACT_I2C_EN                  (0x1   << 8  ) /* EN. Enable I2C clocks.   */

/* CLKACT[COM] - UART clocks enable bit. */
#define CLKACT_COM_BBA                 (*(volatile unsigned long *) 0x4204901C)
#define CLKACT_COM_MSK                 (0x1   << 7  )
#define CLKACT_COM                     (0x1   << 7  )
#define CLKACT_COM_DIS                 (0x0   << 7  ) /* DIS. Disable UART clocks. */
#define CLKACT_COM_EN                  (0x1   << 7  ) /* EN. Enable UART clocks.  */

/* CLKACT[SPI1] - SPI1 clocks enable bit. */
#define CLKACT_SPI1_BBA                (*(volatile unsigned long *) 0x42049018)
#define CLKACT_SPI1_MSK                (0x1   << 6  )
#define CLKACT_SPI1                    (0x1   << 6  )
#define CLKACT_SPI1_DIS                (0x0   << 6  ) /* DIS. Disable SPI1 clocks. */
#define CLKACT_SPI1_EN                 (0x1   << 6  ) /* EN. Enable SPI1 clocks.  */

/* CLKACT[SPI0] - SPI0 clocks enable bit. */
#define CLKACT_SPI0_BBA                (*(volatile unsigned long *) 0x42049014)
#define CLKACT_SPI0_MSK                (0x1   << 5  )
#define CLKACT_SPI0                    (0x1   << 5  )
#define CLKACT_SPI0_DIS                (0x0   << 5  ) /* DIS. Disable SPI0 clocks. */
#define CLKACT_SPI0_EN                 (0x1   << 5  ) /* EN. Enable SPI0 clocks.  */

/* CLKACT[T2] - T2 clocks enable bit. */
#define CLKACT_T2_BBA                  (*(volatile unsigned long *) 0x42049010)
#define CLKACT_T2_MSK                  (0x1   << 4  )
#define CLKACT_T2                      (0x1   << 4  )
#define CLKACT_T2_DIS                  (0x0   << 4  ) /* DIS. Disable T2 clocks.  */
#define CLKACT_T2_EN                   (0x1   << 4  ) /* EN. Enable T2 clocks.    */

/* CLKACT[ADC] - ADC clocks enable bit. */
#define CLKACT_ADC_BBA                 (*(volatile unsigned long *) 0x4204900C)
#define CLKACT_ADC_MSK                 (0x1   << 3  )
#define CLKACT_ADC                     (0x1   << 3  )
#define CLKACT_ADC_DIS                 (0x0   << 3  ) /* DIS. Disable ADC clocks. */
#define CLKACT_ADC_EN                  (0x1   << 3  ) /* EN. Enable ADC clocks.   */

/* CLKACT[SRAM] - SRAM clocks enable bit. */
#define CLKACT_SRAM_BBA                (*(volatile unsigned long *) 0x42049008)
#define CLKACT_SRAM_MSK                (0x1   << 2  )
#define CLKACT_SRAM                    (0x1   << 2  )
#define CLKACT_SRAM_DIS                (0x0   << 2  ) /* DIS. Disable SRAM memory clocks. */
#define CLKACT_SRAM_EN                 (0x1   << 2  ) /* EN. Enable SRAM memory clocks. */

/* CLKACT[FEE] - Flash clocks enable bit. */
#define CLKACT_FEE_BBA                 (*(volatile unsigned long *) 0x42049004)
#define CLKACT_FEE_MSK                 (0x1   << 1  )
#define CLKACT_FEE                     (0x1   << 1  )
#define CLKACT_FEE_DIS                 (0x0   << 1  ) /* DIS. Disable Flash memory clocks. */
#define CLKACT_FEE_EN                  (0x1   << 1  ) /* EN. Enable Flash memory clocks. */

/* CLKACT[DMA] - DMA clock enable bit. */
#define CLKACT_DMA_BBA                 (*(volatile unsigned long *) 0x42049000)
#define CLKACT_DMA_MSK                 (0x1   << 0  )
#define CLKACT_DMA                     (0x1   << 0  )
#define CLKACT_DMA_DIS                 (0x0   << 0  ) /* DIS.Disable DMA clock.   */
#define CLKACT_DMA_EN                  (0x1   << 0  ) /* EN. Enable DMA clock.    */

/* Reset Value for CLKPD*/
#define CLKPD_RVAL                     0x3FFF

/* CLKPD[T1] - T1 clocks enable bit. */
#define CLKPD_T1_BBA                   (*(volatile unsigned long *) 0x420490AC)
#define CLKPD_T1_MSK                   (0x1   << 11 )
#define CLKPD_T1                       (0x1   << 11 )
#define CLKPD_T1_DIS                   (0x0   << 11 ) /* DIS. Disable T1 clocks.  */
#define CLKPD_T1_EN                    (0x1   << 11 ) /* EN. Enable T1 clocks.    */

/* CLKPD[T0] - T0 clocks enable bit. */
#define CLKPD_T0_BBA                   (*(volatile unsigned long *) 0x420490A8)
#define CLKPD_T0_MSK                   (0x1   << 10 )
#define CLKPD_T0                       (0x1   << 10 )
#define CLKPD_T0_DIS                   (0x0   << 10 ) /* DIS. Disable T0 clocks.  */
#define CLKPD_T0_EN                    (0x1   << 10 ) /* EN. Enable T0 clocks.    */

/* CLKPD[PWM] - PWM clocks enable bit. */
#define CLKPD_PWM_BBA                  (*(volatile unsigned long *) 0x420490A4)
#define CLKPD_PWM_MSK                  (0x1   << 9  )
#define CLKPD_PWM                      (0x1   << 9  )
#define CLKPD_PWM_DIS                  (0x0   << 9  ) /* DIS. Disable PWM clocks. */
#define CLKPD_PWM_EN                   (0x1   << 9  ) /* EN. Enable PWM clocks.   */

/* CLKPD[I2C] - I2C clocks enable bit. */
#define CLKPD_I2C_BBA                  (*(volatile unsigned long *) 0x420490A0)
#define CLKPD_I2C_MSK                  (0x1   << 8  )
#define CLKPD_I2C                      (0x1   << 8  )
#define CLKPD_I2C_DIS                  (0x0   << 8  ) /* DIS. Disable I2C clocks. */
#define CLKPD_I2C_EN                   (0x1   << 8  ) /* EN. Enable I2C clocks.   */

/* CLKPD[COM] - UART clocks enable bit. */
#define CLKPD_COM_BBA                  (*(volatile unsigned long *) 0x4204909C)
#define CLKPD_COM_MSK                  (0x1   << 7  )
#define CLKPD_COM                      (0x1   << 7  )
#define CLKPD_COM_DIS                  (0x0   << 7  ) /* DIS. Disable UART clocks. */
#define CLKPD_COM_EN                   (0x1   << 7  ) /* EN. Enable UART clocks.  */

/* CLKPD[SPI1] - SPI1 clocks enable bit. */
#define CLKPD_SPI1_BBA                 (*(volatile unsigned long *) 0x42049098)
#define CLKPD_SPI1_MSK                 (0x1   << 6  )
#define CLKPD_SPI1                     (0x1   << 6  )
#define CLKPD_SPI1_DIS                 (0x0   << 6  ) /* DIS. Disable SPI1 clocks. */
#define CLKPD_SPI1_EN                  (0x1   << 6  ) /* EN. Enable SPI1 clocks.  */

/* CLKPD[SPI0] - SPI0 clocks enable bit. */
#define CLKPD_SPI0_BBA                 (*(volatile unsigned long *) 0x42049094)
#define CLKPD_SPI0_MSK                 (0x1   << 5  )
#define CLKPD_SPI0                     (0x1   << 5  )
#define CLKPD_SPI0_DIS                 (0x0   << 5  ) /* DIS. Disable SPI0 clocks. */
#define CLKPD_SPI0_EN                  (0x1   << 5  ) /* EN. Enable SPI0 clocks.  */

/* CLKPD[T2] - T2 clocks enable bit. */
#define CLKPD_T2_BBA                   (*(volatile unsigned long *) 0x42049090)
#define CLKPD_T2_MSK                   (0x1   << 4  )
#define CLKPD_T2                       (0x1   << 4  )
#define CLKPD_T2_DIS                   (0x0   << 4  ) /* DIS. Disable T2 clocks.  */
#define CLKPD_T2_EN                    (0x1   << 4  ) /* EN. Enable T2 clocks.    */

/* CLKPD[ADC] - ADC clocks enable bit. */
#define CLKPD_ADC_BBA                  (*(volatile unsigned long *) 0x4204908C)
#define CLKPD_ADC_MSK                  (0x1   << 3  )
#define CLKPD_ADC                      (0x1   << 3  )
#define CLKPD_ADC_DIS                  (0x0   << 3  ) /* DIS. Disable ADC clocks. */
#define CLKPD_ADC_EN                   (0x1   << 3  ) /* EN. Enable ADC clocks.   */

/* CLKPD[SRAM] - SRAM clocks enable bit. */
#define CLKPD_SRAM_BBA                 (*(volatile unsigned long *) 0x42049088)
#define CLKPD_SRAM_MSK                 (0x1   << 2  )
#define CLKPD_SRAM                     (0x1   << 2  )
#define CLKPD_SRAM_DIS                 (0x0   << 2  ) /* DIS. Disable SRAM memory clocks. */
#define CLKPD_SRAM_EN                  (0x1   << 2  ) /* EN. Enable SRAM memory clocks. */

/* CLKPD[FEE] - Flash clocks enable bit. */
#define CLKPD_FEE_BBA                  (*(volatile unsigned long *) 0x42049084)
#define CLKPD_FEE_MSK                  (0x1   << 1  )
#define CLKPD_FEE                      (0x1   << 1  )
#define CLKPD_FEE_DIS                  (0x0   << 1  ) /* DIS. Disable Flash memory clocks. */
#define CLKPD_FEE_EN                   (0x1   << 1  ) /* EN. Enable Flash memory clocks. */

/* CLKPD[DMA] - DMA clock enable bit. */
#define CLKPD_DMA_BBA                  (*(volatile unsigned long *) 0x42049080)
#define CLKPD_DMA_MSK                  (0x1   << 0  )
#define CLKPD_DMA                      (0x1   << 0  )
#define CLKPD_DMA_DIS                  (0x0   << 0  ) /* DIS. Disable DMA clock.  */
#define CLKPD_DMA_EN                   (0x1   << 0  ) /* EN. Enable DMA clock.    */
// ------------------------------------------------------------------------------------------------
// -----                                        DMA                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Direct Memory Access (pADI_DMA)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_DMA Structure                     */
  __IO uint32_t  DMASTA;                    /*!< Status Register                       */
  __IO uint32_t  DMACFG;                    /*!< Configuration Register                */
  __IO uint32_t  DMAPDBPTR;                 /*!< Primary Control Database Pointer Register */
  __IO uint32_t  DMAADBPTR;                 /*!< Alternate Control Database Pointer Register */
  __I  uint32_t  RESERVED0;
  __IO uint32_t  DMASWREQ;                  /*!< Channel Software Request Register     */
  __I  uint32_t  RESERVED1[2];
  __IO uint32_t  DMARMSKSET;                /*!< Channel Request Mask Set Register     */
  __IO uint32_t  DMARMSKCLR;                /*!< Channel Request Mask Clear Register   */
  __IO uint32_t  DMAENSET;                  /*!< Channel Enable Set Register           */
  __IO uint32_t  DMAENCLR;                  /*!< Channel Enable Clear Register         */
  __IO uint32_t  DMAALTSET;                 /*!< Channel Primary-Alternate Set Register */
  __IO uint32_t  DMAALTCLR;                 /*!< Channel Primary-Alternate Clear Register */
  __IO uint32_t  DMAPRISET;                 /*!< Channel Priority Set Register         */
  __IO uint32_t  DMAPRICLR;                 /*!< Channel Priority Clear Register       */
  __I  uint32_t  RESERVED2[3];
  __IO uint32_t  DMAERRCLR;                 /*!< Bus Error Clear Register              */
} ADI_DMA_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          DMASTA                                     (*(volatile unsigned long      *) 0x40010000)
#define          DMACFG                                     (*(volatile unsigned long      *) 0x40010004)
#define          DMAPDBPTR                                  (*(volatile unsigned long      *) 0x40010008)
#define          DMAADBPTR                                  (*(volatile unsigned long      *) 0x4001000C)
#define          DMASWREQ                                   (*(volatile unsigned long      *) 0x40010014)
#define          DMARMSKSET                                 (*(volatile unsigned long      *) 0x40010020)
#define          DMARMSKCLR                                 (*(volatile unsigned long      *) 0x40010024)
#define          DMAENSET                                   (*(volatile unsigned long      *) 0x40010028)
#define          DMAENCLR                                   (*(volatile unsigned long      *) 0x4001002C)
#define          DMAALTSET                                  (*(volatile unsigned long      *) 0x40010030)
#define          DMAALTCLR                                  (*(volatile unsigned long      *) 0x40010034)
#define          DMAPRISET                                  (*(volatile unsigned long      *) 0x40010038)
#define          DMAPRICLR                                  (*(volatile unsigned long      *) 0x4001003C)
#define          DMAERRCLR                                  (*(volatile unsigned long      *) 0x4001004C)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for DMASTA*/
#define DMASTA_RVAL                    0xD0000

/* DMASTA[CHNLSMINUS1] - Number of available DMA channels minus 1. For example, if there are 14 channels available, the register reads back 0xD for these bits. */
#define DMASTA_CHNLSMINUS1_MSK         (0x1F  << 16 )
#define DMASTA_CHNLSMINUS1_FOURTEENCHNLS (0xD   << 16 ) /* FOURTEENCHNLS - Controller configured to use 14 DMA channels. */

/* DMASTA[STATE] - Current state of DMA controller state machine.  Provides insight into the operation performed by the DMA at the time this register is read. */
#define DMASTA_STATE_MSK               (0xF   << 4  )
#define DMASTA_STATE_IDLE              (0x0   << 4  ) /* IDL. Idle.               */
#define DMASTA_STATE_RDCHNLDATA        (0x1   << 4  ) /* RDCHNLDATA. Reading channel controller data. */
#define DMASTA_STATE_RDSRCENDPTR       (0x2   << 4  ) /* RDSRCENDPTR. Reading source data end pointer. */
#define DMASTA_STATE_RDDSTENDPTR       (0x3   << 4  ) /* RDDSTENDPTR. Reading destination end pointer. */
#define DMASTA_STATE_RDSRCDATA         (0x4   << 4  ) /* RDSRCDATA. Reading source data. */
#define DMASTA_STATE_WRDSTDATA         (0x5   << 4  ) /* WRDSTDATA. Writing destination data. */
#define DMASTA_STATE_WAITDMAREQCLR     (0x6   << 4  ) /* WAITDMAREQCLR. Waiting for DMA request to clear. */
#define DMASTA_STATE_WRCHNLDATA        (0x7   << 4  ) /* WRCHNLDATA. Writing channel controller data. */
#define DMASTA_STATE_STALLED           (0x8   << 4  ) /* STALLED. Stalled.        */
#define DMASTA_STATE_DONE              (0x9   << 4  ) /* DONE. Done.              */
#define DMASTA_STATE_SCATRGATHR        (0xA   << 4  ) /* SCATRGATHR. Peripheral scatter-gather transition. */

/* DMASTA[ENABLE] - Enable status of the controller. */
#define DMASTA_ENABLE_BBA              (*(volatile unsigned long *) 0x42200000)
#define DMASTA_ENABLE_MSK              (0x1   << 0  )
#define DMASTA_ENABLE                  (0x1   << 0  )
#define DMASTA_ENABLE_CLR              (0x0   << 0  ) /* CLR. Controller is disabled. */
#define DMASTA_ENABLE_SET              (0x1   << 0  ) /* SET. Controller is enabled. */

/* Reset Value for DMACFG*/
#define DMACFG_RVAL                    0x0

/* DMACFG[ENABLE] - Controller enable. */
#define DMACFG_ENABLE_BBA              (*(volatile unsigned long *) 0x42200080)
#define DMACFG_ENABLE_MSK              (0x1   << 0  )
#define DMACFG_ENABLE                  (0x1   << 0  )
#define DMACFG_ENABLE_DIS              (0x0   << 0  ) /* DIS. Controller is disabled. */
#define DMACFG_ENABLE_EN               (0x1   << 0  ) /* EN. Controller is enabled. */

/* Reset Value for DMAPDBPTR*/
#define DMAPDBPTR_RVAL                 0x0

/* DMAPDBPTR[CTRLBASEPTR] - Pointer to the base address of the primary data structure. 5 + log (2)M LSBs are reserved and must be written 0. M is the number of channels. */
#define DMAPDBPTR_CTRLBASEPTR_MSK      (0xFFFFFFFF << 0  )

/* Reset Value for DMAADBPTR*/
#define DMAADBPTR_RVAL                 0x100

/* DMAADBPTR[ALTCBPTR] - Base address of the alternate data structure. */
#define DMAADBPTR_ALTCBPTR_MSK         (0xFFFFFFFF << 0  )

/* Reset Value for DMASWREQ*/
#define DMASWREQ_RVAL                  0x0

/* DMASWREQ[SPI0RX] - DMA SPI0 RX. */
#define DMASWREQ_SPI0RX_BBA            (*(volatile unsigned long *) 0x422002B4)
#define DMASWREQ_SPI0RX_MSK            (0x1   << 13 )
#define DMASWREQ_SPI0RX                (0x1   << 13 )
#define DMASWREQ_SPI0RX_DIS            (0x0   << 13 ) /* DIS. Does not create a DMA request for SPI0RX. */
#define DMASWREQ_SPI0RX_EN             (0x1   << 13 ) /* EN. Generates a DMA request for SPI0RX. */

/* DMASWREQ[SPI0TX] - DMA SPI0 TX. */
#define DMASWREQ_SPI0TX_BBA            (*(volatile unsigned long *) 0x422002B0)
#define DMASWREQ_SPI0TX_MSK            (0x1   << 12 )
#define DMASWREQ_SPI0TX                (0x1   << 12 )
#define DMASWREQ_SPI0TX_DIS            (0x0   << 12 ) /* DIS.  Does not create a DMA request for SPI0TX. */
#define DMASWREQ_SPI0TX_EN             (0x1   << 12 ) /* EN. Generates a DMA request for SPI0TX. */

/* DMASWREQ[ADC] - DMA ADC. */
#define DMASWREQ_ADC_BBA               (*(volatile unsigned long *) 0x422002AC)
#define DMASWREQ_ADC_MSK               (0x1   << 11 )
#define DMASWREQ_ADC                   (0x1   << 11 )
#define DMASWREQ_ADC_DIS               (0x0   << 11 ) /* DIS. Does not create a DMA request for ADC. */
#define DMASWREQ_ADC_EN                (0x1   << 11 ) /* EN. Generates a DMA request for ADC. */

/* DMASWREQ[I2CMRX] - DMA I2C Master RX. */
#define DMASWREQ_I2CMRX_BBA            (*(volatile unsigned long *) 0x4220029C)
#define DMASWREQ_I2CMRX_MSK            (0x1   << 7  )
#define DMASWREQ_I2CMRX                (0x1   << 7  )
#define DMASWREQ_I2CMRX_DIS            (0x0   << 7  ) /* DIS. Does not create a DMA request for I2CMRX. */
#define DMASWREQ_I2CMRX_EN             (0x1   << 7  ) /* EN. Generates a DMA request for I2CMRX. */

/* DMASWREQ[I2CMTX] - DMA I2C Master TX. */
#define DMASWREQ_I2CMTX_BBA            (*(volatile unsigned long *) 0x42200298)
#define DMASWREQ_I2CMTX_MSK            (0x1   << 6  )
#define DMASWREQ_I2CMTX                (0x1   << 6  )
#define DMASWREQ_I2CMTX_DIS            (0x0   << 6  ) /* DIS. Does not create a DMA request for I2CMTX. */
#define DMASWREQ_I2CMTX_EN             (0x1   << 6  ) /* EN. Generates a DMA request for I2CMTX. */

/* DMASWREQ[I2CSRX] - DMA I2C Slave RX. */
#define DMASWREQ_I2CSRX_BBA            (*(volatile unsigned long *) 0x42200294)
#define DMASWREQ_I2CSRX_MSK            (0x1   << 5  )
#define DMASWREQ_I2CSRX                (0x1   << 5  )
#define DMASWREQ_I2CSRX_DIS            (0x0   << 5  ) /* DIS. Does not create a DMA request for I2CSRX. */
#define DMASWREQ_I2CSRX_EN             (0x1   << 5  ) /* EN. Generates a DMA request for I2CSRX. */

/* DMASWREQ[I2CSTX] - DMA I2C Slave TX. */
#define DMASWREQ_I2CSTX_BBA            (*(volatile unsigned long *) 0x42200290)
#define DMASWREQ_I2CSTX_MSK            (0x1   << 4  )
#define DMASWREQ_I2CSTX                (0x1   << 4  )
#define DMASWREQ_I2CSTX_DIS            (0x0   << 4  ) /* DIS. Does not create a DMA request for I2CSTX. */
#define DMASWREQ_I2CSTX_EN             (0x1   << 4  ) /* EN. Generates a DMA request for I2CSTX. */

/* DMASWREQ[UARTRX] - DMA UART RX. */
#define DMASWREQ_UARTRX_BBA            (*(volatile unsigned long *) 0x4220028C)
#define DMASWREQ_UARTRX_MSK            (0x1   << 3  )
#define DMASWREQ_UARTRX                (0x1   << 3  )
#define DMASWREQ_UARTRX_DIS            (0x0   << 3  ) /* DIS. Does not create a DMA request for UARTRX. */
#define DMASWREQ_UARTRX_EN             (0x1   << 3  ) /* EN. Generates a DMA request for UARTRX. */

/* DMASWREQ[UARTTX] - DMA UART TX. */
#define DMASWREQ_UARTTX_BBA            (*(volatile unsigned long *) 0x42200288)
#define DMASWREQ_UARTTX_MSK            (0x1   << 2  )
#define DMASWREQ_UARTTX                (0x1   << 2  )
#define DMASWREQ_UARTTX_DIS            (0x0   << 2  ) /* DIS.  Does not create a DMA request for UARTTX. */
#define DMASWREQ_UARTTX_EN             (0x1   << 2  ) /* EN. Generates a DMA request for UARTTX. */

/* DMASWREQ[SPI1RX] - DMA SPI 1 RX. */
#define DMASWREQ_SPI1RX_BBA            (*(volatile unsigned long *) 0x42200284)
#define DMASWREQ_SPI1RX_MSK            (0x1   << 1  )
#define DMASWREQ_SPI1RX                (0x1   << 1  )
#define DMASWREQ_SPI1RX_DIS            (0x0   << 1  ) /* DIS.  Does not create a DMA request for SPI1RX. */
#define DMASWREQ_SPI1RX_EN             (0x1   << 1  ) /* EN. Generates a DMA request for SPI1RX. */

/* DMASWREQ[SPI1TX] - DMA SPI 1 TX. */
#define DMASWREQ_SPI1TX_BBA            (*(volatile unsigned long *) 0x42200280)
#define DMASWREQ_SPI1TX_MSK            (0x1   << 0  )
#define DMASWREQ_SPI1TX                (0x1   << 0  )
#define DMASWREQ_SPI1TX_DIS            (0x0   << 0  ) /* DIS. Does not create a DMA request for SPI1TX. */
#define DMASWREQ_SPI1TX_EN             (0x1   << 0  ) /* EN. Generates a DMA request for SPI1TX. */

/* Reset Value for DMARMSKSET*/
#define DMARMSKSET_RVAL                0x0

/* DMARMSKSET[SPI0RX] - DMA SPI0 RX. */
#define DMARMSKSET_SPI0RX_BBA          (*(volatile unsigned long *) 0x42200434)
#define DMARMSKSET_SPI0RX_MSK          (0x1   << 13 )
#define DMARMSKSET_SPI0RX              (0x1   << 13 )
#define DMARMSKSET_SPI0RX_DIS          (0x0   << 13 ) /* DIS. When read: Requests are enabled for SPI0RX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_SPI0RX_EN           (0x1   << 13 ) /* EN. When read: Requests are disabled for SPI0RX. When written: Disables peripheral associated with SPI0RX from generating DMA requests. */

/* DMARMSKSET[SPI0TX] - DMA SPI0 TX. */
#define DMARMSKSET_SPI0TX_BBA          (*(volatile unsigned long *) 0x42200430)
#define DMARMSKSET_SPI0TX_MSK          (0x1   << 12 )
#define DMARMSKSET_SPI0TX              (0x1   << 12 )
#define DMARMSKSET_SPI0TX_DIS          (0x0   << 12 ) /* DIS. When read: Requests are enabled for SPI0TX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_SPI0TX_EN           (0x1   << 12 ) /* EN.  When read: Requests are disabled for SPI0TX. When written: Disables peripheral associated with SPI0TX from generating DMA requests. */

/* DMARMSKSET[ADC] - DMA ADC. */
#define DMARMSKSET_ADC_BBA             (*(volatile unsigned long *) 0x4220042C)
#define DMARMSKSET_ADC_MSK             (0x1   << 11 )
#define DMARMSKSET_ADC                 (0x1   << 11 )
#define DMARMSKSET_ADC_DIS             (0x0   << 11 ) /* DIS. When read: Requests are enabled for ADC. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_ADC_EN              (0x1   << 11 ) /* EN.  When read: Requests are disabled for ADC. When written: Disables peripheral associated with ADC from generating DMA requests. */

/* DMARMSKSET[I2CMRX] - DMA I2C Master RX. */
#define DMARMSKSET_I2CMRX_BBA          (*(volatile unsigned long *) 0x4220041C)
#define DMARMSKSET_I2CMRX_MSK          (0x1   << 7  )
#define DMARMSKSET_I2CMRX              (0x1   << 7  )
#define DMARMSKSET_I2CMRX_DIS          (0x0   << 7  ) /* DIS. When read: Requests are enabled for I2CMRX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_I2CMRX_EN           (0x1   << 7  ) /* EN.  When read: Requests are disabled for I2CMRX. When written: Disables peripheral associated with I2CMRX from generating DMA requests. */

/* DMARMSKSET[I2CMTX] - DMA I2C Master TX. */
#define DMARMSKSET_I2CMTX_BBA          (*(volatile unsigned long *) 0x42200418)
#define DMARMSKSET_I2CMTX_MSK          (0x1   << 6  )
#define DMARMSKSET_I2CMTX              (0x1   << 6  )
#define DMARMSKSET_I2CMTX_DIS          (0x0   << 6  ) /* DIS. When read: Requests are enabled for I2CMTX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_I2CMTX_EN           (0x1   << 6  ) /* EN.  When read: Requests are disabled for I2CMTX. When written: Disables peripheral associated with I2CMTX from generating DMA requests. */

/* DMARMSKSET[I2CSRX] - DMA I2C Slave RX. */
#define DMARMSKSET_I2CSRX_BBA          (*(volatile unsigned long *) 0x42200414)
#define DMARMSKSET_I2CSRX_MSK          (0x1   << 5  )
#define DMARMSKSET_I2CSRX              (0x1   << 5  )
#define DMARMSKSET_I2CSRX_DIS          (0x0   << 5  ) /* DIS. When read: Requests are enabled for I2CSRX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_I2CSRX_EN           (0x1   << 5  ) /* EN.  When read: Requests are disabled for I2CSRX. When written: Disables peripheral associated with I2CSRX from generating DMA requests. */

/* DMARMSKSET[I2CSTX] - DMA I2C Slave TX. */
#define DMARMSKSET_I2CSTX_BBA          (*(volatile unsigned long *) 0x42200410)
#define DMARMSKSET_I2CSTX_MSK          (0x1   << 4  )
#define DMARMSKSET_I2CSTX              (0x1   << 4  )
#define DMARMSKSET_I2CSTX_DIS          (0x0   << 4  ) /* DIS. When read: Requests are enabled forI2CSTX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_I2CSTX_EN           (0x1   << 4  ) /* EN.  When read: Requests are disabled for I2CSTX. When written: Disables peripheral associated with I2CSTX from generating DMA requests. */

/* DMARMSKSET[UARTRX] - DMA UART RX. */
#define DMARMSKSET_UARTRX_BBA          (*(volatile unsigned long *) 0x4220040C)
#define DMARMSKSET_UARTRX_MSK          (0x1   << 3  )
#define DMARMSKSET_UARTRX              (0x1   << 3  )
#define DMARMSKSET_UARTRX_DIS          (0x0   << 3  ) /* DIS. When read: Requests are enabled for UARTRX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_UARTRX_EN           (0x1   << 3  ) /* EN.  When read: Requests are disabled for UARTRX. When written: Disables peripheral associated with UARTRX from generating DMA requests. */

/* DMARMSKSET[UARTTX] - DMA UART TX. */
#define DMARMSKSET_UARTTX_BBA          (*(volatile unsigned long *) 0x42200408)
#define DMARMSKSET_UARTTX_MSK          (0x1   << 2  )
#define DMARMSKSET_UARTTX              (0x1   << 2  )
#define DMARMSKSET_UARTTX_DIS          (0x0   << 2  ) /* DIS. When read: Requests are enabled for UARTTX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_UARTTX_EN           (0x1   << 2  ) /* EN.  When read: Requests are disabled for UARTTX. When written: Disables peripheral associated with UARTTX from generating DMA requests. */

/* DMARMSKSET[SPI1RX] - DMA SPI 1 RX. */
#define DMARMSKSET_SPI1RX_BBA          (*(volatile unsigned long *) 0x42200404)
#define DMARMSKSET_SPI1RX_MSK          (0x1   << 1  )
#define DMARMSKSET_SPI1RX              (0x1   << 1  )
#define DMARMSKSET_SPI1RX_DIS          (0x0   << 1  ) /* DIS. When read: Requests are enabled for SPI1RX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_SPI1RX_EN           (0x1   << 1  ) /* EN.  When read: Requests are disabled for SPI1RX. When written: Disables peripheral associated with SPI1RX from generating DMA requests. */

/* DMARMSKSET[SPI1TX] - DMA SPI 1 TX. */
#define DMARMSKSET_SPI1TX_BBA          (*(volatile unsigned long *) 0x42200400)
#define DMARMSKSET_SPI1TX_MSK          (0x1   << 0  )
#define DMARMSKSET_SPI1TX              (0x1   << 0  )
#define DMARMSKSET_SPI1TX_DIS          (0x0   << 0  ) /* DIS. When read: Requests are enabled for SPI1TX. When written: No effect. Use the DMARMSKCLR register to enable DMA requests. */
#define DMARMSKSET_SPI1TX_EN           (0x1   << 0  ) /* EN.  When read: Requests are disabled for SPI1TX When written: Disables peripheral associated with SPI1TX from generating DMA requests. */

/* Reset Value for DMARMSKCLR*/
#define DMARMSKCLR_RVAL                0x0

/* DMARMSKCLR[SPI0RX] - DMA SPI0 RX. */
#define DMARMSKCLR_SPI0RX_BBA          (*(volatile unsigned long *) 0x422004B4)
#define DMARMSKCLR_SPI0RX_MSK          (0x1   << 13 )
#define DMARMSKCLR_SPI0RX              (0x1   << 13 )
#define DMARMSKCLR_SPI0RX_DIS          (0x0   << 13 ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_SPI0RX_EN           (0x1   << 13 ) /* EN. Enables peripheral associated with SPI0RX to generate DMA requests. */

/* DMARMSKCLR[SPI0TX] - DMA SPI0 TX. */
#define DMARMSKCLR_SPI0TX_BBA          (*(volatile unsigned long *) 0x422004B0)
#define DMARMSKCLR_SPI0TX_MSK          (0x1   << 12 )
#define DMARMSKCLR_SPI0TX              (0x1   << 12 )
#define DMARMSKCLR_SPI0TX_DIS          (0x0   << 12 ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_SPI0TX_EN           (0x1   << 12 ) /* EN. Enables peripheral associated with SPI0TX to generate DMA requests. */

/* DMARMSKCLR[ADC] - DMA ADC. */
#define DMARMSKCLR_ADC_BBA             (*(volatile unsigned long *) 0x422004AC)
#define DMARMSKCLR_ADC_MSK             (0x1   << 11 )
#define DMARMSKCLR_ADC                 (0x1   << 11 )
#define DMARMSKCLR_ADC_DIS             (0x0   << 11 ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_ADC_EN              (0x1   << 11 ) /* EN. Enables peripheral associated with ADC to generate DMA requests. */

/* DMARMSKCLR[I2CMRX] - DMA I2C Master RX. */
#define DMARMSKCLR_I2CMRX_BBA          (*(volatile unsigned long *) 0x4220049C)
#define DMARMSKCLR_I2CMRX_MSK          (0x1   << 7  )
#define DMARMSKCLR_I2CMRX              (0x1   << 7  )
#define DMARMSKCLR_I2CMRX_DIS          (0x0   << 7  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_I2CMRX_EN           (0x1   << 7  ) /* EN. Enables peripheral associated with I2CMRX to generate DMA requests. */

/* DMARMSKCLR[I2CMTX] - DMA I2C Master TX. */
#define DMARMSKCLR_I2CMTX_BBA          (*(volatile unsigned long *) 0x42200498)
#define DMARMSKCLR_I2CMTX_MSK          (0x1   << 6  )
#define DMARMSKCLR_I2CMTX              (0x1   << 6  )
#define DMARMSKCLR_I2CMTX_DIS          (0x0   << 6  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_I2CMTX_EN           (0x1   << 6  ) /* EN. Enables peripheral associated with I2CMTX to generate DMA requests. */

/* DMARMSKCLR[I2CSRX] - DMA I2C Slave RX. */
#define DMARMSKCLR_I2CSRX_BBA          (*(volatile unsigned long *) 0x42200494)
#define DMARMSKCLR_I2CSRX_MSK          (0x1   << 5  )
#define DMARMSKCLR_I2CSRX              (0x1   << 5  )
#define DMARMSKCLR_I2CSRX_DIS          (0x0   << 5  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_I2CSRX_EN           (0x1   << 5  ) /* EN. Enables peripheral associated with I2CSRX to generate DMA requests. */

/* DMARMSKCLR[I2CSTX] - DMA I2C Slave TX. */
#define DMARMSKCLR_I2CSTX_BBA          (*(volatile unsigned long *) 0x42200490)
#define DMARMSKCLR_I2CSTX_MSK          (0x1   << 4  )
#define DMARMSKCLR_I2CSTX              (0x1   << 4  )
#define DMARMSKCLR_I2CSTX_DIS          (0x0   << 4  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_I2CSTX_EN           (0x1   << 4  ) /* EN. Enables peripheral associated with I2CSTX to generate DMA requests. */

/* DMARMSKCLR[UARTRX] - DMA UART RX. */
#define DMARMSKCLR_UARTRX_BBA          (*(volatile unsigned long *) 0x4220048C)
#define DMARMSKCLR_UARTRX_MSK          (0x1   << 3  )
#define DMARMSKCLR_UARTRX              (0x1   << 3  )
#define DMARMSKCLR_UARTRX_DIS          (0x0   << 3  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_UARTRX_EN           (0x1   << 3  ) /* EN. Enables peripheral associated with UARTRX to generate DMA requests. */

/* DMARMSKCLR[UARTTX] - DMA UART TX. */
#define DMARMSKCLR_UARTTX_BBA          (*(volatile unsigned long *) 0x42200488)
#define DMARMSKCLR_UARTTX_MSK          (0x1   << 2  )
#define DMARMSKCLR_UARTTX              (0x1   << 2  )
#define DMARMSKCLR_UARTTX_DIS          (0x0   << 2  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_UARTTX_EN           (0x1   << 2  ) /* EN. Enables peripheral associated with UARTTX to generate DMA requests. */

/* DMARMSKCLR[SPI1RX] - DMA SPI 1 RX. */
#define DMARMSKCLR_SPI1RX_BBA          (*(volatile unsigned long *) 0x42200484)
#define DMARMSKCLR_SPI1RX_MSK          (0x1   << 1  )
#define DMARMSKCLR_SPI1RX              (0x1   << 1  )
#define DMARMSKCLR_SPI1RX_DIS          (0x0   << 1  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_SPI1RX_EN           (0x1   << 1  ) /* EN. Enables peripheral associated with SPI1RX to generate DMA requests. */

/* DMARMSKCLR[SPI1TX] - DMA SPI 1 TX. */
#define DMARMSKCLR_SPI1TX_BBA          (*(volatile unsigned long *) 0x42200480)
#define DMARMSKCLR_SPI1TX_MSK          (0x1   << 0  )
#define DMARMSKCLR_SPI1TX              (0x1   << 0  )
#define DMARMSKCLR_SPI1TX_DIS          (0x0   << 0  ) /* DIS. No effect. Use the DMARMSKSET register to disable DMA requests. */
#define DMARMSKCLR_SPI1TX_EN           (0x1   << 0  ) /* EN. Enables peripheral associated with SPI1TX to generate DMA requests. */

/* Reset Value for DMAENSET*/
#define DMAENSET_RVAL                  0x0

/* DMAENSET[SPI0RX] - DMA SPI0 RX */
#define DMAENSET_SPI0RX_BBA            (*(volatile unsigned long *) 0x42200534)
#define DMAENSET_SPI0RX_MSK            (0x1   << 13 )
#define DMAENSET_SPI0RX                (0x1   << 13 )
#define DMAENSET_SPI0RX_DIS            (0x0   << 13 ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_SPI0RX_EN             (0x1   << 13 ) /* EN. Enables SPI0RX.      */

/* DMAENSET[SPI0TX] - DMA SPI0 TX. */
#define DMAENSET_SPI0TX_BBA            (*(volatile unsigned long *) 0x42200530)
#define DMAENSET_SPI0TX_MSK            (0x1   << 12 )
#define DMAENSET_SPI0TX                (0x1   << 12 )
#define DMAENSET_SPI0TX_DIS            (0x0   << 12 ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_SPI0TX_EN             (0x1   << 12 ) /* EN. Enables SPI0TX.      */

/* DMAENSET[ADC] - DMA ADC. */
#define DMAENSET_ADC_BBA               (*(volatile unsigned long *) 0x4220052C)
#define DMAENSET_ADC_MSK               (0x1   << 11 )
#define DMAENSET_ADC                   (0x1   << 11 )
#define DMAENSET_ADC_DIS               (0x0   << 11 ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_ADC_EN                (0x1   << 11 ) /* EN. Enables ADC.         */

/* DMAENSET[I2CMRX] - DMA I2C Master RX. */
#define DMAENSET_I2CMRX_BBA            (*(volatile unsigned long *) 0x4220051C)
#define DMAENSET_I2CMRX_MSK            (0x1   << 7  )
#define DMAENSET_I2CMRX                (0x1   << 7  )
#define DMAENSET_I2CMRX_DIS            (0x0   << 7  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_I2CMRX_EN             (0x1   << 7  ) /* EN. . Enables I2CMRX.    */

/* DMAENSET[I2CMTX] - DMA I2C Master TX. */
#define DMAENSET_I2CMTX_BBA            (*(volatile unsigned long *) 0x42200518)
#define DMAENSET_I2CMTX_MSK            (0x1   << 6  )
#define DMAENSET_I2CMTX                (0x1   << 6  )
#define DMAENSET_I2CMTX_DIS            (0x0   << 6  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_I2CMTX_EN             (0x1   << 6  ) /* EN. . Enables I2CMTX.    */

/* DMAENSET[I2CSRX] - DMA I2C Slave RX. */
#define DMAENSET_I2CSRX_BBA            (*(volatile unsigned long *) 0x42200514)
#define DMAENSET_I2CSRX_MSK            (0x1   << 5  )
#define DMAENSET_I2CSRX                (0x1   << 5  )
#define DMAENSET_I2CSRX_DIS            (0x0   << 5  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_I2CSRX_EN             (0x1   << 5  ) /* EN. Enables I2CSRX.      */

/* DMAENSET[I2CSTX] - DMA I2C Slave TX. */
#define DMAENSET_I2CSTX_BBA            (*(volatile unsigned long *) 0x42200510)
#define DMAENSET_I2CSTX_MSK            (0x1   << 4  )
#define DMAENSET_I2CSTX                (0x1   << 4  )
#define DMAENSET_I2CSTX_DIS            (0x0   << 4  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_I2CSTX_EN             (0x1   << 4  ) /* EN. Enables I2CSTX.      */

/* DMAENSET[UARTRX] - DMA UART RX. */
#define DMAENSET_UARTRX_BBA            (*(volatile unsigned long *) 0x4220050C)
#define DMAENSET_UARTRX_MSK            (0x1   << 3  )
#define DMAENSET_UARTRX                (0x1   << 3  )
#define DMAENSET_UARTRX_DIS            (0x0   << 3  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_UARTRX_EN             (0x1   << 3  ) /* EN. Enables UARTRX.      */

/* DMAENSET[UARTTX] - DMA UART TX. */
#define DMAENSET_UARTTX_BBA            (*(volatile unsigned long *) 0x42200508)
#define DMAENSET_UARTTX_MSK            (0x1   << 2  )
#define DMAENSET_UARTTX                (0x1   << 2  )
#define DMAENSET_UARTTX_DIS            (0x0   << 2  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_UARTTX_EN             (0x1   << 2  ) /* EN. Enables UARTTX.      */

/* DMAENSET[SPI1RX] - DMA SPI 1 RX. */
#define DMAENSET_SPI1RX_BBA            (*(volatile unsigned long *) 0x42200504)
#define DMAENSET_SPI1RX_MSK            (0x1   << 1  )
#define DMAENSET_SPI1RX                (0x1   << 1  )
#define DMAENSET_SPI1RX_DIS            (0x0   << 1  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_SPI1RX_EN             (0x1   << 1  ) /* EN. Enables SPI1RX.      */

/* DMAENSET[SPI1TX] - DMA SPI 1 TX. */
#define DMAENSET_SPI1TX_BBA            (*(volatile unsigned long *) 0x42200500)
#define DMAENSET_SPI1TX_MSK            (0x1   << 0  )
#define DMAENSET_SPI1TX                (0x1   << 0  )
#define DMAENSET_SPI1TX_DIS            (0x0   << 0  ) /* DIS. No effect. Use the DMAENCLR register to disable the channel. */
#define DMAENSET_SPI1TX_EN             (0x1   << 0  ) /* EN. Enables SPI1TX.      */

/* Reset Value for DMAENCLR*/
#define DMAENCLR_RVAL                  0x0

/* DMAENCLR[SPI0RX] - DMA SPI0 RX */
#define DMAENCLR_SPI0RX_BBA            (*(volatile unsigned long *) 0x422005B4)
#define DMAENCLR_SPI0RX_MSK            (0x1   << 13 )
#define DMAENCLR_SPI0RX                (0x1   << 13 )
#define DMAENCLR_SPI0RX_DIS            (0x0   << 13 ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_SPI0RX_EN             (0x1   << 13 ) /* EN. Disables SPI0RX.     */

/* DMAENCLR[SPI0TX] - DMA SPI0 TX */
#define DMAENCLR_SPI0TX_BBA            (*(volatile unsigned long *) 0x422005B0)
#define DMAENCLR_SPI0TX_MSK            (0x1   << 12 )
#define DMAENCLR_SPI0TX                (0x1   << 12 )
#define DMAENCLR_SPI0TX_DIS            (0x0   << 12 ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_SPI0TX_EN             (0x1   << 12 ) /* EN. Disables SPI0TX.     */

/* DMAENCLR[ADC] - DMA ADC. */
#define DMAENCLR_ADC_BBA               (*(volatile unsigned long *) 0x422005AC)
#define DMAENCLR_ADC_MSK               (0x1   << 11 )
#define DMAENCLR_ADC                   (0x1   << 11 )
#define DMAENCLR_ADC_DIS               (0x0   << 11 ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_ADC_EN                (0x1   << 11 ) /* EN. Disables ADC.        */

/* DMAENCLR[I2CMRX] - DMA I2C Master RX. */
#define DMAENCLR_I2CMRX_BBA            (*(volatile unsigned long *) 0x4220059C)
#define DMAENCLR_I2CMRX_MSK            (0x1   << 7  )
#define DMAENCLR_I2CMRX                (0x1   << 7  )
#define DMAENCLR_I2CMRX_DIS            (0x0   << 7  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_I2CMRX_EN             (0x1   << 7  ) /* EN. Disables I2CMRX.     */

/* DMAENCLR[I2CMTX] - DMA I2C Master TX. */
#define DMAENCLR_I2CMTX_BBA            (*(volatile unsigned long *) 0x42200598)
#define DMAENCLR_I2CMTX_MSK            (0x1   << 6  )
#define DMAENCLR_I2CMTX                (0x1   << 6  )
#define DMAENCLR_I2CMTX_DIS            (0x0   << 6  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_I2CMTX_EN             (0x1   << 6  ) /* EN. Disables I2CMTX.     */

/* DMAENCLR[I2CSRX] - DMA I2C Slave RX. */
#define DMAENCLR_I2CSRX_BBA            (*(volatile unsigned long *) 0x42200594)
#define DMAENCLR_I2CSRX_MSK            (0x1   << 5  )
#define DMAENCLR_I2CSRX                (0x1   << 5  )
#define DMAENCLR_I2CSRX_DIS            (0x0   << 5  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_I2CSRX_EN             (0x1   << 5  ) /* EN. Disables I2CSRX.     */

/* DMAENCLR[I2CSTX] - DMA I2C Slave TX. */
#define DMAENCLR_I2CSTX_BBA            (*(volatile unsigned long *) 0x42200590)
#define DMAENCLR_I2CSTX_MSK            (0x1   << 4  )
#define DMAENCLR_I2CSTX                (0x1   << 4  )
#define DMAENCLR_I2CSTX_DIS            (0x0   << 4  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_I2CSTX_EN             (0x1   << 4  ) /* EN. Disables I2CSTX.     */

/* DMAENCLR[UARTRX] - DMA UART RX. */
#define DMAENCLR_UARTRX_BBA            (*(volatile unsigned long *) 0x4220058C)
#define DMAENCLR_UARTRX_MSK            (0x1   << 3  )
#define DMAENCLR_UARTRX                (0x1   << 3  )
#define DMAENCLR_UARTRX_DIS            (0x0   << 3  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_UARTRX_EN             (0x1   << 3  ) /* EN. Disables UARTRX.     */

/* DMAENCLR[UARTTX] - DMA UART TX. */
#define DMAENCLR_UARTTX_BBA            (*(volatile unsigned long *) 0x42200588)
#define DMAENCLR_UARTTX_MSK            (0x1   << 2  )
#define DMAENCLR_UARTTX                (0x1   << 2  )
#define DMAENCLR_UARTTX_DIS            (0x0   << 2  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_UARTTX_EN             (0x1   << 2  ) /* EN. Disables UARTTX.     */

/* DMAENCLR[SPI1RX] - DMA SPI 1 RX. */
#define DMAENCLR_SPI1RX_BBA            (*(volatile unsigned long *) 0x42200584)
#define DMAENCLR_SPI1RX_MSK            (0x1   << 1  )
#define DMAENCLR_SPI1RX                (0x1   << 1  )
#define DMAENCLR_SPI1RX_DIS            (0x0   << 1  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_SPI1RX_EN             (0x1   << 1  ) /* EN. Disables SPI1RX.     */

/* DMAENCLR[SPI1TX] - DMA SPI 1 TX. */
#define DMAENCLR_SPI1TX_BBA            (*(volatile unsigned long *) 0x42200580)
#define DMAENCLR_SPI1TX_MSK            (0x1   << 0  )
#define DMAENCLR_SPI1TX                (0x1   << 0  )
#define DMAENCLR_SPI1TX_DIS            (0x0   << 0  ) /* DIS. No effect. Use the DMAENSET register to enable the channel. */
#define DMAENCLR_SPI1TX_EN             (0x1   << 0  ) /* EN. Disables SPI1TX.     */

/* Reset Value for DMAALTSET*/
#define DMAALTSET_RVAL                 0x0

/* DMAALTSET[SPI0RX] - DMA SPI0 RX. */
#define DMAALTSET_SPI0RX_BBA           (*(volatile unsigned long *) 0x42200634)
#define DMAALTSET_SPI0RX_MSK           (0x1   << 13 )
#define DMAALTSET_SPI0RX               (0x1   << 13 )
#define DMAALTSET_SPI0RX_DIS           (0x0   << 13 ) /* DIS. When read: DMA SPI0RX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set SPI0RX to 0. */
#define DMAALTSET_SPI0RX_EN            (0x1   << 13 ) /* EN. When read: DMA SPI0RX is using the alternate data structure. When written: Selects the alternate data structure for SPI0RX. */

/* DMAALTSET[SPI0TX] - DMA SPI0 TX. */
#define DMAALTSET_SPI0TX_BBA           (*(volatile unsigned long *) 0x42200630)
#define DMAALTSET_SPI0TX_MSK           (0x1   << 12 )
#define DMAALTSET_SPI0TX               (0x1   << 12 )
#define DMAALTSET_SPI0TX_DIS           (0x0   << 12 ) /* DIS. When read: DMA SPI0TX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set SPI0TX to 0. */
#define DMAALTSET_SPI0TX_EN            (0x1   << 12 ) /* EN. When read: DMA SPI0TX is using the alternate data structure. When written: Selects the alternate data structure for SPI0TX. */

/* DMAALTSET[ADC] - DMA ADC. */
#define DMAALTSET_ADC_BBA              (*(volatile unsigned long *) 0x4220062C)
#define DMAALTSET_ADC_MSK              (0x1   << 11 )
#define DMAALTSET_ADC                  (0x1   << 11 )
#define DMAALTSET_ADC_DIS              (0x0   << 11 ) /* DIS. When read: DMA ADC is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set ADC to 0. */
#define DMAALTSET_ADC_EN               (0x1   << 11 ) /* EN. When read: DMA ADC is using the alternate data structure. When written: Selects the alternate data structure for ADC. */

/* DMAALTSET[I2CMRX] - DMA I2C Master RX. */
#define DMAALTSET_I2CMRX_BBA           (*(volatile unsigned long *) 0x4220061C)
#define DMAALTSET_I2CMRX_MSK           (0x1   << 7  )
#define DMAALTSET_I2CMRX               (0x1   << 7  )
#define DMAALTSET_I2CMRX_DIS           (0x0   << 7  ) /* DIS. When read: DMA I2CMRX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set I2CMRX to 0. */
#define DMAALTSET_I2CMRX_EN            (0x1   << 7  ) /* EN. When read: DMA I2CMRX is using the alternate data structure. When written: Selects the alternate data structure for I2CMRX. */

/* DMAALTSET[I2CMTX] - DMA I2C Master TX. */
#define DMAALTSET_I2CMTX_BBA           (*(volatile unsigned long *) 0x42200618)
#define DMAALTSET_I2CMTX_MSK           (0x1   << 6  )
#define DMAALTSET_I2CMTX               (0x1   << 6  )
#define DMAALTSET_I2CMTX_DIS           (0x0   << 6  ) /* DIS. When read: DMA I2CMTX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set I2CMTX to 0. */
#define DMAALTSET_I2CMTX_EN            (0x1   << 6  ) /* EN. When read: DMA I2CMTX is using the alternate data structure. When written: Selects the alternate data structure forI2CMTX. */

/* DMAALTSET[I2CSRX] - DMA I2C Slave RX. */
#define DMAALTSET_I2CSRX_BBA           (*(volatile unsigned long *) 0x42200614)
#define DMAALTSET_I2CSRX_MSK           (0x1   << 5  )
#define DMAALTSET_I2CSRX               (0x1   << 5  )
#define DMAALTSET_I2CSRX_DIS           (0x0   << 5  ) /* DIS. When read: DMA I2CSRX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set I2CSRX to 0. */
#define DMAALTSET_I2CSRX_EN            (0x1   << 5  ) /* EN. When read: DMA I2CSRX is using the alternate data structure. When written: Selects the alternate data structure for I2CSRX. */

/* DMAALTSET[I2CSTX] - DMA I2C Slave TX. */
#define DMAALTSET_I2CSTX_BBA           (*(volatile unsigned long *) 0x42200610)
#define DMAALTSET_I2CSTX_MSK           (0x1   << 4  )
#define DMAALTSET_I2CSTX               (0x1   << 4  )
#define DMAALTSET_I2CSTX_DIS           (0x0   << 4  ) /* DIS. When read: DMA I2CSTX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set I2CSTX to 0. */
#define DMAALTSET_I2CSTX_EN            (0x1   << 4  ) /* EN. When read: DMA I2CSTX is using the alternate data structure. When written: Selects the alternate data structure for I2CSTX. */

/* DMAALTSET[UARTRX] - DMA UART RX. */
#define DMAALTSET_UARTRX_BBA           (*(volatile unsigned long *) 0x4220060C)
#define DMAALTSET_UARTRX_MSK           (0x1   << 3  )
#define DMAALTSET_UARTRX               (0x1   << 3  )
#define DMAALTSET_UARTRX_DIS           (0x0   << 3  ) /* DIS. When read: DMA UARTRX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set UARTRX to 0. */
#define DMAALTSET_UARTRX_EN            (0x1   << 3  ) /* EN. When read: DMA UARTRX is using the alternate data structure. When written: Selects the alternate data structure for UARTRX. */

/* DMAALTSET[UARTTX] - DMA UART TX. */
#define DMAALTSET_UARTTX_BBA           (*(volatile unsigned long *) 0x42200608)
#define DMAALTSET_UARTTX_MSK           (0x1   << 2  )
#define DMAALTSET_UARTTX               (0x1   << 2  )
#define DMAALTSET_UARTTX_DIS           (0x0   << 2  ) /* DIS. When read: DMA UARTTX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set UARTTX to 0. */
#define DMAALTSET_UARTTX_EN            (0x1   << 2  ) /* EN. When read: DMA UARTTX is using the alternate data structure. When written: Selects the alternate data structure for UARTTX. */

/* DMAALTSET[SPI1RX] - DMA SPI 1 RX. */
#define DMAALTSET_SPI1RX_BBA           (*(volatile unsigned long *) 0x42200604)
#define DMAALTSET_SPI1RX_MSK           (0x1   << 1  )
#define DMAALTSET_SPI1RX               (0x1   << 1  )
#define DMAALTSET_SPI1RX_DIS           (0x0   << 1  ) /* DIS. When read: DMA SPI1RX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set SPI1RX to 0. */
#define DMAALTSET_SPI1RX_EN            (0x1   << 1  ) /* EN. When read: DMA SPI1RX is using the alternate data structure. When written: Selects the alternate data structure for SPI1RX. */

/* DMAALTSET[SPI1TX] - DMA SPI 1 TX. */
#define DMAALTSET_SPI1TX_BBA           (*(volatile unsigned long *) 0x42200600)
#define DMAALTSET_SPI1TX_MSK           (0x1   << 0  )
#define DMAALTSET_SPI1TX               (0x1   << 0  )
#define DMAALTSET_SPI1TX_DIS           (0x0   << 0  ) /* DIS. When read: DMA SPI1TX is using the primary data structure. When written: No effect. Use the DMAALTCLR register to set SPI1TX to 0. */
#define DMAALTSET_SPI1TX_EN            (0x1   << 0  ) /* EN. When read: DMA SPI1TX is using the alternate data structure. When written: Selects the alternate data structure for SPI1TX. */

/* Reset Value for DMAALTCLR*/
#define DMAALTCLR_RVAL                 0x0

/* DMAALTCLR[SPI0RX] - DMA SPI0 RX. */
#define DMAALTCLR_SPI0RX_BBA           (*(volatile unsigned long *) 0x422006B4)
#define DMAALTCLR_SPI0RX_MSK           (0x1   << 13 )
#define DMAALTCLR_SPI0RX               (0x1   << 13 )
#define DMAALTCLR_SPI0RX_DIS           (0x0   << 13 ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_SPI0RX_EN            (0x1   << 13 ) /* EN. Selects the primary data structure for SPI0RX. */

/* DMAALTCLR[SPI0TX] - DMA SPI0 TX. */
#define DMAALTCLR_SPI0TX_BBA           (*(volatile unsigned long *) 0x422006B0)
#define DMAALTCLR_SPI0TX_MSK           (0x1   << 12 )
#define DMAALTCLR_SPI0TX               (0x1   << 12 )
#define DMAALTCLR_SPI0TX_DIS           (0x0   << 12 ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_SPI0TX_EN            (0x1   << 12 ) /* EN. Selects the primary data structure for SPI0TX. */

/* DMAALTCLR[ADC] - DMA ADC. */
#define DMAALTCLR_ADC_BBA              (*(volatile unsigned long *) 0x422006AC)
#define DMAALTCLR_ADC_MSK              (0x1   << 11 )
#define DMAALTCLR_ADC                  (0x1   << 11 )
#define DMAALTCLR_ADC_DIS              (0x0   << 11 ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_ADC_EN               (0x1   << 11 ) /* EN. Selects the primary data structure for ADC. */

/* DMAALTCLR[I2CMRX] - DMA I2C Master RX. */
#define DMAALTCLR_I2CMRX_BBA           (*(volatile unsigned long *) 0x4220069C)
#define DMAALTCLR_I2CMRX_MSK           (0x1   << 7  )
#define DMAALTCLR_I2CMRX               (0x1   << 7  )
#define DMAALTCLR_I2CMRX_DIS           (0x0   << 7  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_I2CMRX_EN            (0x1   << 7  ) /* EN. Selects the primary data structure for I2CMRX. */

/* DMAALTCLR[I2CMTX] - DMA I2C Master TX. */
#define DMAALTCLR_I2CMTX_BBA           (*(volatile unsigned long *) 0x42200698)
#define DMAALTCLR_I2CMTX_MSK           (0x1   << 6  )
#define DMAALTCLR_I2CMTX               (0x1   << 6  )
#define DMAALTCLR_I2CMTX_DIS           (0x0   << 6  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_I2CMTX_EN            (0x1   << 6  ) /* EN. Selects the primary data structure for I2CMTX. */

/* DMAALTCLR[I2CSRX] - DMA I2C Slave RX. */
#define DMAALTCLR_I2CSRX_BBA           (*(volatile unsigned long *) 0x42200694)
#define DMAALTCLR_I2CSRX_MSK           (0x1   << 5  )
#define DMAALTCLR_I2CSRX               (0x1   << 5  )
#define DMAALTCLR_I2CSRX_DIS           (0x0   << 5  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_I2CSRX_EN            (0x1   << 5  ) /* EN. Selects the primary data structure for I2CSRX. */

/* DMAALTCLR[I2CSTX] - DMA I2C Slave TX. */
#define DMAALTCLR_I2CSTX_BBA           (*(volatile unsigned long *) 0x42200690)
#define DMAALTCLR_I2CSTX_MSK           (0x1   << 4  )
#define DMAALTCLR_I2CSTX               (0x1   << 4  )
#define DMAALTCLR_I2CSTX_DIS           (0x0   << 4  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_I2CSTX_EN            (0x1   << 4  ) /* EN. Selects the primary data structure for I2CSTX. */

/* DMAALTCLR[UARTRX] - DMA UART RX. */
#define DMAALTCLR_UARTRX_BBA           (*(volatile unsigned long *) 0x4220068C)
#define DMAALTCLR_UARTRX_MSK           (0x1   << 3  )
#define DMAALTCLR_UARTRX               (0x1   << 3  )
#define DMAALTCLR_UARTRX_DIS           (0x0   << 3  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_UARTRX_EN            (0x1   << 3  ) /* EN. Selects the primary data structure for UARTRX. */

/* DMAALTCLR[UARTTX] - DMA UART TX. */
#define DMAALTCLR_UARTTX_BBA           (*(volatile unsigned long *) 0x42200688)
#define DMAALTCLR_UARTTX_MSK           (0x1   << 2  )
#define DMAALTCLR_UARTTX               (0x1   << 2  )
#define DMAALTCLR_UARTTX_DIS           (0x0   << 2  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_UARTTX_EN            (0x1   << 2  ) /* EN. Selects the primary data structure for UARTTX. */

/* DMAALTCLR[SPI1RX] - DMA SPI 1 RX. */
#define DMAALTCLR_SPI1RX_BBA           (*(volatile unsigned long *) 0x42200684)
#define DMAALTCLR_SPI1RX_MSK           (0x1   << 1  )
#define DMAALTCLR_SPI1RX               (0x1   << 1  )
#define DMAALTCLR_SPI1RX_DIS           (0x0   << 1  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_SPI1RX_EN            (0x1   << 1  ) /* EN. Selects the primary data structure for SPI1RX. */

/* DMAALTCLR[SPI1TX] - DMA SPI 1 TX. */
#define DMAALTCLR_SPI1TX_BBA           (*(volatile unsigned long *) 0x42200680)
#define DMAALTCLR_SPI1TX_MSK           (0x1   << 0  )
#define DMAALTCLR_SPI1TX               (0x1   << 0  )
#define DMAALTCLR_SPI1TX_DIS           (0x0   << 0  ) /* DIS. No effect. Use the DMAALTSET register to select the alternate data structure. */
#define DMAALTCLR_SPI1TX_EN            (0x1   << 0  ) /* EN. Selects the primary data structure for SPI1TX. */

/* Reset Value for DMAPRISET*/
#define DMAPRISET_RVAL                 0x0

/* DMAPRISET[SPI0RX] - DMA SPI0 RX. */
#define DMAPRISET_SPI0RX_BBA           (*(volatile unsigned long *) 0x42200734)
#define DMAPRISET_SPI0RX_MSK           (0x1   << 13 )
#define DMAPRISET_SPI0RX               (0x1   << 13 )
#define DMAPRISET_SPI0RX_DIS           (0x0   << 13 ) /* DIS. When read: DMA SPI0RX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set SPI0RX to the default priority level. */
#define DMAPRISET_SPI0RX_EN            (0x1   << 13 ) /* EN. When read: DMA SPI0RX is using a high priority level. When written: SPI0RX uses the high priority level. */

/* DMAPRISET[SPI0TX] - DMA SPI0 TX. */
#define DMAPRISET_SPI0TX_BBA           (*(volatile unsigned long *) 0x42200730)
#define DMAPRISET_SPI0TX_MSK           (0x1   << 12 )
#define DMAPRISET_SPI0TX               (0x1   << 12 )
#define DMAPRISET_SPI0TX_DIS           (0x0   << 12 ) /* DIS. When read: DMA SPI0TX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set SPI0TX to the default priority level. */
#define DMAPRISET_SPI0TX_EN            (0x1   << 12 ) /* EN. When read: DMA SPI0TX is using a high priority level. When written: SPI0TX uses the high priority level. */

/* DMAPRISET[ADC] - DMA ADC. */
#define DMAPRISET_ADC_BBA              (*(volatile unsigned long *) 0x4220072C)
#define DMAPRISET_ADC_MSK              (0x1   << 11 )
#define DMAPRISET_ADC                  (0x1   << 11 )
#define DMAPRISET_ADC_DIS              (0x0   << 11 ) /* DIS. When read: DMA ADC is using the default priority level. When written: No effect. Use the DMAPRICLR register to set ADC to the default priority level. */
#define DMAPRISET_ADC_EN               (0x1   << 11 ) /* EN. When read: DMA ADCs using a high priority level. When written: ADC uses the high priority level. */

/* DMAPRISET[I2CMRX] - DMA I2C Master RX. */
#define DMAPRISET_I2CMRX_BBA           (*(volatile unsigned long *) 0x4220071C)
#define DMAPRISET_I2CMRX_MSK           (0x1   << 7  )
#define DMAPRISET_I2CMRX               (0x1   << 7  )
#define DMAPRISET_I2CMRX_DIS           (0x0   << 7  ) /* DIS. When read: DMA I2CMRX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set I2CMRX to the default priority level. */
#define DMAPRISET_I2CMRX_EN            (0x1   << 7  ) /* EN. When read: DMA I2CMRX is using a high priority level. When written: I2CMRX uses the high priority level. */

/* DMAPRISET[I2CMTX] - DMA I2C Master TX. */
#define DMAPRISET_I2CMTX_BBA           (*(volatile unsigned long *) 0x42200718)
#define DMAPRISET_I2CMTX_MSK           (0x1   << 6  )
#define DMAPRISET_I2CMTX               (0x1   << 6  )
#define DMAPRISET_I2CMTX_DIS           (0x0   << 6  ) /* DIS. When read: DMA I2CMTX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set I2CMTX to the default priority level. */
#define DMAPRISET_I2CMTX_EN            (0x1   << 6  ) /* EN. When read: DMA I2CMTX is using a high priority level. When written: I2CMTX uses the high priority level. */

/* DMAPRISET[I2CSRX] - DMA I2C Slave RX. */
#define DMAPRISET_I2CSRX_BBA           (*(volatile unsigned long *) 0x42200714)
#define DMAPRISET_I2CSRX_MSK           (0x1   << 5  )
#define DMAPRISET_I2CSRX               (0x1   << 5  )
#define DMAPRISET_I2CSRX_DIS           (0x0   << 5  ) /* DIS. When read: DMA I2CSRX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set I2CSRX to the default priority level. */
#define DMAPRISET_I2CSRX_EN            (0x1   << 5  ) /* EN. When read: DMA I2CSRX is using a high priority level. When written: I2CSRX uses the high priority level. */

/* DMAPRISET[I2CSTX] - DMA I2C Slave TX. */
#define DMAPRISET_I2CSTX_BBA           (*(volatile unsigned long *) 0x42200710)
#define DMAPRISET_I2CSTX_MSK           (0x1   << 4  )
#define DMAPRISET_I2CSTX               (0x1   << 4  )
#define DMAPRISET_I2CSTX_DIS           (0x0   << 4  ) /* DIS. When read: DMA I2CSTX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set I2CSTX to the default priority level. */
#define DMAPRISET_I2CSTX_EN            (0x1   << 4  ) /* EN. When read: DMA I2CSTX is using a high priority level. When written: I2CSTX uses the high priority level. */

/* DMAPRISET[UARTRX] - DMA UART RX. */
#define DMAPRISET_UARTRX_BBA           (*(volatile unsigned long *) 0x4220070C)
#define DMAPRISET_UARTRX_MSK           (0x1   << 3  )
#define DMAPRISET_UARTRX               (0x1   << 3  )
#define DMAPRISET_UARTRX_DIS           (0x0   << 3  ) /* DIS. When read: DMA UARTRX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set UARTRX to the default priority level. */
#define DMAPRISET_UARTRX_EN            (0x1   << 3  ) /* EN. When read: DMA UARTRX is using a high priority level. When written: UARTRX uses the high priority level. */

/* DMAPRISET[UARTTX] - DMA UART TX. */
#define DMAPRISET_UARTTX_BBA           (*(volatile unsigned long *) 0x42200708)
#define DMAPRISET_UARTTX_MSK           (0x1   << 2  )
#define DMAPRISET_UARTTX               (0x1   << 2  )
#define DMAPRISET_UARTTX_DIS           (0x0   << 2  ) /* DIS. When read: DMA UARTTX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set UARTTX to the default priority level. */
#define DMAPRISET_UARTTX_EN            (0x1   << 2  ) /* EN. When read: DMA UARTTX is using a high priority level. When written: UARTTX uses the high priority level. */

/* DMAPRISET[SPI1RX] - DMA SPI 1 RX. */
#define DMAPRISET_SPI1RX_BBA           (*(volatile unsigned long *) 0x42200704)
#define DMAPRISET_SPI1RX_MSK           (0x1   << 1  )
#define DMAPRISET_SPI1RX               (0x1   << 1  )
#define DMAPRISET_SPI1RX_DIS           (0x0   << 1  ) /* DIS. When read: DMA SPI1RX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set SPI1RX to the default priority level. */
#define DMAPRISET_SPI1RX_EN            (0x1   << 1  ) /* EN. When read: DMA SPI1RX is using a high priority level. When written: SPI1RX uses the high priority level. */

/* DMAPRISET[SPI1TX] - DMA SPI 1 TX. */
#define DMAPRISET_SPI1TX_BBA           (*(volatile unsigned long *) 0x42200700)
#define DMAPRISET_SPI1TX_MSK           (0x1   << 0  )
#define DMAPRISET_SPI1TX               (0x1   << 0  )
#define DMAPRISET_SPI1TX_DIS           (0x0   << 0  ) /* DIS. When read: DMA SPI1TX is using the default priority level. When written: No effect. Use the DMAPRICLR register to set SPI1TX to the default priority level. */
#define DMAPRISET_SPI1TX_EN            (0x1   << 0  ) /* EN. When read: DMA SPI1TX is using a high priority level. When written: SPI1TX uses the high priority level. */

/* Reset Value for DMAPRICLR*/
#define DMAPRICLR_RVAL                 0x0

/* DMAPRICLR[SPI0RX] - DMA SPI0 RX. */
#define DMAPRICLR_SPI0RX_BBA           (*(volatile unsigned long *) 0x422007B4)
#define DMAPRICLR_SPI0RX_MSK           (0x1   << 13 )
#define DMAPRICLR_SPI0RX               (0x1   << 13 )
#define DMAPRICLR_SPI0RX_DIS           (0x0   << 13 ) /* DIS. No effect. Use the DMAPRISET register to set SPI0RX to the high priority level. */
#define DMAPRICLR_SPI0RX_EN            (0x1   << 13 ) /* EN. SPI0RX uses the default priority level. */

/* DMAPRICLR[SPI0TX] - DMA SPI0 TX. */
#define DMAPRICLR_SPI0TX_BBA           (*(volatile unsigned long *) 0x422007B0)
#define DMAPRICLR_SPI0TX_MSK           (0x1   << 12 )
#define DMAPRICLR_SPI0TX               (0x1   << 12 )
#define DMAPRICLR_SPI0TX_DIS           (0x0   << 12 ) /* DIS. No effect. Use the DMAPRISET register to set SPI0TX to the high priority level. */
#define DMAPRICLR_SPI0TX_EN            (0x1   << 12 ) /* EN. SPI0TX uses the default priority level. */

/* DMAPRICLR[ADC] - DMA ADC. */
#define DMAPRICLR_ADC_BBA              (*(volatile unsigned long *) 0x422007AC)
#define DMAPRICLR_ADC_MSK              (0x1   << 11 )
#define DMAPRICLR_ADC                  (0x1   << 11 )
#define DMAPRICLR_ADC_DIS              (0x0   << 11 ) /* DIS. No effect. Use the DMAPRISET register to set ADC to the high priority level. */
#define DMAPRICLR_ADC_EN               (0x1   << 11 ) /* EN. ADC uses the default priority level. */

/* DMAPRICLR[I2CMRX] - DMA I2C Master RX. */
#define DMAPRICLR_I2CMRX_BBA           (*(volatile unsigned long *) 0x4220079C)
#define DMAPRICLR_I2CMRX_MSK           (0x1   << 7  )
#define DMAPRICLR_I2CMRX               (0x1   << 7  )
#define DMAPRICLR_I2CMRX_DIS           (0x0   << 7  ) /* DIS. No effect. Use the DMAPRISET register to set I2CMRX to the high priority level. */
#define DMAPRICLR_I2CMRX_EN            (0x1   << 7  ) /* EN. I2CMRX uses the default priority level. */

/* DMAPRICLR[I2CMTX] - DMA I2C Master TX. */
#define DMAPRICLR_I2CMTX_BBA           (*(volatile unsigned long *) 0x42200798)
#define DMAPRICLR_I2CMTX_MSK           (0x1   << 6  )
#define DMAPRICLR_I2CMTX               (0x1   << 6  )
#define DMAPRICLR_I2CMTX_DIS           (0x0   << 6  ) /* DIS. No effect. Use the DMAPRISET register to set I2CMTX to the high priority level. */
#define DMAPRICLR_I2CMTX_EN            (0x1   << 6  ) /* EN. I2CMTX uses the default priority level. */

/* DMAPRICLR[I2CSRX] - DMA I2C Slave RX. */
#define DMAPRICLR_I2CSRX_BBA           (*(volatile unsigned long *) 0x42200794)
#define DMAPRICLR_I2CSRX_MSK           (0x1   << 5  )
#define DMAPRICLR_I2CSRX               (0x1   << 5  )
#define DMAPRICLR_I2CSRX_DIS           (0x0   << 5  ) /* DIS. No effect. Use the DMAPRISET register to set I2CSRX to the high priority level. */
#define DMAPRICLR_I2CSRX_EN            (0x1   << 5  ) /* EN. I2CSRX uses the default priority level. */

/* DMAPRICLR[I2CSTX] - DMA I2C Slave TX. */
#define DMAPRICLR_I2CSTX_BBA           (*(volatile unsigned long *) 0x42200790)
#define DMAPRICLR_I2CSTX_MSK           (0x1   << 4  )
#define DMAPRICLR_I2CSTX               (0x1   << 4  )
#define DMAPRICLR_I2CSTX_DIS           (0x0   << 4  ) /* DIS. No effect. Use the DMAPRISET register to set I2CSTX to the high priority level. */
#define DMAPRICLR_I2CSTX_EN            (0x1   << 4  ) /* EN. I2CSTX uses the default priority level. */

/* DMAPRICLR[UARTRX] - DMA UART RX. */
#define DMAPRICLR_UARTRX_BBA           (*(volatile unsigned long *) 0x4220078C)
#define DMAPRICLR_UARTRX_MSK           (0x1   << 3  )
#define DMAPRICLR_UARTRX               (0x1   << 3  )
#define DMAPRICLR_UARTRX_DIS           (0x0   << 3  ) /* DIS. No effect. Use the DMAPRISET register to set UARTRX to the high priority level. */
#define DMAPRICLR_UARTRX_EN            (0x1   << 3  ) /* EN. UARTRX uses the default priority level. */

/* DMAPRICLR[UARTTX] - DMA UART TX. */
#define DMAPRICLR_UARTTX_BBA           (*(volatile unsigned long *) 0x42200788)
#define DMAPRICLR_UARTTX_MSK           (0x1   << 2  )
#define DMAPRICLR_UARTTX               (0x1   << 2  )
#define DMAPRICLR_UARTTX_DIS           (0x0   << 2  ) /* DIS. No effect. Use the DMAPRISET register to set UARTTX to the high priority level. */
#define DMAPRICLR_UARTTX_EN            (0x1   << 2  ) /* EN. UARTTX uses the default priority level. */

/* DMAPRICLR[SPI1RX] - DMA SPI 1 RX. */
#define DMAPRICLR_SPI1RX_BBA           (*(volatile unsigned long *) 0x42200784)
#define DMAPRICLR_SPI1RX_MSK           (0x1   << 1  )
#define DMAPRICLR_SPI1RX               (0x1   << 1  )
#define DMAPRICLR_SPI1RX_DIS           (0x0   << 1  ) /* DIS. No effect. Use the DMAPRISET register to set SPI1RX to the high priority level. */
#define DMAPRICLR_SPI1RX_EN            (0x1   << 1  ) /* EN. SPI1RX uses the default priority level. */

/* DMAPRICLR[SPI1TX] - DMA SPI 1 TX. */
#define DMAPRICLR_SPI1TX_BBA           (*(volatile unsigned long *) 0x42200780)
#define DMAPRICLR_SPI1TX_MSK           (0x1   << 0  )
#define DMAPRICLR_SPI1TX               (0x1   << 0  )
#define DMAPRICLR_SPI1TX_DIS           (0x0   << 0  ) /* DIS. No effect. Use the DMAPRISET register to set SPI1TX to the high priority level. */
#define DMAPRICLR_SPI1TX_EN            (0x1   << 0  ) /* EN. SPI1TX uses the default priority level. */

/* Reset Value for DMAERRCLR*/
#define DMAERRCLR_RVAL                 0x0

/* DMAERRCLR[ERROR] - DMA Bus Error status. */
#define DMAERRCLR_ERROR_BBA            (*(volatile unsigned long *) 0x42200980)
#define DMAERRCLR_ERROR_MSK            (0x1   << 0  )
#define DMAERRCLR_ERROR                (0x1   << 0  )
#define DMAERRCLR_ERROR_DIS            (0x0   << 0  ) /* DIS. When Read: No bus error occurred. When Written: No effect. */
#define DMAERRCLR_ERROR_EN             (0x1   << 0  ) /* EN. When Read: A bus error is pending. When Written: Bit is cleared. */
// ------------------------------------------------------------------------------------------------
// -----                                        FEE                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Flash Controller (pADI_FEE)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_FEE Structure                     */
  __IO uint16_t  FEESTA;                    /*!< Status Register                       */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  FEECON0;                   /*!< Command Control Register              */
  __I  uint16_t  RESERVED1;
  __IO uint16_t  FEECMD;                    /*!< Command Register                      */
  __I  uint16_t  RESERVED2[3];
  __IO uint16_t  FEEADR0L;                  /*!< Address 0  LSB                        */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  FEEADR0H;                  /*!< Address 0 MSB                         */
  __I  uint16_t  RESERVED4;
  __IO uint16_t  FEEADR1L;                  /*!< Address1 LSB                          */
  __I  uint16_t  RESERVED5;
  __IO uint16_t  FEEADR1H;                  /*!< Address1 MSB                          */
  __I  uint16_t  RESERVED6;
  __IO uint16_t  FEEKEY;                    /*!< Key Register                          */
  __I  uint16_t  RESERVED7[3];
  __IO uint16_t  FEEPROL;                   /*!< Write Protection Register LSB         */
  __I  uint16_t  RESERVED8;
  __IO uint16_t  FEEPROH;                   /*!< Write Protection Register MSB         */
  __I  uint16_t  RESERVED9;
  __IO uint16_t  FEESIGL;                   /*!< Signature LSB                         */
  __I  uint16_t  RESERVED10;
  __IO uint16_t  FEESIGH;                   /*!< Signature MSB                         */
  __I  uint16_t  RESERVED11;
  __IO uint16_t  FEECON1;                   /*!< User Setup Register                   */
  __I  uint16_t  RESERVED12[7];
  __IO uint16_t  FEEADRAL;                  /*!< Abort Address Register LSB            */
  __I  uint16_t  RESERVED13;
  __IO uint16_t  FEEADRAH;                  /*!< Abort Address Register MSB            */
  __I  uint16_t  RESERVED14[21];
  __IO uint16_t  FEEAEN0;                   /*!< Interrupt Abort Register (Interrupt 15 to Interrupt 0) */
  __I  uint16_t  RESERVED15;
  __IO uint16_t  FEEAEN1;                   /*!< Interrupt Abort Register (Interrupt 31 to Interrupt 16) */
  __I  uint16_t  RESERVED16;
  __IO uint16_t  FEEAEN2;                   /*!< Interrupt Abort Register (Interrupt 42 to Interrupt 32) */
} ADI_FEE_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          FEESTA                                     (*(volatile unsigned short int *) 0x40002800)
#define          FEECON0                                    (*(volatile unsigned short int *) 0x40002804)
#define          FEECMD                                     (*(volatile unsigned short int *) 0x40002808)
#define          FEEADR0L                                   (*(volatile unsigned short int *) 0x40002810)
#define          FEEADR0H                                   (*(volatile unsigned short int *) 0x40002814)
#define          FEEADR1L                                   (*(volatile unsigned short int *) 0x40002818)
#define          FEEADR1H                                   (*(volatile unsigned short int *) 0x4000281C)
#define          FEEKEY                                     (*(volatile unsigned short int *) 0x40002820)
#define          FEEPROL                                    (*(volatile unsigned short int *) 0x40002828)
#define          FEEPROH                                    (*(volatile unsigned short int *) 0x4000282C)
#define          FEESIGL                                    (*(volatile unsigned short int *) 0x40002830)
#define          FEESIGH                                    (*(volatile unsigned short int *) 0x40002834)
#define          FEECON1                                    (*(volatile unsigned short int *) 0x40002838)
#define          FEEADRAL                                   (*(volatile unsigned short int *) 0x40002848)
#define          FEEADRAH                                   (*(volatile unsigned short int *) 0x4000284C)
#define          FEEAEN0                                    (*(volatile unsigned short int *) 0x40002878)
#define          FEEAEN1                                    (*(volatile unsigned short int *) 0x4000287C)
#define          FEEAEN2                                    (*(volatile unsigned short int *) 0x40002880)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for FEESTA*/
#define FEESTA_RVAL                    0x0

/* FEESTA[SIGNERR] - Kernel space signature check on reset error */
#define FEESTA_SIGNERR_BBA             (*(volatile unsigned long *) 0x42050018)
#define FEESTA_SIGNERR_MSK             (0x1   << 6  )
#define FEESTA_SIGNERR                 (0x1   << 6  )
#define FEESTA_SIGNERR_CLR             (0x0   << 6  ) /* CLR. Cleared, if the signature check of the kernel passes. */
#define FEESTA_SIGNERR_SET             (0x1   << 6  ) /* SET. Set, if the signature check of the kernel fails. User code does not execute. */

/* FEESTA[CMDRES] - These two bits indicate the status of a command on completion or the status of a write. If multiple commands are executed or there are multiple writes via the AHB bus without a read of the status register, then the first error encountered is stored. */
#define FEESTA_CMDRES_MSK              (0x3   << 4  )
#define FEESTA_CMDRES_SUCCESS          (0x0   << 4  ) /* SUCCESS. Indicates a successful completion of a command or a write. Also cleared after a read of FEESTA. */
#define FEESTA_CMDRES_PROTECTED        (0x1   << 4  ) /* PROTECTED. Indicates an attempted erase of a protected location. */
#define FEESTA_CMDRES_VERIFYERR        (0x2   << 4  ) /* VERIFYERR. Indicates a read verify error. After an erase the controller reads the corresponding word(s) to verify that the transaction completed successfully. If data read is not all 'F's this is the resulting status. If the Sign command is executed and the resulting signature does not match the data in the upper 4 bytes of the upper page in a block then this is the resulting status. */
#define FEESTA_CMDRES_ABORT            (0x3   << 4  ) /* ABORT. Indicates that a command or a write was aborted by an abort command or a system interrupt has caused an abort. */

/* FEESTA[WRDONE] - Write complete. */
#define FEESTA_WRDONE_BBA              (*(volatile unsigned long *) 0x4205000C)
#define FEESTA_WRDONE_MSK              (0x1   << 3  )
#define FEESTA_WRDONE                  (0x1   << 3  )
#define FEESTA_WRDONE_CLR              (0x0   << 3  ) /* CLR. Cleared after a read of FEESTA. */
#define FEESTA_WRDONE_SET              (0x1   << 3  ) /* SET. Set when a write completes. If there are multiple writes or a burst write, this status bit asserts after the first long word written and stays asserted until read. If there is a burst write to flash, then this bit asserts after every long word written, assuming that user code read FEESTA after every long word written. */

/* FEESTA[CMDDONE] - Command complete. */
#define FEESTA_CMDDONE_BBA             (*(volatile unsigned long *) 0x42050008)
#define FEESTA_CMDDONE_MSK             (0x1   << 2  )
#define FEESTA_CMDDONE                 (0x1   << 2  )
#define FEESTA_CMDDONE_CLR             (0x0   << 2  ) /* CLR. Cleared after a read of FEESTA. */
#define FEESTA_CMDDONE_SET             (0x1   << 2  ) /* SET. Set when a command completes. If there are multiple commands, this status bit asserts after the first command completes and stays asserted until read. */

/* FEESTA[WRBUSY] - Write busy. */
#define FEESTA_WRBUSY_BBA              (*(volatile unsigned long *) 0x42050004)
#define FEESTA_WRBUSY_MSK              (0x1   << 1  )
#define FEESTA_WRBUSY                  (0x1   << 1  )
#define FEESTA_WRBUSY_CLR              (0x0   << 1  ) /* CLR. Cleared after a read of FEESTA. */
#define FEESTA_WRBUSY_SET              (0x1   << 1  ) /* SET. Set when the flash block is executing a write. */

/* FEESTA[CMDBUSY] - Command busy. */
#define FEESTA_CMDBUSY_BBA             (*(volatile unsigned long *) 0x42050000)
#define FEESTA_CMDBUSY_MSK             (0x1   << 0  )
#define FEESTA_CMDBUSY                 (0x1   << 0  )
#define FEESTA_CMDBUSY_CLR             (0x0   << 0  ) /* CLR. Cleared after a read of FEESTA. */
#define FEESTA_CMDBUSY_SET             (0x1   << 0  ) /* SET. Set when the flash block is executing any command entered via the command register. */

/* Reset Value for FEECON0*/
#define FEECON0_RVAL                   0x0

/* FEECON0[WREN] - Write enable bit. */
#define FEECON0_WREN_BBA               (*(volatile unsigned long *) 0x42050088)
#define FEECON0_WREN_MSK               (0x1   << 2  )
#define FEECON0_WREN                   (0x1   << 2  )
#define FEECON0_WREN_DIS               (0x0   << 2  ) /* DIS. Disables Flash writes. A flash write when this bit is 0 results in a hard fault system exception error and the write does not take place. */
#define FEECON0_WREN_EN                (0x1   << 2  ) /* EN. Enables Flash writes. */

/* FEECON0[IENERR] - Error interrupt enable bit. */
#define FEECON0_IENERR_BBA             (*(volatile unsigned long *) 0x42050084)
#define FEECON0_IENERR_MSK             (0x1   << 1  )
#define FEECON0_IENERR                 (0x1   << 1  )
#define FEECON0_IENERR_DIS             (0x0   << 1  ) /* DIS. Disables the Flash error interrupt. */
#define FEECON0_IENERR_EN              (0x1   << 1  ) /* EN. An interrupt is generated when a command or flash write completes with an error status. */

/* FEECON0[IENCMD] - Command complete interrupt enable bit. */
#define FEECON0_IENCMD_BBA             (*(volatile unsigned long *) 0x42050080)
#define FEECON0_IENCMD_MSK             (0x1   << 0  )
#define FEECON0_IENCMD                 (0x1   << 0  )
#define FEECON0_IENCMD_DIS             (0x0   << 0  ) /* DIS. Disables the Flash command complete interrupt. */
#define FEECON0_IENCMD_EN              (0x1   << 0  ) /* EN. An interrupt is generated when a command or flash write completes. */

/* Reset Value for FEECMD*/
#define FEECMD_RVAL                    0x0

/* FEECMD[CMD] - Commands supported by the flash controller. */
#define FEECMD_CMD_MSK                 (0xF   << 0  )
#define FEECMD_CMD_IDLE                (0x0   << 0  ) /* IDLE. No command executed. */
#define FEECMD_CMD_ERASEPAGE           (0x1   << 0  ) /* ERASEPAGE. Write the address of the page to be erased to FEEADR0L/H, then write this code to the FEECMD register and the flash will erase the page. When the erase has completed, the flash reads every location in the page to verify that all words in the page are erased. If there is a read verify error, this is indicated in FEESTA. To erase multiple pages, wait until a previous page erase has completed. Check the status, and then issue a command to start the next page erase. Before entering this command, 0xF456 followed by 0xF123 must be written to the key register. */
#define FEECMD_CMD_SIGN                (0x2   << 0  ) /* SIGN. Use this command to generate a signature for a block of data. The signature is generated on a page-by-page basis. To generate a signature, the address of the first page of the block is entered in FEEADR0L/FEEADR0H. The address of the last page is written to FEEADR1L/FEEADR1H. Then write this code to the FEECMD register. When the command has completed, the signature is available for reading in FEESIGL/FEESIGH. The last four bytes of the last page in a block is reserved for storing the signature. Before entering this command, 0xF456 followed 0xF123 must be written to the key register. */
#define FEECMD_CMD_MASSERASE           (0x3   << 0  ) /* MASSERASE. Erase all of user space. To enable this operation, 0xF456 followed by 0xF123 must first be written to FEEKEY (this is to prevent accidental erases). When the mass erase has completed, the controller reads every location to verify that all locations are 0xFFFFFFFF. If there is a read verify error this is indicated in FEESTA. */
#define FEECMD_CMD_ABORT               (0x4   << 0  ) /* ABORT. If this command is issued, then any command currently in progress is stopped. The status indicates command completed with an error status in FEESTA[5:4]. Note that this is the only command that can be issued while another command is already in progress. This command can also be used to stop a write that may be in progress. If a write is aborted, the address of the location being written can be read via the FEEADRAL/FEEADRAH register. While the flash controller is writing one longword, another longword write may be in the pipeline from the Cortex-M3 or DMA engine (depending on how the software implements writes). Therefore, both writes may need to be aborted. If a write or erase is aborted, then the flash timing is violated and it is not possible to determine if the write or erase completed successfully. To enable this operation, 0xF456 followed by 0xF123 must first be written to FEEKEY (this is to prevent accidental aborts). */

/* Reset Value for FEEADR0L*/
#define FEEADR0L_RVAL                  0x0

/* FEEADR0L[VALUE] - Used in conjunction with FEEADR0H, to indicate the page to be erased, or the start of a section to be signed. The address of a memory location inside the page should be stored in FEEADR0L/H, bits[15:0] in FEEADR0L, and bits[17:16] in FEEADR0H. The 9 LSBs of the address are ignored. */
#define FEEADR0L_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for FEEADR0H*/
#define FEEADR0H_RVAL                  0x0

/* FEEADR0H[VALUE] - Used in conjunction with FEEADR0L, to indicate the page to be erased, or the start of a section to be signed. The address of a memory location inside the page should be stored in FEEADR0L/H, bits[15:0] in FEEADR0L, and bits[17:16] in FEEADR0H. */
#define FEEADR0H_VALUE_MSK             (0x3   << 0  )

/* Reset Value for FEEADR1L*/
#define FEEADR1L_RVAL                  0x0

/* FEEADR1L[VALUE] - Used in conjunction with FEEADR1H, to identify the last page used by the Sign command. The address of a memory location inside the page should be stored in FEEADR1L/H, bits[15:0] in FEEADR1L, and bits[17:16] in FEEADR1H. The 9 LSBs of the address are ignored. */
#define FEEADR1L_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for FEEADR1H*/
#define FEEADR1H_RVAL                  0x0

/* FEEADR1H[VALUE] - Used in conjunction with FEEADR1L, to identify the last page used by the Sign command. The address of a memory location inside the page should be stored in FEEADR1L/H, bits[15:0] in FEEADR1L, and bits[17:16] in FEEADR1H. */
#define FEEADR1H_VALUE_MSK             (0x3   << 0  )

/* Reset Value for FEEKEY*/
#define FEEKEY_RVAL                    0x0

/* FEEKEY[VALUE] - Enter 0xF456 followed by 0xF123. Returns 0x0 if read. */
#define FEEKEY_VALUE_MSK               (0xFFFF << 0  )
#define FEEKEY_VALUE_USERKEY1          (0xF456 << 0  ) /* USERKEY1                 */
#define FEEKEY_VALUE_USERKEY2          (0xF123 << 0  ) /* USERKEY2                 */

/* Reset Value for FEEPROL*/
#define FEEPROL_RVAL                   0xFFFF

/* FEEPROL[VALUE] - Lower 16 bits of the write protection. This register is read only if the write protection in flash has been programmed, i.e. FEEPROH/L have previously been configured to protect pages. */
#define FEEPROL_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for FEEPROH*/
#define FEEPROH_RVAL                   0xFFFF

/* FEEPROH[VALUE] - Upper 16 bits of the write protection. This register is read only if the write protection in flash has been programmed, i.e. FEEPROH/L have previously been configured to protect pages. */
#define FEEPROH_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for FEESIGL*/
#define FEESIGL_RVAL                   0xFFFF

/* FEESIGL[VALUE] - Lower 16 bits of the signature. Signature[15:0]. */
#define FEESIGL_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for FEESIGH*/
#define FEESIGH_RVAL                   0xFFFF

/* FEESIGH[VALUE] - Upper eight bits of the signature. Signature[23:16]. */
#define FEESIGH_VALUE_MSK              (0xFF  << 0  )

/* Reset Value for FEECON1*/
#define FEECON1_RVAL                   0x1

/* FEECON1[DBG] - Serial Wire debug enable. */
#define FEECON1_DBG_BBA                (*(volatile unsigned long *) 0x42050700)
#define FEECON1_DBG_MSK                (0x1   << 0  )
#define FEECON1_DBG                    (0x1   << 0  )
#define FEECON1_DBG_DIS                (0x0   << 0  ) /* DIS. Disable access via the serial wire debug interface. */
#define FEECON1_DBG_EN                 (0x1   << 0  ) /* EN. Enable access via the serial wire debug interface. */

/* Reset Value for FEEADRAL*/
#define FEEADRAL_RVAL                  0x800

/* FEEADRAL[VALUE] - Lower 16 bits of the FEEADRA register. If a write is aborted then this will contain the address of the location been written when the write was aborted. */
#define FEEADRAL_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for FEEADRAH*/
#define FEEADRAH_RVAL                  0x2

/* FEEADRAH[VALUE] - Upper 16 bits of the FEEADRA register. */
#define FEEADRAH_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for FEEAEN0*/
#define FEEAEN0_RVAL                   0x0

/* FEEAEN0[FEE] - Flash controller interrupt abort enable bit */
#define FEEAEN0_FEE_BBA                (*(volatile unsigned long *) 0x42050F3C)
#define FEEAEN0_FEE_MSK                (0x1   << 15 )
#define FEEAEN0_FEE                    (0x1   << 15 )
#define FEEAEN0_FEE_DIS                (0x0   << 15 ) /* DIS. Flash controller interrupt abort disabled. */
#define FEEAEN0_FEE_EN                 (0x1   << 15 ) /* EN. Flash controller interrupt abort enabled. */

/* FEEAEN0[ADC] - ADC interrupt abort enable bit */
#define FEEAEN0_ADC_BBA                (*(volatile unsigned long *) 0x42050F38)
#define FEEAEN0_ADC_MSK                (0x1   << 14 )
#define FEEAEN0_ADC                    (0x1   << 14 )
#define FEEAEN0_ADC_DIS                (0x0   << 14 ) /* DIS. ADC interrupt abort disabled. */
#define FEEAEN0_ADC_EN                 (0x1   << 14 ) /* EN. ADC interrupt abort enabled. */

/* FEEAEN0[T1] - Timer1 interrupt abort enable bit */
#define FEEAEN0_T1_BBA                 (*(volatile unsigned long *) 0x42050F34)
#define FEEAEN0_T1_MSK                 (0x1   << 13 )
#define FEEAEN0_T1                     (0x1   << 13 )
#define FEEAEN0_T1_DIS                 (0x0   << 13 ) /* DIS. Timer1 interrupt abort disabled. */
#define FEEAEN0_T1_EN                  (0x1   << 13 ) /* EN. Timer1 interrupt abort enabled. */

/* FEEAEN0[T0] - Timer0 interrupt abort enable bit */
#define FEEAEN0_T0_BBA                 (*(volatile unsigned long *) 0x42050F30)
#define FEEAEN0_T0_MSK                 (0x1   << 12 )
#define FEEAEN0_T0                     (0x1   << 12 )
#define FEEAEN0_T0_DIS                 (0x0   << 12 ) /* DIS. Timer0 interrupt abort disabled. */
#define FEEAEN0_T0_EN                  (0x1   << 12 ) /* EN. Timer0 interrupt abort enabled. */

/* FEEAEN0[T3] - Timer3 interrupt abort enable bit */
#define FEEAEN0_T3_BBA                 (*(volatile unsigned long *) 0x42050F28)
#define FEEAEN0_T3_MSK                 (0x1   << 10 )
#define FEEAEN0_T3                     (0x1   << 10 )
#define FEEAEN0_T3_DIS                 (0x0   << 10 ) /* DIS. Timer3 interrupt abort disabled. */
#define FEEAEN0_T3_EN                  (0x1   << 10 ) /* EN. Timer3 interrupt abort enabled. */

/* FEEAEN0[EXTINT8] - External interrupt 8 abort enable bit */
#define FEEAEN0_EXTINT8_BBA            (*(volatile unsigned long *) 0x42050F24)
#define FEEAEN0_EXTINT8_MSK            (0x1   << 9  )
#define FEEAEN0_EXTINT8                (0x1   << 9  )
#define FEEAEN0_EXTINT8_DIS            (0x0   << 9  ) /* DIS. External interrupt 8 abort disabled. */
#define FEEAEN0_EXTINT8_EN             (0x1   << 9  ) /* EN. External interrupt 8 abort enabled. */

/* FEEAEN0[EXTINT7] - External interrupt 7 abort enable bit */
#define FEEAEN0_EXTINT7_BBA            (*(volatile unsigned long *) 0x42050F20)
#define FEEAEN0_EXTINT7_MSK            (0x1   << 8  )
#define FEEAEN0_EXTINT7                (0x1   << 8  )
#define FEEAEN0_EXTINT7_DIS            (0x0   << 8  ) /* DIS. External interrupt 7 abort disabled. */
#define FEEAEN0_EXTINT7_EN             (0x1   << 8  ) /* EN. External interrupt 7 abort enabled. */

/* FEEAEN0[EXTINT6] - External interrupt 6 abort enable bit */
#define FEEAEN0_EXTINT6_BBA            (*(volatile unsigned long *) 0x42050F1C)
#define FEEAEN0_EXTINT6_MSK            (0x1   << 7  )
#define FEEAEN0_EXTINT6                (0x1   << 7  )
#define FEEAEN0_EXTINT6_DIS            (0x0   << 7  ) /* DIS. External interrupt 6 abort disabled. */
#define FEEAEN0_EXTINT6_EN             (0x1   << 7  ) /* EN. External interrupt 6 abort enabled. */

/* FEEAEN0[EXTINT5] - External interrupt 5 abort enable bit */
#define FEEAEN0_EXTINT5_BBA            (*(volatile unsigned long *) 0x42050F18)
#define FEEAEN0_EXTINT5_MSK            (0x1   << 6  )
#define FEEAEN0_EXTINT5                (0x1   << 6  )
#define FEEAEN0_EXTINT5_DIS            (0x0   << 6  ) /* DIS. External interrupt 5 abort disabled. */
#define FEEAEN0_EXTINT5_EN             (0x1   << 6  ) /* EN. External interrupt 5 abort enabled. */

/* FEEAEN0[EXTINT4] - External interrupt 4 abort enable bit */
#define FEEAEN0_EXTINT4_BBA            (*(volatile unsigned long *) 0x42050F14)
#define FEEAEN0_EXTINT4_MSK            (0x1   << 5  )
#define FEEAEN0_EXTINT4                (0x1   << 5  )
#define FEEAEN0_EXTINT4_DIS            (0x0   << 5  ) /* DIS. External interrupt 4 abort disabled. */
#define FEEAEN0_EXTINT4_EN             (0x1   << 5  ) /* EN. External interrupt 4 abort enabled. */

/* FEEAEN0[EXTINT3] - External interrupt 3 abort enable bit */
#define FEEAEN0_EXTINT3_BBA            (*(volatile unsigned long *) 0x42050F10)
#define FEEAEN0_EXTINT3_MSK            (0x1   << 4  )
#define FEEAEN0_EXTINT3                (0x1   << 4  )
#define FEEAEN0_EXTINT3_DIS            (0x0   << 4  ) /* DIS. External interrupt 3 abort disabled. */
#define FEEAEN0_EXTINT3_EN             (0x1   << 4  ) /* EN. External interrupt 3 abort enabled. */

/* FEEAEN0[EXTINT2] - External interrupt 2 abort enable bit */
#define FEEAEN0_EXTINT2_BBA            (*(volatile unsigned long *) 0x42050F0C)
#define FEEAEN0_EXTINT2_MSK            (0x1   << 3  )
#define FEEAEN0_EXTINT2                (0x1   << 3  )
#define FEEAEN0_EXTINT2_DIS            (0x0   << 3  ) /* DIS. External interrupt 2 abort disabled. */
#define FEEAEN0_EXTINT2_EN             (0x1   << 3  ) /* EN. External interrupt 2 abort enabled. */

/* FEEAEN0[EXTINT1] - External interrupt 1 abort enable bit */
#define FEEAEN0_EXTINT1_BBA            (*(volatile unsigned long *) 0x42050F08)
#define FEEAEN0_EXTINT1_MSK            (0x1   << 2  )
#define FEEAEN0_EXTINT1                (0x1   << 2  )
#define FEEAEN0_EXTINT1_DIS            (0x0   << 2  ) /* DIS. External interrupt 1 abort disabled. */
#define FEEAEN0_EXTINT1_EN             (0x1   << 2  ) /* EN. External interrupt 1 abort enabled. */

/* FEEAEN0[EXTINT0] - External interrupt 0 abort enable bit */
#define FEEAEN0_EXTINT0_BBA            (*(volatile unsigned long *) 0x42050F04)
#define FEEAEN0_EXTINT0_MSK            (0x1   << 1  )
#define FEEAEN0_EXTINT0                (0x1   << 1  )
#define FEEAEN0_EXTINT0_DIS            (0x0   << 1  ) /* DIS. External interrupt 0 abort disabled. */
#define FEEAEN0_EXTINT0_EN             (0x1   << 1  ) /* EN. External interrupt 0 abort enabled. */

/* FEEAEN0[T2] - Timer2 interrupt abort enable bit */
#define FEEAEN0_T2_BBA                 (*(volatile unsigned long *) 0x42050F00)
#define FEEAEN0_T2_MSK                 (0x1   << 0  )
#define FEEAEN0_T2                     (0x1   << 0  )
#define FEEAEN0_T2_DIS                 (0x0   << 0  ) /* DIS. Timer2 interrupt abort disabled. */
#define FEEAEN0_T2_EN                  (0x1   << 0  ) /* EN. Timer2 interrupt abort enabled */

/* Reset Value for FEEAEN1*/
#define FEEAEN1_RVAL                   0x0

/* FEEAEN1[DMAI2CMRX] - I2C master RX DMA interrupt abort enable bit */
#define FEEAEN1_DMAI2CMRX_BBA          (*(volatile unsigned long *) 0x42050FBC)
#define FEEAEN1_DMAI2CMRX_MSK          (0x1   << 15 )
#define FEEAEN1_DMAI2CMRX              (0x1   << 15 )
#define FEEAEN1_DMAI2CMRX_DIS          (0x0   << 15 ) /* DIS. I2C master RX DMA interrupt abort disabled. */
#define FEEAEN1_DMAI2CMRX_EN           (0x1   << 15 ) /* EN. I2C master RX DMA interrupt abort enabled. */

/* FEEAEN1[DMAI2CMTX] - I2C master TX DMA interrupt abort enable bit */
#define FEEAEN1_DMAI2CMTX_BBA          (*(volatile unsigned long *) 0x42050FB8)
#define FEEAEN1_DMAI2CMTX_MSK          (0x1   << 14 )
#define FEEAEN1_DMAI2CMTX              (0x1   << 14 )
#define FEEAEN1_DMAI2CMTX_DIS          (0x0   << 14 ) /* DIS. I2C master TX DMA interrupt abort disabled. */
#define FEEAEN1_DMAI2CMTX_EN           (0x1   << 14 ) /* EN. I2C master TX DMA interrupt abort enabled. */

/* FEEAEN1[DMAI2CSRX] - I2C slave RX DMA interrupt abort enable bit */
#define FEEAEN1_DMAI2CSRX_BBA          (*(volatile unsigned long *) 0x42050FB4)
#define FEEAEN1_DMAI2CSRX_MSK          (0x1   << 13 )
#define FEEAEN1_DMAI2CSRX              (0x1   << 13 )
#define FEEAEN1_DMAI2CSRX_DIS          (0x0   << 13 ) /* DIS. I2C slave RX DMA interrupt abort disabled. */
#define FEEAEN1_DMAI2CSRX_EN           (0x1   << 13 ) /* EN. I2C slave RX DMA interrupt abort enabled. */

/* FEEAEN1[DMAI2CSTX] - I2C slave TX DMA interrupt abort enable bit */
#define FEEAEN1_DMAI2CSTX_BBA          (*(volatile unsigned long *) 0x42050FB0)
#define FEEAEN1_DMAI2CSTX_MSK          (0x1   << 12 )
#define FEEAEN1_DMAI2CSTX              (0x1   << 12 )
#define FEEAEN1_DMAI2CSTX_DIS          (0x0   << 12 ) /* DIS. I2C slave TX DMA interrupt abort disabled. */
#define FEEAEN1_DMAI2CSTX_EN           (0x1   << 12 ) /* EN. I2C slave TX DMA interrupt abort enabled. */

/* FEEAEN1[DMAUARTRX] - UARTRX DMA interrupt abort enable bit */
#define FEEAEN1_DMAUARTRX_BBA          (*(volatile unsigned long *) 0x42050FAC)
#define FEEAEN1_DMAUARTRX_MSK          (0x1   << 11 )
#define FEEAEN1_DMAUARTRX              (0x1   << 11 )
#define FEEAEN1_DMAUARTRX_DIS          (0x0   << 11 ) /* DIS. UARTRX DMA interrupt abort disabled. */
#define FEEAEN1_DMAUARTRX_EN           (0x1   << 11 ) /* EN. UARTRX DMA interrupt abort enabled. */

/* FEEAEN1[DMAUARTTX] - UARTTX DMA interrupt abort enable bit */
#define FEEAEN1_DMAUARTTX_BBA          (*(volatile unsigned long *) 0x42050FA8)
#define FEEAEN1_DMAUARTTX_MSK          (0x1   << 10 )
#define FEEAEN1_DMAUARTTX              (0x1   << 10 )
#define FEEAEN1_DMAUARTTX_DIS          (0x0   << 10 ) /* DIS. UARTTX DMA interrupt abort disabled. */
#define FEEAEN1_DMAUARTTX_EN           (0x1   << 10 ) /* EN. UARTTX DMA interrupt abort enabled. */

/* FEEAEN1[DMASPI1RX] - SPI1RX DMA interrupt abort enable bit */
#define FEEAEN1_DMASPI1RX_BBA          (*(volatile unsigned long *) 0x42050FA4)
#define FEEAEN1_DMASPI1RX_MSK          (0x1   << 9  )
#define FEEAEN1_DMASPI1RX              (0x1   << 9  )
#define FEEAEN1_DMASPI1RX_DIS          (0x0   << 9  ) /* DIS. SPI1RX DMA interrupt abort disabled. */
#define FEEAEN1_DMASPI1RX_EN           (0x1   << 9  ) /* EN. SPI1RX DMA interrupt abort enabled. */

/* FEEAEN1[DMASPI1TX] - SPI1TX DMA interrupt abort enable bit */
#define FEEAEN1_DMASPI1TX_BBA          (*(volatile unsigned long *) 0x42050FA0)
#define FEEAEN1_DMASPI1TX_MSK          (0x1   << 8  )
#define FEEAEN1_DMASPI1TX              (0x1   << 8  )
#define FEEAEN1_DMASPI1TX_DIS          (0x0   << 8  ) /* DIS. SPI1TX DMA interrupt abort disabled. */
#define FEEAEN1_DMASPI1TX_EN           (0x1   << 8  ) /* EN. SPI1TX DMA interrupt abort enabled. */

/* FEEAEN1[DMAERROR] - DMA error interrupt abort enable bit */
#define FEEAEN1_DMAERROR_BBA           (*(volatile unsigned long *) 0x42050F9C)
#define FEEAEN1_DMAERROR_MSK           (0x1   << 7  )
#define FEEAEN1_DMAERROR               (0x1   << 7  )
#define FEEAEN1_DMAERROR_DIS           (0x0   << 7  ) /* DIS. DMA error interrupt abort disabled. */
#define FEEAEN1_DMAERROR_EN            (0x1   << 7  ) /* EN. DMA error interrupt abort enabled. */

/* FEEAEN1[I2CM] - I2C master interrupt abort enable bit */
#define FEEAEN1_I2CM_BBA               (*(volatile unsigned long *) 0x42050F90)
#define FEEAEN1_I2CM_MSK               (0x1   << 4  )
#define FEEAEN1_I2CM                   (0x1   << 4  )
#define FEEAEN1_I2CM_DIS               (0x0   << 4  ) /* DIS. I2C slave interrupt abort disabled. */
#define FEEAEN1_I2CM_EN                (0x1   << 4  ) /* EN. I2C master interrupt abort enabled. */

/* FEEAEN1[I2CS] - I2C slave interrupt abort enable bit */
#define FEEAEN1_I2CS_BBA               (*(volatile unsigned long *) 0x42050F8C)
#define FEEAEN1_I2CS_MSK               (0x1   << 3  )
#define FEEAEN1_I2CS                   (0x1   << 3  )
#define FEEAEN1_I2CS_DIS               (0x0   << 3  ) /* DIS. I2C slave interrupt abort disabled. */
#define FEEAEN1_I2CS_EN                (0x1   << 3  ) /* EN. I2C slave interrupt abort enabled. */

/* FEEAEN1[SPI1] - SPI1 interrupt abort enable bit */
#define FEEAEN1_SPI1_BBA               (*(volatile unsigned long *) 0x42050F88)
#define FEEAEN1_SPI1_MSK               (0x1   << 2  )
#define FEEAEN1_SPI1                   (0x1   << 2  )
#define FEEAEN1_SPI1_DIS               (0x0   << 2  ) /* DIS. SPI1 interrupt abort disabled. */
#define FEEAEN1_SPI1_EN                (0x1   << 2  ) /* EN. SPI1 interrupt abort enabled. */

/* FEEAEN1[SPI0] - SPI0 interrupt abort enable bit */
#define FEEAEN1_SPI0_BBA               (*(volatile unsigned long *) 0x42050F84)
#define FEEAEN1_SPI0_MSK               (0x1   << 1  )
#define FEEAEN1_SPI0                   (0x1   << 1  )
#define FEEAEN1_SPI0_DIS               (0x0   << 1  ) /* DIS. SPI0 interrupt abort disabled. */
#define FEEAEN1_SPI0_EN                (0x1   << 1  ) /* EN. SPI0 interrupt abort enabled. */

/* FEEAEN1[UART] - UART interrupt abort enable bit */
#define FEEAEN1_UART_BBA               (*(volatile unsigned long *) 0x42050F80)
#define FEEAEN1_UART_MSK               (0x1   << 0  )
#define FEEAEN1_UART                   (0x1   << 0  )
#define FEEAEN1_UART_DIS               (0x0   << 0  ) /* DIS. UART interrupt abort disabled. */
#define FEEAEN1_UART_EN                (0x1   << 0  ) /* EN. UART interrupt abort enabled. */

/* Reset Value for FEEAEN2*/
#define FEEAEN2_RVAL                   0x0

/* FEEAEN2[PWM3] - PWM3 interrupt abort enable bit */
#define FEEAEN2_PWM3_BBA               (*(volatile unsigned long *) 0x42051028)
#define FEEAEN2_PWM3_MSK               (0x1   << 10 )
#define FEEAEN2_PWM3                   (0x1   << 10 )
#define FEEAEN2_PWM3_DIS               (0x0   << 10 ) /* DIS.  PWM3 interrupt abort disabled. */
#define FEEAEN2_PWM3_EN                (0x1   << 10 ) /* EN. PWM3 interrupt abort enabled. */

/* FEEAEN2[PWM2] - PWM2 interrupt abort enable bit */
#define FEEAEN2_PWM2_BBA               (*(volatile unsigned long *) 0x42051024)
#define FEEAEN2_PWM2_MSK               (0x1   << 9  )
#define FEEAEN2_PWM2                   (0x1   << 9  )
#define FEEAEN2_PWM2_DIS               (0x0   << 9  ) /* DIS.  PWM2 interrupt abort disabled. */
#define FEEAEN2_PWM2_EN                (0x1   << 9  ) /* EN. PWM2 interrupt abort enabled. */

/* FEEAEN2[PWM1] - PWM1 interrupt abort enable bit */
#define FEEAEN2_PWM1_BBA               (*(volatile unsigned long *) 0x42051020)
#define FEEAEN2_PWM1_MSK               (0x1   << 8  )
#define FEEAEN2_PWM1                   (0x1   << 8  )
#define FEEAEN2_PWM1_DIS               (0x0   << 8  ) /* DIS.  PWM1 interrupt abort disabled. */
#define FEEAEN2_PWM1_EN                (0x1   << 8  ) /* EN. PWM1 interrupt abort enabled. */

/* FEEAEN2[PWM0] - PWM0 interrupt abort enable bit */
#define FEEAEN2_PWM0_BBA               (*(volatile unsigned long *) 0x4205101C)
#define FEEAEN2_PWM0_MSK               (0x1   << 7  )
#define FEEAEN2_PWM0                   (0x1   << 7  )
#define FEEAEN2_PWM0_DIS               (0x0   << 7  ) /* DIS.  PWM0 interrupt abort disabled. */
#define FEEAEN2_PWM0_EN                (0x1   << 7  ) /* EN. PWM0 interrupt abort enabled. */

/* FEEAEN2[PWMTRIP] - PWMTRIP interrupt abort enable bit */
#define FEEAEN2_PWMTRIP_BBA            (*(volatile unsigned long *) 0x42051018)
#define FEEAEN2_PWMTRIP_MSK            (0x1   << 6  )
#define FEEAEN2_PWMTRIP                (0x1   << 6  )
#define FEEAEN2_PWMTRIP_DIS            (0x0   << 6  ) /* DIS. PWMTRIP interrupt abort disabled. */
#define FEEAEN2_PWMTRIP_EN             (0x1   << 6  ) /* EN. PWMTRIP interrupt abort enabled. */

/* FEEAEN2[DMASPI0RX] - SPI0RX DMA interrupt abort enable bit */
#define FEEAEN2_DMASPI0RX_BBA          (*(volatile unsigned long *) 0x42051014)
#define FEEAEN2_DMASPI0RX_MSK          (0x1   << 5  )
#define FEEAEN2_DMASPI0RX              (0x1   << 5  )
#define FEEAEN2_DMASPI0RX_DIS          (0x0   << 5  ) /* DIS. SPI0RX DMA interrupt abort disabled. */
#define FEEAEN2_DMASPI0RX_EN           (0x1   << 5  ) /* EN. SPI0RX DMA interrupt abort enabled. */

/* FEEAEN2[DMASPI0TX] - SPI0TX DMA interrupt abort enable bit */
#define FEEAEN2_DMASPI0TX_BBA          (*(volatile unsigned long *) 0x42051010)
#define FEEAEN2_DMASPI0TX_MSK          (0x1   << 4  )
#define FEEAEN2_DMASPI0TX              (0x1   << 4  )
#define FEEAEN2_DMASPI0TX_DIS          (0x0   << 4  ) /* DIS. SPI0TX DMA interrupt abort disabled. */
#define FEEAEN2_DMASPI0TX_EN           (0x1   << 4  ) /* EN. SPI0TX DMA interrupt abort enabled. */

/* FEEAEN2[DMAADC] - ADC DMA interrupt abort enable bit */
#define FEEAEN2_DMAADC_BBA             (*(volatile unsigned long *) 0x4205100C)
#define FEEAEN2_DMAADC_MSK             (0x1   << 3  )
#define FEEAEN2_DMAADC                 (0x1   << 3  )
#define FEEAEN2_DMAADC_DIS             (0x0   << 3  ) /* DIS. ADC DMA interrupt abort disabled. */
#define FEEAEN2_DMAADC_EN              (0x1   << 3  ) /* EN. ADC DMA interrupt abort enabled. */
// ------------------------------------------------------------------------------------------------
// -----                                        GPIO0                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief General Purpose Input Output (pADI_GP0)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_GP0 Structure                     */
  __IO uint16_t  GPCON;                     /*!< GPIO Port 0 Configuration             */
  __I  uint16_t  RESERVED0;
  __IO uint8_t   GPOEN;                     /*!< GPIO Port 0 Output Enable             */
  __I  uint8_t   RESERVED1[3];
  __IO uint8_t   GPPUL;                     /*!< GPIO Port 0 Pull Up Enable            */
  __I  uint8_t   RESERVED2[3];
  __IO uint8_t   GPOCE;                     /*!< GPIO Port 0 Tri State                 */
  __I  uint8_t   RESERVED3[7];
  __IO uint8_t   GPIN;                      /*!< GPIO Port 0 Data Input                */
  __I  uint8_t   RESERVED4[3];
  __IO uint8_t   GPOUT;                     /*!< GPIO Port 0 Data Out                  */
  __I  uint8_t   RESERVED5[3];
  __IO uint8_t   GPSET;                     /*!< GPIO Port 0 Data Out Set              */
  __I  uint8_t   RESERVED6[3];
  __IO uint8_t   GPCLR;                     /*!< GPIO Port 0 Data Out Clear            */
  __I  uint8_t   RESERVED7[3];
  __IO uint8_t   GPTGL;                     /*!< GPIO Port 0 Pin Toggle                */
} ADI_GPIO_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          GP0CON                                     (*(volatile unsigned short int *) 0x40006000)
#define          GP0OEN                                     (*(volatile unsigned char      *) 0x40006004)
#define          GP0PUL                                     (*(volatile unsigned char      *) 0x40006008)
#define          GP0OCE                                     (*(volatile unsigned char      *) 0x4000600C)
#define          GP0IN                                      (*(volatile unsigned char      *) 0x40006014)
#define          GP0OUT                                     (*(volatile unsigned char      *) 0x40006018)
#define          GP0SET                                     (*(volatile unsigned char      *) 0x4000601C)
#define          GP0CLR                                     (*(volatile unsigned char      *) 0x40006020)
#define          GP0TGL                                     (*(volatile unsigned char      *) 0x40006024)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for GP0CON*/
#define GP0CON_RVAL                    0x0

/* GP0CON[CON7] - Configuration bits for Px.7 (not available for port 1). */
#define GP0CON_CON7_MSK                (0x3   << 14 )
#define GP0CON_CON7_GPIOIRQ3           (0x0   << 14 ) /* GPIOIRQ3. GPIO/IRQ3.     */
#define GP0CON_CON7_SPI1CS4            (0x1   << 14 ) /* SPI1CS4. SPI1 CS4 (SPI1). */
#define GP0CON_CON7_UARTCTS            (0x2   << 14 ) /* UARTCTS. UART CTS.       */

/* GP0CON[CON6] - Configuration bits for Px.6 (not available for port 1). */
#define GP0CON_CON6_MSK                (0x3   << 12 )
#define GP0CON_CON6_GPIOIRQ2           (0x0   << 12 ) /* GPIOIRQ2. GPIO/IRQ2.     */
#define GP0CON_CON6_SPI1CS3            (0x1   << 12 ) /* SPI1CS3. SPI1 CS3 (SPI1). */
#define GP0CON_CON6_UARTRTS            (0x2   << 12 ) /* UARTRTS. UART RTS.       */
#define GP0CON_CON6_PWM0               (0x3   << 12 ) /* PWM0. PWM0.              */

/* GP0CON[CON5] - Configuration bits for Px.5. */
#define GP0CON_CON5_MSK                (0x3   << 10 )
#define GP0CON_CON5_GPIO               (0x0   << 10 ) /* GPIO. GPIO.              */
#define GP0CON_CON5_SPI1CS2            (0x1   << 10 ) /* SPI1CS2. SPI1 CS2 (SPI1). */
#define GP0CON_CON5_ECLKIN             (0x2   << 10 ) /* ECLKIN. ECLKIN.          */

/* GP0CON[CON4] - Configuration bits for Px.4. */
#define GP0CON_CON4_MSK                (0x3   << 8  )
#define GP0CON_CON4_GPIO               (0x0   << 8  ) /* GPIO. GPIO               */
#define GP0CON_CON4_SPI1CS1            (0x1   << 8  ) /* SPI1CS1. SPI1 CS1 (SPI1). */
#define GP0CON_CON4_ECLKOUT            (0x2   << 8  ) /* ECLKOUT. ECLK OUT.       */

/* GP0CON[CON3] - Configuration bits for Px.3. */
#define GP0CON_CON3_MSK                (0x3   << 6  )
#define GP0CON_CON3_GPIOIRQ1           (0x0   << 6  ) /* GPIOIRQ1. GPIO/IRQ1.     */
#define GP0CON_CON3_SPI1CS0            (0x1   << 6  ) /* SPI1CS0. SPI1 CS0  (SPI1). */
#define GP0CON_CON3_ADCCONVST          (0x2   << 6  ) /* ADCCONVST. ADCCONVST.    */
#define GP0CON_CON3_PWM1               (0x3   << 6  ) /* PWM1. PWM1.              */

/* GP0CON[CON2] - Configuration bits for Px.2. */
#define GP0CON_CON2_MSK                (0x3   << 4  )
#define GP0CON_CON2_GPIO               (0x0   << 4  ) /* GPIO. GPIO               */
#define GP0CON_CON2_SPI1MOSI           (0x1   << 4  ) /* SPI1MOSI. SPI MOSI (SPI1). */
#define GP0CON_CON2_PWM0               (0x3   << 4  ) /* PWM0. PWM0               */

/* GP0CON[CON1] - Configuration bits for Px.1. */
#define GP0CON_CON1_MSK                (0x3   << 2  )
#define GP0CON_CON1_GPIO               (0x0   << 2  ) /* GPIO. GPIO.              */
#define GP0CON_CON1_SPI1SCLK           (0x1   << 2  ) /* SPI1SCLK. SPI SCLK (SPI1). */

/* GP0CON[CON0] - Configuration bits for Px.0. */
#define GP0CON_CON0_MSK                (0x3   << 0  )
#define GP0CON_CON0_GPIO               (0x0   << 0  ) /* GPIO. GPIO               */
#define GP0CON_CON0_SPI1MISO           (0x1   << 0  ) /* SPI1MISO. SPI MISO (SPI1) */

/* Reset Value for GP0OEN*/
#define GP0OEN_RVAL                    0x0

/* GP0OEN[OEN7] - Port pin direction. */
#define GP0OEN_OEN7_BBA                (*(volatile unsigned long *) 0x420C009C)
#define GP0OEN_OEN7_MSK                (0x1   << 7  )
#define GP0OEN_OEN7                    (0x1   << 7  )
#define GP0OEN_OEN7_IN                 (0x0   << 7  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN7_OUT                (0x1   << 7  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN6] - Port pin direction. */
#define GP0OEN_OEN6_BBA                (*(volatile unsigned long *) 0x420C0098)
#define GP0OEN_OEN6_MSK                (0x1   << 6  )
#define GP0OEN_OEN6                    (0x1   << 6  )
#define GP0OEN_OEN6_IN                 (0x0   << 6  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN6_OUT                (0x1   << 6  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN5] - Port pin direction. */
#define GP0OEN_OEN5_BBA                (*(volatile unsigned long *) 0x420C0094)
#define GP0OEN_OEN5_MSK                (0x1   << 5  )
#define GP0OEN_OEN5                    (0x1   << 5  )
#define GP0OEN_OEN5_IN                 (0x0   << 5  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN5_OUT                (0x1   << 5  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN4] - Port pin direction. */
#define GP0OEN_OEN4_BBA                (*(volatile unsigned long *) 0x420C0090)
#define GP0OEN_OEN4_MSK                (0x1   << 4  )
#define GP0OEN_OEN4                    (0x1   << 4  )
#define GP0OEN_OEN4_IN                 (0x0   << 4  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN4_OUT                (0x1   << 4  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN3] - Port pin direction. */
#define GP0OEN_OEN3_BBA                (*(volatile unsigned long *) 0x420C008C)
#define GP0OEN_OEN3_MSK                (0x1   << 3  )
#define GP0OEN_OEN3                    (0x1   << 3  )
#define GP0OEN_OEN3_IN                 (0x0   << 3  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN3_OUT                (0x1   << 3  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN2] - Port pin direction. */
#define GP0OEN_OEN2_BBA                (*(volatile unsigned long *) 0x420C0088)
#define GP0OEN_OEN2_MSK                (0x1   << 2  )
#define GP0OEN_OEN2                    (0x1   << 2  )
#define GP0OEN_OEN2_IN                 (0x0   << 2  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN2_OUT                (0x1   << 2  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN1] - Port pin direction. */
#define GP0OEN_OEN1_BBA                (*(volatile unsigned long *) 0x420C0084)
#define GP0OEN_OEN1_MSK                (0x1   << 1  )
#define GP0OEN_OEN1                    (0x1   << 1  )
#define GP0OEN_OEN1_IN                 (0x0   << 1  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN1_OUT                (0x1   << 1  ) /* OUT. Enables the output on corresponding port pin. */

/* GP0OEN[OEN0] - Port pin direction. */
#define GP0OEN_OEN0_BBA                (*(volatile unsigned long *) 0x420C0080)
#define GP0OEN_OEN0_MSK                (0x1   << 0  )
#define GP0OEN_OEN0                    (0x1   << 0  )
#define GP0OEN_OEN0_IN                 (0x0   << 0  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP0OEN_OEN0_OUT                (0x1   << 0  ) /* OUT. Enables the output on corresponding port pin.. */

/* Reset Value for GP0PUL*/
#define GP0PUL_RVAL                    0xFF

/* GP0PUL[PUL7] - Pull Up Enable for port pin. */
#define GP0PUL_PUL7_BBA                (*(volatile unsigned long *) 0x420C011C)
#define GP0PUL_PUL7_MSK                (0x1   << 7  )
#define GP0PUL_PUL7                    (0x1   << 7  )
#define GP0PUL_PUL7_DIS                (0x0   << 7  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL7_EN                 (0x1   << 7  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL6] - Pull Up Enable for port pin. */
#define GP0PUL_PUL6_BBA                (*(volatile unsigned long *) 0x420C0118)
#define GP0PUL_PUL6_MSK                (0x1   << 6  )
#define GP0PUL_PUL6                    (0x1   << 6  )
#define GP0PUL_PUL6_DIS                (0x0   << 6  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL6_EN                 (0x1   << 6  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL5] - Pull Up Enable for port pin. */
#define GP0PUL_PUL5_BBA                (*(volatile unsigned long *) 0x420C0114)
#define GP0PUL_PUL5_MSK                (0x1   << 5  )
#define GP0PUL_PUL5                    (0x1   << 5  )
#define GP0PUL_PUL5_DIS                (0x0   << 5  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL5_EN                 (0x1   << 5  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL4] - Pull Up Enable for port pin. */
#define GP0PUL_PUL4_BBA                (*(volatile unsigned long *) 0x420C0110)
#define GP0PUL_PUL4_MSK                (0x1   << 4  )
#define GP0PUL_PUL4                    (0x1   << 4  )
#define GP0PUL_PUL4_DIS                (0x0   << 4  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL4_EN                 (0x1   << 4  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL3] - Pull Up Enable for port pin. */
#define GP0PUL_PUL3_BBA                (*(volatile unsigned long *) 0x420C010C)
#define GP0PUL_PUL3_MSK                (0x1   << 3  )
#define GP0PUL_PUL3                    (0x1   << 3  )
#define GP0PUL_PUL3_DIS                (0x0   << 3  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL3_EN                 (0x1   << 3  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL2] - Pull Up Enable for port pin. */
#define GP0PUL_PUL2_BBA                (*(volatile unsigned long *) 0x420C0108)
#define GP0PUL_PUL2_MSK                (0x1   << 2  )
#define GP0PUL_PUL2                    (0x1   << 2  )
#define GP0PUL_PUL2_DIS                (0x0   << 2  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL2_EN                 (0x1   << 2  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL1] - Pull Up Enable for port pin. */
#define GP0PUL_PUL1_BBA                (*(volatile unsigned long *) 0x420C0104)
#define GP0PUL_PUL1_MSK                (0x1   << 1  )
#define GP0PUL_PUL1                    (0x1   << 1  )
#define GP0PUL_PUL1_DIS                (0x0   << 1  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL1_EN                 (0x1   << 1  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP0PUL[PUL0] - Pull Up Enable for port pin. */
#define GP0PUL_PUL0_BBA                (*(volatile unsigned long *) 0x420C0100)
#define GP0PUL_PUL0_MSK                (0x1   << 0  )
#define GP0PUL_PUL0                    (0x1   << 0  )
#define GP0PUL_PUL0_DIS                (0x0   << 0  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP0PUL_PUL0_EN                 (0x1   << 0  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* Reset Value for GP0OCE*/
#define GP0OCE_RVAL                    0x0

/* GP0OCE[OCE7] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP0OCE_OCE7_BBA                (*(volatile unsigned long *) 0x420C019C)
#define GP0OCE_OCE7_MSK                (0x1   << 7  )
#define GP0OCE_OCE7                    (0x1   << 7  )
#define GP0OCE_OCE7_DIS                (0x0   << 7  ) /* DIS                      */
#define GP0OCE_OCE7_EN                 (0x1   << 7  ) /* EN                       */

/* GP0OCE[OCE6] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP0OCE_OCE6_BBA                (*(volatile unsigned long *) 0x420C0198)
#define GP0OCE_OCE6_MSK                (0x1   << 6  )
#define GP0OCE_OCE6                    (0x1   << 6  )
#define GP0OCE_OCE6_DIS                (0x0   << 6  ) /* DIS                      */
#define GP0OCE_OCE6_EN                 (0x1   << 6  ) /* EN                       */

/* GP0OCE[OCE5] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP0OCE_OCE5_BBA                (*(volatile unsigned long *) 0x420C0194)
#define GP0OCE_OCE5_MSK                (0x1   << 5  )
#define GP0OCE_OCE5                    (0x1   << 5  )
#define GP0OCE_OCE5_DIS                (0x0   << 5  ) /* DIS                      */
#define GP0OCE_OCE5_EN                 (0x1   << 5  ) /* EN                       */

/* GP0OCE[OCE4] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP0OCE_OCE4_BBA                (*(volatile unsigned long *) 0x420C0190)
#define GP0OCE_OCE4_MSK                (0x1   << 4  )
#define GP0OCE_OCE4                    (0x1   << 4  )
#define GP0OCE_OCE4_DIS                (0x0   << 4  ) /* DIS                      */
#define GP0OCE_OCE4_EN                 (0x1   << 4  ) /* EN                       */

/* GP0OCE[OCE3] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP0OCE_OCE3_BBA                (*(volatile unsigned long *) 0x420C018C)
#define GP0OCE_OCE3_MSK                (0x1   << 3  )
#define GP0OCE_OCE3                    (0x1   << 3  )
#define GP0OCE_OCE3_DIS                (0x0   << 3  ) /* DIS                      */
#define GP0OCE_OCE3_EN                 (0x1   << 3  ) /* EN                       */

/* GP0OCE[OCE2] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP0OCE_OCE2_BBA                (*(volatile unsigned long *) 0x420C0188)
#define GP0OCE_OCE2_MSK                (0x1   << 2  )
#define GP0OCE_OCE2                    (0x1   << 2  )
#define GP0OCE_OCE2_DIS                (0x0   << 2  ) /* DIS                      */
#define GP0OCE_OCE2_EN                 (0x1   << 2  ) /* EN                       */

/* GP0OCE[OCE1] - Output enable. Sets the GPIO pads oncorresponding port to open circuit mode. */
#define GP0OCE_OCE1_BBA                (*(volatile unsigned long *) 0x420C0184)
#define GP0OCE_OCE1_MSK                (0x1   << 1  )
#define GP0OCE_OCE1                    (0x1   << 1  )
#define GP0OCE_OCE1_DIS                (0x0   << 1  ) /* DIS                      */
#define GP0OCE_OCE1_EN                 (0x1   << 1  ) /* EN                       */

/* GP0OCE[OCE0] - Output enable. Sets the GPIO pads on corresponding port to open circuit mode. */
#define GP0OCE_OCE0_BBA                (*(volatile unsigned long *) 0x420C0180)
#define GP0OCE_OCE0_MSK                (0x1   << 0  )
#define GP0OCE_OCE0                    (0x1   << 0  )
#define GP0OCE_OCE0_DIS                (0x0   << 0  ) /* DIS                      */
#define GP0OCE_OCE0_EN                 (0x1   << 0  ) /* EN                       */

/* Reset Value for GP0IN*/
#define GP0IN_RVAL                     0xFF

/* GP0IN[IN7] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN7_BBA                  (*(volatile unsigned long *) 0x420C029C)
#define GP0IN_IN7_MSK                  (0x1   << 7  )
#define GP0IN_IN7                      (0x1   << 7  )
#define GP0IN_IN7_LOW                  (0x0   << 7  ) /* LOW                      */
#define GP0IN_IN7_HIGH                 (0x1   << 7  ) /* HIGH                     */

/* GP0IN[IN6] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN6_BBA                  (*(volatile unsigned long *) 0x420C0298)
#define GP0IN_IN6_MSK                  (0x1   << 6  )
#define GP0IN_IN6                      (0x1   << 6  )
#define GP0IN_IN6_LOW                  (0x0   << 6  ) /* LOW                      */
#define GP0IN_IN6_HIGH                 (0x1   << 6  ) /* HIGH                     */

/* GP0IN[IN5] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN5_BBA                  (*(volatile unsigned long *) 0x420C0294)
#define GP0IN_IN5_MSK                  (0x1   << 5  )
#define GP0IN_IN5                      (0x1   << 5  )
#define GP0IN_IN5_LOW                  (0x0   << 5  ) /* LOW                      */
#define GP0IN_IN5_HIGH                 (0x1   << 5  ) /* HIGH                     */

/* GP0IN[IN4] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN4_BBA                  (*(volatile unsigned long *) 0x420C0290)
#define GP0IN_IN4_MSK                  (0x1   << 4  )
#define GP0IN_IN4                      (0x1   << 4  )
#define GP0IN_IN4_LOW                  (0x0   << 4  ) /* LOW                      */
#define GP0IN_IN4_HIGH                 (0x1   << 4  ) /* HIGH                     */

/* GP0IN[IN3] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN3_BBA                  (*(volatile unsigned long *) 0x420C028C)
#define GP0IN_IN3_MSK                  (0x1   << 3  )
#define GP0IN_IN3                      (0x1   << 3  )
#define GP0IN_IN3_LOW                  (0x0   << 3  ) /* LOW                      */
#define GP0IN_IN3_HIGH                 (0x1   << 3  ) /* HIGH                     */

/* GP0IN[IN2] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN2_BBA                  (*(volatile unsigned long *) 0x420C0288)
#define GP0IN_IN2_MSK                  (0x1   << 2  )
#define GP0IN_IN2                      (0x1   << 2  )
#define GP0IN_IN2_LOW                  (0x0   << 2  ) /* LOW                      */
#define GP0IN_IN2_HIGH                 (0x1   << 2  ) /* HIGH                     */

/* GP0IN[IN1] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN1_BBA                  (*(volatile unsigned long *) 0x420C0284)
#define GP0IN_IN1_MSK                  (0x1   << 1  )
#define GP0IN_IN1                      (0x1   << 1  )
#define GP0IN_IN1_LOW                  (0x0   << 1  ) /* LOW                      */
#define GP0IN_IN1_HIGH                 (0x1   << 1  ) /* HIGH                     */

/* GP0IN[IN0] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP0IN_IN0_BBA                  (*(volatile unsigned long *) 0x420C0280)
#define GP0IN_IN0_MSK                  (0x1   << 0  )
#define GP0IN_IN0                      (0x1   << 0  )
#define GP0IN_IN0_LOW                  (0x0   << 0  ) /* LOW                      */
#define GP0IN_IN0_HIGH                 (0x1   << 0  ) /* HIGH                     */

/* Reset Value for GP0OUT*/
#define GP0OUT_RVAL                    0x0

/* GP0OUT[OUT7] - Data out register. */
#define GP0OUT_OUT7_BBA                (*(volatile unsigned long *) 0x420C031C)
#define GP0OUT_OUT7_MSK                (0x1   << 7  )
#define GP0OUT_OUT7                    (0x1   << 7  )
#define GP0OUT_OUT7_LOW                (0x0   << 7  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT7_HIGH               (0x1   << 7  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT6] - Data out register. */
#define GP0OUT_OUT6_BBA                (*(volatile unsigned long *) 0x420C0318)
#define GP0OUT_OUT6_MSK                (0x1   << 6  )
#define GP0OUT_OUT6                    (0x1   << 6  )
#define GP0OUT_OUT6_LOW                (0x0   << 6  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT6_HIGH               (0x1   << 6  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT5] - Data out register. */
#define GP0OUT_OUT5_BBA                (*(volatile unsigned long *) 0x420C0314)
#define GP0OUT_OUT5_MSK                (0x1   << 5  )
#define GP0OUT_OUT5                    (0x1   << 5  )
#define GP0OUT_OUT5_LOW                (0x0   << 5  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT5_HIGH               (0x1   << 5  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT4] - Data out register. */
#define GP0OUT_OUT4_BBA                (*(volatile unsigned long *) 0x420C0310)
#define GP0OUT_OUT4_MSK                (0x1   << 4  )
#define GP0OUT_OUT4                    (0x1   << 4  )
#define GP0OUT_OUT4_LOW                (0x0   << 4  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT4_HIGH               (0x1   << 4  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT3] - Data out register. */
#define GP0OUT_OUT3_BBA                (*(volatile unsigned long *) 0x420C030C)
#define GP0OUT_OUT3_MSK                (0x1   << 3  )
#define GP0OUT_OUT3                    (0x1   << 3  )
#define GP0OUT_OUT3_LOW                (0x0   << 3  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT3_HIGH               (0x1   << 3  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT2] - Data out register. */
#define GP0OUT_OUT2_BBA                (*(volatile unsigned long *) 0x420C0308)
#define GP0OUT_OUT2_MSK                (0x1   << 2  )
#define GP0OUT_OUT2                    (0x1   << 2  )
#define GP0OUT_OUT2_LOW                (0x0   << 2  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT2_HIGH               (0x1   << 2  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT1] - Data out register. */
#define GP0OUT_OUT1_BBA                (*(volatile unsigned long *) 0x420C0304)
#define GP0OUT_OUT1_MSK                (0x1   << 1  )
#define GP0OUT_OUT1                    (0x1   << 1  )
#define GP0OUT_OUT1_LOW                (0x0   << 1  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT1_HIGH               (0x1   << 1  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP0OUT[OUT0] - Data out register. */
#define GP0OUT_OUT0_BBA                (*(volatile unsigned long *) 0x420C0300)
#define GP0OUT_OUT0_MSK                (0x1   << 0  )
#define GP0OUT_OUT0                    (0x1   << 0  )
#define GP0OUT_OUT0_LOW                (0x0   << 0  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP0OUT_OUT0_HIGH               (0x1   << 0  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP0SET*/
#define GP0SET_RVAL                    0x0

/* GP0SET[SET7] - Set output high for corresponding port pin. */
#define GP0SET_SET7_BBA                (*(volatile unsigned long *) 0x420C039C)
#define GP0SET_SET7_MSK                (0x1   << 7  )
#define GP0SET_SET7                    (0x1   << 7  )
#define GP0SET_SET7_SET                (0x1   << 7  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET6] - Set output high for corresponding port pin. */
#define GP0SET_SET6_BBA                (*(volatile unsigned long *) 0x420C0398)
#define GP0SET_SET6_MSK                (0x1   << 6  )
#define GP0SET_SET6                    (0x1   << 6  )
#define GP0SET_SET6_SET                (0x1   << 6  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET5] - Set output high for corresponding port pin. */
#define GP0SET_SET5_BBA                (*(volatile unsigned long *) 0x420C0394)
#define GP0SET_SET5_MSK                (0x1   << 5  )
#define GP0SET_SET5                    (0x1   << 5  )
#define GP0SET_SET5_SET                (0x1   << 5  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET4] - Set output high for corresponding port pin. */
#define GP0SET_SET4_BBA                (*(volatile unsigned long *) 0x420C0390)
#define GP0SET_SET4_MSK                (0x1   << 4  )
#define GP0SET_SET4                    (0x1   << 4  )
#define GP0SET_SET4_SET                (0x1   << 4  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET3] - Set output high for corresponding port pin. */
#define GP0SET_SET3_BBA                (*(volatile unsigned long *) 0x420C038C)
#define GP0SET_SET3_MSK                (0x1   << 3  )
#define GP0SET_SET3                    (0x1   << 3  )
#define GP0SET_SET3_SET                (0x1   << 3  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET2] - Set output high for corresponding port pin. */
#define GP0SET_SET2_BBA                (*(volatile unsigned long *) 0x420C0388)
#define GP0SET_SET2_MSK                (0x1   << 2  )
#define GP0SET_SET2                    (0x1   << 2  )
#define GP0SET_SET2_SET                (0x1   << 2  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET1] - Set output high for corresponding port pin. */
#define GP0SET_SET1_BBA                (*(volatile unsigned long *) 0x420C0384)
#define GP0SET_SET1_MSK                (0x1   << 1  )
#define GP0SET_SET1                    (0x1   << 1  )
#define GP0SET_SET1_SET                (0x1   << 1  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP0SET[SET0] - Set output high for corresponding port pin. */
#define GP0SET_SET0_BBA                (*(volatile unsigned long *) 0x420C0380)
#define GP0SET_SET0_MSK                (0x1   << 0  )
#define GP0SET_SET0                    (0x1   << 0  )
#define GP0SET_SET0_SET                (0x1   << 0  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP0CLR*/
#define GP0CLR_RVAL                    0x0

/* GP0CLR[CLR7] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR7_BBA                (*(volatile unsigned long *) 0x420C041C)
#define GP0CLR_CLR7_MSK                (0x1   << 7  )
#define GP0CLR_CLR7                    (0x1   << 7  )
#define GP0CLR_CLR7_CLR                (0x1   << 7  ) /* CLR                      */

/* GP0CLR[CLR6] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR6_BBA                (*(volatile unsigned long *) 0x420C0418)
#define GP0CLR_CLR6_MSK                (0x1   << 6  )
#define GP0CLR_CLR6                    (0x1   << 6  )
#define GP0CLR_CLR6_CLR                (0x1   << 6  ) /* CLR                      */

/* GP0CLR[CLR5] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR5_BBA                (*(volatile unsigned long *) 0x420C0414)
#define GP0CLR_CLR5_MSK                (0x1   << 5  )
#define GP0CLR_CLR5                    (0x1   << 5  )
#define GP0CLR_CLR5_CLR                (0x1   << 5  ) /* CLR                      */

/* GP0CLR[CLR4] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR4_BBA                (*(volatile unsigned long *) 0x420C0410)
#define GP0CLR_CLR4_MSK                (0x1   << 4  )
#define GP0CLR_CLR4                    (0x1   << 4  )
#define GP0CLR_CLR4_CLR                (0x1   << 4  ) /* CLR                      */

/* GP0CLR[CLR3] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR3_BBA                (*(volatile unsigned long *) 0x420C040C)
#define GP0CLR_CLR3_MSK                (0x1   << 3  )
#define GP0CLR_CLR3                    (0x1   << 3  )
#define GP0CLR_CLR3_CLR                (0x1   << 3  ) /* CLR                      */

/* GP0CLR[CLR2] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR2_BBA                (*(volatile unsigned long *) 0x420C0408)
#define GP0CLR_CLR2_MSK                (0x1   << 2  )
#define GP0CLR_CLR2                    (0x1   << 2  )
#define GP0CLR_CLR2_CLR                (0x1   << 2  ) /* CLR                      */

/* GP0CLR[CLR1] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR1_BBA                (*(volatile unsigned long *) 0x420C0404)
#define GP0CLR_CLR1_MSK                (0x1   << 1  )
#define GP0CLR_CLR1                    (0x1   << 1  )
#define GP0CLR_CLR1_CLR                (0x1   << 1  ) /* CLR                      */

/* GP0CLR[CLR0] - Set by user code to drive the corresponding GPIO low. */
#define GP0CLR_CLR0_BBA                (*(volatile unsigned long *) 0x420C0400)
#define GP0CLR_CLR0_MSK                (0x1   << 0  )
#define GP0CLR_CLR0                    (0x1   << 0  )
#define GP0CLR_CLR0_CLR                (0x1   << 0  ) /* CLR                      */

/* Reset Value for GP0TGL*/
#define GP0TGL_RVAL                    0x0

/* GP0TGL[TGL7] - Toggle for corresponding port pin. */
#define GP0TGL_TGL7_BBA                (*(volatile unsigned long *) 0x420C049C)
#define GP0TGL_TGL7_MSK                (0x1   << 7  )
#define GP0TGL_TGL7                    (0x1   << 7  )
#define GP0TGL_TGL7_TGL                (0x1   << 7  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL6] - Toggle for corresponding port pin. */
#define GP0TGL_TGL6_BBA                (*(volatile unsigned long *) 0x420C0498)
#define GP0TGL_TGL6_MSK                (0x1   << 6  )
#define GP0TGL_TGL6                    (0x1   << 6  )
#define GP0TGL_TGL6_TGL                (0x1   << 6  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL5] - Toggle for corresponding port pin. */
#define GP0TGL_TGL5_BBA                (*(volatile unsigned long *) 0x420C0494)
#define GP0TGL_TGL5_MSK                (0x1   << 5  )
#define GP0TGL_TGL5                    (0x1   << 5  )
#define GP0TGL_TGL5_TGL                (0x1   << 5  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL4] - Toggle for corresponding port pin. */
#define GP0TGL_TGL4_BBA                (*(volatile unsigned long *) 0x420C0490)
#define GP0TGL_TGL4_MSK                (0x1   << 4  )
#define GP0TGL_TGL4                    (0x1   << 4  )
#define GP0TGL_TGL4_TGL                (0x1   << 4  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL3] - Toggle for corresponding port pin. */
#define GP0TGL_TGL3_BBA                (*(volatile unsigned long *) 0x420C048C)
#define GP0TGL_TGL3_MSK                (0x1   << 3  )
#define GP0TGL_TGL3                    (0x1   << 3  )
#define GP0TGL_TGL3_TGL                (0x1   << 3  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL2] - Toggle for corresponding port pin. */
#define GP0TGL_TGL2_BBA                (*(volatile unsigned long *) 0x420C0488)
#define GP0TGL_TGL2_MSK                (0x1   << 2  )
#define GP0TGL_TGL2                    (0x1   << 2  )
#define GP0TGL_TGL2_TGL                (0x1   << 2  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL1] - Toggle for corresponding port pin. */
#define GP0TGL_TGL1_BBA                (*(volatile unsigned long *) 0x420C0484)
#define GP0TGL_TGL1_MSK                (0x1   << 1  )
#define GP0TGL_TGL1                    (0x1   << 1  )
#define GP0TGL_TGL1_TGL                (0x1   << 1  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP0TGL[TGL0] - Toggle for corresponding port pin. */
#define GP0TGL_TGL0_BBA                (*(volatile unsigned long *) 0x420C0480)
#define GP0TGL_TGL0_MSK                (0x1   << 0  )
#define GP0TGL_TGL0                    (0x1   << 0  )
#define GP0TGL_TGL0_TGL                (0x1   << 0  ) /* TGL. Set by user code to invert the corresponding GPIO. */
#if (__NO_MMR_STRUCTS__==1)

#define          GP1CON                                     (*(volatile unsigned short int *) 0x40006030)
#define          GP1OEN                                     (*(volatile unsigned char      *) 0x40006034)
#define          GP1PUL                                     (*(volatile unsigned char      *) 0x40006038)
#define          GP1OCE                                     (*(volatile unsigned char      *) 0x4000603C)
#define          GP1IN                                      (*(volatile unsigned char      *) 0x40006044)
#define          GP1OUT                                     (*(volatile unsigned char      *) 0x40006048)
#define          GP1SET                                     (*(volatile unsigned char      *) 0x4000604C)
#define          GP1CLR                                     (*(volatile unsigned char      *) 0x40006050)
#define          GP1TGL                                     (*(volatile unsigned char      *) 0x40006054)
#endif // (__NO_MMR_STRUCTS__==1)

/* Reset Value for GP1CON*/
#define GP1CON_RVAL                    0x0

/* GP1CON[CON6] - Configuration bits for P1.6 */
#define GP1CON_CON6_MSK                (0x3   << 12 )
#define GP1CON_CON6_GPIO               (0x0   << 12 ) /* GPIO                     */
#define GP1CON_CON6_ADCCONVST          (0x1   << 12 ) /* ADCCONVST                */
#define GP1CON_CON6_PWMSYNC            (0x3   << 12 ) /* PWMSYNC                  */

/* GP1CON[CON5] - Configuration bits for P1.5 */
#define GP1CON_CON5_MSK                (0x3   << 10 )
#define GP1CON_CON5_GPIOIRQ6           (0x0   << 10 ) /* GPIOIRQ6                 */
#define GP1CON_CON5_I2C0SDA            (0x1   << 10 ) /* I2C0SDA                  */
#define GP1CON_CON5_PWM7               (0x2   << 10 ) /* PWM7                     */

/* GP1CON[CON4] - Configuration bits for P1.4 */
#define GP1CON_CON4_MSK                (0x3   << 8  )
#define GP1CON_CON4_GPIOIRQ5           (0x0   << 8  ) /* GPIOIRQ5                 */
#define GP1CON_CON4_I2C0SCL            (0x1   << 8  ) /* I2C0SCL                  */
#define GP1CON_CON4_PWM6               (0x2   << 8  ) /* PWM6                     */

/* GP1CON[CON3] - Configuration bits for P1.3 */
#define GP1CON_CON3_MSK                (0x3   << 6  )
#define GP1CON_CON3_GPIO               (0x1   << 6  ) /* GPIO                     */
#define GP1CON_CON3_PWM5               (0x3   << 6  ) /* PWM5                     */

/* GP1CON[CON2] - Configuration bits for P1.2 */
#define GP1CON_CON2_MSK                (0x3   << 4  )
#define GP1CON_CON2_GPIO               (0x1   << 4  ) /* GPIO                     */
#define GP1CON_CON2_PWM4               (0x3   << 4  ) /* PWM4                     */

/* GP1CON[CON1] - Configuration bits for P1.1 */
#define GP1CON_CON1_MSK                (0x3   << 2  )
#define GP1CON_CON1_PORB               (0x0   << 2  ) /* PORB                     */
#define GP1CON_CON1_GPIO               (0x1   << 2  ) /* GPIO                     */
#define GP1CON_CON1_UART0TXD           (0x2   << 2  ) /* UART0TXD                 */
#define GP1CON_CON1_PWM3               (0x3   << 2  ) /* PWM3                     */

/* GP1CON[CON0] - Configuration bits for P1.0 */
#define GP1CON_CON0_MSK                (0x3   << 0  )
#define GP1CON_CON0_GPIOIRQ4           (0x0   << 0  ) /* GPIOIRQ4                 */
#define GP1CON_CON0_UART0RXD           (0x1   << 0  ) /* UART0RXD                 */
#define GP1CON_CON0_SPI1MOSI           (0x2   << 0  ) /* SPI1MOSI                 */
#define GP1CON_CON0_PWM2               (0x3   << 0  ) /* PWM2                     */

/* Reset Value for GP1OEN*/
#define GP1OEN_RVAL                    0x0

/* GP1OEN[OEN6] - Port pin direction. */
#define GP1OEN_OEN6_BBA                (*(volatile unsigned long *) 0x420C0698)
#define GP1OEN_OEN6_MSK                (0x1   << 6  )
#define GP1OEN_OEN6                    (0x1   << 6  )
#define GP1OEN_OEN6_IN                 (0x0   << 6  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN6_OUT                (0x1   << 6  ) /* OUT. Enables the output on corresponding port pin. */

/* GP1OEN[OEN5] - Port pin direction. */
#define GP1OEN_OEN5_BBA                (*(volatile unsigned long *) 0x420C0694)
#define GP1OEN_OEN5_MSK                (0x1   << 5  )
#define GP1OEN_OEN5                    (0x1   << 5  )
#define GP1OEN_OEN5_IN                 (0x0   << 5  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN5_OUT                (0x1   << 5  ) /* OUT. Enables the output on corresponding port pin. */

/* GP1OEN[OEN4] - Port pin direction. */
#define GP1OEN_OEN4_BBA                (*(volatile unsigned long *) 0x420C0690)
#define GP1OEN_OEN4_MSK                (0x1   << 4  )
#define GP1OEN_OEN4                    (0x1   << 4  )
#define GP1OEN_OEN4_IN                 (0x0   << 4  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN4_OUT                (0x1   << 4  ) /* OUT. Enables the output on corresponding port pin. */

/* GP1OEN[OEN3] - Port pin direction. */
#define GP1OEN_OEN3_BBA                (*(volatile unsigned long *) 0x420C068C)
#define GP1OEN_OEN3_MSK                (0x1   << 3  )
#define GP1OEN_OEN3                    (0x1   << 3  )
#define GP1OEN_OEN3_IN                 (0x0   << 3  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN3_OUT                (0x1   << 3  ) /* OUT. Enables the output on corresponding port pin. */

/* GP1OEN[OEN2] - Port pin direction. */
#define GP1OEN_OEN2_BBA                (*(volatile unsigned long *) 0x420C0688)
#define GP1OEN_OEN2_MSK                (0x1   << 2  )
#define GP1OEN_OEN2                    (0x1   << 2  )
#define GP1OEN_OEN2_IN                 (0x0   << 2  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN2_OUT                (0x1   << 2  ) /* OUT. Enables the output on corresponding port pin. */

/* GP1OEN[OEN1] - Port pin direction. */
#define GP1OEN_OEN1_BBA                (*(volatile unsigned long *) 0x420C0684)
#define GP1OEN_OEN1_MSK                (0x1   << 1  )
#define GP1OEN_OEN1                    (0x1   << 1  )
#define GP1OEN_OEN1_IN                 (0x0   << 1  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN1_OUT                (0x1   << 1  ) /* OUT. Enables the output on corresponding port pin. */

/* GP1OEN[OEN0] - Port pin direction. */
#define GP1OEN_OEN0_BBA                (*(volatile unsigned long *) 0x420C0680)
#define GP1OEN_OEN0_MSK                (0x1   << 0  )
#define GP1OEN_OEN0                    (0x1   << 0  )
#define GP1OEN_OEN0_IN                 (0x0   << 0  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP1OEN_OEN0_OUT                (0x1   << 0  ) /* OUT. Enables the output on corresponding port pin.. */

/* Reset Value for GP1PUL*/
#define GP1PUL_RVAL                    0x7F

/* GP1PUL[PUL6] - Pull Up Enable for port pin. */
#define GP1PUL_PUL6_BBA                (*(volatile unsigned long *) 0x420C0718)
#define GP1PUL_PUL6_MSK                (0x1   << 6  )
#define GP1PUL_PUL6                    (0x1   << 6  )
#define GP1PUL_PUL6_DIS                (0x0   << 6  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL6_EN                 (0x1   << 6  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP1PUL[PUL5] - Pull Up Enable for port pin. */
#define GP1PUL_PUL5_BBA                (*(volatile unsigned long *) 0x420C0714)
#define GP1PUL_PUL5_MSK                (0x1   << 5  )
#define GP1PUL_PUL5                    (0x1   << 5  )
#define GP1PUL_PUL5_DIS                (0x0   << 5  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL5_EN                 (0x1   << 5  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP1PUL[PUL4] - Pull Up Enable for port pin. */
#define GP1PUL_PUL4_BBA                (*(volatile unsigned long *) 0x420C0710)
#define GP1PUL_PUL4_MSK                (0x1   << 4  )
#define GP1PUL_PUL4                    (0x1   << 4  )
#define GP1PUL_PUL4_DIS                (0x0   << 4  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL4_EN                 (0x1   << 4  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP1PUL[PUL3] - Pull Up Enable for port pin. */
#define GP1PUL_PUL3_BBA                (*(volatile unsigned long *) 0x420C070C)
#define GP1PUL_PUL3_MSK                (0x1   << 3  )
#define GP1PUL_PUL3                    (0x1   << 3  )
#define GP1PUL_PUL3_DIS                (0x0   << 3  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL3_EN                 (0x1   << 3  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP1PUL[PUL2] - Pull Up Enable for port pin. */
#define GP1PUL_PUL2_BBA                (*(volatile unsigned long *) 0x420C0708)
#define GP1PUL_PUL2_MSK                (0x1   << 2  )
#define GP1PUL_PUL2                    (0x1   << 2  )
#define GP1PUL_PUL2_DIS                (0x0   << 2  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL2_EN                 (0x1   << 2  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP1PUL[PUL1] - Pull Up Enable for port pin. */
#define GP1PUL_PUL1_BBA                (*(volatile unsigned long *) 0x420C0704)
#define GP1PUL_PUL1_MSK                (0x1   << 1  )
#define GP1PUL_PUL1                    (0x1   << 1  )
#define GP1PUL_PUL1_DIS                (0x0   << 1  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL1_EN                 (0x1   << 1  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP1PUL[PUL0] - Pull Up Enable for port pin. */
#define GP1PUL_PUL0_BBA                (*(volatile unsigned long *) 0x420C0700)
#define GP1PUL_PUL0_MSK                (0x1   << 0  )
#define GP1PUL_PUL0                    (0x1   << 0  )
#define GP1PUL_PUL0_DIS                (0x0   << 0  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP1PUL_PUL0_EN                 (0x1   << 0  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* Reset Value for GP1OCE*/
#define GP1OCE_RVAL                    0x0

/* GP1OCE[OCE6] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE6_BBA                (*(volatile unsigned long *) 0x420C0798)
#define GP1OCE_OCE6_MSK                (0x1   << 6  )
#define GP1OCE_OCE6                    (0x1   << 6  )
#define GP1OCE_OCE6_DIS                (0x0   << 6  ) /* DIS                      */
#define GP1OCE_OCE6_EN                 (0x1   << 6  ) /* EN                       */

/* GP1OCE[OCE5] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE5_BBA                (*(volatile unsigned long *) 0x420C0794)
#define GP1OCE_OCE5_MSK                (0x1   << 5  )
#define GP1OCE_OCE5                    (0x1   << 5  )
#define GP1OCE_OCE5_DIS                (0x0   << 5  ) /* DIS                      */
#define GP1OCE_OCE5_EN                 (0x1   << 5  ) /* EN                       */

/* GP1OCE[OCE4] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE4_BBA                (*(volatile unsigned long *) 0x420C0790)
#define GP1OCE_OCE4_MSK                (0x1   << 4  )
#define GP1OCE_OCE4                    (0x1   << 4  )
#define GP1OCE_OCE4_DIS                (0x0   << 4  ) /* DIS                      */
#define GP1OCE_OCE4_EN                 (0x1   << 4  ) /* EN                       */

/* GP1OCE[OCE3] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE3_BBA                (*(volatile unsigned long *) 0x420C078C)
#define GP1OCE_OCE3_MSK                (0x1   << 3  )
#define GP1OCE_OCE3                    (0x1   << 3  )
#define GP1OCE_OCE3_DIS                (0x0   << 3  ) /* DIS                      */
#define GP1OCE_OCE3_EN                 (0x1   << 3  ) /* EN                       */

/* GP1OCE[OCE2] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE2_BBA                (*(volatile unsigned long *) 0x420C0788)
#define GP1OCE_OCE2_MSK                (0x1   << 2  )
#define GP1OCE_OCE2                    (0x1   << 2  )
#define GP1OCE_OCE2_DIS                (0x0   << 2  ) /* DIS                      */
#define GP1OCE_OCE2_EN                 (0x1   << 2  ) /* EN                       */

/* GP1OCE[OCE1] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE1_BBA                (*(volatile unsigned long *) 0x420C0784)
#define GP1OCE_OCE1_MSK                (0x1   << 1  )
#define GP1OCE_OCE1                    (0x1   << 1  )
#define GP1OCE_OCE1_DIS                (0x0   << 1  ) /* DIS                      */
#define GP1OCE_OCE1_EN                 (0x1   << 1  ) /* EN                       */

/* GP1OCE[OCE0] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP1OCE_OCE0_BBA                (*(volatile unsigned long *) 0x420C0780)
#define GP1OCE_OCE0_MSK                (0x1   << 0  )
#define GP1OCE_OCE0                    (0x1   << 0  )
#define GP1OCE_OCE0_DIS                (0x0   << 0  ) /* DIS                      */
#define GP1OCE_OCE0_EN                 (0x1   << 0  ) /* EN                       */

/* Reset Value for GP1IN*/
#define GP1IN_RVAL                     0x7F

/* GP1IN[IN6] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN6_BBA                  (*(volatile unsigned long *) 0x420C0898)
#define GP1IN_IN6_MSK                  (0x1   << 6  )
#define GP1IN_IN6                      (0x1   << 6  )
#define GP1IN_IN6_LOW                  (0x0   << 6  ) /* LOW                      */
#define GP1IN_IN6_HIGH                 (0x1   << 6  ) /* HIGH                     */

/* GP1IN[IN5] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN5_BBA                  (*(volatile unsigned long *) 0x420C0894)
#define GP1IN_IN5_MSK                  (0x1   << 5  )
#define GP1IN_IN5                      (0x1   << 5  )
#define GP1IN_IN5_LOW                  (0x0   << 5  ) /* LOW                      */
#define GP1IN_IN5_HIGH                 (0x1   << 5  ) /* HIGH                     */

/* GP1IN[IN4] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN4_BBA                  (*(volatile unsigned long *) 0x420C0890)
#define GP1IN_IN4_MSK                  (0x1   << 4  )
#define GP1IN_IN4                      (0x1   << 4  )
#define GP1IN_IN4_LOW                  (0x0   << 4  ) /* LOW                      */
#define GP1IN_IN4_HIGH                 (0x1   << 4  ) /* HIGH                     */

/* GP1IN[IN3] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN3_BBA                  (*(volatile unsigned long *) 0x420C088C)
#define GP1IN_IN3_MSK                  (0x1   << 3  )
#define GP1IN_IN3                      (0x1   << 3  )
#define GP1IN_IN3_LOW                  (0x0   << 3  ) /* LOW                      */
#define GP1IN_IN3_HIGH                 (0x1   << 3  ) /* HIGH                     */

/* GP1IN[IN2] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN2_BBA                  (*(volatile unsigned long *) 0x420C0888)
#define GP1IN_IN2_MSK                  (0x1   << 2  )
#define GP1IN_IN2                      (0x1   << 2  )
#define GP1IN_IN2_LOW                  (0x0   << 2  ) /* LOW                      */
#define GP1IN_IN2_HIGH                 (0x1   << 2  ) /* HIGH                     */

/* GP1IN[IN1] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN1_BBA                  (*(volatile unsigned long *) 0x420C0884)
#define GP1IN_IN1_MSK                  (0x1   << 1  )
#define GP1IN_IN1                      (0x1   << 1  )
#define GP1IN_IN1_LOW                  (0x0   << 1  ) /* LOW                      */
#define GP1IN_IN1_HIGH                 (0x1   << 1  ) /* HIGH                     */

/* GP1IN[IN0] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP1IN_IN0_BBA                  (*(volatile unsigned long *) 0x420C0880)
#define GP1IN_IN0_MSK                  (0x1   << 0  )
#define GP1IN_IN0                      (0x1   << 0  )
#define GP1IN_IN0_LOW                  (0x0   << 0  ) /* LOW                      */
#define GP1IN_IN0_HIGH                 (0x1   << 0  ) /* HIGH                     */

/* Reset Value for GP1OUT*/
#define GP1OUT_RVAL                    0x0

/* GP1OUT[OUT6] - Output for port pin. */
#define GP1OUT_OUT6_BBA                (*(volatile unsigned long *) 0x420C0918)
#define GP1OUT_OUT6_MSK                (0x1   << 6  )
#define GP1OUT_OUT6                    (0x1   << 6  )
#define GP1OUT_OUT6_LOW                (0x0   << 6  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT6_HIGH               (0x1   << 6  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP1OUT[OUT5] - Output for port pin. */
#define GP1OUT_OUT5_BBA                (*(volatile unsigned long *) 0x420C0914)
#define GP1OUT_OUT5_MSK                (0x1   << 5  )
#define GP1OUT_OUT5                    (0x1   << 5  )
#define GP1OUT_OUT5_LOW                (0x0   << 5  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT5_HIGH               (0x1   << 5  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP1OUT[OUT4] - Output for port pin. */
#define GP1OUT_OUT4_BBA                (*(volatile unsigned long *) 0x420C0910)
#define GP1OUT_OUT4_MSK                (0x1   << 4  )
#define GP1OUT_OUT4                    (0x1   << 4  )
#define GP1OUT_OUT4_LOW                (0x0   << 4  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT4_HIGH               (0x1   << 4  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP1OUT[OUT3] - Output for port pin. */
#define GP1OUT_OUT3_BBA                (*(volatile unsigned long *) 0x420C090C)
#define GP1OUT_OUT3_MSK                (0x1   << 3  )
#define GP1OUT_OUT3                    (0x1   << 3  )
#define GP1OUT_OUT3_LOW                (0x0   << 3  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT3_HIGH               (0x1   << 3  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP1OUT[OUT2] - Output for port pin. */
#define GP1OUT_OUT2_BBA                (*(volatile unsigned long *) 0x420C0908)
#define GP1OUT_OUT2_MSK                (0x1   << 2  )
#define GP1OUT_OUT2                    (0x1   << 2  )
#define GP1OUT_OUT2_LOW                (0x0   << 2  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT2_HIGH               (0x1   << 2  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP1OUT[OUT1] - Output for port pin. */
#define GP1OUT_OUT1_BBA                (*(volatile unsigned long *) 0x420C0904)
#define GP1OUT_OUT1_MSK                (0x1   << 1  )
#define GP1OUT_OUT1                    (0x1   << 1  )
#define GP1OUT_OUT1_LOW                (0x0   << 1  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT1_HIGH               (0x1   << 1  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP1OUT[OUT0] - Output for port pin. */
#define GP1OUT_OUT0_BBA                (*(volatile unsigned long *) 0x420C0900)
#define GP1OUT_OUT0_MSK                (0x1   << 0  )
#define GP1OUT_OUT0                    (0x1   << 0  )
#define GP1OUT_OUT0_LOW                (0x0   << 0  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP1OUT_OUT0_HIGH               (0x1   << 0  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP1SET*/
#define GP1SET_RVAL                    0x0

/* GP1SET[SET6] - Set output high for corresponding port pin. */
#define GP1SET_SET6_BBA                (*(volatile unsigned long *) 0x420C0998)
#define GP1SET_SET6_MSK                (0x1   << 6  )
#define GP1SET_SET6                    (0x1   << 6  )
#define GP1SET_SET6_SET                (0x1   << 6  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP1SET[SET5] - Set output high for corresponding port pin. */
#define GP1SET_SET5_BBA                (*(volatile unsigned long *) 0x420C0994)
#define GP1SET_SET5_MSK                (0x1   << 5  )
#define GP1SET_SET5                    (0x1   << 5  )
#define GP1SET_SET5_SET                (0x1   << 5  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP1SET[SET4] - Set output high for corresponding port pin. */
#define GP1SET_SET4_BBA                (*(volatile unsigned long *) 0x420C0990)
#define GP1SET_SET4_MSK                (0x1   << 4  )
#define GP1SET_SET4                    (0x1   << 4  )
#define GP1SET_SET4_SET                (0x1   << 4  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP1SET[SET3] - Set output high for corresponding port pin. */
#define GP1SET_SET3_BBA                (*(volatile unsigned long *) 0x420C098C)
#define GP1SET_SET3_MSK                (0x1   << 3  )
#define GP1SET_SET3                    (0x1   << 3  )
#define GP1SET_SET3_SET                (0x1   << 3  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP1SET[SET2] - Set output high for corresponding port pin. */
#define GP1SET_SET2_BBA                (*(volatile unsigned long *) 0x420C0988)
#define GP1SET_SET2_MSK                (0x1   << 2  )
#define GP1SET_SET2                    (0x1   << 2  )
#define GP1SET_SET2_SET                (0x1   << 2  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP1SET[SET1] - Set output high for corresponding port pin. */
#define GP1SET_SET1_BBA                (*(volatile unsigned long *) 0x420C0984)
#define GP1SET_SET1_MSK                (0x1   << 1  )
#define GP1SET_SET1                    (0x1   << 1  )
#define GP1SET_SET1_SET                (0x1   << 1  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP1SET[SET0] - Set output high for corresponding port pin. */
#define GP1SET_SET0_BBA                (*(volatile unsigned long *) 0x420C0980)
#define GP1SET_SET0_MSK                (0x1   << 0  )
#define GP1SET_SET0                    (0x1   << 0  )
#define GP1SET_SET0_SET                (0x1   << 0  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP1CLR*/
#define GP1CLR_RVAL                    0x0

/* GP1CLR[CLR6] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR6_BBA                (*(volatile unsigned long *) 0x420C0A18)
#define GP1CLR_CLR6_MSK                (0x1   << 6  )
#define GP1CLR_CLR6                    (0x1   << 6  )
#define GP1CLR_CLR6_CLR                (0x1   << 6  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP1CLR[CLR5] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR5_BBA                (*(volatile unsigned long *) 0x420C0A14)
#define GP1CLR_CLR5_MSK                (0x1   << 5  )
#define GP1CLR_CLR5                    (0x1   << 5  )
#define GP1CLR_CLR5_CLR                (0x1   << 5  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP1CLR[CLR4] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR4_BBA                (*(volatile unsigned long *) 0x420C0A10)
#define GP1CLR_CLR4_MSK                (0x1   << 4  )
#define GP1CLR_CLR4                    (0x1   << 4  )
#define GP1CLR_CLR4_CLR                (0x1   << 4  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP1CLR[CLR3] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR3_BBA                (*(volatile unsigned long *) 0x420C0A0C)
#define GP1CLR_CLR3_MSK                (0x1   << 3  )
#define GP1CLR_CLR3                    (0x1   << 3  )
#define GP1CLR_CLR3_CLR                (0x1   << 3  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP1CLR[CLR2] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR2_BBA                (*(volatile unsigned long *) 0x420C0A08)
#define GP1CLR_CLR2_MSK                (0x1   << 2  )
#define GP1CLR_CLR2                    (0x1   << 2  )
#define GP1CLR_CLR2_CLR                (0x1   << 2  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP1CLR[CLR1] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR1_BBA                (*(volatile unsigned long *) 0x420C0A04)
#define GP1CLR_CLR1_MSK                (0x1   << 1  )
#define GP1CLR_CLR1                    (0x1   << 1  )
#define GP1CLR_CLR1_CLR                (0x1   << 1  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP1CLR[CLR0] - Set by user code to drive the corresponding GPIO low. */
#define GP1CLR_CLR0_BBA                (*(volatile unsigned long *) 0x420C0A00)
#define GP1CLR_CLR0_MSK                (0x1   << 0  )
#define GP1CLR_CLR0                    (0x1   << 0  )
#define GP1CLR_CLR0_CLR                (0x1   << 0  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* Reset Value for GP1TGL*/
#define GP1TGL_RVAL                    0x0

/* GP1TGL[TGL6] - Toggle for corresponding port pin. */
#define GP1TGL_TGL6_BBA                (*(volatile unsigned long *) 0x420C0A98)
#define GP1TGL_TGL6_MSK                (0x1   << 6  )
#define GP1TGL_TGL6                    (0x1   << 6  )
#define GP1TGL_TGL6_TGL                (0x1   << 6  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP1TGL[TGL5] - Toggle for corresponding port pin. */
#define GP1TGL_TGL5_BBA                (*(volatile unsigned long *) 0x420C0A94)
#define GP1TGL_TGL5_MSK                (0x1   << 5  )
#define GP1TGL_TGL5                    (0x1   << 5  )
#define GP1TGL_TGL5_TGL                (0x1   << 5  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP1TGL[TGL4] - Toggle for corresponding port pin. */
#define GP1TGL_TGL4_BBA                (*(volatile unsigned long *) 0x420C0A90)
#define GP1TGL_TGL4_MSK                (0x1   << 4  )
#define GP1TGL_TGL4                    (0x1   << 4  )
#define GP1TGL_TGL4_TGL                (0x1   << 4  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP1TGL[TGL3] - Toggle for corresponding port pin. */
#define GP1TGL_TGL3_BBA                (*(volatile unsigned long *) 0x420C0A8C)
#define GP1TGL_TGL3_MSK                (0x1   << 3  )
#define GP1TGL_TGL3                    (0x1   << 3  )
#define GP1TGL_TGL3_TGL                (0x1   << 3  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP1TGL[TGL2] - Toggle for corresponding port pin. */
#define GP1TGL_TGL2_BBA                (*(volatile unsigned long *) 0x420C0A88)
#define GP1TGL_TGL2_MSK                (0x1   << 2  )
#define GP1TGL_TGL2                    (0x1   << 2  )
#define GP1TGL_TGL2_TGL                (0x1   << 2  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP1TGL[TGL1] - Toggle for corresponding port pin. */
#define GP1TGL_TGL1_BBA                (*(volatile unsigned long *) 0x420C0A84)
#define GP1TGL_TGL1_MSK                (0x1   << 1  )
#define GP1TGL_TGL1                    (0x1   << 1  )
#define GP1TGL_TGL1_TGL                (0x1   << 1  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP1TGL[TGL0] - Toggle for corresponding port pin. */
#define GP1TGL_TGL0_BBA                (*(volatile unsigned long *) 0x420C0A80)
#define GP1TGL_TGL0_MSK                (0x1   << 0  )
#define GP1TGL_TGL0                    (0x1   << 0  )
#define GP1TGL_TGL0_TGL                (0x1   << 0  ) /* TGL. Set by user code to invert the corresponding GPIO. */
#if (__NO_MMR_STRUCTS__==1)

#define          GP2CON                                     (*(volatile unsigned short int *) 0x40006060)
#define          GP2OEN                                     (*(volatile unsigned char      *) 0x40006064)
#define          GP2PUL                                     (*(volatile unsigned char      *) 0x40006068)
#define          GP2OCE                                     (*(volatile unsigned char      *) 0x4000606C)
#define          GP2IN                                      (*(volatile unsigned char      *) 0x40006074)
#define          GP2OUT                                     (*(volatile unsigned char      *) 0x40006078)
#define          GP2SET                                     (*(volatile unsigned char      *) 0x4000607C)
#define          GP2CLR                                     (*(volatile unsigned char      *) 0x40006080)
#define          GP2TGL                                     (*(volatile unsigned char      *) 0x40006084)
#endif // (__NO_MMR_STRUCTS__==1)

/* Reset Value for GP2CON*/
#define GP2CON_RVAL                    0x0

/* GP2CON[CON7] - Configuration bits for P2.7 */
#define GP2CON_CON7_MSK                (0x3   << 14 )
#define GP2CON_CON7_GPIOIRQ7           (0x0   << 14 ) /* GPIOIRQ7                 */

/* GP2CON[CON6] - Configuration bits for P2.6 */
#define GP2CON_CON6_MSK                (0x3   << 12 )
#define GP2CON_CON6_GPIO               (0x1   << 12 ) /* GPIO                     */

/* GP2CON[CON5] - Configuration bits for P2.5 */
#define GP2CON_CON5_MSK                (0x3   << 10 )
#define GP2CON_CON5_GPIO               (0x2   << 10 ) /* GPIO                     */
#define GP2CON_CON5_RF32KHZCLK         (0x3   << 10 ) /* RF32KHZCLK               */

/* GP2CON[CON4] - Configuration bits for P2.4 */
#define GP2CON_CON4_MSK                (0x3   << 8  )
#define GP2CON_CON4_IRQ8               (0x0   << 8  ) /* IRQ8                     */
#define GP2CON_CON4_GPIO               (0x1   << 8  ) /* GPIO                     */

/* GP2CON[CON3] - Configuration bits for P2.3 */
#define GP2CON_CON3_MSK                (0x3   << 6  )
#define GP2CON_CON3_SPI0CS             (0x0   << 6  ) /* SPI0CS                   */
#define GP2CON_CON3_GPIO               (0x1   << 6  ) /* GPIO                     */

/* GP2CON[CON2] - Configuration bits for P2.2 */
#define GP2CON_CON2_MSK                (0x3   << 4  )
#define GP2CON_CON2_SPI0MOSI           (0x0   << 4  ) /* SPI0MOSI                 */
#define GP2CON_CON2_GPIO               (0x1   << 4  ) /* GPIO                     */

/* GP2CON[CON1] - Configuration bits for P2.1 */
#define GP2CON_CON1_MSK                (0x3   << 2  )
#define GP2CON_CON1_SPI0SCLK           (0x0   << 2  ) /* SPI0SCLK                 */
#define GP2CON_CON1_GPIO               (0x1   << 2  ) /* GPIO                     */

/* GP2CON[CON0] - Configuration bits for P2.0 */
#define GP2CON_CON0_MSK                (0x3   << 0  )
#define GP2CON_CON0_SPI0MISO           (0x0   << 0  ) /* SPI0MISO                 */
#define GP2CON_CON0_GPIO               (0x1   << 0  ) /* GPIO                     */

/* Reset Value for GP2OEN*/
#define GP2OEN_RVAL                    0x0

/* GP2OEN[OEN7] - Port pin direction. */
#define GP2OEN_OEN7_BBA                (*(volatile unsigned long *) 0x420C0C9C)
#define GP2OEN_OEN7_MSK                (0x1   << 7  )
#define GP2OEN_OEN7                    (0x1   << 7  )
#define GP2OEN_OEN7_IN                 (0x0   << 7  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN7_OUT                (0x1   << 7  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN6] - Port pin direction. */
#define GP2OEN_OEN6_BBA                (*(volatile unsigned long *) 0x420C0C98)
#define GP2OEN_OEN6_MSK                (0x1   << 6  )
#define GP2OEN_OEN6                    (0x1   << 6  )
#define GP2OEN_OEN6_IN                 (0x0   << 6  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN6_OUT                (0x1   << 6  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN5] - Port pin direction. */
#define GP2OEN_OEN5_BBA                (*(volatile unsigned long *) 0x420C0C94)
#define GP2OEN_OEN5_MSK                (0x1   << 5  )
#define GP2OEN_OEN5                    (0x1   << 5  )
#define GP2OEN_OEN5_IN                 (0x0   << 5  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN5_OUT                (0x1   << 5  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN4] - Port pin direction. */
#define GP2OEN_OEN4_BBA                (*(volatile unsigned long *) 0x420C0C90)
#define GP2OEN_OEN4_MSK                (0x1   << 4  )
#define GP2OEN_OEN4                    (0x1   << 4  )
#define GP2OEN_OEN4_IN                 (0x0   << 4  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN4_OUT                (0x1   << 4  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN3] - Port pin direction. */
#define GP2OEN_OEN3_BBA                (*(volatile unsigned long *) 0x420C0C8C)
#define GP2OEN_OEN3_MSK                (0x1   << 3  )
#define GP2OEN_OEN3                    (0x1   << 3  )
#define GP2OEN_OEN3_IN                 (0x0   << 3  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN3_OUT                (0x1   << 3  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN2] - Port pin direction. */
#define GP2OEN_OEN2_BBA                (*(volatile unsigned long *) 0x420C0C88)
#define GP2OEN_OEN2_MSK                (0x1   << 2  )
#define GP2OEN_OEN2                    (0x1   << 2  )
#define GP2OEN_OEN2_IN                 (0x0   << 2  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN2_OUT                (0x1   << 2  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN1] - Port pin direction. */
#define GP2OEN_OEN1_BBA                (*(volatile unsigned long *) 0x420C0C84)
#define GP2OEN_OEN1_MSK                (0x1   << 1  )
#define GP2OEN_OEN1                    (0x1   << 1  )
#define GP2OEN_OEN1_IN                 (0x0   << 1  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN1_OUT                (0x1   << 1  ) /* OUT. Enables the output on corresponding port pin. */

/* GP2OEN[OEN0] - Port pin direction. */
#define GP2OEN_OEN0_BBA                (*(volatile unsigned long *) 0x420C0C80)
#define GP2OEN_OEN0_MSK                (0x1   << 0  )
#define GP2OEN_OEN0                    (0x1   << 0  )
#define GP2OEN_OEN0_IN                 (0x0   << 0  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP2OEN_OEN0_OUT                (0x1   << 0  ) /* OUT. Enables the output on corresponding port pin. */

/* Reset Value for GP2PUL*/
#define GP2PUL_RVAL                    0xFF

/* GP2PUL[PUL7] - Pull Up Enable for port pin. */
#define GP2PUL_PUL7_BBA                (*(volatile unsigned long *) 0x420C0D1C)
#define GP2PUL_PUL7_MSK                (0x1   << 7  )
#define GP2PUL_PUL7                    (0x1   << 7  )
#define GP2PUL_PUL7_DIS                (0x0   << 7  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL7_EN                 (0x1   << 7  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL6] - Pull Up Enable for port pin. */
#define GP2PUL_PUL6_BBA                (*(volatile unsigned long *) 0x420C0D18)
#define GP2PUL_PUL6_MSK                (0x1   << 6  )
#define GP2PUL_PUL6                    (0x1   << 6  )
#define GP2PUL_PUL6_DIS                (0x0   << 6  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL6_EN                 (0x1   << 6  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL5] - Pull Up Enable for port pin. */
#define GP2PUL_PUL5_BBA                (*(volatile unsigned long *) 0x420C0D14)
#define GP2PUL_PUL5_MSK                (0x1   << 5  )
#define GP2PUL_PUL5                    (0x1   << 5  )
#define GP2PUL_PUL5_DIS                (0x0   << 5  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL5_EN                 (0x1   << 5  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL4] - Pull Up Enable for port pin. */
#define GP2PUL_PUL4_BBA                (*(volatile unsigned long *) 0x420C0D10)
#define GP2PUL_PUL4_MSK                (0x1   << 4  )
#define GP2PUL_PUL4                    (0x1   << 4  )
#define GP2PUL_PUL4_DIS                (0x0   << 4  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL4_EN                 (0x1   << 4  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL3] - Pull Up Enable for port pin. */
#define GP2PUL_PUL3_BBA                (*(volatile unsigned long *) 0x420C0D0C)
#define GP2PUL_PUL3_MSK                (0x1   << 3  )
#define GP2PUL_PUL3                    (0x1   << 3  )
#define GP2PUL_PUL3_DIS                (0x0   << 3  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL3_EN                 (0x1   << 3  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL2] - Pull Up Enable for port pin. */
#define GP2PUL_PUL2_BBA                (*(volatile unsigned long *) 0x420C0D08)
#define GP2PUL_PUL2_MSK                (0x1   << 2  )
#define GP2PUL_PUL2                    (0x1   << 2  )
#define GP2PUL_PUL2_DIS                (0x0   << 2  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL2_EN                 (0x1   << 2  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL1] - Pull Up Enable for port pin. */
#define GP2PUL_PUL1_BBA                (*(volatile unsigned long *) 0x420C0D04)
#define GP2PUL_PUL1_MSK                (0x1   << 1  )
#define GP2PUL_PUL1                    (0x1   << 1  )
#define GP2PUL_PUL1_DIS                (0x0   << 1  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL1_EN                 (0x1   << 1  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP2PUL[PUL0] - Pull Up Enable for port pin. */
#define GP2PUL_PUL0_BBA                (*(volatile unsigned long *) 0x420C0D00)
#define GP2PUL_PUL0_MSK                (0x1   << 0  )
#define GP2PUL_PUL0                    (0x1   << 0  )
#define GP2PUL_PUL0_DIS                (0x0   << 0  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP2PUL_PUL0_EN                 (0x1   << 0  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* Reset Value for GP2OCE*/
#define GP2OCE_RVAL                    0x0

/* GP2OCE[OCE7] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE7_BBA                (*(volatile unsigned long *) 0x420C0D9C)
#define GP2OCE_OCE7_MSK                (0x1   << 7  )
#define GP2OCE_OCE7                    (0x1   << 7  )
#define GP2OCE_OCE7_DIS                (0x0   << 7  ) /* DIS                      */
#define GP2OCE_OCE7_EN                 (0x1   << 7  ) /* EN                       */

/* GP2OCE[OCE6] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE6_BBA                (*(volatile unsigned long *) 0x420C0D98)
#define GP2OCE_OCE6_MSK                (0x1   << 6  )
#define GP2OCE_OCE6                    (0x1   << 6  )
#define GP2OCE_OCE6_DIS                (0x0   << 6  ) /* DIS                      */
#define GP2OCE_OCE6_EN                 (0x1   << 6  ) /* EN                       */

/* GP2OCE[OCE5] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE5_BBA                (*(volatile unsigned long *) 0x420C0D94)
#define GP2OCE_OCE5_MSK                (0x1   << 5  )
#define GP2OCE_OCE5                    (0x1   << 5  )
#define GP2OCE_OCE5_DIS                (0x0   << 5  ) /* DIS                      */
#define GP2OCE_OCE5_EN                 (0x1   << 5  ) /* EN                       */

/* GP2OCE[OCE4] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE4_BBA                (*(volatile unsigned long *) 0x420C0D90)
#define GP2OCE_OCE4_MSK                (0x1   << 4  )
#define GP2OCE_OCE4                    (0x1   << 4  )
#define GP2OCE_OCE4_DIS                (0x0   << 4  ) /* DIS                      */
#define GP2OCE_OCE4_EN                 (0x1   << 4  ) /* EN                       */

/* GP2OCE[OCE3] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE3_BBA                (*(volatile unsigned long *) 0x420C0D8C)
#define GP2OCE_OCE3_MSK                (0x1   << 3  )
#define GP2OCE_OCE3                    (0x1   << 3  )
#define GP2OCE_OCE3_DIS                (0x0   << 3  ) /* DIS                      */
#define GP2OCE_OCE3_EN                 (0x1   << 3  ) /* EN                       */

/* GP2OCE[OCE2] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE2_BBA                (*(volatile unsigned long *) 0x420C0D88)
#define GP2OCE_OCE2_MSK                (0x1   << 2  )
#define GP2OCE_OCE2                    (0x1   << 2  )
#define GP2OCE_OCE2_DIS                (0x0   << 2  ) /* DIS                      */
#define GP2OCE_OCE2_EN                 (0x1   << 2  ) /* EN                       */

/* GP2OCE[OCE1] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE1_BBA                (*(volatile unsigned long *) 0x420C0D84)
#define GP2OCE_OCE1_MSK                (0x1   << 1  )
#define GP2OCE_OCE1                    (0x1   << 1  )
#define GP2OCE_OCE1_DIS                (0x0   << 1  ) /* DIS                      */
#define GP2OCE_OCE1_EN                 (0x1   << 1  ) /* EN                       */

/* GP2OCE[OCE0] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP2OCE_OCE0_BBA                (*(volatile unsigned long *) 0x420C0D80)
#define GP2OCE_OCE0_MSK                (0x1   << 0  )
#define GP2OCE_OCE0                    (0x1   << 0  )
#define GP2OCE_OCE0_DIS                (0x0   << 0  ) /* DIS                      */
#define GP2OCE_OCE0_EN                 (0x1   << 0  ) /* EN                       */

/* Reset Value for GP2IN*/
#define GP2IN_RVAL                     0xFF

/* GP2IN[IN7] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN7_BBA                  (*(volatile unsigned long *) 0x420C0E9C)
#define GP2IN_IN7_MSK                  (0x1   << 7  )
#define GP2IN_IN7                      (0x1   << 7  )
#define GP2IN_IN7_LOW                  (0x0   << 7  ) /* LOW                      */
#define GP2IN_IN7_HIGH                 (0x1   << 7  ) /* HIGH                     */

/* GP2IN[IN6] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN6_BBA                  (*(volatile unsigned long *) 0x420C0E98)
#define GP2IN_IN6_MSK                  (0x1   << 6  )
#define GP2IN_IN6                      (0x1   << 6  )
#define GP2IN_IN6_LOW                  (0x0   << 6  ) /* LOW                      */
#define GP2IN_IN6_HIGH                 (0x1   << 6  ) /* HIGH                     */

/* GP2IN[IN5] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN5_BBA                  (*(volatile unsigned long *) 0x420C0E94)
#define GP2IN_IN5_MSK                  (0x1   << 5  )
#define GP2IN_IN5                      (0x1   << 5  )
#define GP2IN_IN5_LOW                  (0x0   << 5  ) /* LOW                      */
#define GP2IN_IN5_HIGH                 (0x1   << 5  ) /* HIGH                     */

/* GP2IN[IN4] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN4_BBA                  (*(volatile unsigned long *) 0x420C0E90)
#define GP2IN_IN4_MSK                  (0x1   << 4  )
#define GP2IN_IN4                      (0x1   << 4  )
#define GP2IN_IN4_LOW                  (0x0   << 4  ) /* LOW                      */
#define GP2IN_IN4_HIGH                 (0x1   << 4  ) /* HIGH                     */

/* GP2IN[IN3] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN3_BBA                  (*(volatile unsigned long *) 0x420C0E8C)
#define GP2IN_IN3_MSK                  (0x1   << 3  )
#define GP2IN_IN3                      (0x1   << 3  )
#define GP2IN_IN3_LOW                  (0x0   << 3  ) /* LOW                      */
#define GP2IN_IN3_HIGH                 (0x1   << 3  ) /* HIGH                     */

/* GP2IN[IN2] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN2_BBA                  (*(volatile unsigned long *) 0x420C0E88)
#define GP2IN_IN2_MSK                  (0x1   << 2  )
#define GP2IN_IN2                      (0x1   << 2  )
#define GP2IN_IN2_LOW                  (0x0   << 2  ) /* LOW                      */
#define GP2IN_IN2_HIGH                 (0x1   << 2  ) /* HIGH                     */

/* GP2IN[IN1] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN1_BBA                  (*(volatile unsigned long *) 0x420C0E84)
#define GP2IN_IN1_MSK                  (0x1   << 1  )
#define GP2IN_IN1                      (0x1   << 1  )
#define GP2IN_IN1_LOW                  (0x0   << 1  ) /* LOW                      */
#define GP2IN_IN1_HIGH                 (0x1   << 1  ) /* HIGH                     */

/* GP2IN[IN0] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP2IN_IN0_BBA                  (*(volatile unsigned long *) 0x420C0E80)
#define GP2IN_IN0_MSK                  (0x1   << 0  )
#define GP2IN_IN0                      (0x1   << 0  )
#define GP2IN_IN0_LOW                  (0x0   << 0  ) /* LOW                      */
#define GP2IN_IN0_HIGH                 (0x1   << 0  ) /* HIGH                     */

/* Reset Value for GP2OUT*/
#define GP2OUT_RVAL                    0x0

/* GP2OUT[OUT7] - Output for port pin. */
#define GP2OUT_OUT7_BBA                (*(volatile unsigned long *) 0x420C0F1C)
#define GP2OUT_OUT7_MSK                (0x1   << 7  )
#define GP2OUT_OUT7                    (0x1   << 7  )
#define GP2OUT_OUT7_LOW                (0x0   << 7  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT7_HIGH               (0x1   << 7  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT6] - Output for port pin. */
#define GP2OUT_OUT6_BBA                (*(volatile unsigned long *) 0x420C0F18)
#define GP2OUT_OUT6_MSK                (0x1   << 6  )
#define GP2OUT_OUT6                    (0x1   << 6  )
#define GP2OUT_OUT6_LOW                (0x0   << 6  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT6_HIGH               (0x1   << 6  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT5] - Output for port pin. */
#define GP2OUT_OUT5_BBA                (*(volatile unsigned long *) 0x420C0F14)
#define GP2OUT_OUT5_MSK                (0x1   << 5  )
#define GP2OUT_OUT5                    (0x1   << 5  )
#define GP2OUT_OUT5_LOW                (0x0   << 5  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT5_HIGH               (0x1   << 5  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT4] - Output for port pin. */
#define GP2OUT_OUT4_BBA                (*(volatile unsigned long *) 0x420C0F10)
#define GP2OUT_OUT4_MSK                (0x1   << 4  )
#define GP2OUT_OUT4                    (0x1   << 4  )
#define GP2OUT_OUT4_LOW                (0x0   << 4  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT4_HIGH               (0x1   << 4  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT3] - Output for port pin. */
#define GP2OUT_OUT3_BBA                (*(volatile unsigned long *) 0x420C0F0C)
#define GP2OUT_OUT3_MSK                (0x1   << 3  )
#define GP2OUT_OUT3                    (0x1   << 3  )
#define GP2OUT_OUT3_LOW                (0x0   << 3  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT3_HIGH               (0x1   << 3  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT2] - Output for port pin. */
#define GP2OUT_OUT2_BBA                (*(volatile unsigned long *) 0x420C0F08)
#define GP2OUT_OUT2_MSK                (0x1   << 2  )
#define GP2OUT_OUT2                    (0x1   << 2  )
#define GP2OUT_OUT2_LOW                (0x0   << 2  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT2_HIGH               (0x1   << 2  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT1] - Output for port pin. */
#define GP2OUT_OUT1_BBA                (*(volatile unsigned long *) 0x420C0F04)
#define GP2OUT_OUT1_MSK                (0x1   << 1  )
#define GP2OUT_OUT1                    (0x1   << 1  )
#define GP2OUT_OUT1_LOW                (0x0   << 1  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT1_HIGH               (0x1   << 1  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP2OUT[OUT0] - Output for port pin. */
#define GP2OUT_OUT0_BBA                (*(volatile unsigned long *) 0x420C0F00)
#define GP2OUT_OUT0_MSK                (0x1   << 0  )
#define GP2OUT_OUT0                    (0x1   << 0  )
#define GP2OUT_OUT0_LOW                (0x0   << 0  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP2OUT_OUT0_HIGH               (0x1   << 0  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP2SET*/
#define GP2SET_RVAL                    0x0

/* GP2SET[SET7] - Set output high for corresponding port pin. */
#define GP2SET_SET7_BBA                (*(volatile unsigned long *) 0x420C0F9C)
#define GP2SET_SET7_MSK                (0x1   << 7  )
#define GP2SET_SET7                    (0x1   << 7  )
#define GP2SET_SET7_SET                (0x1   << 7  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET6] - Set output high for corresponding port pin. */
#define GP2SET_SET6_BBA                (*(volatile unsigned long *) 0x420C0F98)
#define GP2SET_SET6_MSK                (0x1   << 6  )
#define GP2SET_SET6                    (0x1   << 6  )
#define GP2SET_SET6_SET                (0x1   << 6  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET5] - Set output high for corresponding port pin. */
#define GP2SET_SET5_BBA                (*(volatile unsigned long *) 0x420C0F94)
#define GP2SET_SET5_MSK                (0x1   << 5  )
#define GP2SET_SET5                    (0x1   << 5  )
#define GP2SET_SET5_SET                (0x1   << 5  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET4] - Set output high for corresponding port pin. */
#define GP2SET_SET4_BBA                (*(volatile unsigned long *) 0x420C0F90)
#define GP2SET_SET4_MSK                (0x1   << 4  )
#define GP2SET_SET4                    (0x1   << 4  )
#define GP2SET_SET4_SET                (0x1   << 4  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET3] - Set output high for corresponding port pin. */
#define GP2SET_SET3_BBA                (*(volatile unsigned long *) 0x420C0F8C)
#define GP2SET_SET3_MSK                (0x1   << 3  )
#define GP2SET_SET3                    (0x1   << 3  )
#define GP2SET_SET3_SET                (0x1   << 3  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET2] - Set output high for corresponding port pin. */
#define GP2SET_SET2_BBA                (*(volatile unsigned long *) 0x420C0F88)
#define GP2SET_SET2_MSK                (0x1   << 2  )
#define GP2SET_SET2                    (0x1   << 2  )
#define GP2SET_SET2_SET                (0x1   << 2  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET1] - Set output high for corresponding port pin. */
#define GP2SET_SET1_BBA                (*(volatile unsigned long *) 0x420C0F84)
#define GP2SET_SET1_MSK                (0x1   << 1  )
#define GP2SET_SET1                    (0x1   << 1  )
#define GP2SET_SET1_SET                (0x1   << 1  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP2SET[SET0] - Set output high for corresponding port pin. */
#define GP2SET_SET0_BBA                (*(volatile unsigned long *) 0x420C0F80)
#define GP2SET_SET0_MSK                (0x1   << 0  )
#define GP2SET_SET0                    (0x1   << 0  )
#define GP2SET_SET0_SET                (0x1   << 0  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP2CLR*/
#define GP2CLR_RVAL                    0x0

/* GP2CLR[CLR7] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR7_BBA                (*(volatile unsigned long *) 0x420C101C)
#define GP2CLR_CLR7_MSK                (0x1   << 7  )
#define GP2CLR_CLR7                    (0x1   << 7  )
#define GP2CLR_CLR7_CLR                (0x1   << 7  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR6] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR6_BBA                (*(volatile unsigned long *) 0x420C1018)
#define GP2CLR_CLR6_MSK                (0x1   << 6  )
#define GP2CLR_CLR6                    (0x1   << 6  )
#define GP2CLR_CLR6_CLR                (0x1   << 6  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR5] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR5_BBA                (*(volatile unsigned long *) 0x420C1014)
#define GP2CLR_CLR5_MSK                (0x1   << 5  )
#define GP2CLR_CLR5                    (0x1   << 5  )
#define GP2CLR_CLR5_CLR                (0x1   << 5  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR4] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR4_BBA                (*(volatile unsigned long *) 0x420C1010)
#define GP2CLR_CLR4_MSK                (0x1   << 4  )
#define GP2CLR_CLR4                    (0x1   << 4  )
#define GP2CLR_CLR4_CLR                (0x1   << 4  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR3] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR3_BBA                (*(volatile unsigned long *) 0x420C100C)
#define GP2CLR_CLR3_MSK                (0x1   << 3  )
#define GP2CLR_CLR3                    (0x1   << 3  )
#define GP2CLR_CLR3_CLR                (0x1   << 3  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR2] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR2_BBA                (*(volatile unsigned long *) 0x420C1008)
#define GP2CLR_CLR2_MSK                (0x1   << 2  )
#define GP2CLR_CLR2                    (0x1   << 2  )
#define GP2CLR_CLR2_CLR                (0x1   << 2  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR1] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR1_BBA                (*(volatile unsigned long *) 0x420C1004)
#define GP2CLR_CLR1_MSK                (0x1   << 1  )
#define GP2CLR_CLR1                    (0x1   << 1  )
#define GP2CLR_CLR1_CLR                (0x1   << 1  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP2CLR[CLR0] - Set by user code to drive the corresponding GPIO low. */
#define GP2CLR_CLR0_BBA                (*(volatile unsigned long *) 0x420C1000)
#define GP2CLR_CLR0_MSK                (0x1   << 0  )
#define GP2CLR_CLR0                    (0x1   << 0  )
#define GP2CLR_CLR0_CLR                (0x1   << 0  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* Reset Value for GP2TGL*/
#define GP2TGL_RVAL                    0x0

/* GP2TGL[TGL7] - Toggle for corresponding port pin. */
#define GP2TGL_TGL7_BBA                (*(volatile unsigned long *) 0x420C109C)
#define GP2TGL_TGL7_MSK                (0x1   << 7  )
#define GP2TGL_TGL7                    (0x1   << 7  )
#define GP2TGL_TGL7_TGL                (0x1   << 7  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL6] - Toggle for corresponding port pin. */
#define GP2TGL_TGL6_BBA                (*(volatile unsigned long *) 0x420C1098)
#define GP2TGL_TGL6_MSK                (0x1   << 6  )
#define GP2TGL_TGL6                    (0x1   << 6  )
#define GP2TGL_TGL6_TGL                (0x1   << 6  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL5] - Toggle for corresponding port pin. */
#define GP2TGL_TGL5_BBA                (*(volatile unsigned long *) 0x420C1094)
#define GP2TGL_TGL5_MSK                (0x1   << 5  )
#define GP2TGL_TGL5                    (0x1   << 5  )
#define GP2TGL_TGL5_TGL                (0x1   << 5  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL4] - Toggle for corresponding port pin. */
#define GP2TGL_TGL4_BBA                (*(volatile unsigned long *) 0x420C1090)
#define GP2TGL_TGL4_MSK                (0x1   << 4  )
#define GP2TGL_TGL4                    (0x1   << 4  )
#define GP2TGL_TGL4_TGL                (0x1   << 4  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL3] - Toggle for corresponding port pin. */
#define GP2TGL_TGL3_BBA                (*(volatile unsigned long *) 0x420C108C)
#define GP2TGL_TGL3_MSK                (0x1   << 3  )
#define GP2TGL_TGL3                    (0x1   << 3  )
#define GP2TGL_TGL3_TGL                (0x1   << 3  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL2] - Toggle for corresponding port pin. */
#define GP2TGL_TGL2_BBA                (*(volatile unsigned long *) 0x420C1088)
#define GP2TGL_TGL2_MSK                (0x1   << 2  )
#define GP2TGL_TGL2                    (0x1   << 2  )
#define GP2TGL_TGL2_TGL                (0x1   << 2  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL1] - Toggle for corresponding port pin. */
#define GP2TGL_TGL1_BBA                (*(volatile unsigned long *) 0x420C1084)
#define GP2TGL_TGL1_MSK                (0x1   << 1  )
#define GP2TGL_TGL1                    (0x1   << 1  )
#define GP2TGL_TGL1_TGL                (0x1   << 1  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP2TGL[TGL0] - Toggle for corresponding port pin. */
#define GP2TGL_TGL0_BBA                (*(volatile unsigned long *) 0x420C1080)
#define GP2TGL_TGL0_MSK                (0x1   << 0  )
#define GP2TGL_TGL0                    (0x1   << 0  )
#define GP2TGL_TGL0_TGL                (0x1   << 0  ) /* TGL. Set by user code to invert the corresponding GPIO. */
#if (__NO_MMR_STRUCTS__==1)

#define          GP3CON                                     (*(volatile unsigned short int *) 0x40006090)
#define          GP3OEN                                     (*(volatile unsigned char      *) 0x40006094)
#define          GP3PUL                                     (*(volatile unsigned char      *) 0x40006098)
#define          GP3OCE                                     (*(volatile unsigned char      *) 0x4000609C)
#define          GP3IN                                      (*(volatile unsigned char      *) 0x400060A4)
#define          GP3OUT                                     (*(volatile unsigned char      *) 0x400060A8)
#define          GP3SET                                     (*(volatile unsigned char      *) 0x400060AC)
#define          GP3CLR                                     (*(volatile unsigned char      *) 0x400060B0)
#define          GP3TGL                                     (*(volatile unsigned char      *) 0x400060B4)
#endif // (__NO_MMR_STRUCTS__==1)

/* Reset Value for GP3CON*/
#define GP3CON_RVAL                    0x0

/* GP3CON[CON7] - Configuration bits for P3.7 */
#define GP3CON_CON7_MSK                (0x3   << 14 )
#define GP3CON_CON7_GPIOIRQ0           (0x1   << 14 ) /* GPIOIRQ0                 */

/* GP3CON[CON6] - Configuration bits for P3.6 */
#define GP3CON_CON6_MSK                (0x3   << 12 )
#define GP3CON_CON6_GPIO               (0x1   << 12 ) /* GPIO                     */

/* GP3CON[CON5] - Configuration bits for P3.5 */
#define GP3CON_CON5_MSK                (0x3   << 10 )
#define GP3CON_CON5_GPIO               (0x1   << 10 ) /* GPIO                     */
#define GP3CON_CON5_SPI0MOSI           (0x3   << 10 ) /* SPI0MOSI                 */

/* GP3CON[CON4] - Configuration bits for P3.4 */
#define GP3CON_CON4_MSK                (0x3   << 8  )
#define GP3CON_CON4_GPIO               (0x1   << 8  ) /* GPIO                     */

/* GP3CON[CON3] - Configuration bits for P3.3 */
#define GP3CON_CON3_MSK                (0x3   << 6  )
#define GP3CON_CON3_GPIO               (0x1   << 6  ) /* GPIO                     */
#define GP3CON_CON3_PWMTRIP            (0x2   << 6  ) /* PWMTRIP                  */
#define GP3CON_CON3_SPI0SCLK           (0x3   << 6  ) /* SPI0SCLK                 */

/* GP3CON[CON2] - Configuration bits for P3.2 */
#define GP3CON_CON2_MSK                (0x3   << 4  )
#define GP3CON_CON2_GPIO               (0x1   << 4  ) /* GPIO                     */
#define GP3CON_CON2_PWMSYNC            (0x2   << 4  ) /* PWMSYNC                  */
#define GP3CON_CON2_SPI0MISO           (0x3   << 4  ) /* SPI0MISO                 */

/* GP3CON[CON1] - Configuration bits for P3.1 */
#define GP3CON_CON1_MSK                (0x3   << 2  )
#define GP3CON_CON1_GPIO               (0x0   << 2  ) /* GPIO                     */

/* GP3CON[CON0] - Configuration bits for P3.0 */
#define GP3CON_CON0_MSK                (0x3   << 0  )
#define GP3CON_CON0_GPIO               (0x0   << 0  ) /* GPIO                     */
#define GP3CON_CON0_PWMTRIP            (0x3   << 0  ) /* PWMTRIP                  */

/* Reset Value for GP3OEN*/
#define GP3OEN_RVAL                    0x0

/* GP3OEN[OEN7] - Port pin direction. */
#define GP3OEN_OEN7_BBA                (*(volatile unsigned long *) 0x420C129C)
#define GP3OEN_OEN7_MSK                (0x1   << 7  )
#define GP3OEN_OEN7                    (0x1   << 7  )
#define GP3OEN_OEN7_IN                 (0x0   << 7  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN7_OUT                (0x1   << 7  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN6] - Port pin direction. */
#define GP3OEN_OEN6_BBA                (*(volatile unsigned long *) 0x420C1298)
#define GP3OEN_OEN6_MSK                (0x1   << 6  )
#define GP3OEN_OEN6                    (0x1   << 6  )
#define GP3OEN_OEN6_IN                 (0x0   << 6  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN6_OUT                (0x1   << 6  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN5] - Port pin direction. */
#define GP3OEN_OEN5_BBA                (*(volatile unsigned long *) 0x420C1294)
#define GP3OEN_OEN5_MSK                (0x1   << 5  )
#define GP3OEN_OEN5                    (0x1   << 5  )
#define GP3OEN_OEN5_IN                 (0x0   << 5  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN5_OUT                (0x1   << 5  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN4] - Port pin direction. */
#define GP3OEN_OEN4_BBA                (*(volatile unsigned long *) 0x420C1290)
#define GP3OEN_OEN4_MSK                (0x1   << 4  )
#define GP3OEN_OEN4                    (0x1   << 4  )
#define GP3OEN_OEN4_IN                 (0x0   << 4  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN4_OUT                (0x1   << 4  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN3] - Port pin direction. */
#define GP3OEN_OEN3_BBA                (*(volatile unsigned long *) 0x420C128C)
#define GP3OEN_OEN3_MSK                (0x1   << 3  )
#define GP3OEN_OEN3                    (0x1   << 3  )
#define GP3OEN_OEN3_IN                 (0x0   << 3  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN3_OUT                (0x1   << 3  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN2] - Port pin direction. */
#define GP3OEN_OEN2_BBA                (*(volatile unsigned long *) 0x420C1288)
#define GP3OEN_OEN2_MSK                (0x1   << 2  )
#define GP3OEN_OEN2                    (0x1   << 2  )
#define GP3OEN_OEN2_IN                 (0x0   << 2  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN2_OUT                (0x1   << 2  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN1] - Port pin direction. */
#define GP3OEN_OEN1_BBA                (*(volatile unsigned long *) 0x420C1284)
#define GP3OEN_OEN1_MSK                (0x1   << 1  )
#define GP3OEN_OEN1                    (0x1   << 1  )
#define GP3OEN_OEN1_IN                 (0x0   << 1  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN1_OUT                (0x1   << 1  ) /* OUT. Enables the output on corresponding port pin. */

/* GP3OEN[OEN0] - Port pin direction. */
#define GP3OEN_OEN0_BBA                (*(volatile unsigned long *) 0x420C1280)
#define GP3OEN_OEN0_MSK                (0x1   << 0  )
#define GP3OEN_OEN0                    (0x1   << 0  )
#define GP3OEN_OEN0_IN                 (0x0   << 0  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP3OEN_OEN0_OUT                (0x1   << 0  ) /* OUT. Enables the output on corresponding port pin. */

/* Reset Value for GP3PUL*/
#define GP3PUL_RVAL                    0xFF

/* GP3PUL[PUL7] - Pull Up Enable for port pin. */
#define GP3PUL_PUL7_BBA                (*(volatile unsigned long *) 0x420C131C)
#define GP3PUL_PUL7_MSK                (0x1   << 7  )
#define GP3PUL_PUL7                    (0x1   << 7  )
#define GP3PUL_PUL7_DIS                (0x0   << 7  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL7_EN                 (0x1   << 7  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP3PUL[PUL6] - Pull Up Enable for port pin. */
#define GP3PUL_PUL6_BBA                (*(volatile unsigned long *) 0x420C1318)
#define GP3PUL_PUL6_MSK                (0x1   << 6  )
#define GP3PUL_PUL6                    (0x1   << 6  )
#define GP3PUL_PUL6_DIS                (0x0   << 6  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL6_EN                 (0x1   << 6  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP3PUL[PUL5] - Pull Up Enable for port pin. */
#define GP3PUL_PUL5_BBA                (*(volatile unsigned long *) 0x420C1314)
#define GP3PUL_PUL5_MSK                (0x1   << 5  )
#define GP3PUL_PUL5                    (0x1   << 5  )
#define GP3PUL_PUL5_DIS                (0x0   << 5  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL5_EN                 (0x1   << 5  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP3PUL[PUL4] - Pull Up Enable for port pin. */
#define GP3PUL_PUL4_BBA                (*(volatile unsigned long *) 0x420C1310)
#define GP3PUL_PUL4_MSK                (0x1   << 4  )
#define GP3PUL_PUL4                    (0x1   << 4  )
#define GP3PUL_PUL4_DIS                (0x0   << 4  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL4_EN                 (0x1   << 4  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP3PUL[PUL3] - Pull Up Enable for port pin. */
#define GP3PUL_PUL3_BBA                (*(volatile unsigned long *) 0x420C130C)
#define GP3PUL_PUL3_MSK                (0x1   << 3  )
#define GP3PUL_PUL3                    (0x1   << 3  )
#define GP3PUL_PUL3_DIS                (0x0   << 3  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL3_EN                 (0x1   << 3  ) /* EN. Enables the internal pull up oncorresponding port pin. */

/* GP3PUL[PUL2] - Pull Up Enable for port pin. */
#define GP3PUL_PUL2_BBA                (*(volatile unsigned long *) 0x420C1308)
#define GP3PUL_PUL2_MSK                (0x1   << 2  )
#define GP3PUL_PUL2                    (0x1   << 2  )
#define GP3PUL_PUL2_DIS                (0x0   << 2  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL2_EN                 (0x1   << 2  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP3PUL[PUL1] - Pull Up Enable for port pin. */
#define GP3PUL_PUL1_BBA                (*(volatile unsigned long *) 0x420C1304)
#define GP3PUL_PUL1_MSK                (0x1   << 1  )
#define GP3PUL_PUL1                    (0x1   << 1  )
#define GP3PUL_PUL1_DIS                (0x0   << 1  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL1_EN                 (0x1   << 1  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP3PUL[PUL0] - Pull Up Enable for port pin. */
#define GP3PUL_PUL0_BBA                (*(volatile unsigned long *) 0x420C1300)
#define GP3PUL_PUL0_MSK                (0x1   << 0  )
#define GP3PUL_PUL0                    (0x1   << 0  )
#define GP3PUL_PUL0_DIS                (0x0   << 0  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP3PUL_PUL0_EN                 (0x1   << 0  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* Reset Value for GP3OCE*/
#define GP3OCE_RVAL                    0x0

/* GP3OCE[OCE7] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE7_BBA                (*(volatile unsigned long *) 0x420C139C)
#define GP3OCE_OCE7_MSK                (0x1   << 7  )
#define GP3OCE_OCE7                    (0x1   << 7  )
#define GP3OCE_OCE7_DIS                (0x0   << 7  ) /* DIS                      */
#define GP3OCE_OCE7_EN                 (0x1   << 7  ) /* EN                       */

/* GP3OCE[OCE6] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE6_BBA                (*(volatile unsigned long *) 0x420C1398)
#define GP3OCE_OCE6_MSK                (0x1   << 6  )
#define GP3OCE_OCE6                    (0x1   << 6  )
#define GP3OCE_OCE6_DIS                (0x0   << 6  ) /* DIS                      */
#define GP3OCE_OCE6_EN                 (0x1   << 6  ) /* EN                       */

/* GP3OCE[OCE5] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE5_BBA                (*(volatile unsigned long *) 0x420C1394)
#define GP3OCE_OCE5_MSK                (0x1   << 5  )
#define GP3OCE_OCE5                    (0x1   << 5  )
#define GP3OCE_OCE5_DIS                (0x0   << 5  ) /* DIS                      */
#define GP3OCE_OCE5_EN                 (0x1   << 5  ) /* EN                       */

/* GP3OCE[OCE4] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE4_BBA                (*(volatile unsigned long *) 0x420C1390)
#define GP3OCE_OCE4_MSK                (0x1   << 4  )
#define GP3OCE_OCE4                    (0x1   << 4  )
#define GP3OCE_OCE4_DIS                (0x0   << 4  ) /* DIS                      */
#define GP3OCE_OCE4_EN                 (0x1   << 4  ) /* EN                       */

/* GP3OCE[OCE3] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE3_BBA                (*(volatile unsigned long *) 0x420C138C)
#define GP3OCE_OCE3_MSK                (0x1   << 3  )
#define GP3OCE_OCE3                    (0x1   << 3  )
#define GP3OCE_OCE3_DIS                (0x0   << 3  ) /* DIS                      */
#define GP3OCE_OCE3_EN                 (0x1   << 3  ) /* EN                       */

/* GP3OCE[OCE2] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE2_BBA                (*(volatile unsigned long *) 0x420C1388)
#define GP3OCE_OCE2_MSK                (0x1   << 2  )
#define GP3OCE_OCE2                    (0x1   << 2  )
#define GP3OCE_OCE2_DIS                (0x0   << 2  ) /* DIS                      */
#define GP3OCE_OCE2_EN                 (0x1   << 2  ) /* EN                       */

/* GP3OCE[OCE1] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE1_BBA                (*(volatile unsigned long *) 0x420C1384)
#define GP3OCE_OCE1_MSK                (0x1   << 1  )
#define GP3OCE_OCE1                    (0x1   << 1  )
#define GP3OCE_OCE1_DIS                (0x0   << 1  ) /* DIS                      */
#define GP3OCE_OCE1_EN                 (0x1   << 1  ) /* EN                       */

/* GP3OCE[OCE0] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP3OCE_OCE0_BBA                (*(volatile unsigned long *) 0x420C1380)
#define GP3OCE_OCE0_MSK                (0x1   << 0  )
#define GP3OCE_OCE0                    (0x1   << 0  )
#define GP3OCE_OCE0_DIS                (0x0   << 0  ) /* DIS                      */
#define GP3OCE_OCE0_EN                 (0x1   << 0  ) /* EN                       */

/* Reset Value for GP3IN*/
#define GP3IN_RVAL                     0xFF

/* GP3IN[IN7] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN7_BBA                  (*(volatile unsigned long *) 0x420C149C)
#define GP3IN_IN7_MSK                  (0x1   << 7  )
#define GP3IN_IN7                      (0x1   << 7  )
#define GP3IN_IN7_LOW                  (0x0   << 7  ) /* LOW                      */
#define GP3IN_IN7_HIGH                 (0x1   << 7  ) /* HIGH                     */

/* GP3IN[IN6] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN6_BBA                  (*(volatile unsigned long *) 0x420C1498)
#define GP3IN_IN6_MSK                  (0x1   << 6  )
#define GP3IN_IN6                      (0x1   << 6  )
#define GP3IN_IN6_LOW                  (0x0   << 6  ) /* LOW                      */
#define GP3IN_IN6_HIGH                 (0x1   << 6  ) /* HIGH                     */

/* GP3IN[IN5] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN5_BBA                  (*(volatile unsigned long *) 0x420C1494)
#define GP3IN_IN5_MSK                  (0x1   << 5  )
#define GP3IN_IN5                      (0x1   << 5  )
#define GP3IN_IN5_LOW                  (0x0   << 5  ) /* LOW                      */
#define GP3IN_IN5_HIGH                 (0x1   << 5  ) /* HIGH                     */

/* GP3IN[IN4] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN4_BBA                  (*(volatile unsigned long *) 0x420C1490)
#define GP3IN_IN4_MSK                  (0x1   << 4  )
#define GP3IN_IN4                      (0x1   << 4  )
#define GP3IN_IN4_LOW                  (0x0   << 4  ) /* LOW                      */
#define GP3IN_IN4_HIGH                 (0x1   << 4  ) /* HIGH                     */

/* GP3IN[IN3] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN3_BBA                  (*(volatile unsigned long *) 0x420C148C)
#define GP3IN_IN3_MSK                  (0x1   << 3  )
#define GP3IN_IN3                      (0x1   << 3  )
#define GP3IN_IN3_LOW                  (0x0   << 3  ) /* LOW                      */
#define GP3IN_IN3_HIGH                 (0x1   << 3  ) /* HIGH                     */

/* GP3IN[IN2] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN2_BBA                  (*(volatile unsigned long *) 0x420C1488)
#define GP3IN_IN2_MSK                  (0x1   << 2  )
#define GP3IN_IN2                      (0x1   << 2  )
#define GP3IN_IN2_LOW                  (0x0   << 2  ) /* LOW                      */
#define GP3IN_IN2_HIGH                 (0x1   << 2  ) /* HIGH                     */

/* GP3IN[IN1] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN1_BBA                  (*(volatile unsigned long *) 0x420C1484)
#define GP3IN_IN1_MSK                  (0x1   << 1  )
#define GP3IN_IN1                      (0x1   << 1  )
#define GP3IN_IN1_LOW                  (0x0   << 1  ) /* LOW                      */
#define GP3IN_IN1_HIGH                 (0x1   << 1  ) /* HIGH                     */

/* GP3IN[IN0] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP3IN_IN0_BBA                  (*(volatile unsigned long *) 0x420C1480)
#define GP3IN_IN0_MSK                  (0x1   << 0  )
#define GP3IN_IN0                      (0x1   << 0  )
#define GP3IN_IN0_LOW                  (0x0   << 0  ) /* LOW                      */
#define GP3IN_IN0_HIGH                 (0x1   << 0  ) /* HIGH                     */

/* Reset Value for GP3OUT*/
#define GP3OUT_RVAL                    0x0

/* GP3OUT[OUT7] - Output for port pin. */
#define GP3OUT_OUT7_BBA                (*(volatile unsigned long *) 0x420C151C)
#define GP3OUT_OUT7_MSK                (0x1   << 7  )
#define GP3OUT_OUT7                    (0x1   << 7  )
#define GP3OUT_OUT7_LOW                (0x0   << 7  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT7_HIGH               (0x1   << 7  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT6] - Output for port pin. */
#define GP3OUT_OUT6_BBA                (*(volatile unsigned long *) 0x420C1518)
#define GP3OUT_OUT6_MSK                (0x1   << 6  )
#define GP3OUT_OUT6                    (0x1   << 6  )
#define GP3OUT_OUT6_LOW                (0x0   << 6  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT6_HIGH               (0x1   << 6  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT5] - Output for port pin. */
#define GP3OUT_OUT5_BBA                (*(volatile unsigned long *) 0x420C1514)
#define GP3OUT_OUT5_MSK                (0x1   << 5  )
#define GP3OUT_OUT5                    (0x1   << 5  )
#define GP3OUT_OUT5_LOW                (0x0   << 5  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT5_HIGH               (0x1   << 5  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT4] - Output for port pin. */
#define GP3OUT_OUT4_BBA                (*(volatile unsigned long *) 0x420C1510)
#define GP3OUT_OUT4_MSK                (0x1   << 4  )
#define GP3OUT_OUT4                    (0x1   << 4  )
#define GP3OUT_OUT4_LOW                (0x0   << 4  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT4_HIGH               (0x1   << 4  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT3] - Output for port pin. */
#define GP3OUT_OUT3_BBA                (*(volatile unsigned long *) 0x420C150C)
#define GP3OUT_OUT3_MSK                (0x1   << 3  )
#define GP3OUT_OUT3                    (0x1   << 3  )
#define GP3OUT_OUT3_LOW                (0x0   << 3  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT3_HIGH               (0x1   << 3  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT2] - Output for port pin. */
#define GP3OUT_OUT2_BBA                (*(volatile unsigned long *) 0x420C1508)
#define GP3OUT_OUT2_MSK                (0x1   << 2  )
#define GP3OUT_OUT2                    (0x1   << 2  )
#define GP3OUT_OUT2_LOW                (0x0   << 2  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT2_HIGH               (0x1   << 2  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT1] - Output for port pin. */
#define GP3OUT_OUT1_BBA                (*(volatile unsigned long *) 0x420C1504)
#define GP3OUT_OUT1_MSK                (0x1   << 1  )
#define GP3OUT_OUT1                    (0x1   << 1  )
#define GP3OUT_OUT1_LOW                (0x0   << 1  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT1_HIGH               (0x1   << 1  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP3OUT[OUT0] - Output for port pin. */
#define GP3OUT_OUT0_BBA                (*(volatile unsigned long *) 0x420C1500)
#define GP3OUT_OUT0_MSK                (0x1   << 0  )
#define GP3OUT_OUT0                    (0x1   << 0  )
#define GP3OUT_OUT0_LOW                (0x0   << 0  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP3OUT_OUT0_HIGH               (0x1   << 0  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP3SET*/
#define GP3SET_RVAL                    0x0

/* GP3SET[SET7] - Set output high for corresponding port pin. */
#define GP3SET_SET7_BBA                (*(volatile unsigned long *) 0x420C159C)
#define GP3SET_SET7_MSK                (0x1   << 7  )
#define GP3SET_SET7                    (0x1   << 7  )
#define GP3SET_SET7_SET                (0x1   << 7  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET6] - Set output high for corresponding port pin. */
#define GP3SET_SET6_BBA                (*(volatile unsigned long *) 0x420C1598)
#define GP3SET_SET6_MSK                (0x1   << 6  )
#define GP3SET_SET6                    (0x1   << 6  )
#define GP3SET_SET6_SET                (0x1   << 6  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET5] - Set output high for corresponding port pin. */
#define GP3SET_SET5_BBA                (*(volatile unsigned long *) 0x420C1594)
#define GP3SET_SET5_MSK                (0x1   << 5  )
#define GP3SET_SET5                    (0x1   << 5  )
#define GP3SET_SET5_SET                (0x1   << 5  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET4] - Set output high for corresponding port pin. */
#define GP3SET_SET4_BBA                (*(volatile unsigned long *) 0x420C1590)
#define GP3SET_SET4_MSK                (0x1   << 4  )
#define GP3SET_SET4                    (0x1   << 4  )
#define GP3SET_SET4_SET                (0x1   << 4  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET3] - Set output high for corresponding port pin. */
#define GP3SET_SET3_BBA                (*(volatile unsigned long *) 0x420C158C)
#define GP3SET_SET3_MSK                (0x1   << 3  )
#define GP3SET_SET3                    (0x1   << 3  )
#define GP3SET_SET3_SET                (0x1   << 3  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET2] - Set output high for corresponding port pin. */
#define GP3SET_SET2_BBA                (*(volatile unsigned long *) 0x420C1588)
#define GP3SET_SET2_MSK                (0x1   << 2  )
#define GP3SET_SET2                    (0x1   << 2  )
#define GP3SET_SET2_SET                (0x1   << 2  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET1] - Set output high for corresponding port pin. */
#define GP3SET_SET1_BBA                (*(volatile unsigned long *) 0x420C1584)
#define GP3SET_SET1_MSK                (0x1   << 1  )
#define GP3SET_SET1                    (0x1   << 1  )
#define GP3SET_SET1_SET                (0x1   << 1  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP3SET[SET0] - Set output high for corresponding port pin. */
#define GP3SET_SET0_BBA                (*(volatile unsigned long *) 0x420C1580)
#define GP3SET_SET0_MSK                (0x1   << 0  )
#define GP3SET_SET0                    (0x1   << 0  )
#define GP3SET_SET0_SET                (0x1   << 0  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP3CLR*/
#define GP3CLR_RVAL                    0x0

/* GP3CLR[CLR7] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR7_BBA                (*(volatile unsigned long *) 0x420C161C)
#define GP3CLR_CLR7_MSK                (0x1   << 7  )
#define GP3CLR_CLR7                    (0x1   << 7  )
#define GP3CLR_CLR7_CLR                (0x1   << 7  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR6] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR6_BBA                (*(volatile unsigned long *) 0x420C1618)
#define GP3CLR_CLR6_MSK                (0x1   << 6  )
#define GP3CLR_CLR6                    (0x1   << 6  )
#define GP3CLR_CLR6_CLR                (0x1   << 6  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR5] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR5_BBA                (*(volatile unsigned long *) 0x420C1614)
#define GP3CLR_CLR5_MSK                (0x1   << 5  )
#define GP3CLR_CLR5                    (0x1   << 5  )
#define GP3CLR_CLR5_CLR                (0x1   << 5  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR4] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR4_BBA                (*(volatile unsigned long *) 0x420C1610)
#define GP3CLR_CLR4_MSK                (0x1   << 4  )
#define GP3CLR_CLR4                    (0x1   << 4  )
#define GP3CLR_CLR4_CLR                (0x1   << 4  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR3] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR3_BBA                (*(volatile unsigned long *) 0x420C160C)
#define GP3CLR_CLR3_MSK                (0x1   << 3  )
#define GP3CLR_CLR3                    (0x1   << 3  )
#define GP3CLR_CLR3_CLR                (0x1   << 3  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR2] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR2_BBA                (*(volatile unsigned long *) 0x420C1608)
#define GP3CLR_CLR2_MSK                (0x1   << 2  )
#define GP3CLR_CLR2                    (0x1   << 2  )
#define GP3CLR_CLR2_CLR                (0x1   << 2  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR1] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR1_BBA                (*(volatile unsigned long *) 0x420C1604)
#define GP3CLR_CLR1_MSK                (0x1   << 1  )
#define GP3CLR_CLR1                    (0x1   << 1  )
#define GP3CLR_CLR1_CLR                (0x1   << 1  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP3CLR[CLR0] - Set by user code to drive the corresponding GPIO low. */
#define GP3CLR_CLR0_BBA                (*(volatile unsigned long *) 0x420C1600)
#define GP3CLR_CLR0_MSK                (0x1   << 0  )
#define GP3CLR_CLR0                    (0x1   << 0  )
#define GP3CLR_CLR0_CLR                (0x1   << 0  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* Reset Value for GP3TGL*/
#define GP3TGL_RVAL                    0x0

/* GP3TGL[TGL7] - Toggle for corresponding port pin. */
#define GP3TGL_TGL7_BBA                (*(volatile unsigned long *) 0x420C169C)
#define GP3TGL_TGL7_MSK                (0x1   << 7  )
#define GP3TGL_TGL7                    (0x1   << 7  )
#define GP3TGL_TGL7_TGL                (0x1   << 7  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL6] - Toggle for corresponding port pin. */
#define GP3TGL_TGL6_BBA                (*(volatile unsigned long *) 0x420C1698)
#define GP3TGL_TGL6_MSK                (0x1   << 6  )
#define GP3TGL_TGL6                    (0x1   << 6  )
#define GP3TGL_TGL6_TGL                (0x1   << 6  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL5] - Toggle for corresponding port pin. */
#define GP3TGL_TGL5_BBA                (*(volatile unsigned long *) 0x420C1694)
#define GP3TGL_TGL5_MSK                (0x1   << 5  )
#define GP3TGL_TGL5                    (0x1   << 5  )
#define GP3TGL_TGL5_TGL                (0x1   << 5  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL4] - Toggle for corresponding port pin. */
#define GP3TGL_TGL4_BBA                (*(volatile unsigned long *) 0x420C1690)
#define GP3TGL_TGL4_MSK                (0x1   << 4  )
#define GP3TGL_TGL4                    (0x1   << 4  )
#define GP3TGL_TGL4_TGL                (0x1   << 4  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL3] - Toggle for corresponding port pin. */
#define GP3TGL_TGL3_BBA                (*(volatile unsigned long *) 0x420C168C)
#define GP3TGL_TGL3_MSK                (0x1   << 3  )
#define GP3TGL_TGL3                    (0x1   << 3  )
#define GP3TGL_TGL3_TGL                (0x1   << 3  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL2] - Toggle for corresponding port pin. */
#define GP3TGL_TGL2_BBA                (*(volatile unsigned long *) 0x420C1688)
#define GP3TGL_TGL2_MSK                (0x1   << 2  )
#define GP3TGL_TGL2                    (0x1   << 2  )
#define GP3TGL_TGL2_TGL                (0x1   << 2  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL1] - Toggle for corresponding port pin. */
#define GP3TGL_TGL1_BBA                (*(volatile unsigned long *) 0x420C1684)
#define GP3TGL_TGL1_MSK                (0x1   << 1  )
#define GP3TGL_TGL1                    (0x1   << 1  )
#define GP3TGL_TGL1_TGL                (0x1   << 1  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP3TGL[TGL0] - Toggle for corresponding port pin. */
#define GP3TGL_TGL0_BBA                (*(volatile unsigned long *) 0x420C1680)
#define GP3TGL_TGL0_MSK                (0x1   << 0  )
#define GP3TGL_TGL0                    (0x1   << 0  )
#define GP3TGL_TGL0_TGL                (0x1   << 0  ) /* TGL. Set by user code to invert the corresponding GPIO. */
#if (__NO_MMR_STRUCTS__==1)

#define          GP4CON                                     (*(volatile unsigned short int *) 0x400060C0)
#define          GP4OEN                                     (*(volatile unsigned char      *) 0x400060C4)
#define          GP4PUL                                     (*(volatile unsigned char      *) 0x400060C8)
#define          GP4OCE                                     (*(volatile unsigned char      *) 0x400060CC)
#define          GP4IN                                      (*(volatile unsigned char      *) 0x400060D4)
#define          GP4OUT                                     (*(volatile unsigned char      *) 0x400060D8)
#define          GP4SET                                     (*(volatile unsigned char      *) 0x400060DC)
#define          GP4CLR                                     (*(volatile unsigned char      *) 0x400060E0)
#define          GP4TGL                                     (*(volatile unsigned char      *) 0x400060E4)
#endif // (__NO_MMR_STRUCTS__==1)

/* Reset Value for GP4CON*/
#define GP4CON_RVAL                    0x0

/* GP4CON[CON7] - Configuration bits for P4.7 */
#define GP4CON_CON7_MSK                (0x3   << 14 )
#define GP4CON_CON7_GPIO               (0x1   << 14 ) /* GPIO                     */
#define GP4CON_CON7_PWM7               (0x2   << 14 ) /* PWM7                     */

/* GP4CON[CON6] - Configuration bits for P4.6 */
#define GP4CON_CON6_MSK                (0x3   << 12 )
#define GP4CON_CON6_GPIO               (0x1   << 12 ) /* GPIO                     */
#define GP4CON_CON6_PWM6               (0x2   << 12 ) /* PWM6                     */

/* GP4CON[CON5] - Configuration bits for P4.5 */
#define GP4CON_CON5_MSK                (0x3   << 10 )
#define GP4CON_CON5_GPIO               (0x1   << 10 ) /* GPIO                     */
#define GP4CON_CON5_PWM5               (0x2   << 10 ) /* PWM5                     */

/* GP4CON[CON4] - Configuration bits for P4.4 */
#define GP4CON_CON4_MSK                (0x3   << 8  )
#define GP4CON_CON4_GPIO               (0x1   << 8  ) /* GPIO                     */
#define GP4CON_CON4_PWM4               (0x2   << 8  ) /* PWM4                     */

/* GP4CON[CON3] - Configuration bits for P4.3 */
#define GP4CON_CON3_MSK                (0x3   << 6  )
#define GP4CON_CON3_GPIO               (0x1   << 6  ) /* GPIO                     */
#define GP4CON_CON3_PWM3               (0x2   << 6  ) /* PWM3                     */

/* GP4CON[CON2] - Configuration bits for P4.2 */
#define GP4CON_CON2_MSK                (0x3   << 4  )
#define GP4CON_CON2_GPIO               (0x1   << 4  ) /* GPIO                     */
#define GP4CON_CON2_PWM2               (0x2   << 4  ) /* PWM2                     */
#define GP4CON_CON2_SPI0CS             (0x3   << 4  ) /* SPI0CS                   */

/* GP4CON[CON1] - Configuration bits for P4.1 */
#define GP4CON_CON1_MSK                (0x3   << 2  )
#define GP4CON_CON1_GPIO               (0x1   << 2  ) /* GPIO                     */
#define GP4CON_CON1_PWM1               (0x2   << 2  ) /* PWM1                     */

/* GP4CON[CON0] - Configuration bits for P4.0 */
#define GP4CON_CON0_MSK                (0x3   << 0  )
#define GP4CON_CON0_GPIO               (0x1   << 0  ) /* GPIO                     */
#define GP4CON_CON0_PWM0               (0x2   << 0  ) /* PWM0                     */

/* Reset Value for GP4OEN*/
#define GP4OEN_RVAL                    0x0

/* GP4OEN[OEN7] - Port pin direction. */
#define GP4OEN_OEN7_BBA                (*(volatile unsigned long *) 0x420C189C)
#define GP4OEN_OEN7_MSK                (0x1   << 7  )
#define GP4OEN_OEN7                    (0x1   << 7  )
#define GP4OEN_OEN7_IN                 (0x0   << 7  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN7_OUT                (0x1   << 7  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN6] - Port pin direction. */
#define GP4OEN_OEN6_BBA                (*(volatile unsigned long *) 0x420C1898)
#define GP4OEN_OEN6_MSK                (0x1   << 6  )
#define GP4OEN_OEN6                    (0x1   << 6  )
#define GP4OEN_OEN6_IN                 (0x0   << 6  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN6_OUT                (0x1   << 6  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN5] - Port pin direction. */
#define GP4OEN_OEN5_BBA                (*(volatile unsigned long *) 0x420C1894)
#define GP4OEN_OEN5_MSK                (0x1   << 5  )
#define GP4OEN_OEN5                    (0x1   << 5  )
#define GP4OEN_OEN5_IN                 (0x0   << 5  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN5_OUT                (0x1   << 5  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN4] - Port pin direction. */
#define GP4OEN_OEN4_BBA                (*(volatile unsigned long *) 0x420C1890)
#define GP4OEN_OEN4_MSK                (0x1   << 4  )
#define GP4OEN_OEN4                    (0x1   << 4  )
#define GP4OEN_OEN4_IN                 (0x0   << 4  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN4_OUT                (0x1   << 4  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN3] - Port pin direction. */
#define GP4OEN_OEN3_BBA                (*(volatile unsigned long *) 0x420C188C)
#define GP4OEN_OEN3_MSK                (0x1   << 3  )
#define GP4OEN_OEN3                    (0x1   << 3  )
#define GP4OEN_OEN3_IN                 (0x0   << 3  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN3_OUT                (0x1   << 3  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN2] - Port pin direction. */
#define GP4OEN_OEN2_BBA                (*(volatile unsigned long *) 0x420C1888)
#define GP4OEN_OEN2_MSK                (0x1   << 2  )
#define GP4OEN_OEN2                    (0x1   << 2  )
#define GP4OEN_OEN2_IN                 (0x0   << 2  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN2_OUT                (0x1   << 2  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN1] - Port pin direction. */
#define GP4OEN_OEN1_BBA                (*(volatile unsigned long *) 0x420C1884)
#define GP4OEN_OEN1_MSK                (0x1   << 1  )
#define GP4OEN_OEN1                    (0x1   << 1  )
#define GP4OEN_OEN1_IN                 (0x0   << 1  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN1_OUT                (0x1   << 1  ) /* OUT. Enables the output on corresponding port pin. */

/* GP4OEN[OEN0] - Port pin direction. */
#define GP4OEN_OEN0_BBA                (*(volatile unsigned long *) 0x420C1880)
#define GP4OEN_OEN0_MSK                (0x1   << 0  )
#define GP4OEN_OEN0                    (0x1   << 0  )
#define GP4OEN_OEN0_IN                 (0x0   << 0  ) /* IN. Configures pin as an input.  Disables the output on corresponding port pin. */
#define GP4OEN_OEN0_OUT                (0x1   << 0  ) /* OUT. Enables the output on corresponding port pin. */

/* Reset Value for GP4PUL*/
#define GP4PUL_RVAL                    0xFF

/* GP4PUL[PUL7] - Pull Up Enable for port pin. */
#define GP4PUL_PUL7_BBA                (*(volatile unsigned long *) 0x420C191C)
#define GP4PUL_PUL7_MSK                (0x1   << 7  )
#define GP4PUL_PUL7                    (0x1   << 7  )
#define GP4PUL_PUL7_DIS                (0x0   << 7  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL7_EN                 (0x1   << 7  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL6] - Pull Up Enable for port pin. */
#define GP4PUL_PUL6_BBA                (*(volatile unsigned long *) 0x420C1918)
#define GP4PUL_PUL6_MSK                (0x1   << 6  )
#define GP4PUL_PUL6                    (0x1   << 6  )
#define GP4PUL_PUL6_DIS                (0x0   << 6  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL6_EN                 (0x1   << 6  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL5] - Pull Up Enable for port pin. */
#define GP4PUL_PUL5_BBA                (*(volatile unsigned long *) 0x420C1914)
#define GP4PUL_PUL5_MSK                (0x1   << 5  )
#define GP4PUL_PUL5                    (0x1   << 5  )
#define GP4PUL_PUL5_DIS                (0x0   << 5  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL5_EN                 (0x1   << 5  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL4] - Pull Up Enable for port pin. */
#define GP4PUL_PUL4_BBA                (*(volatile unsigned long *) 0x420C1910)
#define GP4PUL_PUL4_MSK                (0x1   << 4  )
#define GP4PUL_PUL4                    (0x1   << 4  )
#define GP4PUL_PUL4_DIS                (0x0   << 4  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL4_EN                 (0x1   << 4  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL3] - Pull Up Enable for port pin. */
#define GP4PUL_PUL3_BBA                (*(volatile unsigned long *) 0x420C190C)
#define GP4PUL_PUL3_MSK                (0x1   << 3  )
#define GP4PUL_PUL3                    (0x1   << 3  )
#define GP4PUL_PUL3_DIS                (0x0   << 3  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL3_EN                 (0x1   << 3  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL2] - Pull Up Enable for port pin. */
#define GP4PUL_PUL2_BBA                (*(volatile unsigned long *) 0x420C1908)
#define GP4PUL_PUL2_MSK                (0x1   << 2  )
#define GP4PUL_PUL2                    (0x1   << 2  )
#define GP4PUL_PUL2_DIS                (0x0   << 2  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL2_EN                 (0x1   << 2  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL1] - Pull Up Enable for port pin. */
#define GP4PUL_PUL1_BBA                (*(volatile unsigned long *) 0x420C1904)
#define GP4PUL_PUL1_MSK                (0x1   << 1  )
#define GP4PUL_PUL1                    (0x1   << 1  )
#define GP4PUL_PUL1_DIS                (0x0   << 1  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL1_EN                 (0x1   << 1  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* GP4PUL[PUL0] - Pull Up Enable for port pin. */
#define GP4PUL_PUL0_BBA                (*(volatile unsigned long *) 0x420C1900)
#define GP4PUL_PUL0_MSK                (0x1   << 0  )
#define GP4PUL_PUL0                    (0x1   << 0  )
#define GP4PUL_PUL0_DIS                (0x0   << 0  ) /* DIS.  Disables the internal pull up on corresponding port pin. */
#define GP4PUL_PUL0_EN                 (0x1   << 0  ) /* EN. Enables the internal pull up on corresponding port pin. */

/* Reset Value for GP4OCE*/
#define GP4OCE_RVAL                    0x0

/* GP4OCE[OCE7] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE7_BBA                (*(volatile unsigned long *) 0x420C199C)
#define GP4OCE_OCE7_MSK                (0x1   << 7  )
#define GP4OCE_OCE7                    (0x1   << 7  )
#define GP4OCE_OCE7_DIS                (0x0   << 7  ) /* DIS                      */
#define GP4OCE_OCE7_EN                 (0x1   << 7  ) /* EN                       */

/* GP4OCE[OCE6] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE6_BBA                (*(volatile unsigned long *) 0x420C1998)
#define GP4OCE_OCE6_MSK                (0x1   << 6  )
#define GP4OCE_OCE6                    (0x1   << 6  )
#define GP4OCE_OCE6_DIS                (0x0   << 6  ) /* DIS                      */
#define GP4OCE_OCE6_EN                 (0x1   << 6  ) /* EN                       */

/* GP4OCE[OCE5] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE5_BBA                (*(volatile unsigned long *) 0x420C1994)
#define GP4OCE_OCE5_MSK                (0x1   << 5  )
#define GP4OCE_OCE5                    (0x1   << 5  )
#define GP4OCE_OCE5_DIS                (0x0   << 5  ) /* DIS                      */
#define GP4OCE_OCE5_EN                 (0x1   << 5  ) /* EN                       */

/* GP4OCE[OCE4] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE4_BBA                (*(volatile unsigned long *) 0x420C1990)
#define GP4OCE_OCE4_MSK                (0x1   << 4  )
#define GP4OCE_OCE4                    (0x1   << 4  )
#define GP4OCE_OCE4_DIS                (0x0   << 4  ) /* DIS                      */
#define GP4OCE_OCE4_EN                 (0x1   << 4  ) /* EN                       */

/* GP4OCE[OCE3] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE3_BBA                (*(volatile unsigned long *) 0x420C198C)
#define GP4OCE_OCE3_MSK                (0x1   << 3  )
#define GP4OCE_OCE3                    (0x1   << 3  )
#define GP4OCE_OCE3_DIS                (0x0   << 3  ) /* DIS                      */
#define GP4OCE_OCE3_EN                 (0x1   << 3  ) /* EN                       */

/* GP4OCE[OCE2] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE2_BBA                (*(volatile unsigned long *) 0x420C1988)
#define GP4OCE_OCE2_MSK                (0x1   << 2  )
#define GP4OCE_OCE2                    (0x1   << 2  )
#define GP4OCE_OCE2_DIS                (0x0   << 2  ) /* DIS                      */
#define GP4OCE_OCE2_EN                 (0x1   << 2  ) /* EN                       */

/* GP4OCE[OCE1] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE1_BBA                (*(volatile unsigned long *) 0x420C1984)
#define GP4OCE_OCE1_MSK                (0x1   << 1  )
#define GP4OCE_OCE1                    (0x1   << 1  )
#define GP4OCE_OCE1_DIS                (0x0   << 1  ) /* DIS                      */
#define GP4OCE_OCE1_EN                 (0x1   << 1  ) /* EN                       */

/* GP4OCE[OCE0] - Output enable. Sets the GPIO pads on corresponding port to open circuit  mode. */
#define GP4OCE_OCE0_BBA                (*(volatile unsigned long *) 0x420C1980)
#define GP4OCE_OCE0_MSK                (0x1   << 0  )
#define GP4OCE_OCE0                    (0x1   << 0  )
#define GP4OCE_OCE0_DIS                (0x0   << 0  ) /* DIS                      */
#define GP4OCE_OCE0_EN                 (0x1   << 0  ) /* EN                       */

/* Reset Value for GP4IN*/
#define GP4IN_RVAL                     0xFF

/* GP4IN[IN7] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN7_BBA                  (*(volatile unsigned long *) 0x420C1A9C)
#define GP4IN_IN7_MSK                  (0x1   << 7  )
#define GP4IN_IN7                      (0x1   << 7  )
#define GP4IN_IN7_LOW                  (0x0   << 7  ) /* LOW                      */
#define GP4IN_IN7_HIGH                 (0x1   << 7  ) /* HIGH                     */

/* GP4IN[IN6] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN6_BBA                  (*(volatile unsigned long *) 0x420C1A98)
#define GP4IN_IN6_MSK                  (0x1   << 6  )
#define GP4IN_IN6                      (0x1   << 6  )
#define GP4IN_IN6_LOW                  (0x0   << 6  ) /* LOW                      */
#define GP4IN_IN6_HIGH                 (0x1   << 6  ) /* HIGH                     */

/* GP4IN[IN5] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN5_BBA                  (*(volatile unsigned long *) 0x420C1A94)
#define GP4IN_IN5_MSK                  (0x1   << 5  )
#define GP4IN_IN5                      (0x1   << 5  )
#define GP4IN_IN5_LOW                  (0x0   << 5  ) /* LOW                      */
#define GP4IN_IN5_HIGH                 (0x1   << 5  ) /* HIGH                     */

/* GP4IN[IN4] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN4_BBA                  (*(volatile unsigned long *) 0x420C1A90)
#define GP4IN_IN4_MSK                  (0x1   << 4  )
#define GP4IN_IN4                      (0x1   << 4  )
#define GP4IN_IN4_LOW                  (0x0   << 4  ) /* LOW                      */
#define GP4IN_IN4_HIGH                 (0x1   << 4  ) /* HIGH                     */

/* GP4IN[IN3] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN3_BBA                  (*(volatile unsigned long *) 0x420C1A8C)
#define GP4IN_IN3_MSK                  (0x1   << 3  )
#define GP4IN_IN3                      (0x1   << 3  )
#define GP4IN_IN3_LOW                  (0x0   << 3  ) /* LOW                      */
#define GP4IN_IN3_HIGH                 (0x1   << 3  ) /* HIGH                     */

/* GP4IN[IN2] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN2_BBA                  (*(volatile unsigned long *) 0x420C1A88)
#define GP4IN_IN2_MSK                  (0x1   << 2  )
#define GP4IN_IN2                      (0x1   << 2  )
#define GP4IN_IN2_LOW                  (0x0   << 2  ) /* LOW                      */
#define GP4IN_IN2_HIGH                 (0x1   << 2  ) /* HIGH                     */

/* GP4IN[IN1] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN1_BBA                  (*(volatile unsigned long *) 0x420C1A84)
#define GP4IN_IN1_MSK                  (0x1   << 1  )
#define GP4IN_IN1                      (0x1   << 1  )
#define GP4IN_IN1_LOW                  (0x0   << 1  ) /* LOW                      */
#define GP4IN_IN1_HIGH                 (0x1   << 1  ) /* HIGH                     */

/* GP4IN[IN0] - Reflects the level on the corresponding GPIO pins except when in configured in open circuit. */
#define GP4IN_IN0_BBA                  (*(volatile unsigned long *) 0x420C1A80)
#define GP4IN_IN0_MSK                  (0x1   << 0  )
#define GP4IN_IN0                      (0x1   << 0  )
#define GP4IN_IN0_LOW                  (0x0   << 0  ) /* LOW                      */
#define GP4IN_IN0_HIGH                 (0x1   << 0  ) /* HIGH                     */

/* Reset Value for GP4OUT*/
#define GP4OUT_RVAL                    0x0

/* GP4OUT[OUT7] - Output for port pin. */
#define GP4OUT_OUT7_BBA                (*(volatile unsigned long *) 0x420C1B1C)
#define GP4OUT_OUT7_MSK                (0x1   << 7  )
#define GP4OUT_OUT7                    (0x1   << 7  )
#define GP4OUT_OUT7_LOW                (0x0   << 7  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT7_HIGH               (0x1   << 7  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT6] - Output for port pin. */
#define GP4OUT_OUT6_BBA                (*(volatile unsigned long *) 0x420C1B18)
#define GP4OUT_OUT6_MSK                (0x1   << 6  )
#define GP4OUT_OUT6                    (0x1   << 6  )
#define GP4OUT_OUT6_LOW                (0x0   << 6  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT6_HIGH               (0x1   << 6  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT5] - Output for port pin. */
#define GP4OUT_OUT5_BBA                (*(volatile unsigned long *) 0x420C1B14)
#define GP4OUT_OUT5_MSK                (0x1   << 5  )
#define GP4OUT_OUT5                    (0x1   << 5  )
#define GP4OUT_OUT5_LOW                (0x0   << 5  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT5_HIGH               (0x1   << 5  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT4] - Output for port pin. */
#define GP4OUT_OUT4_BBA                (*(volatile unsigned long *) 0x420C1B10)
#define GP4OUT_OUT4_MSK                (0x1   << 4  )
#define GP4OUT_OUT4                    (0x1   << 4  )
#define GP4OUT_OUT4_LOW                (0x0   << 4  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT4_HIGH               (0x1   << 4  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT3] - Output for port pin. */
#define GP4OUT_OUT3_BBA                (*(volatile unsigned long *) 0x420C1B0C)
#define GP4OUT_OUT3_MSK                (0x1   << 3  )
#define GP4OUT_OUT3                    (0x1   << 3  )
#define GP4OUT_OUT3_LOW                (0x0   << 3  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT3_HIGH               (0x1   << 3  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT2] - Output for port pin. */
#define GP4OUT_OUT2_BBA                (*(volatile unsigned long *) 0x420C1B08)
#define GP4OUT_OUT2_MSK                (0x1   << 2  )
#define GP4OUT_OUT2                    (0x1   << 2  )
#define GP4OUT_OUT2_LOW                (0x0   << 2  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT2_HIGH               (0x1   << 2  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT1] - Output for port pin. */
#define GP4OUT_OUT1_BBA                (*(volatile unsigned long *) 0x420C1B04)
#define GP4OUT_OUT1_MSK                (0x1   << 1  )
#define GP4OUT_OUT1                    (0x1   << 1  )
#define GP4OUT_OUT1_LOW                (0x0   << 1  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT1_HIGH               (0x1   << 1  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* GP4OUT[OUT0] - Output for port pin. */
#define GP4OUT_OUT0_BBA                (*(volatile unsigned long *) 0x420C1B00)
#define GP4OUT_OUT0_MSK                (0x1   << 0  )
#define GP4OUT_OUT0                    (0x1   << 0  )
#define GP4OUT_OUT0_LOW                (0x0   << 0  ) /* LOW. Cleared by user to drive the corresponding GPIO low. */
#define GP4OUT_OUT0_HIGH               (0x1   << 0  ) /* HIGH. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP4SET*/
#define GP4SET_RVAL                    0x0

/* GP4SET[SET7] - Set output high for corresponding port pin. */
#define GP4SET_SET7_BBA                (*(volatile unsigned long *) 0x420C1B9C)
#define GP4SET_SET7_MSK                (0x1   << 7  )
#define GP4SET_SET7                    (0x1   << 7  )
#define GP4SET_SET7_SET                (0x1   << 7  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET6] - Set output high for corresponding port pin. */
#define GP4SET_SET6_BBA                (*(volatile unsigned long *) 0x420C1B98)
#define GP4SET_SET6_MSK                (0x1   << 6  )
#define GP4SET_SET6                    (0x1   << 6  )
#define GP4SET_SET6_SET                (0x1   << 6  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET5] - Set output high for corresponding port pin. */
#define GP4SET_SET5_BBA                (*(volatile unsigned long *) 0x420C1B94)
#define GP4SET_SET5_MSK                (0x1   << 5  )
#define GP4SET_SET5                    (0x1   << 5  )
#define GP4SET_SET5_SET                (0x1   << 5  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET4] - Set output high for corresponding port pin. */
#define GP4SET_SET4_BBA                (*(volatile unsigned long *) 0x420C1B90)
#define GP4SET_SET4_MSK                (0x1   << 4  )
#define GP4SET_SET4                    (0x1   << 4  )
#define GP4SET_SET4_SET                (0x1   << 4  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET3] - Set output high for corresponding port pin. */
#define GP4SET_SET3_BBA                (*(volatile unsigned long *) 0x420C1B8C)
#define GP4SET_SET3_MSK                (0x1   << 3  )
#define GP4SET_SET3                    (0x1   << 3  )
#define GP4SET_SET3_SET                (0x1   << 3  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET2] - Set output high for corresponding port pin. */
#define GP4SET_SET2_BBA                (*(volatile unsigned long *) 0x420C1B88)
#define GP4SET_SET2_MSK                (0x1   << 2  )
#define GP4SET_SET2                    (0x1   << 2  )
#define GP4SET_SET2_SET                (0x1   << 2  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET1] - Set output high for corresponding port pin. */
#define GP4SET_SET1_BBA                (*(volatile unsigned long *) 0x420C1B84)
#define GP4SET_SET1_MSK                (0x1   << 1  )
#define GP4SET_SET1                    (0x1   << 1  )
#define GP4SET_SET1_SET                (0x1   << 1  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* GP4SET[SET0] - Set output high for corresponding port pin. */
#define GP4SET_SET0_BBA                (*(volatile unsigned long *) 0x420C1B80)
#define GP4SET_SET0_MSK                (0x1   << 0  )
#define GP4SET_SET0                    (0x1   << 0  )
#define GP4SET_SET0_SET                (0x1   << 0  ) /* SET. Set by user code to drive the corresponding GPIO high. */

/* Reset Value for GP4CLR*/
#define GP4CLR_RVAL                    0x0

/* GP4CLR[CLR7] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR7_BBA                (*(volatile unsigned long *) 0x420C1C1C)
#define GP4CLR_CLR7_MSK                (0x1   << 7  )
#define GP4CLR_CLR7                    (0x1   << 7  )
#define GP4CLR_CLR7_CLR                (0x1   << 7  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR6] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR6_BBA                (*(volatile unsigned long *) 0x420C1C18)
#define GP4CLR_CLR6_MSK                (0x1   << 6  )
#define GP4CLR_CLR6                    (0x1   << 6  )
#define GP4CLR_CLR6_CLR                (0x1   << 6  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR5] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR5_BBA                (*(volatile unsigned long *) 0x420C1C14)
#define GP4CLR_CLR5_MSK                (0x1   << 5  )
#define GP4CLR_CLR5                    (0x1   << 5  )
#define GP4CLR_CLR5_CLR                (0x1   << 5  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR4] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR4_BBA                (*(volatile unsigned long *) 0x420C1C10)
#define GP4CLR_CLR4_MSK                (0x1   << 4  )
#define GP4CLR_CLR4                    (0x1   << 4  )
#define GP4CLR_CLR4_CLR                (0x1   << 4  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR3] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR3_BBA                (*(volatile unsigned long *) 0x420C1C0C)
#define GP4CLR_CLR3_MSK                (0x1   << 3  )
#define GP4CLR_CLR3                    (0x1   << 3  )
#define GP4CLR_CLR3_CLR                (0x1   << 3  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR2] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR2_BBA                (*(volatile unsigned long *) 0x420C1C08)
#define GP4CLR_CLR2_MSK                (0x1   << 2  )
#define GP4CLR_CLR2                    (0x1   << 2  )
#define GP4CLR_CLR2_CLR                (0x1   << 2  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR1] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR1_BBA                (*(volatile unsigned long *) 0x420C1C04)
#define GP4CLR_CLR1_MSK                (0x1   << 1  )
#define GP4CLR_CLR1                    (0x1   << 1  )
#define GP4CLR_CLR1_CLR                (0x1   << 1  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* GP4CLR[CLR0] - Set by user code to drive the corresponding GPIO low. */
#define GP4CLR_CLR0_BBA                (*(volatile unsigned long *) 0x420C1C00)
#define GP4CLR_CLR0_MSK                (0x1   << 0  )
#define GP4CLR_CLR0                    (0x1   << 0  )
#define GP4CLR_CLR0_CLR                (0x1   << 0  ) /* CLR. Set by user code to drive the corresponding GPIO low. */

/* Reset Value for GP4TGL*/
#define GP4TGL_RVAL                    0x0

/* GP4TGL[TGL7] - Toggle for corresponding port pin. */
#define GP4TGL_TGL7_BBA                (*(volatile unsigned long *) 0x420C1C9C)
#define GP4TGL_TGL7_MSK                (0x1   << 7  )
#define GP4TGL_TGL7                    (0x1   << 7  )
#define GP4TGL_TGL7_TGL                (0x1   << 7  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL6] - Toggle for corresponding port pin. */
#define GP4TGL_TGL6_BBA                (*(volatile unsigned long *) 0x420C1C98)
#define GP4TGL_TGL6_MSK                (0x1   << 6  )
#define GP4TGL_TGL6                    (0x1   << 6  )
#define GP4TGL_TGL6_TGL                (0x1   << 6  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL5] - Toggle for corresponding port pin. */
#define GP4TGL_TGL5_BBA                (*(volatile unsigned long *) 0x420C1C94)
#define GP4TGL_TGL5_MSK                (0x1   << 5  )
#define GP4TGL_TGL5                    (0x1   << 5  )
#define GP4TGL_TGL5_TGL                (0x1   << 5  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL4] - Toggle for corresponding port pin. */
#define GP4TGL_TGL4_BBA                (*(volatile unsigned long *) 0x420C1C90)
#define GP4TGL_TGL4_MSK                (0x1   << 4  )
#define GP4TGL_TGL4                    (0x1   << 4  )
#define GP4TGL_TGL4_TGL                (0x1   << 4  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL3] - Toggle for corresponding port pin. */
#define GP4TGL_TGL3_BBA                (*(volatile unsigned long *) 0x420C1C8C)
#define GP4TGL_TGL3_MSK                (0x1   << 3  )
#define GP4TGL_TGL3                    (0x1   << 3  )
#define GP4TGL_TGL3_TGL                (0x1   << 3  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL2] - Toggle for corresponding port pin. */
#define GP4TGL_TGL2_BBA                (*(volatile unsigned long *) 0x420C1C88)
#define GP4TGL_TGL2_MSK                (0x1   << 2  )
#define GP4TGL_TGL2                    (0x1   << 2  )
#define GP4TGL_TGL2_TGL                (0x1   << 2  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL1] - Toggle for corresponding port pin. */
#define GP4TGL_TGL1_BBA                (*(volatile unsigned long *) 0x420C1C84)
#define GP4TGL_TGL1_MSK                (0x1   << 1  )
#define GP4TGL_TGL1                    (0x1   << 1  )
#define GP4TGL_TGL1_TGL                (0x1   << 1  ) /* TGL. Set by user code to invert the corresponding GPIO. */

/* GP4TGL[TGL0] - Toggle for corresponding port pin. */
#define GP4TGL_TGL0_BBA                (*(volatile unsigned long *) 0x420C1C80)
#define GP4TGL_TGL0_MSK                (0x1   << 0  )
#define GP4TGL_TGL0                    (0x1   << 0  )
#define GP4TGL_TGL0_TGL                (0x1   << 0  ) /* TGL. Set by user code to invert the corresponding GPIO. */
// ------------------------------------------------------------------------------------------------
// -----                                        GPIOCMN                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief General Purpose Input Output (pADI_GPIOCMN)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_GPIOCMN Structure                 */
  __IO uint8_t   GPDWN;                     /*!< GPIO P3.4 Pull Down Control           */
} ADI_GPIOCMN_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          GPDWN                                      (*(volatile unsigned char      *) 0x400060F0)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for GPDWN*/
#define GPDWN_RVAL                     0x1

/* GPDWN[DWN1] - Pull down resistor control bit */
#define GPDWN_DWN1_BBA                 (*(volatile unsigned long *) 0x420C1E04)
#define GPDWN_DWN1_MSK                 (0x1   << 1  )
#define GPDWN_DWN1                     (0x1   << 1  )
#define GPDWN_DWN1_EN                  (0x0   << 1  ) /* EN to enable the pull down resistor on P3.4 by software. The hardware only enables this pull down automatically at power up. */
#define GPDWN_DWN1_DIS                 (0x1   << 1  ) /* DIS to disable the pull down resistor on P3.4. Disabled automatically by hardware if GP3PUL[4] =1 or if GP3OEN[4]=1. */
// ------------------------------------------------------------------------------------------------
// -----                                        MISC                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief General Purpose Input Output (pADI_MISC)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_MISC Structure                    */
  __I  uint32_t  RESERVED0;
  __IO uint16_t  RFTST;                     /*!< Internal Radio Test Mode Access       */
  __I  uint16_t  RESERVED1[5];
  __IO uint8_t   SWACT;                     /*!< Serial Wire Activity Register         */
} ADI_MISC_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          RFTST                                      (*(volatile unsigned short int *) 0x40008824)
#define          SWACT                                      (*(volatile unsigned char      *) 0x40008830)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for RFTST*/
#define RFTST_RVAL                     0x0

/* RFTST[DIR] - Controls the pin direction in RF test mode. */
#define RFTST_DIR_MSK                  (0x7FF << 5  )

/* RFTST[AN1] - Enable RF Analog test 2 mode. */
#define RFTST_AN1_BBA                  (*(volatile unsigned long *) 0x4211048C)
#define RFTST_AN1_MSK                  (0x1   << 3  )
#define RFTST_AN1                      (0x1   << 3  )
#define RFTST_AN1_DIS                  (0x0   << 3  ) /* DIS                      */
#define RFTST_AN1_EN                   (0x1   << 3  ) /* EN                       */

/* RFTST[AN0] - Enable RF Analog test mode. */
#define RFTST_AN0_BBA                  (*(volatile unsigned long *) 0x42110488)
#define RFTST_AN0_MSK                  (0x1   << 2  )
#define RFTST_AN0                      (0x1   << 2  )
#define RFTST_AN0_DIS                  (0x0   << 2  ) /* DIS                      */
#define RFTST_AN0_EN                   (0x1   << 2  ) /* EN                       */

/* RFTST[SPI0] - Enable the internal SPI0 signals to P0.0, P0.1, P0.2 and P0.3. */
#define RFTST_SPI0_BBA                 (*(volatile unsigned long *) 0x42110484)
#define RFTST_SPI0_MSK                 (0x1   << 1  )
#define RFTST_SPI0                     (0x1   << 1  )
#define RFTST_SPI0_DIS                 (0x0   << 1  ) /* DIS                      */
#define RFTST_SPI0_EN                  (0x1   << 1  ) /* EN                       */

/* RFTST[GPX] - Connect the internal GPIOs GP0-GP5 of the RF transceiver to external GPIOs P0.6, P0.7, P1.0, P1.1, P1.4, P1.5. */
#define RFTST_GPX_BBA                  (*(volatile unsigned long *) 0x42110480)
#define RFTST_GPX_MSK                  (0x1   << 0  )
#define RFTST_GPX                      (0x1   << 0  )
#define RFTST_GPX_DIS                  (0x0   << 0  ) /* DIS                      */
#define RFTST_GPX_EN                   (0x1   << 0  ) /* EN                       */

/* Reset Value for SWACT*/
#define SWACT_RVAL                     0x0

/* SWACT[ACT] - Serial Wire Activity */
#define SWACT_ACT_BBA                  (*(volatile unsigned long *) 0x42110600)
#define SWACT_ACT_MSK                  (0x1   << 0  )
#define SWACT_ACT                      (0x1   << 0  )
#define SWACT_ACT_DIS                  (0x0   << 0  ) /* DIS                      */
#define SWACT_ACT_EN                   (0x1   << 0  ) /* EN                       */
// ------------------------------------------------------------------------------------------------
// -----                                        I2C                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief I2C (pADI_I2C)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_I2C Structure                     */
  __IO uint16_t  I2CMCON;                   /*!< Master Control Register               */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  I2CMSTA;                   /*!< Master Status Register                */
  __I  uint16_t  RESERVED1;
  __IO uint16_t  I2CMRX;                    /*!< Master Receive Data Register          */
  __I  uint16_t  RESERVED2;
  __IO uint16_t  I2CMTX;                    /*!< Master Transmit Data Register         */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  I2CMRXCNT;                 /*!< Master Receive Data Count Register    */
  __I  uint16_t  RESERVED4;
  __IO uint16_t  I2CMCRXCNT;                /*!< Master Current Receive Data Count Register */
  __I  uint16_t  RESERVED5;
  __IO uint8_t   I2CADR0;                   /*!< First Master Address Byte Register    */
  __I  uint8_t   RESERVED6[3];
  __IO uint8_t   I2CADR1;                   /*!< Second Master Address Byte Register   */
  __I  uint8_t   RESERVED7[7];
  __IO uint16_t  I2CDIV;                    /*!< Serial Clock Period Divisor Register  */
  __I  uint16_t  RESERVED8;
  __IO uint16_t  I2CSCON;                   /*!< Slave Control Register                */
  __I  uint16_t  RESERVED9;
  __IO uint16_t  I2CSSTA;                   /*!< Slave I2C Status, Error and Interrupt Register */
  __I  uint16_t  RESERVED10;
  __IO uint16_t  I2CSRX;                    /*!< Slave Receive Data Register           */
  __I  uint16_t  RESERVED11;
  __IO uint16_t  I2CSTX;                    /*!< Slave Transmit Data Register          */
  __I  uint16_t  RESERVED12;
  __IO uint16_t  I2CALT;                    /*!< Hardware General Call ID Register     */
  __I  uint16_t  RESERVED13;
  __IO uint16_t  I2CID0;                    /*!< First Slave Address Device ID         */
  __I  uint16_t  RESERVED14;
  __IO uint16_t  I2CID1;                    /*!< Second Slave Address Device ID        */
  __I  uint16_t  RESERVED15;
  __IO uint16_t  I2CID2;                    /*!< Third Slave Address Device ID         */
  __I  uint16_t  RESERVED16;
  __IO uint16_t  I2CID3;                    /*!< Fourth Slave Address Device ID        */
  __I  uint16_t  RESERVED17;
  __IO uint16_t  I2CFSTA;                   /*!< Master and Slave Rx/Tx FIFO Status Register */
} ADI_I2C_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          I2CMCON                                    (*(volatile unsigned short int *) 0x40003000)
#define          I2CMSTA                                    (*(volatile unsigned short int *) 0x40003004)
#define          I2CMRX                                     (*(volatile unsigned short int *) 0x40003008)
#define          I2CMTX                                     (*(volatile unsigned short int *) 0x4000300C)
#define          I2CMRXCNT                                  (*(volatile unsigned short int *) 0x40003010)
#define          I2CMCRXCNT                                 (*(volatile unsigned short int *) 0x40003014)
#define          I2CADR0                                    (*(volatile unsigned char      *) 0x40003018)
#define          I2CADR1                                    (*(volatile unsigned char      *) 0x4000301C)
#define          I2CDIV                                     (*(volatile unsigned short int *) 0x40003024)
#define          I2CSCON                                    (*(volatile unsigned short int *) 0x40003028)
#define          I2CSSTA                                    (*(volatile unsigned short int *) 0x4000302C)
#define          I2CSRX                                     (*(volatile unsigned short int *) 0x40003030)
#define          I2CSTX                                     (*(volatile unsigned short int *) 0x40003034)
#define          I2CALT                                     (*(volatile unsigned short int *) 0x40003038)
#define          I2CID0                                     (*(volatile unsigned short int *) 0x4000303C)
#define          I2CID1                                     (*(volatile unsigned short int *) 0x40003040)
#define          I2CID2                                     (*(volatile unsigned short int *) 0x40003044)
#define          I2CID3                                     (*(volatile unsigned short int *) 0x40003048)
#define          I2CFSTA                                    (*(volatile unsigned short int *) 0x4000304C)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for I2CMCON*/
#define I2CMCON_RVAL                   0x0

/* I2CMCON[TXDMA] - Enable master Tx DMA request. */
#define I2CMCON_TXDMA_BBA              (*(volatile unsigned long *) 0x4206002C)
#define I2CMCON_TXDMA_MSK              (0x1   << 11 )
#define I2CMCON_TXDMA                  (0x1   << 11 )
#define I2CMCON_TXDMA_DIS              (0x0   << 11 ) /* DIS. Disable Tx DMA mode. */
#define I2CMCON_TXDMA_EN               (0x1   << 11 ) /* EN. Enable I2C master DMA requests. */

/* I2CMCON[RXDMA] - Enable master Rx DMA request. */
#define I2CMCON_RXDMA_BBA              (*(volatile unsigned long *) 0x42060028)
#define I2CMCON_RXDMA_MSK              (0x1   << 10 )
#define I2CMCON_RXDMA                  (0x1   << 10 )
#define I2CMCON_RXDMA_DIS              (0x0   << 10 ) /* DIS. Disable Rx DMA mode. */
#define I2CMCON_RXDMA_EN               (0x1   << 10 ) /* EN. Enable I2C master DMA requests. */

/* I2CMCON[IENCMP] - Transaction completed (or stop detected) interrupt enable. */
#define I2CMCON_IENCMP_BBA             (*(volatile unsigned long *) 0x42060020)
#define I2CMCON_IENCMP_MSK             (0x1   << 8  )
#define I2CMCON_IENCMP                 (0x1   << 8  )
#define I2CMCON_IENCMP_DIS             (0x0   << 8  ) /* DIS. Interrupt disabled. */
#define I2CMCON_IENCMP_EN              (0x1   << 8  ) /* EN. Interrupt enabled. A master I2C interrupt is generated when a STOP is detected. Enables TCOMP to geneerate an interrupt. */

/* I2CMCON[IENNACK] - NACK received interrupt enable. */
#define I2CMCON_IENNACK_BBA            (*(volatile unsigned long *) 0x4206001C)
#define I2CMCON_IENNACK_MSK            (0x1   << 7  )
#define I2CMCON_IENNACK                (0x1   << 7  )
#define I2CMCON_IENNACK_DIS            (0x0   << 7  ) /* DIS. Interrupt disabled. */
#define I2CMCON_IENNACK_EN             (0x1   << 7  ) /* EN, enables NACKADDR(I2CMSTA[4]) and NACKDATA (I2CMSTA[7]) to generate an interrupt. */

/* I2CMCON[IENALOST] - Arbitration lost interrupt enable. */
#define I2CMCON_IENALOST_BBA           (*(volatile unsigned long *) 0x42060018)
#define I2CMCON_IENALOST_MSK           (0x1   << 6  )
#define I2CMCON_IENALOST               (0x1   << 6  )
#define I2CMCON_IENALOST_DIS           (0x0   << 6  ) /* DIS. Interrupt disabled. */
#define I2CMCON_IENALOST_EN            (0x1   << 6  ) /* EN. Interrupt enabled. A master I2C interrupt is generated if the master looses arbitration.Enables ALOST to generate an interrupt. */

/* I2CMCON[IENTX] - Transmit request interrupt enable. */
#define I2CMCON_IENTX_BBA              (*(volatile unsigned long *) 0x42060014)
#define I2CMCON_IENTX_MSK              (0x1   << 5  )
#define I2CMCON_IENTX                  (0x1   << 5  )
#define I2CMCON_IENTX_DIS              (0x0   << 5  ) /* DIS. Interrupt disabled. */
#define I2CMCON_IENTX_EN               (0x1   << 5  ) /* EN. Interrupt enabled. A master I2C interrupt is generated when the Tx FIFO is not full and the direction bit is 0. */

/* I2CMCON[IENRX] - Receive request interrupt enable. */
#define I2CMCON_IENRX_BBA              (*(volatile unsigned long *) 0x42060010)
#define I2CMCON_IENRX_MSK              (0x1   << 4  )
#define I2CMCON_IENRX                  (0x1   << 4  )
#define I2CMCON_IENRX_DIS              (0x0   << 4  ) /* DIS. Interrupt disabled. */
#define I2CMCON_IENRX_EN               (0x1   << 4  ) /* EN. Interrupt enabled. A master I2C interrupt is generated when data is in the receive FIFO. */

/* I2CMCON[STRETCH] - Stretch I2CSCL enable. */
#define I2CMCON_STRETCH_BBA            (*(volatile unsigned long *) 0x4206000C)
#define I2CMCON_STRETCH_MSK            (0x1   << 3  )
#define I2CMCON_STRETCH                (0x1   << 3  )
#define I2CMCON_STRETCH_DIS            (0x0   << 3  ) /* DIS. Disable             */
#define I2CMCON_STRETCH_EN             (0x1   << 3  ) /* EN. Setting this bit instructs the device that if I2CSCL is 0, hold it at 0. Or if I2CSCL is 1, then when it next goes to 0, hold it at 0. */

/* I2CMCON[LOOPBACK] - Internal loop back enable. */
#define I2CMCON_LOOPBACK_BBA           (*(volatile unsigned long *) 0x42060008)
#define I2CMCON_LOOPBACK_MSK           (0x1   << 2  )
#define I2CMCON_LOOPBACK               (0x1   << 2  )
#define I2CMCON_LOOPBACK_DIS           (0x0   << 2  ) /* DIS. Disable.            */
#define I2CMCON_LOOPBACK_EN            (0x1   << 2  ) /* EN. I2CSCL and I2CSDA out of the device are muxed onto their corresponding inputs. Note that is also possible for the master to loop back a transfer to the slave as long as the device address corresponds, that is, external loopback. */

/* I2CMCON[COMPETE] - Start back-off disable. */
#define I2CMCON_COMPETE_BBA            (*(volatile unsigned long *) 0x42060004)
#define I2CMCON_COMPETE_MSK            (0x1   << 1  )
#define I2CMCON_COMPETE                (0x1   << 1  )
#define I2CMCON_COMPETE_DIS            (0x0   << 1  ) /* DIS. Disable.            */
#define I2CMCON_COMPETE_EN             (0x1   << 1  ) /* EN. Enables the device to compete for ownership even if another device is currently driving a start condition. */

/* I2CMCON[MAS] - Master enable bit. */
#define I2CMCON_MAS_BBA                (*(volatile unsigned long *) 0x42060000)
#define I2CMCON_MAS_MSK                (0x1   << 0  )
#define I2CMCON_MAS                    (0x1   << 0  )
#define I2CMCON_MAS_DIS                (0x0   << 0  ) /* DIS. The master is disabled. The master state machine is reset.The master should be disabled when not in use. This bit should not be cleared until a transaction has completed. TCOMP in I2CMSTA indicates when a transaction is complete. */
#define I2CMCON_MAS_EN                 (0x1   << 0  ) /* EN. Enable master.       */

/* Reset Value for I2CMSTA*/
#define I2CMSTA_RVAL                   0x0

/* I2CMSTA[TXUR] - Master transmit FIFO underrun. */
#define I2CMSTA_TXUR_BBA               (*(volatile unsigned long *) 0x420600B0)
#define I2CMSTA_TXUR_MSK               (0x1   << 12 )
#define I2CMSTA_TXUR                   (0x1   << 12 )
#define I2CMSTA_TXUR_CLR               (0x0   << 12 ) /* CLR. Cleared.            */
#define I2CMSTA_TXUR_SET               (0x1   << 12 ) /* SET. Set when the I2C master ends the transaction due to a Tx FIFO empty condition. This bit is only set when IENTX (I2CSCON[5]) is set. */

/* I2CMSTA[MSTOP] - STOP driven by the I2C master. */
#define I2CMSTA_MSTOP_BBA              (*(volatile unsigned long *) 0x420600AC)
#define I2CMSTA_MSTOP_MSK              (0x1   << 11 )
#define I2CMSTA_MSTOP                  (0x1   << 11 )
#define I2CMSTA_MSTOP_CLR              (0x0   << 11 ) /* CLR. Cleared.            */
#define I2CMSTA_MSTOP_SET              (0x1   << 11 ) /* SET. Set when the I2C master drives a stop condition on the I2C bus, therefore indicating a transaction completion, Tx underrun, Rx overflow, or a NACK by the slave. It is different from TCOMP because it is not set when the stop condition occurs due to any other master on the I2C bus.  This bit does not generate an interrupt. See the TCOMP description for available interrupts related to the stop condition. */

/* I2CMSTA[LINEBUSY] - Line is busy. */
#define I2CMSTA_LINEBUSY_BBA           (*(volatile unsigned long *) 0x420600A8)
#define I2CMSTA_LINEBUSY_MSK           (0x1   << 10 )
#define I2CMSTA_LINEBUSY               (0x1   << 10 )
#define I2CMSTA_LINEBUSY_CLR           (0x0   << 10 ) /* CLR. Cleared when a stop is detected on the I2C bus. */
#define I2CMSTA_LINEBUSY_SET           (0x1   << 10 ) /* SET. Set when a start is detected on the I2C bus. */

/* I2CMSTA[RXOF] - Receive FIFO overflow. */
#define I2CMSTA_RXOF_BBA               (*(volatile unsigned long *) 0x420600A4)
#define I2CMSTA_RXOF_MSK               (0x1   << 9  )
#define I2CMSTA_RXOF                   (0x1   << 9  )
#define I2CMSTA_RXOF_CLR               (0x0   << 9  ) /* CLR. CLeared.            */
#define I2CMSTA_RXOF_SET               (0x1   << 9  ) /* SET. Set when a byte is written to the receive FIFO when the FIFO is already full. */

/* I2CMSTA[TCOMP] - Transaction completed (or stop detected). (Can drive an interrupt). */
#define I2CMSTA_TCOMP_BBA              (*(volatile unsigned long *) 0x420600A0)
#define I2CMSTA_TCOMP_MSK              (0x1   << 8  )
#define I2CMSTA_TCOMP                  (0x1   << 8  )
#define I2CMSTA_TCOMP_CLR              (0x0   << 8  ) /* CLR. Cleared.            */
#define I2CMSTA_TCOMP_SET              (0x1   << 8  ) /* SET. Set when a STOP condition is detected on the I2C bus.  If IENCMP is 1, an interrupt is generated when this bit asserts. This bit only asserts if the master is enabled (MASEN = 1).  This bit should be used to determine when it is safe to disable the master. It can also be used to wait for another master's transaction to complete on the I2C bus when this master loses arbitration. */

/* I2CMSTA[NACKDATA] - NACK received in response to data write. (Can drive an interrupt). */
#define I2CMSTA_NACKDATA_BBA           (*(volatile unsigned long *) 0x4206009C)
#define I2CMSTA_NACKDATA_MSK           (0x1   << 7  )
#define I2CMSTA_NACKDATA               (0x1   << 7  )
#define I2CMSTA_NACKDATA_CLR           (0x0   << 7  ) /* CLR. Cleared on a read of the I2CMSTA register. */
#define I2CMSTA_NACKDATA_SET           (0x1   << 7  ) /* SET. Set when a NACK is received in response to a data write transfer. If IENNACK is 1, an interrupt is generated when this bit asserts. */

/* I2CMSTA[BUSY] - Master busy. */
#define I2CMSTA_BUSY_BBA               (*(volatile unsigned long *) 0x42060098)
#define I2CMSTA_BUSY_MSK               (0x1   << 6  )
#define I2CMSTA_BUSY                   (0x1   << 6  )
#define I2CMSTA_BUSY_CLR               (0x0   << 6  ) /* CLR. Cleared if the state machine is idle or another device has control of the I2C bus. */
#define I2CMSTA_BUSY_SET               (0x1   << 6  ) /* SET. Set when the master state machine is servicing a transaction. */

/* I2CMSTA[ALOST] - Arbitration lost.  (Can drive an interrupt). */
#define I2CMSTA_ALOST_BBA              (*(volatile unsigned long *) 0x42060094)
#define I2CMSTA_ALOST_MSK              (0x1   << 5  )
#define I2CMSTA_ALOST                  (0x1   << 5  )
#define I2CMSTA_ALOST_CLR              (0x0   << 5  ) /* CLR. Cleared on a read of the I2CMSTA register. */
#define I2CMSTA_ALOST_SET              (0x1   << 5  ) /* SET. Set if the master looses arbitration.  If IENALOST is 1, an interrupt is generated when this bit asserts. */

/* I2CMSTA[NACKADDR] - NACK received in response to an address. (Can drive an interrupt). */
#define I2CMSTA_NACKADDR_BBA           (*(volatile unsigned long *) 0x42060090)
#define I2CMSTA_NACKADDR_MSK           (0x1   << 4  )
#define I2CMSTA_NACKADDR               (0x1   << 4  )
#define I2CMSTA_NACKADDR_CLR           (0x0   << 4  ) /* CLR. Cleared on a read of the I2CMSTA register. */
#define I2CMSTA_NACKADDR_SET           (0x1   << 4  ) /* SET. Set if a NACK received in response to an address.  If IENNACK is 1, an interrupt is generated when this bit asserts. */

/* I2CMSTA[RXREQ] - Receive Request. (Can drive an interrupt). */
#define I2CMSTA_RXREQ_BBA              (*(volatile unsigned long *) 0x4206008C)
#define I2CMSTA_RXREQ_MSK              (0x1   << 3  )
#define I2CMSTA_RXREQ                  (0x1   << 3  )
#define I2CMSTA_RXREQ_CLR              (0x0   << 3  ) /* CLR. Cleared.            */
#define I2CMSTA_RXREQ_SET              (0x1   << 3  ) /* SET. Set when there is data in the receive FIFO.  If IENRX is 1, an interrupt is generated when this bit asserts. */

/* I2CMSTA[TXREQ] - Transmit Request. (Can drive an interrupt). */
#define I2CMSTA_TXREQ_BBA              (*(volatile unsigned long *) 0x42060088)
#define I2CMSTA_TXREQ_MSK              (0x1   << 2  )
#define I2CMSTA_TXREQ                  (0x1   << 2  )
#define I2CMSTA_TXREQ_CLR              (0x0   << 2  ) /* CLR. Cleared when the transmit FIFO underrun condition is not met. */
#define I2CMSTA_TXREQ_SET              (0x1   << 2  ) /* SET. Set  when the direction bit is 0 and the transmit FIFO is either empty or not full. If IENTX is 1, an interrupt is generated when this bit asserts. */

/* I2CMSTA[TXFSTA] - Transmit FIFO Status. */
#define I2CMSTA_TXFSTA_MSK             (0x3   << 0  )
#define I2CMSTA_TXFSTA_EMPTY           (0x0   << 0  ) /* EMPTY. FIFO empty.       */
#define I2CMSTA_TXFSTA_ONEBYTE         (0x2   << 0  ) /* ONEBYTE. 1 byte in FIFO. */
#define I2CMSTA_TXFSTA_FULL            (0x3   << 0  ) /* FULL. FIFO full.         */

/* Reset Value for I2CMRX*/
#define I2CMRX_RVAL                    0x0

/* I2CMRX[VALUE] - Receive register. This register allows access to the receive data FIFO.  The FIFO can hold two bytes. */
#define I2CMRX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CMTX*/
#define I2CMTX_RVAL                    0x0

/* I2CMTX[VALUE] - Transmit register. This register allows access to the transmit data FIFO.  The FIFO can hold two bytes. */
#define I2CMTX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CMRXCNT*/
#define I2CMRXCNT_RVAL                 0x0

/* I2CMRXCNT[EXTEND] - Extended read: Use this bit if greater than 256 bytes are required on a read.  For example: To receive 412 bytes, write 0x100 (EXTEND = 1) to this register (I2CMRXCNT).  Wait for the first byte to be received, then check the I2CMCRXCNT register for every byte received thereafter.  When I2CMCRXCNT returns to 0, 256 bytes have been received.  Then, write 0x09C (412 - 256 = 156 decimal (equal to 0x9C)  with the EXTEND bit set to 0) to this register (I2CMRXCNT). */
#define I2CMRXCNT_EXTEND_BBA           (*(volatile unsigned long *) 0x42060220)
#define I2CMRXCNT_EXTEND_MSK           (0x1   << 8  )
#define I2CMRXCNT_EXTEND               (0x1   << 8  )
#define I2CMRXCNT_EXTEND_DIS           (0x0   << 8  ) /* DIS                      */
#define I2CMRXCNT_EXTEND_EN            (0x1   << 8  ) /* EN                       */

/* I2CMRXCNT[COUNT] - Receive count. Program the number of bytes required minus one to this register.  If just one byte is required write 0 to this register.  If greater than 256 bytes are required, then use EXTEND. */
#define I2CMRXCNT_COUNT_MSK            (0xFF  << 0  )

/* Reset Value for I2CMCRXCNT*/
#define I2CMCRXCNT_RVAL                0x0

/* I2CMCRXCNT[VALUE] - Current receive count. This register gives the total number of bytes received so far. If 256 bytes are requested, then this register reads 0 when the transaction has completed. */
#define I2CMCRXCNT_VALUE_MSK           (0xFF  << 0  )

/* Reset Value for I2CADR0*/
#define I2CADR0_RVAL                   0x0

/* I2CADR0[VALUE] - Address byte. If a 7-bit address is required, then I2CADR0[7:1] is programmed with the address and I2CADR0[0] is programmed with the direction (read or write). If a 10-bit address is required then I2CADR0[7:3] is programmed with '11110', I2CADR0[2:1] is programmed with the two MSBs of the address, and, again, I2CADR0[0] is programmed with the direction bit (read or write). */
#define I2CADR0_VALUE_MSK              (0xFF  << 0  )

/* Reset Value for I2CADR1*/
#define I2CADR1_RVAL                   0x0

/* I2CADR1[VALUE] - Address byte. This register is only required when addressing a slave with 10-bit addressing.  I2CADR1[7:0] is programmed with the lower eight bits of the address. */
#define I2CADR1_VALUE_MSK              (0xFF  << 0  )

/* Reset Value for I2CDIV*/
#define I2CDIV_RVAL                    0x1F1F

/* I2CDIV[HIGH] - Serial clock high time. This register controls the clock high time. See the serial clock generation section for more details. */
#define I2CDIV_HIGH_MSK                (0xFF  << 8  )

/* I2CDIV[LOW] - Serial clock low time. This register controls the clock low time. See the serial clock generation section for more details. */
#define I2CDIV_LOW_MSK                 (0xFF  << 0  )

/* Reset Value for I2CSCON*/
#define I2CSCON_RVAL                   0x0

/* I2CSCON[TXDMA] - Enable slave Tx DMA request. */
#define I2CSCON_TXDMA_BBA              (*(volatile unsigned long *) 0x42060538)
#define I2CSCON_TXDMA_MSK              (0x1   << 14 )
#define I2CSCON_TXDMA                  (0x1   << 14 )
#define I2CSCON_TXDMA_DIS              (0x0   << 14 ) /* DIS. Disable DMA mode.   */
#define I2CSCON_TXDMA_EN               (0x1   << 14 ) /* EN. Enable I2C slave DMA requests. */

/* I2CSCON[RXDMA] - Enable slave Rx DMA request. */
#define I2CSCON_RXDMA_BBA              (*(volatile unsigned long *) 0x42060534)
#define I2CSCON_RXDMA_MSK              (0x1   << 13 )
#define I2CSCON_RXDMA                  (0x1   << 13 )
#define I2CSCON_RXDMA_DIS              (0x0   << 13 ) /* DIS. Disable DMA mode.   */
#define I2CSCON_RXDMA_EN               (0x1   << 13 ) /* EN. Enable I2C slave DMA requests. */

/* I2CSCON[IENREPST] - Repeated start interrupt enable. */
#define I2CSCON_IENREPST_BBA           (*(volatile unsigned long *) 0x42060530)
#define I2CSCON_IENREPST_MSK           (0x1   << 12 )
#define I2CSCON_IENREPST               (0x1   << 12 )
#define I2CSCON_IENREPST_DIS           (0x0   << 12 ) /* DIS. Disable an interrupt when the REPSTART status bit asserts. */
#define I2CSCON_IENREPST_EN            (0x1   << 12 ) /* EN.  Generate an interrupt when the REPSTART status bit asserts. */

/* I2CSCON[IENTX] - Transmit request interrupt enable. */
#define I2CSCON_IENTX_BBA              (*(volatile unsigned long *) 0x42060528)
#define I2CSCON_IENTX_MSK              (0x1   << 10 )
#define I2CSCON_IENTX                  (0x1   << 10 )
#define I2CSCON_IENTX_DIS              (0x0   << 10 ) /* DIS. Disable transmit request interrupt. */
#define I2CSCON_IENTX_EN               (0x1   << 10 ) /* EN. Enable transmit request interrupt. */

/* I2CSCON[IENRX] - Receive request interrupt enable. */
#define I2CSCON_IENRX_BBA              (*(volatile unsigned long *) 0x42060524)
#define I2CSCON_IENRX_MSK              (0x1   << 9  )
#define I2CSCON_IENRX                  (0x1   << 9  )
#define I2CSCON_IENRX_DIS              (0x0   << 9  ) /* DIS. Disable receive request interrupt. */
#define I2CSCON_IENRX_EN               (0x1   << 9  ) /* EN. Enable receive request interrupt. */

/* I2CSCON[IENSTOP] - Stop condition detected interrupt enable. */
#define I2CSCON_IENSTOP_BBA            (*(volatile unsigned long *) 0x42060520)
#define I2CSCON_IENSTOP_MSK            (0x1   << 8  )
#define I2CSCON_IENSTOP                (0x1   << 8  )
#define I2CSCON_IENSTOP_DIS            (0x0   << 8  ) /* DIS. Disable stop condition detect interrupt. */
#define I2CSCON_IENSTOP_EN             (0x1   << 8  ) /* EN.  Enable stop condition detect interrupt. Enables STOP (I2CSSTA[10]) to generate an interrupt */

/* I2CSCON[NACK] - NACK next communication. */
#define I2CSCON_NACK_BBA               (*(volatile unsigned long *) 0x4206051C)
#define I2CSCON_NACK_MSK               (0x1   << 7  )
#define I2CSCON_NACK                   (0x1   << 7  )
#define I2CSCON_NACK_DIS               (0x0   << 7  ) /* DIS. Disable.            */
#define I2CSCON_NACK_EN                (0x1   << 7  ) /* EN. Allow the next communication to be NACK'ed.  This can be used for example if during a 24xx I2C serial eeprom style access, an attempt was made to write to a read only or nonexisting location in system memory. That is the indirect address in a 24xx I2C serial eeprom style write pointed to an unwritable memory location. */

/* I2CSCON[STRETCH] - Stretch I2CSCL enable. */
#define I2CSCON_STRETCH_BBA            (*(volatile unsigned long *) 0x42060518)
#define I2CSCON_STRETCH_MSK            (0x1   << 6  )
#define I2CSCON_STRETCH                (0x1   << 6  )
#define I2CSCON_STRETCH_DIS            (0x0   << 6  ) /* DIS. Disable.            */
#define I2CSCON_STRETCH_EN             (0x1   << 6  ) /* EN. Tell the device that, if I2CSCL is 0, hold it at 0.  Or if I2CSCL is 1, then when it next goes to 0 hold it at 0. */

/* I2CSCON[EARLYTXR] - Early transmit request mode. */
#define I2CSCON_EARLYTXR_BBA           (*(volatile unsigned long *) 0x42060514)
#define I2CSCON_EARLYTXR_MSK           (0x1   << 5  )
#define I2CSCON_EARLYTXR               (0x1   << 5  )
#define I2CSCON_EARLYTXR_DIS           (0x0   << 5  ) /* DIS. Disable.            */
#define I2CSCON_EARLYTXR_EN            (0x1   << 5  ) /* EN. Enable a transmit request just after the positive edge of the direction bit (READ/WRITE) I2CSCL clock pulse. */

/* I2CSCON[GCSB] - General call status bit clear. */
#define I2CSCON_GCSB_BBA               (*(volatile unsigned long *) 0x42060510)
#define I2CSCON_GCSB_MSK               (0x1   << 4  )
#define I2CSCON_GCSB                   (0x1   << 4  )
#define I2CSCON_GCSB_CLR               (0x1   << 4  ) /* CLR. Clear the General Call status and General Call ID bits. The General Call status and General Call ID bits are not reset by anything other than a write to this bit or a full reset. */

/* I2CSCON[HGC] - Hardware general call enable. */
#define I2CSCON_HGC_BBA                (*(volatile unsigned long *) 0x4206050C)
#define I2CSCON_HGC_MSK                (0x1   << 3  )
#define I2CSCON_HGC                    (0x1   << 3  )
#define I2CSCON_HGC_DIS                (0x0   << 3  ) /* DIS. Disable.            */
#define I2CSCON_HGC_EN                 (0x1   << 3  ) /* EN. When this bit and the General Call enable bit are set the device after receiving a general call, Address 0x00 and a data byte checks the contents of the I2CALT against the receive shift register. If they match, the device has received a hardware general call. This is used if a device needs urgent attention from a master device without knowing which master it needs to turn to. This is a broadcast message to all master devices in the bus. The device that requires attention embeds its own address into the message. The LSB of the I2CALT register should always be written to a 1. */

/* I2CSCON[GC] - General call enable. */
#define I2CSCON_GC_BBA                 (*(volatile unsigned long *) 0x42060508)
#define I2CSCON_GC_MSK                 (0x1   << 2  )
#define I2CSCON_GC                     (0x1   << 2  )
#define I2CSCON_GC_DIS                 (0x0   << 2  ) /* DIS. Disable.            */
#define I2CSCON_GC_EN                  (0x1   << 2  ) /* EN. Enable the I2C slave to ACK an I2C general call, Address 0x00 (write). */

/* I2CSCON[ADR10] - Enable 10 bit addressing. */
#define I2CSCON_ADR10_BBA              (*(volatile unsigned long *) 0x42060504)
#define I2CSCON_ADR10_MSK              (0x1   << 1  )
#define I2CSCON_ADR10                  (0x1   << 1  )
#define I2CSCON_ADR10_DIS              (0x0   << 1  ) /* DIS. If this bit is clear, the slave can support four slave addresses, programmed in Registers I2CID0 to I2CID3. */
#define I2CSCON_ADR10_EN               (0x1   << 1  ) /* EN. Enable 10-bit addressing. One 10-bit address is supported by the slave and is stored in I2CID0 and I2CID1, where I2CID0 contains the first byte of the address and the upper five bits must be programmed to 11110' I2CID2 and I2CID3 can be programmed with 7-bit addresses at the same time. */

/* I2CSCON[SLV] - Slave enable. */
#define I2CSCON_SLV_BBA                (*(volatile unsigned long *) 0x42060500)
#define I2CSCON_SLV_MSK                (0x1   << 0  )
#define I2CSCON_SLV                    (0x1   << 0  )
#define I2CSCON_SLV_DIS                (0x0   << 0  ) /* DIS. Disable the slave and all slave state machine flops are held in reset. */
#define I2CSCON_SLV_EN                 (0x1   << 0  ) /* EN. Enable slave.        */

/* Reset Value for I2CSSTA*/
#define I2CSSTA_RVAL                   0x1

/* I2CSSTA[START] - Start and matching address. */
#define I2CSSTA_START_BBA              (*(volatile unsigned long *) 0x420605B8)
#define I2CSSTA_START_MSK              (0x1   << 14 )
#define I2CSSTA_START                  (0x1   << 14 )
#define I2CSSTA_START_CLR              (0x0   << 14 ) /* CLR. Cleared on receipt of either a stop or start condition. */
#define I2CSSTA_START_SET              (0x1   << 14 ) /* SET. Set if a start is detected on I2CSCL/I2CSDA and one of the following is true: The device address is matched.  A general call (GC = 0000_0000) code is received and GC is enabled.  A high speed (HS = 0000_1XXX) code is received. A start byte (0000_0001) is received. */

/* I2CSSTA[REPSTART] - Repeated start and matching address. (Can drive an interrupt). */
#define I2CSSTA_REPSTART_BBA           (*(volatile unsigned long *) 0x420605B4)
#define I2CSSTA_REPSTART_MSK           (0x1   << 13 )
#define I2CSSTA_REPSTART               (0x1   << 13 )
#define I2CSSTA_REPSTART_CLR           (0x0   << 13 ) /* CLR. Cleared when read or on receipt of a stop condition. */
#define I2CSSTA_REPSTART_SET           (0x1   << 13 ) /* SET. Set if START (I2CSSTA[14]) is already asserted and then a repeated start is detected. */

/* I2CSSTA[IDMAT] - Device ID matched. */
#define I2CSSTA_IDMAT_MSK              (0x3   << 11 )

/* I2CSSTA[STOP] - Stop after start and matching address. (Can drive an interrupt). */
#define I2CSSTA_STOP_BBA               (*(volatile unsigned long *) 0x420605A8)
#define I2CSSTA_STOP_MSK               (0x1   << 10 )
#define I2CSSTA_STOP                   (0x1   << 10 )
#define I2CSSTA_STOP_CLR               (0x0   << 10 ) /* CLR. Cleared by a read of the status register. */
#define I2CSSTA_STOP_SET               (0x1   << 10 ) /* SET. Set if the slave device received a stop condition after a previous start condition and a matching address. */

/* I2CSSTA[GCID] - General call ID. Cleared when the GCSBCLR (I2CSCON[4]) is written to 1. These status bits are not cleared by a general call reset. */
#define I2CSSTA_GCID_MSK               (0x3   << 8  )

/* I2CSSTA[GCINT] - General call interrupt. (Always drives an interrupt). */
#define I2CSSTA_GCINT_BBA              (*(volatile unsigned long *) 0x4206059C)
#define I2CSSTA_GCINT_MSK              (0x1   << 7  )
#define I2CSSTA_GCINT                  (0x1   << 7  )
#define I2CSSTA_GCINT_CLR              (0x0   << 7  ) /* CLR. To clear this bit, write 1 to the I2CSCON[4].  If it was a general call reset, all registers are at their default values. If it was a hardware general call, the Rx FIFO holds the second byte of the general call and this can be compared with the ALT register. */
#define I2CSSTA_GCINT_SET              (0x1   << 7  ) /* SET. Set if the slave device receives a general call of any type. */

/* I2CSSTA[BUSY] - Slave busy. */
#define I2CSSTA_BUSY_BBA               (*(volatile unsigned long *) 0x42060598)
#define I2CSSTA_BUSY_MSK               (0x1   << 6  )
#define I2CSSTA_BUSY                   (0x1   << 6  )
#define I2CSSTA_BUSY_CLR               (0x0   << 6  ) /* CLR. Cleared by hardware on any of the following conditions: The address does not match an ID register, the slave device receives a I2C stop condition or if a repeated start address doesnt match. */
#define I2CSSTA_BUSY_SET               (0x1   << 6  ) /* SET. Set if the slave device receives an I2C start condition. */

/* I2CSSTA[NOACK] - NACK generated by the slave. */
#define I2CSSTA_NOACK_BBA              (*(volatile unsigned long *) 0x42060594)
#define I2CSSTA_NOACK_MSK              (0x1   << 5  )
#define I2CSSTA_NOACK                  (0x1   << 5  )
#define I2CSSTA_NOACK_CLR              (0x0   << 5  ) /* CLR. Cleared on a read of the I2CSSTA register. */
#define I2CSSTA_NOACK_SET              (0x1   << 5  ) /* SET. Set to indicate that the slave responded to its device address with a NACK. Set under any of the following conditions: If there was no data to transmit and sequence was a slave read, the device address is NACK'ed or if the NACK bit was set in the slave control register and the device was addressed. */

/* I2CSSTA[RXOF] - Receive FIFO overflow. */
#define I2CSSTA_RXOF_BBA               (*(volatile unsigned long *) 0x42060590)
#define I2CSSTA_RXOF_MSK               (0x1   << 4  )
#define I2CSSTA_RXOF                   (0x1   << 4  )
#define I2CSSTA_RXOF_CLR               (0x0   << 4  ) /* CLR. Cleared.            */
#define I2CSSTA_RXOF_SET               (0x1   << 4  ) /* SET. Set when a byte is written to the receive FIFO when the FIFO is already full. */

/* I2CSSTA[RXREQ] - Receive request. (Can drive an interrupt). */
#define I2CSSTA_RXREQ_BBA              (*(volatile unsigned long *) 0x4206058C)
#define I2CSSTA_RXREQ_MSK              (0x1   << 3  )
#define I2CSSTA_RXREQ                  (0x1   << 3  )
#define I2CSSTA_RXREQ_CLR              (0x0   << 3  ) /* CLR. Cleared when the receive FIFO is read or flushed. */
#define I2CSSTA_RXREQ_SET              (0x1   << 3  ) /* SET. Set when the receive FIFO is not empty. Set on the falling edge of the I2CSCL clock pulse that clocks in the last data bit of a byte. */

/* I2CSSTA[TXREQ] - Transmit request. (Can drive an interrupt). */
#define I2CSSTA_TXREQ_BBA              (*(volatile unsigned long *) 0x42060588)
#define I2CSSTA_TXREQ_MSK              (0x1   << 2  )
#define I2CSSTA_TXREQ                  (0x1   << 2  )
#define I2CSSTA_TXREQ_CLR              (0x0   << 2  ) /* CLR. This bit is cleared on a read of the I2CSSTA register. */
#define I2CSSTA_TXREQ_SET              (0x1   << 2  ) /* SET. If EARLYTXR = 0, TXREQ is set when the direction bit for a transfer is received high. Thereafter, as long as the transmit FIFO is not full, this bit remains asserted. Initially, it is asserted on the negative edge of the SCL pulse that clocks in the direction bit (if the device address matched also). If EARLYTXR = 1, TXREQ is set when the direction bit for a transfer is received high. Thereafter, as long as the transmit FIFO is not full, this bit will remain asserted. Initially, it is asserted after the positive edge of the SCL pulse that clocks in the direction bit (if the device address matched also). */

/* I2CSSTA[TXUR] - Transmit FIFO underflow. */
#define I2CSSTA_TXUR_BBA               (*(volatile unsigned long *) 0x42060584)
#define I2CSSTA_TXUR_MSK               (0x1   << 1  )
#define I2CSSTA_TXUR                   (0x1   << 1  )
#define I2CSSTA_TXUR_CLR               (0x0   << 1  ) /* CLR. Cleared.            */
#define I2CSSTA_TXUR_SET               (0x1   << 1  ) /* SET. Set to 1 if a master requests data from the device and the Tx FIFO is empty for the rising edge of SCL. */

/* I2CSSTA[TXFSEREQ] - Tx FIFO status. */
#define I2CSSTA_TXFSEREQ_BBA           (*(volatile unsigned long *) 0x42060580)
#define I2CSSTA_TXFSEREQ_MSK           (0x1   << 0  )
#define I2CSSTA_TXFSEREQ               (0x1   << 0  )
#define I2CSSTA_TXFSEREQ_CLR           (0x0   << 0  ) /* CLR. Cleared.            */
#define I2CSSTA_TXFSEREQ_SET           (0x1   << 0  ) /* SET. Set whenever the slave Tx FIFO is empty. */

/* Reset Value for I2CSRX*/
#define I2CSRX_RVAL                    0x0

/* I2CSRX[VALUE] - Receive register. */
#define I2CSRX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CSTX*/
#define I2CSTX_RVAL                    0x0

/* I2CSTX[VALUE] - Transmit register. */
#define I2CSTX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CALT*/
#define I2CALT_RVAL                    0x0

/* I2CALT[VALUE] - ALT register.This register is used in conjunction with HGC (I2CSCON[3]) to match a master generating a hardware general call. It is used in the case where a master device cannot be programmed with a slaves address and, instead, the slave must recognize the masters address. */
#define I2CALT_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CID0*/
#define I2CID0_RVAL                    0x0

/* I2CID0[VALUE] - Slave ID. */
#define I2CID0_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CID1*/
#define I2CID1_RVAL                    0x0

/* I2CID1[VALUE] - Slave ID. */
#define I2CID1_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CID2*/
#define I2CID2_RVAL                    0x0

/* I2CID2[VALUE] - Slave ID. */
#define I2CID2_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CID3*/
#define I2CID3_RVAL                    0x0

/* I2CID3[VALUE] - Slave ID. */
#define I2CID3_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for I2CFSTA*/
#define I2CFSTA_RVAL                   0x0

/* I2CFSTA[MFLUSH] - Master Transmit FIFO Flush. */
#define I2CFSTA_MFLUSH_BBA             (*(volatile unsigned long *) 0x420609A4)
#define I2CFSTA_MFLUSH_MSK             (0x1   << 9  )
#define I2CFSTA_MFLUSH                 (0x1   << 9  )
#define I2CFSTA_MFLUSH_DIS             (0x0   << 9  ) /* DIS. For normal FIFO operation. */
#define I2CFSTA_MFLUSH_EN              (0x1   << 9  ) /* EN. FIFO flush enabled, to keep the FIFO empty. */

/* I2CFSTA[SFLUSH] - Slave Transmit FIFO Flush. */
#define I2CFSTA_SFLUSH_BBA             (*(volatile unsigned long *) 0x420609A0)
#define I2CFSTA_SFLUSH_MSK             (0x1   << 8  )
#define I2CFSTA_SFLUSH                 (0x1   << 8  )
#define I2CFSTA_SFLUSH_DIS             (0x0   << 8  ) /* DIS. For normal FIFO operation. */
#define I2CFSTA_SFLUSH_EN              (0x1   << 8  ) /* EN. FIFO flush enabled, to keep the FIFO empty. */

/* I2CFSTA[MRXFSTA] - Master Receive FIFO status. */
#define I2CFSTA_MRXFSTA_MSK            (0x3   << 6  )
#define I2CFSTA_MRXFSTA_EMPTY          (0x0   << 6  ) /* EMPTY                    */
#define I2CFSTA_MRXFSTA_ONEBYTE        (0x1   << 6  ) /* ONEBYTE                  */
#define I2CFSTA_MRXFSTA_TWOBYTES       (0x2   << 6  ) /* TWOBYTES                 */

/* I2CFSTA[MTXFSTA] - Master Transmit FIFO status. */
#define I2CFSTA_MTXFSTA_MSK            (0x3   << 4  )
#define I2CFSTA_MTXFSTA_EMPTY          (0x0   << 4  ) /* EMPTY                    */
#define I2CFSTA_MTXFSTA_ONEBYTE        (0x1   << 4  ) /* ONEBYTE                  */
#define I2CFSTA_MTXFSTA_TWOBYTES       (0x2   << 4  ) /* TWOBYTES                 */

/* I2CFSTA[SRXFSTA] - Slave Receive FIFO status. */
#define I2CFSTA_SRXFSTA_MSK            (0x3   << 2  )
#define I2CFSTA_SRXFSTA_EMPTY          (0x0   << 2  ) /* EMPTY                    */
#define I2CFSTA_SRXFSTA_ONEBYTE        (0x1   << 2  ) /* ONEBYTE                  */
#define I2CFSTA_SRXFSTA_TWOBYTES       (0x2   << 2  ) /* TWOBYTES                 */

/* I2CFSTA[STXFSTA] - Slave Transmit FIFO status. */
#define I2CFSTA_STXFSTA_MSK            (0x3   << 0  )
#define I2CFSTA_STXFSTA_EMPTY          (0x0   << 0  ) /* EMPTY                    */
#define I2CFSTA_STXFSTA_ONEBYTE        (0x1   << 0  ) /* ONEBYTE                  */
#define I2CFSTA_STXFSTA_TWOBYTES       (0x2   << 0  ) /* TWOBYTES                 */
// ------------------------------------------------------------------------------------------------
// -----                                        INTERRUPT                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Interrupts (pADI_INTERRUPT)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_INTERRUPT Structure               */
  __IO uint16_t  EI0CFG;                    /*!< External Interrupt Configuration Register 0 */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  EI1CFG;                    /*!< External Interrupt Configuration Register 1 */
  __I  uint16_t  RESERVED1;
  __IO uint16_t  EI2CFG;                    /*!< External Interrupt Configuration Register 2 */
  __I  uint16_t  RESERVED2[3];
  __IO uint16_t  EICLR;                     /*!< External Interrupts Clear Register    */
  __I  uint16_t  RESERVED3;
  __IO uint8_t   NMICLR;                    /*!< NMI Clear Register                    */
} ADI_INTERRUPT_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          EI0CFG                                     (*(volatile unsigned short int *) 0x40002420)
#define          EI1CFG                                     (*(volatile unsigned short int *) 0x40002424)
#define          EI2CFG                                     (*(volatile unsigned short int *) 0x40002428)
#define          EICLR                                      (*(volatile unsigned short int *) 0x40002430)
#define          NMICLR                                     (*(volatile unsigned char      *) 0x40002434)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for EI0CFG*/
#define EI0CFG_RVAL                    0x0

/* EI0CFG[IRQ3EN] - External interrupt 3 enable bit. */
#define EI0CFG_IRQ3EN_BBA              (*(volatile unsigned long *) 0x4204843C)
#define EI0CFG_IRQ3EN_MSK              (0x1   << 15 )
#define EI0CFG_IRQ3EN                  (0x1   << 15 )
#define EI0CFG_IRQ3EN_DIS              (0x0   << 15 ) /* DIS. External interrupt 3 disabled. */
#define EI0CFG_IRQ3EN_EN               (0x1   << 15 ) /* EN. External Interrupt 3 enabled. */

/* EI0CFG[IRQ3MDE] - External interrupt 3 detection mode. */
#define EI0CFG_IRQ3MDE_MSK             (0x7   << 12 )
#define EI0CFG_IRQ3MDE_RISE            (0x0   << 12 ) /* RISE. Rising edge.       */
#define EI0CFG_IRQ3MDE_FALL            (0x1   << 12 ) /* FALL. Falling edge.      */
#define EI0CFG_IRQ3MDE_RISEORFALL      (0x2   << 12 ) /* RISEORFALL. Rising or falling edge. */
#define EI0CFG_IRQ3MDE_HIGHLEVEL       (0x3   << 12 ) /* HIGHLEVEL. High level.   */
#define EI0CFG_IRQ3MDE_LOWLEVEL        (0x4   << 12 ) /* LOWLEVEL. Low level.     */

/* EI0CFG[IRQ2EN] - External interrupt 2 enable bit. */
#define EI0CFG_IRQ2EN_BBA              (*(volatile unsigned long *) 0x4204842C)
#define EI0CFG_IRQ2EN_MSK              (0x1   << 11 )
#define EI0CFG_IRQ2EN                  (0x1   << 11 )
#define EI0CFG_IRQ2EN_DIS              (0x0   << 11 ) /* DIS. External interrupt 2 disabled. */
#define EI0CFG_IRQ2EN_EN               (0x1   << 11 ) /* EN. External Interrupt 2 enabled. */

/* EI0CFG[IRQ2MDE] - External interrupt 2 detection mode. */
#define EI0CFG_IRQ2MDE_MSK             (0x7   << 8  )
#define EI0CFG_IRQ2MDE_RISE            (0x0   << 8  ) /* RISE. Rising edge.       */
#define EI0CFG_IRQ2MDE_FALL            (0x1   << 8  ) /* FALL. Falling edge.      */
#define EI0CFG_IRQ2MDE_RISEORFALL      (0x2   << 8  ) /* RISEORFALL. Rising or falling edge. */
#define EI0CFG_IRQ2MDE_HIGHLEVEL       (0x3   << 8  ) /* HIGHLEVEL. High level.   */
#define EI0CFG_IRQ2MDE_LOWLEVEL        (0x4   << 8  ) /* LOWLEVEL. Low level.     */

/* EI0CFG[IRQ1EN] - External interrupt 1 enable bit. */
#define EI0CFG_IRQ1EN_BBA              (*(volatile unsigned long *) 0x4204841C)
#define EI0CFG_IRQ1EN_MSK              (0x1   << 7  )
#define EI0CFG_IRQ1EN                  (0x1   << 7  )
#define EI0CFG_IRQ1EN_DIS              (0x0   << 7  ) /* DIS. External interrupt 1 disabled. */
#define EI0CFG_IRQ1EN_EN               (0x1   << 7  ) /* EN. External Interrupt 1 enabled. */

/* EI0CFG[IRQ1MDE] - External interrupt 1 detection mode. */
#define EI0CFG_IRQ1MDE_MSK             (0x7   << 4  )
#define EI0CFG_IRQ1MDE_RISE            (0x0   << 4  ) /* RISE. Rising edge.       */
#define EI0CFG_IRQ1MDE_FALL            (0x1   << 4  ) /* FALL. Falling edge.      */
#define EI0CFG_IRQ1MDE_RISEORFALL      (0x2   << 4  ) /* RISEORFALL. Rising or falling edge. */
#define EI0CFG_IRQ1MDE_HIGHLEVEL       (0x3   << 4  ) /* HIGHLEVEL. High level.   */
#define EI0CFG_IRQ1MDE_LOWLEVEL        (0x4   << 4  ) /* LOWLEVEL. Low level.     */

/* EI0CFG[IRQ0EN] - RF transceiver clock IRQ enable bit. */
#define EI0CFG_IRQ0EN_BBA              (*(volatile unsigned long *) 0x4204840C)
#define EI0CFG_IRQ0EN_MSK              (0x1   << 3  )
#define EI0CFG_IRQ0EN                  (0x1   << 3  )
#define EI0CFG_IRQ0EN_DIS              (0x0   << 3  ) /* DIS. RF transceiver clock IRQ disabled. */
#define EI0CFG_IRQ0EN_EN               (0x1   << 3  ) /* EN. RF transceiver clock IRQ enabled. */

/* EI0CFG[IRQ0MDE] - RF transceiver clock detection mode. */
#define EI0CFG_IRQ0MDE_MSK             (0x7   << 0  )
#define EI0CFG_IRQ0MDE_RISE            (0x0   << 0  ) /* RISE. Rising edge.       */
#define EI0CFG_IRQ0MDE_FALL            (0x1   << 0  ) /* FALL. Falling edge.      */
#define EI0CFG_IRQ0MDE_RISEORFALL      (0x2   << 0  ) /* RISEORFALL. Rising or falling edge. */
#define EI0CFG_IRQ0MDE_HIGHLEVEL       (0x3   << 0  ) /* HIGHLEVEL. High level.   */
#define EI0CFG_IRQ0MDE_LOWLEVEL        (0x4   << 0  ) /* LOWLEVEL. Low level.     */

/* Reset Value for EI1CFG*/
#define EI1CFG_RVAL                    0x0

/* EI1CFG[IRQ7EN] - External interrupt 7 enable bit. */
#define EI1CFG_IRQ7EN_BBA              (*(volatile unsigned long *) 0x420484BC)
#define EI1CFG_IRQ7EN_MSK              (0x1   << 15 )
#define EI1CFG_IRQ7EN                  (0x1   << 15 )
#define EI1CFG_IRQ7EN_DIS              (0x0   << 15 ) /* DIS. External interrupt 7 disabled. */
#define EI1CFG_IRQ7EN_EN               (0x1   << 15 ) /* EN. External interrupt 7 enabled. */

/* EI1CFG[IRQ7MDE] - External interrupt 7 detection mode. */
#define EI1CFG_IRQ7MDE_MSK             (0x7   << 12 )
#define EI1CFG_IRQ7MDE_RISE            (0x0   << 12 ) /* RISE. Rising edge.       */
#define EI1CFG_IRQ7MDE_FALL            (0x1   << 12 ) /* FALL. Falling edge.      */
#define EI1CFG_IRQ7MDE_RISEORFALL      (0x2   << 12 ) /* RISEORFALL. Rising or falling edge. */
#define EI1CFG_IRQ7MDE_HIGHLEVEL       (0x3   << 12 ) /* HIGHLEVEL. High level.   */
#define EI1CFG_IRQ7MDE_LOWLEVEL        (0x4   << 12 ) /* LOWLEVEL. Low level.     */

/* EI1CFG[IRQ6EN] - External interrupt 6 enable bit. */
#define EI1CFG_IRQ6EN_BBA              (*(volatile unsigned long *) 0x420484AC)
#define EI1CFG_IRQ6EN_MSK              (0x1   << 11 )
#define EI1CFG_IRQ6EN                  (0x1   << 11 )
#define EI1CFG_IRQ6EN_DIS              (0x0   << 11 ) /* DIS. External interrupt 6 disabled. */
#define EI1CFG_IRQ6EN_EN               (0x1   << 11 ) /* EN. External Interrupt 6 enabled. */

/* EI1CFG[IRQ6MDE] - External interrupt 6 detection mode. */
#define EI1CFG_IRQ6MDE_MSK             (0x7   << 8  )
#define EI1CFG_IRQ6MDE_RISE            (0x0   << 8  ) /* RISE. Rising edge.       */
#define EI1CFG_IRQ6MDE_FALL            (0x1   << 8  ) /* FALL. Falling edge.      */
#define EI1CFG_IRQ6MDE_RISEORFALL      (0x2   << 8  ) /* RISEORFALL. Rising or falling edge. */
#define EI1CFG_IRQ6MDE_HIGHLEVEL       (0x3   << 8  ) /* HIGHLEVEL. High level.   */
#define EI1CFG_IRQ6MDE_LOWLEVEL        (0x4   << 8  ) /* LOWLEVEL. Low Level.     */

/* EI1CFG[IRQ5EN] - External interrupt 5 enable bit. */
#define EI1CFG_IRQ5EN_BBA              (*(volatile unsigned long *) 0x4204849C)
#define EI1CFG_IRQ5EN_MSK              (0x1   << 7  )
#define EI1CFG_IRQ5EN                  (0x1   << 7  )
#define EI1CFG_IRQ5EN_DIS              (0x0   << 7  ) /* DIS. External interrupt 5 disabled. */
#define EI1CFG_IRQ5EN_EN               (0x1   << 7  ) /* EN. External Interrupt 5 enabled. */

/* EI1CFG[IRQ5MDE] - External interrupt 5 detection mode. */
#define EI1CFG_IRQ5MDE_MSK             (0x7   << 4  )
#define EI1CFG_IRQ5MDE_RISE            (0x0   << 4  ) /* RISE. Rising edge.       */
#define EI1CFG_IRQ5MDE_FALL            (0x1   << 4  ) /* FALL. Falling edge.      */
#define EI1CFG_IRQ5MDE_RISEORFALL      (0x2   << 4  ) /* RISEORFALL. Rising or falling edge. */
#define EI1CFG_IRQ5MDE_HIGHLEVEL       (0x3   << 4  ) /* HIGHLEVEL. High level.   */
#define EI1CFG_IRQ5MDE_LOWLEVEL        (0x4   << 4  ) /* LOWLEVEL. Low Level.     */

/* EI1CFG[IRQ4EN] - External interrupt 4 enable bit. */
#define EI1CFG_IRQ4EN_BBA              (*(volatile unsigned long *) 0x4204848C)
#define EI1CFG_IRQ4EN_MSK              (0x1   << 3  )
#define EI1CFG_IRQ4EN                  (0x1   << 3  )
#define EI1CFG_IRQ4EN_DIS              (0x0   << 3  ) /* DIS. External interrupt 4 disabled. */
#define EI1CFG_IRQ4EN_EN               (0x1   << 3  ) /* EN. External Interrupt 4 enabled. */

/* EI1CFG[IRQ4MDE] - External interrupt 4 detection mode. */
#define EI1CFG_IRQ4MDE_MSK             (0x7   << 0  )
#define EI1CFG_IRQ4MDE_RISE            (0x0   << 0  ) /* RISE. Rising edge.       */
#define EI1CFG_IRQ4MDE_FALL            (0x1   << 0  ) /* FALL. Falling edge.      */
#define EI1CFG_IRQ4MDE_RISEORFALL      (0x2   << 0  ) /* RISEORFALL. Rising or falling edge. */
#define EI1CFG_IRQ4MDE_HIGHLEVEL       (0x3   << 0  ) /* HIGHLEVEL. High level.   */
#define EI1CFG_IRQ4MDE_LOWLEVEL        (0x4   << 0  ) /* LOWLEVEL. Low Level.     */

/* Reset Value for EI2CFG*/
#define EI2CFG_RVAL                    0x0

/* EI2CFG[IRQ8EN] - RF transceiver IRQ enable bit. */
#define EI2CFG_IRQ8EN_BBA              (*(volatile unsigned long *) 0x4204850C)
#define EI2CFG_IRQ8EN_MSK              (0x1   << 3  )
#define EI2CFG_IRQ8EN                  (0x1   << 3  )
#define EI2CFG_IRQ8EN_DIS              (0x0   << 3  ) /* DIS. RF transceiver IRQ disabled. */
#define EI2CFG_IRQ8EN_EN               (0x1   << 3  ) /* EN.  RF transceiver IRQ enabled. */

/* EI2CFG[IRQ8MDE] - RF transceiver IRQ detection mode. */
#define EI2CFG_IRQ8MDE_MSK             (0x7   << 0  )
#define EI2CFG_IRQ8MDE_RISE            (0x0   << 0  ) /* RISE. Rising edge.       */
#define EI2CFG_IRQ8MDE_FALL            (0x1   << 0  ) /* FALL. Falling edge.      */
#define EI2CFG_IRQ8MDE_RISEORFALL      (0x2   << 0  ) /* RISEORFALL. Rising or falling edge. */
#define EI2CFG_IRQ8MDE_HIGHLEVEL       (0x3   << 0  ) /* HIGHLEVEL. High level.   */
#define EI2CFG_IRQ8MDE_LOWLEVEL        (0x4   << 0  ) /* LOWLEVEL. Low level.     */

/* Reset Value for EICLR*/
#define EICLR_RVAL                     0x0

/* EICLR[IRQ8] - External interrupt 8 (RF transceiver) clear bit. */
#define EICLR_IRQ8_BBA                 (*(volatile unsigned long *) 0x42048620)
#define EICLR_IRQ8_MSK                 (0x1   << 8  )
#define EICLR_IRQ8                     (0x1   << 8  )
#define EICLR_IRQ8_CLR                 (0x1   << 8  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ7] - External interrupt 7 clear bit. */
#define EICLR_IRQ7_BBA                 (*(volatile unsigned long *) 0x4204861C)
#define EICLR_IRQ7_MSK                 (0x1   << 7  )
#define EICLR_IRQ7                     (0x1   << 7  )
#define EICLR_IRQ7_CLR                 (0x1   << 7  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ6] - External interrupt 6 clear bit. */
#define EICLR_IRQ6_BBA                 (*(volatile unsigned long *) 0x42048618)
#define EICLR_IRQ6_MSK                 (0x1   << 6  )
#define EICLR_IRQ6                     (0x1   << 6  )
#define EICLR_IRQ6_CLR                 (0x1   << 6  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ5] - External interrupt 5 clear bit. */
#define EICLR_IRQ5_BBA                 (*(volatile unsigned long *) 0x42048614)
#define EICLR_IRQ5_MSK                 (0x1   << 5  )
#define EICLR_IRQ5                     (0x1   << 5  )
#define EICLR_IRQ5_CLR                 (0x1   << 5  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ4] - External interrupt 4 clear bit. */
#define EICLR_IRQ4_BBA                 (*(volatile unsigned long *) 0x42048610)
#define EICLR_IRQ4_MSK                 (0x1   << 4  )
#define EICLR_IRQ4                     (0x1   << 4  )
#define EICLR_IRQ4_CLR                 (0x1   << 4  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ3] - External interrupt 3 clear bit. */
#define EICLR_IRQ3_BBA                 (*(volatile unsigned long *) 0x4204860C)
#define EICLR_IRQ3_MSK                 (0x1   << 3  )
#define EICLR_IRQ3                     (0x1   << 3  )
#define EICLR_IRQ3_CLR                 (0x1   << 3  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ2] - External interrupt 2 clear bit. */
#define EICLR_IRQ2_BBA                 (*(volatile unsigned long *) 0x42048608)
#define EICLR_IRQ2_MSK                 (0x1   << 2  )
#define EICLR_IRQ2                     (0x1   << 2  )
#define EICLR_IRQ2_CLR                 (0x1   << 2  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ1] - External interrupt 1 clear bit. */
#define EICLR_IRQ1_BBA                 (*(volatile unsigned long *) 0x42048604)
#define EICLR_IRQ1_MSK                 (0x1   << 1  )
#define EICLR_IRQ1                     (0x1   << 1  )
#define EICLR_IRQ1_CLR                 (0x1   << 1  ) /* CLR. Clear an internal interrupt flag. */

/* EICLR[IRQ0] - External interrupt 0 clear bit. */
#define EICLR_IRQ0_BBA                 (*(volatile unsigned long *) 0x42048600)
#define EICLR_IRQ0_MSK                 (0x1   << 0  )
#define EICLR_IRQ0                     (0x1   << 0  )
#define EICLR_IRQ0_CLR                 (0x1   << 0  ) /* CLR. Clear an internal interrupt flag. */

/* Reset Value for NMICLR*/
#define NMICLR_RVAL                    0x0

/* NMICLR[CLEAR] - NMI clear bit. */
#define NMICLR_CLEAR_BBA               (*(volatile unsigned long *) 0x42048680)
#define NMICLR_CLEAR_MSK               (0x1   << 0  )
#define NMICLR_CLEAR                   (0x1   << 0  )
#define NMICLR_CLEAR_EN                (0x1   << 0  ) /* EN. Clear an internal interrupt flag when the NMI interrupt is set. */
// ------------------------------------------------------------------------------------------------
// -----                                        NVIC                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Nested Vectored Interrupt Controller (pADI_NVIC)
  */

#if (__NO_MMR_STRUCTS__==0)
#else // (__NO_MMR_STRUCTS__==0)
#define          ICTR                                       (*(volatile unsigned long      *) 0xE000E004)
#define          STCSR                                      (*(volatile unsigned long      *) 0xE000E010)
#define          STRVR                                      (*(volatile unsigned long      *) 0xE000E014)
#define          STCVR                                      (*(volatile unsigned long      *) 0xE000E018)
#define          STCR                                       (*(volatile unsigned long      *) 0xE000E01C)
#define          ISER0                                      (*(volatile unsigned long      *) 0xE000E100)
#define          ISER1                                      (*(volatile unsigned long      *) 0xE000E104)
#define          ICER0                                      (*(volatile unsigned long      *) 0xE000E180)
#define          ICER1                                      (*(volatile unsigned long      *) 0xE000E184)
#define          ISPR0                                      (*(volatile unsigned long      *) 0xE000E200)
#define          ISPR1                                      (*(volatile unsigned long      *) 0xE000E204)
#define          ICPR0                                      (*(volatile unsigned long      *) 0xE000E280)
#define          ICPR1                                      (*(volatile unsigned long      *) 0xE000E284)
#define          IABR0                                      (*(volatile unsigned long      *) 0xE000E300)
#define          IABR1                                      (*(volatile unsigned long      *) 0xE000E304)
#define          IPR0                                       (*(volatile unsigned long      *) 0xE000E400)
#define          IPR1                                       (*(volatile unsigned long      *) 0xE000E404)
#define          IPR2                                       (*(volatile unsigned long      *) 0xE000E408)
#define          IPR3                                       (*(volatile unsigned long      *) 0xE000E40C)
#define          IPR4                                       (*(volatile unsigned long      *) 0xE000E410)
#define          IPR5                                       (*(volatile unsigned long      *) 0xE000E414)
#define          IPR6                                       (*(volatile unsigned long      *) 0xE000E418)
#define          IPR7                                       (*(volatile unsigned long      *) 0xE000E41C)
#define          IPR8                                       (*(volatile unsigned long      *) 0xE000E420)
#define          IPR9                                       (*(volatile unsigned long      *) 0xE000E424)
#define          IPR10                                      (*(volatile unsigned long      *) 0xE000E428)
#define          CPUID                                      (*(volatile unsigned long      *) 0xE000ED00)
#define          ICSR                                       (*(volatile unsigned long      *) 0xE000ED04)
#define          VTOR                                       (*(volatile unsigned long      *) 0xE000ED08)
#define          AIRCR                                      (*(volatile unsigned long      *) 0xE000ED0C)
#define          SCR                                        (*(volatile unsigned long      *) 0xE000ED10)
#define          CCR                                        (*(volatile unsigned long      *) 0xE000ED14)
#define          SHPR1                                      (*(volatile unsigned long      *) 0xE000ED18)
#define          SHPR2                                      (*(volatile unsigned long      *) 0xE000ED1C)
#define          SHPR3                                      (*(volatile unsigned long      *) 0xE000ED20)
#define          SHCSR                                      (*(volatile unsigned long      *) 0xE000ED24)
#define          CFSR                                       (*(volatile unsigned long      *) 0xE000ED28)
#define          HFSR                                       (*(volatile unsigned long      *) 0xE000ED2C)
#define          MMFAR                                      (*(volatile unsigned long      *) 0xE000ED34)
#define          BFAR                                       (*(volatile unsigned long      *) 0xE000ED38)
#define          STIR                                       (*(volatile unsigned long      *) 0xE000EF00)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for ICTR*/
#define ICTR_RVAL                      0x1

/* ICTR[INTLINESNUM] - Total number of interrupt lines in groups of 32 */
#define ICTR_INTLINESNUM_MSK           (0xF   << 0  )

/* Reset Value for STCSR*/
#define STCSR_RVAL                     0x0

/* STCSR[COUNTFLAG] - Returns 1 if timer counted to 0 since last time this register was read */
#define STCSR_COUNTFLAG_MSK            (0x1   << 16 )
#define STCSR_COUNTFLAG                (0x1   << 16 )
#define STCSR_COUNTFLAG_DIS            (0x0   << 16 ) /* DIS                      */
#define STCSR_COUNTFLAG_EN             (0x1   << 16 ) /* EN                       */

/* STCSR[CLKSOURCE] - clock source used for SysTick */
#define STCSR_CLKSOURCE_MSK            (0x1   << 2  )
#define STCSR_CLKSOURCE                (0x1   << 2  )
#define STCSR_CLKSOURCE_DIS            (0x0   << 2  ) /* DIS                      */
#define STCSR_CLKSOURCE_EN             (0x1   << 2  ) /* EN                       */

/* STCSR[TICKINT] - If 1, counting down to 0 will cause the SysTick exception to pended. */
#define STCSR_TICKINT_MSK              (0x1   << 1  )
#define STCSR_TICKINT                  (0x1   << 1  )
#define STCSR_TICKINT_DIS              (0x0   << 1  ) /* DIS                      */
#define STCSR_TICKINT_EN               (0x1   << 1  ) /* EN                       */

/* STCSR[ENABLE] - Enable bit */
#define STCSR_ENABLE_MSK               (0x1   << 0  )
#define STCSR_ENABLE                   (0x1   << 0  )
#define STCSR_ENABLE_DIS               (0x0   << 0  ) /* DIS                      */
#define STCSR_ENABLE_EN                (0x1   << 0  ) /* EN                       */

/* Reset Value for STRVR*/
#define STRVR_RVAL                     0x0

/* STRVR[RELOAD] - Value to load into the Current Value register when the counter reaches 0 */
#define STRVR_RELOAD_MSK               (0xFFFFFF << 0  )

/* Reset Value for STCVR*/
#define STCVR_RVAL                     0x0

/* STCVR[CURRENT] - Current counter value */
#define STCVR_CURRENT_MSK              (0xFFFFFFFF << 0  )

/* Reset Value for STCR*/
#define STCR_RVAL                      0x0

/* STCR[NOREF] - If reads as 1, the Reference clock is not provided */
#define STCR_NOREF_MSK                 (0x1   << 31 )
#define STCR_NOREF                     (0x1   << 31 )
#define STCR_NOREF_DIS                 (0x0   << 31 ) /* DIS                      */
#define STCR_NOREF_EN                  (0x1   << 31 ) /* EN                       */

/* STCR[SKEW] - If reads as 1, the calibration value for 10ms is inexact */
#define STCR_SKEW_MSK                  (0x1   << 30 )
#define STCR_SKEW                      (0x1   << 30 )
#define STCR_SKEW_DIS                  (0x0   << 30 ) /* DIS                      */
#define STCR_SKEW_EN                   (0x1   << 30 ) /* EN                       */

/* STCR[TENMS] - An optional Reload value to be used for 10ms (100Hz) timing */
#define STCR_TENMS_MSK                 (0xFFFFFF << 0  )

/* Reset Value for ISER0*/
#define ISER0_RVAL                     0x0

/* ISER0[DMAI2CMRX] -  */
#define ISER0_DMAI2CMRX_MSK            (0x1   << 30 )
#define ISER0_DMAI2CMRX                (0x1   << 30 )
#define ISER0_DMAI2CMRX_DIS            (0x0   << 30 ) /* DIS                      */
#define ISER0_DMAI2CMRX_EN             (0x1   << 30 ) /* EN                       */

/* ISER0[DMAI2CMTX] -  */
#define ISER0_DMAI2CMTX_MSK            (0x1   << 29 )
#define ISER0_DMAI2CMTX                (0x1   << 29 )
#define ISER0_DMAI2CMTX_DIS            (0x0   << 29 ) /* DIS                      */
#define ISER0_DMAI2CMTX_EN             (0x1   << 29 ) /* EN                       */

/* ISER0[DMAI2CSRX] -  */
#define ISER0_DMAI2CSRX_MSK            (0x1   << 28 )
#define ISER0_DMAI2CSRX                (0x1   << 28 )
#define ISER0_DMAI2CSRX_DIS            (0x0   << 28 ) /* DIS                      */
#define ISER0_DMAI2CSRX_EN             (0x1   << 28 ) /* EN                       */

/* ISER0[DMAI2CSTX] -  */
#define ISER0_DMAI2CSTX_MSK            (0x1   << 27 )
#define ISER0_DMAI2CSTX                (0x1   << 27 )
#define ISER0_DMAI2CSTX_DIS            (0x0   << 27 ) /* DIS                      */
#define ISER0_DMAI2CSTX_EN             (0x1   << 27 ) /* EN                       */

/* ISER0[DMAUARTRX] -  */
#define ISER0_DMAUARTRX_MSK            (0x1   << 26 )
#define ISER0_DMAUARTRX                (0x1   << 26 )
#define ISER0_DMAUARTRX_DIS            (0x0   << 26 ) /* DIS                      */
#define ISER0_DMAUARTRX_EN             (0x1   << 26 ) /* EN                       */

/* ISER0[DMAUARTTX] -  */
#define ISER0_DMAUARTTX_MSK            (0x1   << 25 )
#define ISER0_DMAUARTTX                (0x1   << 25 )
#define ISER0_DMAUARTTX_DIS            (0x0   << 25 ) /* DIS                      */
#define ISER0_DMAUARTTX_EN             (0x1   << 25 ) /* EN                       */

/* ISER0[DMASPI1RX] -  */
#define ISER0_DMASPI1RX_MSK            (0x1   << 24 )
#define ISER0_DMASPI1RX                (0x1   << 24 )
#define ISER0_DMASPI1RX_DIS            (0x0   << 24 ) /* DIS                      */
#define ISER0_DMASPI1RX_EN             (0x1   << 24 ) /* EN                       */

/* ISER0[DMASPI1TX] -  */
#define ISER0_DMASPI1TX_MSK            (0x1   << 23 )
#define ISER0_DMASPI1TX                (0x1   << 23 )
#define ISER0_DMASPI1TX_DIS            (0x0   << 23 ) /* DIS                      */
#define ISER0_DMASPI1TX_EN             (0x1   << 23 ) /* EN                       */

/* ISER0[DMAERROR] -  */
#define ISER0_DMAERROR_MSK             (0x1   << 22 )
#define ISER0_DMAERROR                 (0x1   << 22 )
#define ISER0_DMAERROR_DIS             (0x0   << 22 ) /* DIS                      */
#define ISER0_DMAERROR_EN              (0x1   << 22 ) /* EN                       */

/* ISER0[I2CM] -  */
#define ISER0_I2CM_MSK                 (0x1   << 20 )
#define ISER0_I2CM                     (0x1   << 20 )
#define ISER0_I2CM_DIS                 (0x0   << 20 ) /* DIS                      */
#define ISER0_I2CM_EN                  (0x1   << 20 ) /* EN                       */

/* ISER0[I2CS] -  */
#define ISER0_I2CS_MSK                 (0x1   << 19 )
#define ISER0_I2CS                     (0x1   << 19 )
#define ISER0_I2CS_DIS                 (0x0   << 19 ) /* DIS                      */
#define ISER0_I2CS_EN                  (0x1   << 19 ) /* EN                       */

/* ISER0[SPI1] -  */
#define ISER0_SPI1_MSK                 (0x1   << 18 )
#define ISER0_SPI1                     (0x1   << 18 )
#define ISER0_SPI1_DIS                 (0x0   << 18 ) /* DIS                      */
#define ISER0_SPI1_EN                  (0x1   << 18 ) /* EN                       */

/* ISER0[SPI0] -  */
#define ISER0_SPI0_MSK                 (0x1   << 17 )
#define ISER0_SPI0                     (0x1   << 17 )
#define ISER0_SPI0_DIS                 (0x0   << 17 ) /* DIS                      */
#define ISER0_SPI0_EN                  (0x1   << 17 ) /* EN                       */

/* ISER0[UART] -  */
#define ISER0_UART_MSK                 (0x1   << 16 )
#define ISER0_UART                     (0x1   << 16 )
#define ISER0_UART_DIS                 (0x0   << 16 ) /* DIS                      */
#define ISER0_UART_EN                  (0x1   << 16 ) /* EN                       */

/* ISER0[FEE] -  */
#define ISER0_FEE_MSK                  (0x1   << 15 )
#define ISER0_FEE                      (0x1   << 15 )
#define ISER0_FEE_DIS                  (0x0   << 15 ) /* DIS                      */
#define ISER0_FEE_EN                   (0x1   << 15 ) /* EN                       */

/* ISER0[ADC] -  */
#define ISER0_ADC_MSK                  (0x1   << 14 )
#define ISER0_ADC                      (0x1   << 14 )
#define ISER0_ADC_DIS                  (0x0   << 14 ) /* DIS                      */
#define ISER0_ADC_EN                   (0x1   << 14 ) /* EN                       */

/* ISER0[T1] -  */
#define ISER0_T1_MSK                   (0x1   << 13 )
#define ISER0_T1                       (0x1   << 13 )
#define ISER0_T1_DIS                   (0x0   << 13 ) /* DIS                      */
#define ISER0_T1_EN                    (0x1   << 13 ) /* EN                       */

/* ISER0[T0] -  */
#define ISER0_T0_MSK                   (0x1   << 12 )
#define ISER0_T0                       (0x1   << 12 )
#define ISER0_T0_DIS                   (0x0   << 12 ) /* DIS                      */
#define ISER0_T0_EN                    (0x1   << 12 ) /* EN                       */

/* ISER0[T3] -  */
#define ISER0_T3_MSK                   (0x1   << 10 )
#define ISER0_T3                       (0x1   << 10 )
#define ISER0_T3_DIS                   (0x0   << 10 ) /* DIS                      */
#define ISER0_T3_EN                    (0x1   << 10 ) /* EN                       */

/* ISER0[EXTINT8] -  */
#define ISER0_EXTINT8_MSK              (0x1   << 9  )
#define ISER0_EXTINT8                  (0x1   << 9  )
#define ISER0_EXTINT8_DIS              (0x0   << 9  ) /* DIS                      */
#define ISER0_EXTINT8_EN               (0x1   << 9  ) /* EN                       */

/* ISER0[EXTINT7] -  */
#define ISER0_EXTINT7_MSK              (0x1   << 8  )
#define ISER0_EXTINT7                  (0x1   << 8  )
#define ISER0_EXTINT7_DIS              (0x0   << 8  ) /* DIS                      */
#define ISER0_EXTINT7_EN               (0x1   << 8  ) /* EN                       */

/* ISER0[EXTINT6] -  */
#define ISER0_EXTINT6_MSK              (0x1   << 7  )
#define ISER0_EXTINT6                  (0x1   << 7  )
#define ISER0_EXTINT6_DIS              (0x0   << 7  ) /* DIS                      */
#define ISER0_EXTINT6_EN               (0x1   << 7  ) /* EN                       */

/* ISER0[EXTINT5] -  */
#define ISER0_EXTINT5_MSK              (0x1   << 6  )
#define ISER0_EXTINT5                  (0x1   << 6  )
#define ISER0_EXTINT5_DIS              (0x0   << 6  ) /* DIS                      */
#define ISER0_EXTINT5_EN               (0x1   << 6  ) /* EN                       */

/* ISER0[EXTINT4] -  */
#define ISER0_EXTINT4_MSK              (0x1   << 5  )
#define ISER0_EXTINT4                  (0x1   << 5  )
#define ISER0_EXTINT4_DIS              (0x0   << 5  ) /* DIS                      */
#define ISER0_EXTINT4_EN               (0x1   << 5  ) /* EN                       */

/* ISER0[EXTINT3] -  */
#define ISER0_EXTINT3_MSK              (0x1   << 4  )
#define ISER0_EXTINT3                  (0x1   << 4  )
#define ISER0_EXTINT3_DIS              (0x0   << 4  ) /* DIS                      */
#define ISER0_EXTINT3_EN               (0x1   << 4  ) /* EN                       */

/* ISER0[EXTINT2] -  */
#define ISER0_EXTINT2_MSK              (0x1   << 3  )
#define ISER0_EXTINT2                  (0x1   << 3  )
#define ISER0_EXTINT2_DIS              (0x0   << 3  ) /* DIS                      */
#define ISER0_EXTINT2_EN               (0x1   << 3  ) /* EN                       */

/* ISER0[EXTINT1] -  */
#define ISER0_EXTINT1_MSK              (0x1   << 2  )
#define ISER0_EXTINT1                  (0x1   << 2  )
#define ISER0_EXTINT1_DIS              (0x0   << 2  ) /* DIS                      */
#define ISER0_EXTINT1_EN               (0x1   << 2  ) /* EN                       */

/* ISER0[EXTINT0] -  */
#define ISER0_EXTINT0_MSK              (0x1   << 1  )
#define ISER0_EXTINT0                  (0x1   << 1  )
#define ISER0_EXTINT0_DIS              (0x0   << 1  ) /* DIS                      */
#define ISER0_EXTINT0_EN               (0x1   << 1  ) /* EN                       */

/* ISER0[T2] -  */
#define ISER0_T2_MSK                   (0x1   << 0  )
#define ISER0_T2                       (0x1   << 0  )
#define ISER0_T2_DIS                   (0x0   << 0  ) /* DIS                      */
#define ISER0_T2_EN                    (0x1   << 0  ) /* EN                       */

/* Reset Value for ISER1*/
#define ISER1_RVAL                     0x0

/* ISER1[PWM3] -  */
#define ISER1_PWM3_MSK                 (0x1   << 9  )
#define ISER1_PWM3                     (0x1   << 9  )
#define ISER1_PWM3_DIS                 (0x0   << 9  ) /* DIS                      */
#define ISER1_PWM3_EN                  (0x1   << 9  ) /* EN                       */

/* ISER1[PWM2] -  */
#define ISER1_PWM2_MSK                 (0x1   << 8  )
#define ISER1_PWM2                     (0x1   << 8  )
#define ISER1_PWM2_DIS                 (0x0   << 8  ) /* DIS                      */
#define ISER1_PWM2_EN                  (0x1   << 8  ) /* EN                       */

/* ISER1[PWM1] -  */
#define ISER1_PWM1_MSK                 (0x1   << 7  )
#define ISER1_PWM1                     (0x1   << 7  )
#define ISER1_PWM1_DIS                 (0x0   << 7  ) /* DIS                      */
#define ISER1_PWM1_EN                  (0x1   << 7  ) /* EN                       */

/* ISER1[PWM0] -  */
#define ISER1_PWM0_MSK                 (0x1   << 6  )
#define ISER1_PWM0                     (0x1   << 6  )
#define ISER1_PWM0_DIS                 (0x0   << 6  ) /* DIS                      */
#define ISER1_PWM0_EN                  (0x1   << 6  ) /* EN                       */

/* ISER1[PWMTRIP] -  */
#define ISER1_PWMTRIP_MSK              (0x1   << 5  )
#define ISER1_PWMTRIP                  (0x1   << 5  )
#define ISER1_PWMTRIP_DIS              (0x0   << 5  ) /* DIS                      */
#define ISER1_PWMTRIP_EN               (0x1   << 5  ) /* EN                       */

/* ISER1[DMASPI0RX] -  */
#define ISER1_DMASPI0RX_MSK            (0x1   << 4  )
#define ISER1_DMASPI0RX                (0x1   << 4  )
#define ISER1_DMASPI0RX_DIS            (0x0   << 4  ) /* DIS                      */
#define ISER1_DMASPI0RX_EN             (0x1   << 4  ) /* EN                       */

/* ISER1[DMASPI0TX] -  */
#define ISER1_DMASPI0TX_MSK            (0x1   << 3  )
#define ISER1_DMASPI0TX                (0x1   << 3  )
#define ISER1_DMASPI0TX_DIS            (0x0   << 3  ) /* DIS                      */
#define ISER1_DMASPI0TX_EN             (0x1   << 3  ) /* EN                       */

/* ISER1[DMAADC] -  */
#define ISER1_DMAADC_MSK               (0x1   << 2  )
#define ISER1_DMAADC                   (0x1   << 2  )
#define ISER1_DMAADC_DIS               (0x0   << 2  ) /* DIS                      */
#define ISER1_DMAADC_EN                (0x1   << 2  ) /* EN                       */

/* Reset Value for ICER0*/
#define ICER0_RVAL                     0x0

/* ICER0[DMAI2CMRX] -  */
#define ICER0_DMAI2CMRX_MSK            (0x1   << 30 )
#define ICER0_DMAI2CMRX                (0x1   << 30 )
#define ICER0_DMAI2CMRX_DIS            (0x0   << 30 ) /* DIS                      */
#define ICER0_DMAI2CMRX_EN             (0x1   << 30 ) /* EN                       */

/* ICER0[DMAI2CMTX] -  */
#define ICER0_DMAI2CMTX_MSK            (0x1   << 29 )
#define ICER0_DMAI2CMTX                (0x1   << 29 )
#define ICER0_DMAI2CMTX_DIS            (0x0   << 29 ) /* DIS                      */
#define ICER0_DMAI2CMTX_EN             (0x1   << 29 ) /* EN                       */

/* ICER0[DMAI2CSRX] -  */
#define ICER0_DMAI2CSRX_MSK            (0x1   << 28 )
#define ICER0_DMAI2CSRX                (0x1   << 28 )
#define ICER0_DMAI2CSRX_DIS            (0x0   << 28 ) /* DIS                      */
#define ICER0_DMAI2CSRX_EN             (0x1   << 28 ) /* EN                       */

/* ICER0[DMAI2CSTX] -  */
#define ICER0_DMAI2CSTX_MSK            (0x1   << 27 )
#define ICER0_DMAI2CSTX                (0x1   << 27 )
#define ICER0_DMAI2CSTX_DIS            (0x0   << 27 ) /* DIS                      */
#define ICER0_DMAI2CSTX_EN             (0x1   << 27 ) /* EN                       */

/* ICER0[DMAUARTRX] -  */
#define ICER0_DMAUARTRX_MSK            (0x1   << 26 )
#define ICER0_DMAUARTRX                (0x1   << 26 )
#define ICER0_DMAUARTRX_DIS            (0x0   << 26 ) /* DIS                      */
#define ICER0_DMAUARTRX_EN             (0x1   << 26 ) /* EN                       */

/* ICER0[DMAUARTTX] -  */
#define ICER0_DMAUARTTX_MSK            (0x1   << 25 )
#define ICER0_DMAUARTTX                (0x1   << 25 )
#define ICER0_DMAUARTTX_DIS            (0x0   << 25 ) /* DIS                      */
#define ICER0_DMAUARTTX_EN             (0x1   << 25 ) /* EN                       */

/* ICER0[DMASPI1RX] -  */
#define ICER0_DMASPI1RX_MSK            (0x1   << 24 )
#define ICER0_DMASPI1RX                (0x1   << 24 )
#define ICER0_DMASPI1RX_DIS            (0x0   << 24 ) /* DIS                      */
#define ICER0_DMASPI1RX_EN             (0x1   << 24 ) /* EN                       */

/* ICER0[DMASPI1TX] -  */
#define ICER0_DMASPI1TX_MSK            (0x1   << 23 )
#define ICER0_DMASPI1TX                (0x1   << 23 )
#define ICER0_DMASPI1TX_DIS            (0x0   << 23 ) /* DIS                      */
#define ICER0_DMASPI1TX_EN             (0x1   << 23 ) /* EN                       */

/* ICER0[DMAERROR] -  */
#define ICER0_DMAERROR_MSK             (0x1   << 22 )
#define ICER0_DMAERROR                 (0x1   << 22 )
#define ICER0_DMAERROR_DIS             (0x0   << 22 ) /* DIS                      */
#define ICER0_DMAERROR_EN              (0x1   << 22 ) /* EN                       */

/* ICER0[I2CM] -  */
#define ICER0_I2CM_MSK                 (0x1   << 20 )
#define ICER0_I2CM                     (0x1   << 20 )
#define ICER0_I2CM_DIS                 (0x0   << 20 ) /* DIS                      */
#define ICER0_I2CM_EN                  (0x1   << 20 ) /* EN                       */

/* ICER0[I2CS] -  */
#define ICER0_I2CS_MSK                 (0x1   << 19 )
#define ICER0_I2CS                     (0x1   << 19 )
#define ICER0_I2CS_DIS                 (0x0   << 19 ) /* DIS                      */
#define ICER0_I2CS_EN                  (0x1   << 19 ) /* EN                       */

/* ICER0[SPI1] -  */
#define ICER0_SPI1_MSK                 (0x1   << 18 )
#define ICER0_SPI1                     (0x1   << 18 )
#define ICER0_SPI1_DIS                 (0x0   << 18 ) /* DIS                      */
#define ICER0_SPI1_EN                  (0x1   << 18 ) /* EN                       */

/* ICER0[SPI0] -  */
#define ICER0_SPI0_MSK                 (0x1   << 17 )
#define ICER0_SPI0                     (0x1   << 17 )
#define ICER0_SPI0_DIS                 (0x0   << 17 ) /* DIS                      */
#define ICER0_SPI0_EN                  (0x1   << 17 ) /* EN                       */

/* ICER0[UART] -  */
#define ICER0_UART_MSK                 (0x1   << 16 )
#define ICER0_UART                     (0x1   << 16 )
#define ICER0_UART_DIS                 (0x0   << 16 ) /* DIS                      */
#define ICER0_UART_EN                  (0x1   << 16 ) /* EN                       */

/* ICER0[FEE] -  */
#define ICER0_FEE_MSK                  (0x1   << 15 )
#define ICER0_FEE                      (0x1   << 15 )
#define ICER0_FEE_DIS                  (0x0   << 15 ) /* DIS                      */
#define ICER0_FEE_EN                   (0x1   << 15 ) /* EN                       */

/* ICER0[ADC] -  */
#define ICER0_ADC_MSK                  (0x1   << 14 )
#define ICER0_ADC                      (0x1   << 14 )
#define ICER0_ADC_DIS                  (0x0   << 14 ) /* DIS                      */
#define ICER0_ADC_EN                   (0x1   << 14 ) /* EN                       */

/* ICER0[T1] -  */
#define ICER0_T1_MSK                   (0x1   << 13 )
#define ICER0_T1                       (0x1   << 13 )
#define ICER0_T1_DIS                   (0x0   << 13 ) /* DIS                      */
#define ICER0_T1_EN                    (0x1   << 13 ) /* EN                       */

/* ICER0[T0] -  */
#define ICER0_T0_MSK                   (0x1   << 12 )
#define ICER0_T0                       (0x1   << 12 )
#define ICER0_T0_DIS                   (0x0   << 12 ) /* DIS                      */
#define ICER0_T0_EN                    (0x1   << 12 ) /* EN                       */

/* ICER0[T3] -  */
#define ICER0_T3_MSK                   (0x1   << 10 )
#define ICER0_T3                       (0x1   << 10 )
#define ICER0_T3_DIS                   (0x0   << 10 ) /* DIS                      */
#define ICER0_T3_EN                    (0x1   << 10 ) /* EN                       */

/* ICER0[EXTINT8] -  */
#define ICER0_EXTINT8_MSK              (0x1   << 9  )
#define ICER0_EXTINT8                  (0x1   << 9  )
#define ICER0_EXTINT8_DIS              (0x0   << 9  ) /* DIS                      */
#define ICER0_EXTINT8_EN               (0x1   << 9  ) /* EN                       */

/* ICER0[EXTINT7] -  */
#define ICER0_EXTINT7_MSK              (0x1   << 8  )
#define ICER0_EXTINT7                  (0x1   << 8  )
#define ICER0_EXTINT7_DIS              (0x0   << 8  ) /* DIS                      */
#define ICER0_EXTINT7_EN               (0x1   << 8  ) /* EN                       */

/* ICER0[EXTINT6] -  */
#define ICER0_EXTINT6_MSK              (0x1   << 7  )
#define ICER0_EXTINT6                  (0x1   << 7  )
#define ICER0_EXTINT6_DIS              (0x0   << 7  ) /* DIS                      */
#define ICER0_EXTINT6_EN               (0x1   << 7  ) /* EN                       */

/* ICER0[EXTINT5] -  */
#define ICER0_EXTINT5_MSK              (0x1   << 6  )
#define ICER0_EXTINT5                  (0x1   << 6  )
#define ICER0_EXTINT5_DIS              (0x0   << 6  ) /* DIS                      */
#define ICER0_EXTINT5_EN               (0x1   << 6  ) /* EN                       */

/* ICER0[EXTINT4] -  */
#define ICER0_EXTINT4_MSK              (0x1   << 5  )
#define ICER0_EXTINT4                  (0x1   << 5  )
#define ICER0_EXTINT4_DIS              (0x0   << 5  ) /* DIS                      */
#define ICER0_EXTINT4_EN               (0x1   << 5  ) /* EN                       */

/* ICER0[EXTINT3] -  */
#define ICER0_EXTINT3_MSK              (0x1   << 4  )
#define ICER0_EXTINT3                  (0x1   << 4  )
#define ICER0_EXTINT3_DIS              (0x0   << 4  ) /* DIS                      */
#define ICER0_EXTINT3_EN               (0x1   << 4  ) /* EN                       */

/* ICER0[EXTINT2] -  */
#define ICER0_EXTINT2_MSK              (0x1   << 3  )
#define ICER0_EXTINT2                  (0x1   << 3  )
#define ICER0_EXTINT2_DIS              (0x0   << 3  ) /* DIS                      */
#define ICER0_EXTINT2_EN               (0x1   << 3  ) /* EN                       */

/* ICER0[EXTINT1] -  */
#define ICER0_EXTINT1_MSK              (0x1   << 2  )
#define ICER0_EXTINT1                  (0x1   << 2  )
#define ICER0_EXTINT1_DIS              (0x0   << 2  ) /* DIS                      */
#define ICER0_EXTINT1_EN               (0x1   << 2  ) /* EN                       */

/* ICER0[EXTINT0] -  */
#define ICER0_EXTINT0_MSK              (0x1   << 1  )
#define ICER0_EXTINT0                  (0x1   << 1  )
#define ICER0_EXTINT0_DIS              (0x0   << 1  ) /* DIS                      */
#define ICER0_EXTINT0_EN               (0x1   << 1  ) /* EN                       */

/* ICER0[T2] -  */
#define ICER0_T2_MSK                   (0x1   << 0  )
#define ICER0_T2                       (0x1   << 0  )
#define ICER0_T2_DIS                   (0x0   << 0  ) /* DIS                      */
#define ICER0_T2_EN                    (0x1   << 0  ) /* EN                       */

/* Reset Value for ICER1*/
#define ICER1_RVAL                     0x0

/* ICER1[PWM3] -  */
#define ICER1_PWM3_MSK                 (0x1   << 9  )
#define ICER1_PWM3                     (0x1   << 9  )
#define ICER1_PWM3_DIS                 (0x0   << 9  ) /* DIS                      */
#define ICER1_PWM3_EN                  (0x1   << 9  ) /* EN                       */

/* ICER1[PWM2] -  */
#define ICER1_PWM2_MSK                 (0x1   << 8  )
#define ICER1_PWM2                     (0x1   << 8  )
#define ICER1_PWM2_DIS                 (0x0   << 8  ) /* DIS                      */
#define ICER1_PWM2_EN                  (0x1   << 8  ) /* EN                       */

/* ICER1[PWM1] -  */
#define ICER1_PWM1_MSK                 (0x1   << 7  )
#define ICER1_PWM1                     (0x1   << 7  )
#define ICER1_PWM1_DIS                 (0x0   << 7  ) /* DIS                      */
#define ICER1_PWM1_EN                  (0x1   << 7  ) /* EN                       */

/* ICER1[PWM0] -  */
#define ICER1_PWM0_MSK                 (0x1   << 6  )
#define ICER1_PWM0                     (0x1   << 6  )
#define ICER1_PWM0_DIS                 (0x0   << 6  ) /* DIS                      */
#define ICER1_PWM0_EN                  (0x1   << 6  ) /* EN                       */

/* ICER1[PWMTRIP] -  */
#define ICER1_PWMTRIP_MSK              (0x1   << 5  )
#define ICER1_PWMTRIP                  (0x1   << 5  )
#define ICER1_PWMTRIP_DIS              (0x0   << 5  ) /* DIS                      */
#define ICER1_PWMTRIP_EN               (0x1   << 5  ) /* EN                       */

/* ICER1[DMASPI0RX] -  */
#define ICER1_DMASPI0RX_MSK            (0x1   << 4  )
#define ICER1_DMASPI0RX                (0x1   << 4  )
#define ICER1_DMASPI0RX_DIS            (0x0   << 4  ) /* DIS                      */
#define ICER1_DMASPI0RX_EN             (0x1   << 4  ) /* EN                       */

/* ICER1[DMASPI0TX] -  */
#define ICER1_DMASPI0TX_MSK            (0x1   << 3  )
#define ICER1_DMASPI0TX                (0x1   << 3  )
#define ICER1_DMASPI0TX_DIS            (0x0   << 3  ) /* DIS                      */
#define ICER1_DMASPI0TX_EN             (0x1   << 3  ) /* EN                       */

/* ICER1[DMAADC] -  */
#define ICER1_DMAADC_MSK               (0x1   << 2  )
#define ICER1_DMAADC                   (0x1   << 2  )
#define ICER1_DMAADC_DIS               (0x0   << 2  ) /* DIS                      */
#define ICER1_DMAADC_EN                (0x1   << 2  ) /* EN                       */

/* Reset Value for ISPR0*/
#define ISPR0_RVAL                     0x0

/* ISPR0[DMAI2CMRX] -  */
#define ISPR0_DMAI2CMRX_MSK            (0x1   << 30 )
#define ISPR0_DMAI2CMRX                (0x1   << 30 )
#define ISPR0_DMAI2CMRX_DIS            (0x0   << 30 ) /* DIS                      */
#define ISPR0_DMAI2CMRX_EN             (0x1   << 30 ) /* EN                       */

/* ISPR0[DMAI2CMTX] -  */
#define ISPR0_DMAI2CMTX_MSK            (0x1   << 29 )
#define ISPR0_DMAI2CMTX                (0x1   << 29 )
#define ISPR0_DMAI2CMTX_DIS            (0x0   << 29 ) /* DIS                      */
#define ISPR0_DMAI2CMTX_EN             (0x1   << 29 ) /* EN                       */

/* ISPR0[DMAI2CSRX] -  */
#define ISPR0_DMAI2CSRX_MSK            (0x1   << 28 )
#define ISPR0_DMAI2CSRX                (0x1   << 28 )
#define ISPR0_DMAI2CSRX_DIS            (0x0   << 28 ) /* DIS                      */
#define ISPR0_DMAI2CSRX_EN             (0x1   << 28 ) /* EN                       */

/* ISPR0[DMAI2CSTX] -  */
#define ISPR0_DMAI2CSTX_MSK            (0x1   << 27 )
#define ISPR0_DMAI2CSTX                (0x1   << 27 )
#define ISPR0_DMAI2CSTX_DIS            (0x0   << 27 ) /* DIS                      */
#define ISPR0_DMAI2CSTX_EN             (0x1   << 27 ) /* EN                       */

/* ISPR0[DMAUARTRX] -  */
#define ISPR0_DMAUARTRX_MSK            (0x1   << 26 )
#define ISPR0_DMAUARTRX                (0x1   << 26 )
#define ISPR0_DMAUARTRX_DIS            (0x0   << 26 ) /* DIS                      */
#define ISPR0_DMAUARTRX_EN             (0x1   << 26 ) /* EN                       */

/* ISPR0[DMAUARTTX] -  */
#define ISPR0_DMAUARTTX_MSK            (0x1   << 25 )
#define ISPR0_DMAUARTTX                (0x1   << 25 )
#define ISPR0_DMAUARTTX_DIS            (0x0   << 25 ) /* DIS                      */
#define ISPR0_DMAUARTTX_EN             (0x1   << 25 ) /* EN                       */

/* ISPR0[DMASPI1RX] -  */
#define ISPR0_DMASPI1RX_MSK            (0x1   << 24 )
#define ISPR0_DMASPI1RX                (0x1   << 24 )
#define ISPR0_DMASPI1RX_DIS            (0x0   << 24 ) /* DIS                      */
#define ISPR0_DMASPI1RX_EN             (0x1   << 24 ) /* EN                       */

/* ISPR0[DMASPI1TX] -  */
#define ISPR0_DMASPI1TX_MSK            (0x1   << 23 )
#define ISPR0_DMASPI1TX                (0x1   << 23 )
#define ISPR0_DMASPI1TX_DIS            (0x0   << 23 ) /* DIS                      */
#define ISPR0_DMASPI1TX_EN             (0x1   << 23 ) /* EN                       */

/* ISPR0[DMAERROR] -  */
#define ISPR0_DMAERROR_MSK             (0x1   << 22 )
#define ISPR0_DMAERROR                 (0x1   << 22 )
#define ISPR0_DMAERROR_DIS             (0x0   << 22 ) /* DIS                      */
#define ISPR0_DMAERROR_EN              (0x1   << 22 ) /* EN                       */

/* ISPR0[I2CM] -  */
#define ISPR0_I2CM_MSK                 (0x1   << 20 )
#define ISPR0_I2CM                     (0x1   << 20 )
#define ISPR0_I2CM_DIS                 (0x0   << 20 ) /* DIS                      */
#define ISPR0_I2CM_EN                  (0x1   << 20 ) /* EN                       */

/* ISPR0[I2CS] -  */
#define ISPR0_I2CS_MSK                 (0x1   << 19 )
#define ISPR0_I2CS                     (0x1   << 19 )
#define ISPR0_I2CS_DIS                 (0x0   << 19 ) /* DIS                      */
#define ISPR0_I2CS_EN                  (0x1   << 19 ) /* EN                       */

/* ISPR0[SPI1] -  */
#define ISPR0_SPI1_MSK                 (0x1   << 18 )
#define ISPR0_SPI1                     (0x1   << 18 )
#define ISPR0_SPI1_DIS                 (0x0   << 18 ) /* DIS                      */
#define ISPR0_SPI1_EN                  (0x1   << 18 ) /* EN                       */

/* ISPR0[SPI0] -  */
#define ISPR0_SPI0_MSK                 (0x1   << 17 )
#define ISPR0_SPI0                     (0x1   << 17 )
#define ISPR0_SPI0_DIS                 (0x0   << 17 ) /* DIS                      */
#define ISPR0_SPI0_EN                  (0x1   << 17 ) /* EN                       */

/* ISPR0[UART] -  */
#define ISPR0_UART_MSK                 (0x1   << 16 )
#define ISPR0_UART                     (0x1   << 16 )
#define ISPR0_UART_DIS                 (0x0   << 16 ) /* DIS                      */
#define ISPR0_UART_EN                  (0x1   << 16 ) /* EN                       */

/* ISPR0[FEE] -  */
#define ISPR0_FEE_MSK                  (0x1   << 15 )
#define ISPR0_FEE                      (0x1   << 15 )
#define ISPR0_FEE_DIS                  (0x0   << 15 ) /* DIS                      */
#define ISPR0_FEE_EN                   (0x1   << 15 ) /* EN                       */

/* ISPR0[ADC] -  */
#define ISPR0_ADC_MSK                  (0x1   << 14 )
#define ISPR0_ADC                      (0x1   << 14 )
#define ISPR0_ADC_DIS                  (0x0   << 14 ) /* DIS                      */
#define ISPR0_ADC_EN                   (0x1   << 14 ) /* EN                       */

/* ISPR0[T1] -  */
#define ISPR0_T1_MSK                   (0x1   << 13 )
#define ISPR0_T1                       (0x1   << 13 )
#define ISPR0_T1_DIS                   (0x0   << 13 ) /* DIS                      */
#define ISPR0_T1_EN                    (0x1   << 13 ) /* EN                       */

/* ISPR0[T0] -  */
#define ISPR0_T0_MSK                   (0x1   << 12 )
#define ISPR0_T0                       (0x1   << 12 )
#define ISPR0_T0_DIS                   (0x0   << 12 ) /* DIS                      */
#define ISPR0_T0_EN                    (0x1   << 12 ) /* EN                       */

/* ISPR0[T3] -  */
#define ISPR0_T3_MSK                   (0x1   << 10 )
#define ISPR0_T3                       (0x1   << 10 )
#define ISPR0_T3_DIS                   (0x0   << 10 ) /* DIS                      */
#define ISPR0_T3_EN                    (0x1   << 10 ) /* EN                       */

/* ISPR0[EXTINT8] -  */
#define ISPR0_EXTINT8_MSK              (0x1   << 9  )
#define ISPR0_EXTINT8                  (0x1   << 9  )
#define ISPR0_EXTINT8_DIS              (0x0   << 9  ) /* DIS                      */
#define ISPR0_EXTINT8_EN               (0x1   << 9  ) /* EN                       */

/* ISPR0[EXTINT7] -  */
#define ISPR0_EXTINT7_MSK              (0x1   << 8  )
#define ISPR0_EXTINT7                  (0x1   << 8  )
#define ISPR0_EXTINT7_DIS              (0x0   << 8  ) /* DIS                      */
#define ISPR0_EXTINT7_EN               (0x1   << 8  ) /* EN                       */

/* ISPR0[EXTINT6] -  */
#define ISPR0_EXTINT6_MSK              (0x1   << 7  )
#define ISPR0_EXTINT6                  (0x1   << 7  )
#define ISPR0_EXTINT6_DIS              (0x0   << 7  ) /* DIS                      */
#define ISPR0_EXTINT6_EN               (0x1   << 7  ) /* EN                       */

/* ISPR0[EXTINT5] -  */
#define ISPR0_EXTINT5_MSK              (0x1   << 6  )
#define ISPR0_EXTINT5                  (0x1   << 6  )
#define ISPR0_EXTINT5_DIS              (0x0   << 6  ) /* DIS                      */
#define ISPR0_EXTINT5_EN               (0x1   << 6  ) /* EN                       */

/* ISPR0[EXTINT4] -  */
#define ISPR0_EXTINT4_MSK              (0x1   << 5  )
#define ISPR0_EXTINT4                  (0x1   << 5  )
#define ISPR0_EXTINT4_DIS              (0x0   << 5  ) /* DIS                      */
#define ISPR0_EXTINT4_EN               (0x1   << 5  ) /* EN                       */

/* ISPR0[EXTINT3] -  */
#define ISPR0_EXTINT3_MSK              (0x1   << 4  )
#define ISPR0_EXTINT3                  (0x1   << 4  )
#define ISPR0_EXTINT3_DIS              (0x0   << 4  ) /* DIS                      */
#define ISPR0_EXTINT3_EN               (0x1   << 4  ) /* EN                       */

/* ISPR0[EXTINT2] -  */
#define ISPR0_EXTINT2_MSK              (0x1   << 3  )
#define ISPR0_EXTINT2                  (0x1   << 3  )
#define ISPR0_EXTINT2_DIS              (0x0   << 3  ) /* DIS                      */
#define ISPR0_EXTINT2_EN               (0x1   << 3  ) /* EN                       */

/* ISPR0[EXTINT1] -  */
#define ISPR0_EXTINT1_MSK              (0x1   << 2  )
#define ISPR0_EXTINT1                  (0x1   << 2  )
#define ISPR0_EXTINT1_DIS              (0x0   << 2  ) /* DIS                      */
#define ISPR0_EXTINT1_EN               (0x1   << 2  ) /* EN                       */

/* ISPR0[EXTINT0] -  */
#define ISPR0_EXTINT0_MSK              (0x1   << 1  )
#define ISPR0_EXTINT0                  (0x1   << 1  )
#define ISPR0_EXTINT0_DIS              (0x0   << 1  ) /* DIS                      */
#define ISPR0_EXTINT0_EN               (0x1   << 1  ) /* EN                       */

/* ISPR0[T2] -  */
#define ISPR0_T2_MSK                   (0x1   << 0  )
#define ISPR0_T2                       (0x1   << 0  )
#define ISPR0_T2_DIS                   (0x0   << 0  ) /* DIS                      */
#define ISPR0_T2_EN                    (0x1   << 0  ) /* EN                       */

/* Reset Value for ISPR1*/
#define ISPR1_RVAL                     0x0

/* ISPR1[PWM3] -  */
#define ISPR1_PWM3_MSK                 (0x1   << 9  )
#define ISPR1_PWM3                     (0x1   << 9  )
#define ISPR1_PWM3_DIS                 (0x0   << 9  ) /* DIS                      */
#define ISPR1_PWM3_EN                  (0x1   << 9  ) /* EN                       */

/* ISPR1[PWM2] -  */
#define ISPR1_PWM2_MSK                 (0x1   << 8  )
#define ISPR1_PWM2                     (0x1   << 8  )
#define ISPR1_PWM2_DIS                 (0x0   << 8  ) /* DIS                      */
#define ISPR1_PWM2_EN                  (0x1   << 8  ) /* EN                       */

/* ISPR1[PWM1] -  */
#define ISPR1_PWM1_MSK                 (0x1   << 7  )
#define ISPR1_PWM1                     (0x1   << 7  )
#define ISPR1_PWM1_DIS                 (0x0   << 7  ) /* DIS                      */
#define ISPR1_PWM1_EN                  (0x1   << 7  ) /* EN                       */

/* ISPR1[PWM0] -  */
#define ISPR1_PWM0_MSK                 (0x1   << 6  )
#define ISPR1_PWM0                     (0x1   << 6  )
#define ISPR1_PWM0_DIS                 (0x0   << 6  ) /* DIS                      */
#define ISPR1_PWM0_EN                  (0x1   << 6  ) /* EN                       */

/* ISPR1[PWMTRIP] -  */
#define ISPR1_PWMTRIP_MSK              (0x1   << 5  )
#define ISPR1_PWMTRIP                  (0x1   << 5  )
#define ISPR1_PWMTRIP_DIS              (0x0   << 5  ) /* DIS                      */
#define ISPR1_PWMTRIP_EN               (0x1   << 5  ) /* EN                       */

/* ISPR1[DMASPI0RX] -  */
#define ISPR1_DMASPI0RX_MSK            (0x1   << 4  )
#define ISPR1_DMASPI0RX                (0x1   << 4  )
#define ISPR1_DMASPI0RX_DIS            (0x0   << 4  ) /* DIS                      */
#define ISPR1_DMASPI0RX_EN             (0x1   << 4  ) /* EN                       */

/* ISPR1[DMASPI0TX] -  */
#define ISPR1_DMASPI0TX_MSK            (0x1   << 3  )
#define ISPR1_DMASPI0TX                (0x1   << 3  )
#define ISPR1_DMASPI0TX_DIS            (0x0   << 3  ) /* DIS                      */
#define ISPR1_DMASPI0TX_EN             (0x1   << 3  ) /* EN                       */

/* ISPR1[DMAADC] -  */
#define ISPR1_DMAADC_MSK               (0x1   << 2  )
#define ISPR1_DMAADC                   (0x1   << 2  )
#define ISPR1_DMAADC_DIS               (0x0   << 2  ) /* DIS                      */
#define ISPR1_DMAADC_EN                (0x1   << 2  ) /* EN                       */

/* Reset Value for ICPR0*/
#define ICPR0_RVAL                     0x0

/* ICPR0[DMAI2CMRX] -  */
#define ICPR0_DMAI2CMRX_MSK            (0x1   << 30 )
#define ICPR0_DMAI2CMRX                (0x1   << 30 )
#define ICPR0_DMAI2CMRX_DIS            (0x0   << 30 ) /* DIS                      */
#define ICPR0_DMAI2CMRX_EN             (0x1   << 30 ) /* EN                       */

/* ICPR0[DMAI2CMTX] -  */
#define ICPR0_DMAI2CMTX_MSK            (0x1   << 29 )
#define ICPR0_DMAI2CMTX                (0x1   << 29 )
#define ICPR0_DMAI2CMTX_DIS            (0x0   << 29 ) /* DIS                      */
#define ICPR0_DMAI2CMTX_EN             (0x1   << 29 ) /* EN                       */

/* ICPR0[DMAI2CSRX] -  */
#define ICPR0_DMAI2CSRX_MSK            (0x1   << 28 )
#define ICPR0_DMAI2CSRX                (0x1   << 28 )
#define ICPR0_DMAI2CSRX_DIS            (0x0   << 28 ) /* DIS                      */
#define ICPR0_DMAI2CSRX_EN             (0x1   << 28 ) /* EN                       */

/* ICPR0[DMAI2CSTX] -  */
#define ICPR0_DMAI2CSTX_MSK            (0x1   << 27 )
#define ICPR0_DMAI2CSTX                (0x1   << 27 )
#define ICPR0_DMAI2CSTX_DIS            (0x0   << 27 ) /* DIS                      */
#define ICPR0_DMAI2CSTX_EN             (0x1   << 27 ) /* EN                       */

/* ICPR0[DMAUARTRX] -  */
#define ICPR0_DMAUARTRX_MSK            (0x1   << 26 )
#define ICPR0_DMAUARTRX                (0x1   << 26 )
#define ICPR0_DMAUARTRX_DIS            (0x0   << 26 ) /* DIS                      */
#define ICPR0_DMAUARTRX_EN             (0x1   << 26 ) /* EN                       */

/* ICPR0[DMAUARTTX] -  */
#define ICPR0_DMAUARTTX_MSK            (0x1   << 25 )
#define ICPR0_DMAUARTTX                (0x1   << 25 )
#define ICPR0_DMAUARTTX_DIS            (0x0   << 25 ) /* DIS                      */
#define ICPR0_DMAUARTTX_EN             (0x1   << 25 ) /* EN                       */

/* ICPR0[DMASPI1RX] -  */
#define ICPR0_DMASPI1RX_MSK            (0x1   << 24 )
#define ICPR0_DMASPI1RX                (0x1   << 24 )
#define ICPR0_DMASPI1RX_DIS            (0x0   << 24 ) /* DIS                      */
#define ICPR0_DMASPI1RX_EN             (0x1   << 24 ) /* EN                       */

/* ICPR0[DMASPI1TX] -  */
#define ICPR0_DMASPI1TX_MSK            (0x1   << 23 )
#define ICPR0_DMASPI1TX                (0x1   << 23 )
#define ICPR0_DMASPI1TX_DIS            (0x0   << 23 ) /* DIS                      */
#define ICPR0_DMASPI1TX_EN             (0x1   << 23 ) /* EN                       */

/* ICPR0[DMAERROR] -  */
#define ICPR0_DMAERROR_MSK             (0x1   << 22 )
#define ICPR0_DMAERROR                 (0x1   << 22 )
#define ICPR0_DMAERROR_DIS             (0x0   << 22 ) /* DIS                      */
#define ICPR0_DMAERROR_EN              (0x1   << 22 ) /* EN                       */

/* ICPR0[I2CM] -  */
#define ICPR0_I2CM_MSK                 (0x1   << 20 )
#define ICPR0_I2CM                     (0x1   << 20 )
#define ICPR0_I2CM_DIS                 (0x0   << 20 ) /* DIS                      */
#define ICPR0_I2CM_EN                  (0x1   << 20 ) /* EN                       */

/* ICPR0[I2CS] -  */
#define ICPR0_I2CS_MSK                 (0x1   << 19 )
#define ICPR0_I2CS                     (0x1   << 19 )
#define ICPR0_I2CS_DIS                 (0x0   << 19 ) /* DIS                      */
#define ICPR0_I2CS_EN                  (0x1   << 19 ) /* EN                       */

/* ICPR0[SPI1] -  */
#define ICPR0_SPI1_MSK                 (0x1   << 18 )
#define ICPR0_SPI1                     (0x1   << 18 )
#define ICPR0_SPI1_DIS                 (0x0   << 18 ) /* DIS                      */
#define ICPR0_SPI1_EN                  (0x1   << 18 ) /* EN                       */

/* ICPR0[SPI0] -  */
#define ICPR0_SPI0_MSK                 (0x1   << 17 )
#define ICPR0_SPI0                     (0x1   << 17 )
#define ICPR0_SPI0_DIS                 (0x0   << 17 ) /* DIS                      */
#define ICPR0_SPI0_EN                  (0x1   << 17 ) /* EN                       */

/* ICPR0[UART] -  */
#define ICPR0_UART_MSK                 (0x1   << 16 )
#define ICPR0_UART                     (0x1   << 16 )
#define ICPR0_UART_DIS                 (0x0   << 16 ) /* DIS                      */
#define ICPR0_UART_EN                  (0x1   << 16 ) /* EN                       */

/* ICPR0[FEE] -  */
#define ICPR0_FEE_MSK                  (0x1   << 15 )
#define ICPR0_FEE                      (0x1   << 15 )
#define ICPR0_FEE_DIS                  (0x0   << 15 ) /* DIS                      */
#define ICPR0_FEE_EN                   (0x1   << 15 ) /* EN                       */

/* ICPR0[ADC] -  */
#define ICPR0_ADC_MSK                  (0x1   << 14 )
#define ICPR0_ADC                      (0x1   << 14 )
#define ICPR0_ADC_DIS                  (0x0   << 14 ) /* DIS                      */
#define ICPR0_ADC_EN                   (0x1   << 14 ) /* EN                       */

/* ICPR0[T1] -  */
#define ICPR0_T1_MSK                   (0x1   << 13 )
#define ICPR0_T1                       (0x1   << 13 )
#define ICPR0_T1_DIS                   (0x0   << 13 ) /* DIS                      */
#define ICPR0_T1_EN                    (0x1   << 13 ) /* EN                       */

/* ICPR0[T0] -  */
#define ICPR0_T0_MSK                   (0x1   << 12 )
#define ICPR0_T0                       (0x1   << 12 )
#define ICPR0_T0_DIS                   (0x0   << 12 ) /* DIS                      */
#define ICPR0_T0_EN                    (0x1   << 12 ) /* EN                       */

/* ICPR0[T3] -  */
#define ICPR0_T3_MSK                   (0x1   << 10 )
#define ICPR0_T3                       (0x1   << 10 )
#define ICPR0_T3_DIS                   (0x0   << 10 ) /* DIS                      */
#define ICPR0_T3_EN                    (0x1   << 10 ) /* EN                       */

/* ICPR0[EXTINT8] -  */
#define ICPR0_EXTINT8_MSK              (0x1   << 9  )
#define ICPR0_EXTINT8                  (0x1   << 9  )
#define ICPR0_EXTINT8_DIS              (0x0   << 9  ) /* DIS                      */
#define ICPR0_EXTINT8_EN               (0x1   << 9  ) /* EN                       */

/* ICPR0[EXTINT7] -  */
#define ICPR0_EXTINT7_MSK              (0x1   << 8  )
#define ICPR0_EXTINT7                  (0x1   << 8  )
#define ICPR0_EXTINT7_DIS              (0x0   << 8  ) /* DIS                      */
#define ICPR0_EXTINT7_EN               (0x1   << 8  ) /* EN                       */

/* ICPR0[EXTINT6] -  */
#define ICPR0_EXTINT6_MSK              (0x1   << 7  )
#define ICPR0_EXTINT6                  (0x1   << 7  )
#define ICPR0_EXTINT6_DIS              (0x0   << 7  ) /* DIS                      */
#define ICPR0_EXTINT6_EN               (0x1   << 7  ) /* EN                       */

/* ICPR0[EXTINT5] -  */
#define ICPR0_EXTINT5_MSK              (0x1   << 6  )
#define ICPR0_EXTINT5                  (0x1   << 6  )
#define ICPR0_EXTINT5_DIS              (0x0   << 6  ) /* DIS                      */
#define ICPR0_EXTINT5_EN               (0x1   << 6  ) /* EN                       */

/* ICPR0[EXTINT4] -  */
#define ICPR0_EXTINT4_MSK              (0x1   << 5  )
#define ICPR0_EXTINT4                  (0x1   << 5  )
#define ICPR0_EXTINT4_DIS              (0x0   << 5  ) /* DIS                      */
#define ICPR0_EXTINT4_EN               (0x1   << 5  ) /* EN                       */

/* ICPR0[EXTINT3] -  */
#define ICPR0_EXTINT3_MSK              (0x1   << 4  )
#define ICPR0_EXTINT3                  (0x1   << 4  )
#define ICPR0_EXTINT3_DIS              (0x0   << 4  ) /* DIS                      */
#define ICPR0_EXTINT3_EN               (0x1   << 4  ) /* EN                       */

/* ICPR0[EXTINT2] -  */
#define ICPR0_EXTINT2_MSK              (0x1   << 3  )
#define ICPR0_EXTINT2                  (0x1   << 3  )
#define ICPR0_EXTINT2_DIS              (0x0   << 3  ) /* DIS                      */
#define ICPR0_EXTINT2_EN               (0x1   << 3  ) /* EN                       */

/* ICPR0[EXTINT1] -  */
#define ICPR0_EXTINT1_MSK              (0x1   << 2  )
#define ICPR0_EXTINT1                  (0x1   << 2  )
#define ICPR0_EXTINT1_DIS              (0x0   << 2  ) /* DIS                      */
#define ICPR0_EXTINT1_EN               (0x1   << 2  ) /* EN                       */

/* ICPR0[EXTINT0] -  */
#define ICPR0_EXTINT0_MSK              (0x1   << 1  )
#define ICPR0_EXTINT0                  (0x1   << 1  )
#define ICPR0_EXTINT0_DIS              (0x0   << 1  ) /* DIS                      */
#define ICPR0_EXTINT0_EN               (0x1   << 1  ) /* EN                       */

/* ICPR0[T2] -  */
#define ICPR0_T2_MSK                   (0x1   << 0  )
#define ICPR0_T2                       (0x1   << 0  )
#define ICPR0_T2_DIS                   (0x0   << 0  ) /* DIS                      */
#define ICPR0_T2_EN                    (0x1   << 0  ) /* EN                       */

/* Reset Value for ICPR1*/
#define ICPR1_RVAL                     0x0

/* ICPR1[PWM3] -  */
#define ICPR1_PWM3_MSK                 (0x1   << 9  )
#define ICPR1_PWM3                     (0x1   << 9  )
#define ICPR1_PWM3_DIS                 (0x0   << 9  ) /* DIS                      */
#define ICPR1_PWM3_EN                  (0x1   << 9  ) /* EN                       */

/* ICPR1[PWM2] -  */
#define ICPR1_PWM2_MSK                 (0x1   << 8  )
#define ICPR1_PWM2                     (0x1   << 8  )
#define ICPR1_PWM2_DIS                 (0x0   << 8  ) /* DIS                      */
#define ICPR1_PWM2_EN                  (0x1   << 8  ) /* EN                       */

/* ICPR1[PWM1] -  */
#define ICPR1_PWM1_MSK                 (0x1   << 7  )
#define ICPR1_PWM1                     (0x1   << 7  )
#define ICPR1_PWM1_DIS                 (0x0   << 7  ) /* DIS                      */
#define ICPR1_PWM1_EN                  (0x1   << 7  ) /* EN                       */

/* ICPR1[PWM0] -  */
#define ICPR1_PWM0_MSK                 (0x1   << 6  )
#define ICPR1_PWM0                     (0x1   << 6  )
#define ICPR1_PWM0_DIS                 (0x0   << 6  ) /* DIS                      */
#define ICPR1_PWM0_EN                  (0x1   << 6  ) /* EN                       */

/* ICPR1[PWMTRIP] -  */
#define ICPR1_PWMTRIP_MSK              (0x1   << 5  )
#define ICPR1_PWMTRIP                  (0x1   << 5  )
#define ICPR1_PWMTRIP_DIS              (0x0   << 5  ) /* DIS                      */
#define ICPR1_PWMTRIP_EN               (0x1   << 5  ) /* EN                       */

/* ICPR1[DMASPI0RX] -  */
#define ICPR1_DMASPI0RX_MSK            (0x1   << 4  )
#define ICPR1_DMASPI0RX                (0x1   << 4  )
#define ICPR1_DMASPI0RX_DIS            (0x0   << 4  ) /* DIS                      */
#define ICPR1_DMASPI0RX_EN             (0x1   << 4  ) /* EN                       */

/* ICPR1[DMASPI0TX] -  */
#define ICPR1_DMASPI0TX_MSK            (0x1   << 3  )
#define ICPR1_DMASPI0TX                (0x1   << 3  )
#define ICPR1_DMASPI0TX_DIS            (0x0   << 3  ) /* DIS                      */
#define ICPR1_DMASPI0TX_EN             (0x1   << 3  ) /* EN                       */

/* ICPR1[DMAADC] -  */
#define ICPR1_DMAADC_MSK               (0x1   << 2  )
#define ICPR1_DMAADC                   (0x1   << 2  )
#define ICPR1_DMAADC_DIS               (0x0   << 2  ) /* DIS                      */
#define ICPR1_DMAADC_EN                (0x1   << 2  ) /* EN                       */

/* Reset Value for IABR0*/
#define IABR0_RVAL                     0x0

/* IABR0[DMAI2CMRX] -  */
#define IABR0_DMAI2CMRX_MSK            (0x1   << 30 )
#define IABR0_DMAI2CMRX                (0x1   << 30 )
#define IABR0_DMAI2CMRX_DIS            (0x0   << 30 ) /* DIS                      */
#define IABR0_DMAI2CMRX_EN             (0x1   << 30 ) /* EN                       */

/* IABR0[DMAI2CMTX] -  */
#define IABR0_DMAI2CMTX_MSK            (0x1   << 29 )
#define IABR0_DMAI2CMTX                (0x1   << 29 )
#define IABR0_DMAI2CMTX_DIS            (0x0   << 29 ) /* DIS                      */
#define IABR0_DMAI2CMTX_EN             (0x1   << 29 ) /* EN                       */

/* IABR0[DMAI2CSRX] -  */
#define IABR0_DMAI2CSRX_MSK            (0x1   << 28 )
#define IABR0_DMAI2CSRX                (0x1   << 28 )
#define IABR0_DMAI2CSRX_DIS            (0x0   << 28 ) /* DIS                      */
#define IABR0_DMAI2CSRX_EN             (0x1   << 28 ) /* EN                       */

/* IABR0[DMAI2CSTX] -  */
#define IABR0_DMAI2CSTX_MSK            (0x1   << 27 )
#define IABR0_DMAI2CSTX                (0x1   << 27 )
#define IABR0_DMAI2CSTX_DIS            (0x0   << 27 ) /* DIS                      */
#define IABR0_DMAI2CSTX_EN             (0x1   << 27 ) /* EN                       */

/* IABR0[DMAUARTRX] -  */
#define IABR0_DMAUARTRX_MSK            (0x1   << 26 )
#define IABR0_DMAUARTRX                (0x1   << 26 )
#define IABR0_DMAUARTRX_DIS            (0x0   << 26 ) /* DIS                      */
#define IABR0_DMAUARTRX_EN             (0x1   << 26 ) /* EN                       */

/* IABR0[DMAUARTTX] -  */
#define IABR0_DMAUARTTX_MSK            (0x1   << 25 )
#define IABR0_DMAUARTTX                (0x1   << 25 )
#define IABR0_DMAUARTTX_DIS            (0x0   << 25 ) /* DIS                      */
#define IABR0_DMAUARTTX_EN             (0x1   << 25 ) /* EN                       */

/* IABR0[DMASPI1RX] -  */
#define IABR0_DMASPI1RX_MSK            (0x1   << 24 )
#define IABR0_DMASPI1RX                (0x1   << 24 )
#define IABR0_DMASPI1RX_DIS            (0x0   << 24 ) /* DIS                      */
#define IABR0_DMASPI1RX_EN             (0x1   << 24 ) /* EN                       */

/* IABR0[DMASPI1TX] -  */
#define IABR0_DMASPI1TX_MSK            (0x1   << 23 )
#define IABR0_DMASPI1TX                (0x1   << 23 )
#define IABR0_DMASPI1TX_DIS            (0x0   << 23 ) /* DIS                      */
#define IABR0_DMASPI1TX_EN             (0x1   << 23 ) /* EN                       */

/* IABR0[DMAERROR] -  */
#define IABR0_DMAERROR_MSK             (0x1   << 22 )
#define IABR0_DMAERROR                 (0x1   << 22 )
#define IABR0_DMAERROR_DIS             (0x0   << 22 ) /* DIS                      */
#define IABR0_DMAERROR_EN              (0x1   << 22 ) /* EN                       */

/* IABR0[I2CM] -  */
#define IABR0_I2CM_MSK                 (0x1   << 20 )
#define IABR0_I2CM                     (0x1   << 20 )
#define IABR0_I2CM_DIS                 (0x0   << 20 ) /* DIS                      */
#define IABR0_I2CM_EN                  (0x1   << 20 ) /* EN                       */

/* IABR0[I2CS] -  */
#define IABR0_I2CS_MSK                 (0x1   << 19 )
#define IABR0_I2CS                     (0x1   << 19 )
#define IABR0_I2CS_DIS                 (0x0   << 19 ) /* DIS                      */
#define IABR0_I2CS_EN                  (0x1   << 19 ) /* EN                       */

/* IABR0[SPI1] -  */
#define IABR0_SPI1_MSK                 (0x1   << 18 )
#define IABR0_SPI1                     (0x1   << 18 )
#define IABR0_SPI1_DIS                 (0x0   << 18 ) /* DIS                      */
#define IABR0_SPI1_EN                  (0x1   << 18 ) /* EN                       */

/* IABR0[SPI0] -  */
#define IABR0_SPI0_MSK                 (0x1   << 17 )
#define IABR0_SPI0                     (0x1   << 17 )
#define IABR0_SPI0_DIS                 (0x0   << 17 ) /* DIS                      */
#define IABR0_SPI0_EN                  (0x1   << 17 ) /* EN                       */

/* IABR0[UART] -  */
#define IABR0_UART_MSK                 (0x1   << 16 )
#define IABR0_UART                     (0x1   << 16 )
#define IABR0_UART_DIS                 (0x0   << 16 ) /* DIS                      */
#define IABR0_UART_EN                  (0x1   << 16 ) /* EN                       */

/* IABR0[FEE] -  */
#define IABR0_FEE_MSK                  (0x1   << 15 )
#define IABR0_FEE                      (0x1   << 15 )
#define IABR0_FEE_DIS                  (0x0   << 15 ) /* DIS                      */
#define IABR0_FEE_EN                   (0x1   << 15 ) /* EN                       */

/* IABR0[ADC] -  */
#define IABR0_ADC_MSK                  (0x1   << 14 )
#define IABR0_ADC                      (0x1   << 14 )
#define IABR0_ADC_DIS                  (0x0   << 14 ) /* DIS                      */
#define IABR0_ADC_EN                   (0x1   << 14 ) /* EN                       */

/* IABR0[T1] -  */
#define IABR0_T1_MSK                   (0x1   << 13 )
#define IABR0_T1                       (0x1   << 13 )
#define IABR0_T1_DIS                   (0x0   << 13 ) /* DIS                      */
#define IABR0_T1_EN                    (0x1   << 13 ) /* EN                       */

/* IABR0[T0] -  */
#define IABR0_T0_MSK                   (0x1   << 12 )
#define IABR0_T0                       (0x1   << 12 )
#define IABR0_T0_DIS                   (0x0   << 12 ) /* DIS                      */
#define IABR0_T0_EN                    (0x1   << 12 ) /* EN                       */

/* IABR0[T3] -  */
#define IABR0_T3_MSK                   (0x1   << 10 )
#define IABR0_T3                       (0x1   << 10 )
#define IABR0_T3_DIS                   (0x0   << 10 ) /* DIS                      */
#define IABR0_T3_EN                    (0x1   << 10 ) /* EN                       */

/* IABR0[EXTINT8] -  */
#define IABR0_EXTINT8_MSK              (0x1   << 9  )
#define IABR0_EXTINT8                  (0x1   << 9  )
#define IABR0_EXTINT8_DIS              (0x0   << 9  ) /* DIS                      */
#define IABR0_EXTINT8_EN               (0x1   << 9  ) /* EN                       */

/* IABR0[EXTINT7] -  */
#define IABR0_EXTINT7_MSK              (0x1   << 8  )
#define IABR0_EXTINT7                  (0x1   << 8  )
#define IABR0_EXTINT7_DIS              (0x0   << 8  ) /* DIS                      */
#define IABR0_EXTINT7_EN               (0x1   << 8  ) /* EN                       */

/* IABR0[EXTINT6] -  */
#define IABR0_EXTINT6_MSK              (0x1   << 7  )
#define IABR0_EXTINT6                  (0x1   << 7  )
#define IABR0_EXTINT6_DIS              (0x0   << 7  ) /* DIS                      */
#define IABR0_EXTINT6_EN               (0x1   << 7  ) /* EN                       */

/* IABR0[EXTINT5] -  */
#define IABR0_EXTINT5_MSK              (0x1   << 6  )
#define IABR0_EXTINT5                  (0x1   << 6  )
#define IABR0_EXTINT5_DIS              (0x0   << 6  ) /* DIS                      */
#define IABR0_EXTINT5_EN               (0x1   << 6  ) /* EN                       */

/* IABR0[EXTINT4] -  */
#define IABR0_EXTINT4_MSK              (0x1   << 5  )
#define IABR0_EXTINT4                  (0x1   << 5  )
#define IABR0_EXTINT4_DIS              (0x0   << 5  ) /* DIS                      */
#define IABR0_EXTINT4_EN               (0x1   << 5  ) /* EN                       */

/* IABR0[EXTINT3] -  */
#define IABR0_EXTINT3_MSK              (0x1   << 4  )
#define IABR0_EXTINT3                  (0x1   << 4  )
#define IABR0_EXTINT3_DIS              (0x0   << 4  ) /* DIS                      */
#define IABR0_EXTINT3_EN               (0x1   << 4  ) /* EN                       */

/* IABR0[EXTINT2] -  */
#define IABR0_EXTINT2_MSK              (0x1   << 3  )
#define IABR0_EXTINT2                  (0x1   << 3  )
#define IABR0_EXTINT2_DIS              (0x0   << 3  ) /* DIS                      */
#define IABR0_EXTINT2_EN               (0x1   << 3  ) /* EN                       */

/* IABR0[EXTINT1] -  */
#define IABR0_EXTINT1_MSK              (0x1   << 2  )
#define IABR0_EXTINT1                  (0x1   << 2  )
#define IABR0_EXTINT1_DIS              (0x0   << 2  ) /* DIS                      */
#define IABR0_EXTINT1_EN               (0x1   << 2  ) /* EN                       */

/* IABR0[EXTINT0] -  */
#define IABR0_EXTINT0_MSK              (0x1   << 1  )
#define IABR0_EXTINT0                  (0x1   << 1  )
#define IABR0_EXTINT0_DIS              (0x0   << 1  ) /* DIS                      */
#define IABR0_EXTINT0_EN               (0x1   << 1  ) /* EN                       */

/* IABR0[T2] -  */
#define IABR0_T2_MSK                   (0x1   << 0  )
#define IABR0_T2                       (0x1   << 0  )
#define IABR0_T2_DIS                   (0x0   << 0  ) /* DIS                      */
#define IABR0_T2_EN                    (0x1   << 0  ) /* EN                       */

/* Reset Value for IABR1*/
#define IABR1_RVAL                     0x0

/* IABR1[PWM3] -  */
#define IABR1_PWM3_MSK                 (0x1   << 9  )
#define IABR1_PWM3                     (0x1   << 9  )
#define IABR1_PWM3_DIS                 (0x0   << 9  ) /* DIS                      */
#define IABR1_PWM3_EN                  (0x1   << 9  ) /* EN                       */

/* IABR1[PWM2] -  */
#define IABR1_PWM2_MSK                 (0x1   << 8  )
#define IABR1_PWM2                     (0x1   << 8  )
#define IABR1_PWM2_DIS                 (0x0   << 8  ) /* DIS                      */
#define IABR1_PWM2_EN                  (0x1   << 8  ) /* EN                       */

/* IABR1[PWM1] -  */
#define IABR1_PWM1_MSK                 (0x1   << 7  )
#define IABR1_PWM1                     (0x1   << 7  )
#define IABR1_PWM1_DIS                 (0x0   << 7  ) /* DIS                      */
#define IABR1_PWM1_EN                  (0x1   << 7  ) /* EN                       */

/* IABR1[PWM0] -  */
#define IABR1_PWM0_MSK                 (0x1   << 6  )
#define IABR1_PWM0                     (0x1   << 6  )
#define IABR1_PWM0_DIS                 (0x0   << 6  ) /* DIS                      */
#define IABR1_PWM0_EN                  (0x1   << 6  ) /* EN                       */

/* IABR1[PWMTRIP] -  */
#define IABR1_PWMTRIP_MSK              (0x1   << 5  )
#define IABR1_PWMTRIP                  (0x1   << 5  )
#define IABR1_PWMTRIP_DIS              (0x0   << 5  ) /* DIS                      */
#define IABR1_PWMTRIP_EN               (0x1   << 5  ) /* EN                       */

/* IABR1[DMASPI0RX] -  */
#define IABR1_DMASPI0RX_MSK            (0x1   << 4  )
#define IABR1_DMASPI0RX                (0x1   << 4  )
#define IABR1_DMASPI0RX_DIS            (0x0   << 4  ) /* DIS                      */
#define IABR1_DMASPI0RX_EN             (0x1   << 4  ) /* EN                       */

/* IABR1[DMASPI0TX] -  */
#define IABR1_DMASPI0TX_MSK            (0x1   << 3  )
#define IABR1_DMASPI0TX                (0x1   << 3  )
#define IABR1_DMASPI0TX_DIS            (0x0   << 3  ) /* DIS                      */
#define IABR1_DMASPI0TX_EN             (0x1   << 3  ) /* EN                       */

/* IABR1[DMAADC] -  */
#define IABR1_DMAADC_MSK               (0x1   << 2  )
#define IABR1_DMAADC                   (0x1   << 2  )
#define IABR1_DMAADC_DIS               (0x0   << 2  ) /* DIS                      */
#define IABR1_DMAADC_EN                (0x1   << 2  ) /* EN                       */

/* Reset Value for IPR0*/
#define IPR0_RVAL                      0x0

/* IPR0[EXTINT2] -  */
#define IPR0_EXTINT2_MSK               (0xFF  << 24 )

/* IPR0[EXTINT1] -  */
#define IPR0_EXTINT1_MSK               (0xFF  << 16 )

/* IPR0[EXTINT0] - Priority of interrupt number 1 */
#define IPR0_EXTINT0_MSK               (0xFF  << 8  )

/* IPR0[T2] - Priority of interrupt number 0 */
#define IPR0_T2_MSK                    (0xFF  << 0  )

/* Reset Value for IPR1*/
#define IPR1_RVAL                      0x0

/* IPR1[EXTINT6] -  */
#define IPR1_EXTINT6_MSK               (0xFF  << 24 )

/* IPR1[EXTINT5] -  */
#define IPR1_EXTINT5_MSK               (0xFF  << 16 )

/* IPR1[EXTINT4] -  */
#define IPR1_EXTINT4_MSK               (0xFF  << 8  )

/* IPR1[EXTINT3] -  */
#define IPR1_EXTINT3_MSK               (0xFF  << 0  )

/* Reset Value for IPR2*/
#define IPR2_RVAL                      0x0

/* IPR2[T3] -  */
#define IPR2_T3_MSK                    (0xFF  << 16 )

/* IPR2[EXTINT8] -  */
#define IPR2_EXTINT8_MSK               (0xFF  << 8  )

/* IPR2[EXTINT7] -  */
#define IPR2_EXTINT7_MSK               (0xFF  << 0  )

/* Reset Value for IPR3*/
#define IPR3_RVAL                      0x0

/* IPR3[FEE] -  */
#define IPR3_FEE_MSK                   (0xFF  << 24 )

/* IPR3[ADC] -  */
#define IPR3_ADC_MSK                   (0xFF  << 16 )

/* IPR3[T1] -  */
#define IPR3_T1_MSK                    (0xFF  << 8  )

/* IPR3[T0] -  */
#define IPR3_T0_MSK                    (0xFF  << 0  )

/* Reset Value for IPR4*/
#define IPR4_RVAL                      0x0

/* IPR4[I2CS] -  */
#define IPR4_I2CS_MSK                  (0xFF  << 24 )

/* IPR4[SPI1] -  */
#define IPR4_SPI1_MSK                  (0xFF  << 16 )

/* IPR4[SPI0] -  */
#define IPR4_SPI0_MSK                  (0xFF  << 8  )

/* IPR4[UART] -  */
#define IPR4_UART_MSK                  (0xFF  << 0  )

/* Reset Value for IPR5*/
#define IPR5_RVAL                      0x0

/* IPR5[DMASPI1TX] -  */
#define IPR5_DMASPI1TX_MSK             (0xFF  << 24 )

/* IPR5[DMAERROR] -  */
#define IPR5_DMAERROR_MSK              (0xFF  << 16 )

/* IPR5[I2CM] - I2CM */
#define IPR5_I2CM_MSK                  (0xFF  << 0  )

/* Reset Value for IPR6*/
#define IPR6_RVAL                      0x0

/* IPR6[DMAI2CSTX] -  */
#define IPR6_DMAI2CSTX_MSK             (0xFF  << 24 )

/* IPR6[DMAUARTRX] -  */
#define IPR6_DMAUARTRX_MSK             (0xFF  << 16 )

/* IPR6[DMAUARTTX] -  */
#define IPR6_DMAUARTTX_MSK             (0xFF  << 8  )

/* IPR6[DMASPI1RX] -  */
#define IPR6_DMASPI1RX_MSK             (0xFF  << 0  )

/* Reset Value for IPR7*/
#define IPR7_RVAL                      0x0

/* IPR7[DMAI2CMRX] -  */
#define IPR7_DMAI2CMRX_MSK             (0xFF  << 16 )

/* IPR7[DMAI2CMTX] -  */
#define IPR7_DMAI2CMTX_MSK             (0xFF  << 8  )

/* IPR7[DMAI2CSRX] -  */
#define IPR7_DMAI2CSRX_MSK             (0xFF  << 0  )

/* Reset Value for IPR8*/
#define IPR8_RVAL                      0x0

/* IPR8[DMASPI0TX] -  */
#define IPR8_DMASPI0TX_MSK             (0xFF  << 24 )

/* IPR8[DMAADC] -  */
#define IPR8_DMAADC_MSK                (0xFF  << 16 )

/* Reset Value for IPR9*/
#define IPR9_RVAL                      0x0

/* IPR9[PWM1] -  */
#define IPR9_PWM1_MSK                  (0xFF  << 24 )

/* IPR9[PWM0] -  */
#define IPR9_PWM0_MSK                  (0xFF  << 16 )

/* IPR9[PWMTRIP] -  */
#define IPR9_PWMTRIP_MSK               (0xFF  << 8  )

/* IPR9[DMASPI0RX] -  */
#define IPR9_DMASPI0RX_MSK             (0xFF  << 0  )

/* Reset Value for IPR10*/
#define IPR10_RVAL                     0x0

/* IPR10[PWM3] -  */
#define IPR10_PWM3_MSK                 (0xFF  << 8  )

/* IPR10[PWM2] -  */
#define IPR10_PWM2_MSK                 (0xFF  << 0  )

/* Reset Value for CPUID*/
#define CPUID_RVAL                     0x412FC230

/* CPUID[IMPLEMENTER] - Indicates implementor */
#define CPUID_IMPLEMENTER_MSK          (0xFF  << 24 )

/* CPUID[VARIANT] - Indicates processor revision */
#define CPUID_VARIANT_MSK              (0xF   << 20 )

/* CPUID[PARTNO] - Indicates part number */
#define CPUID_PARTNO_MSK               (0xFFF << 4  )

/* CPUID[REVISION] - Indicates patch release */
#define CPUID_REVISION_MSK             (0xF   << 0  )

/* Reset Value for ICSR*/
#define ICSR_RVAL                      0x0

/* ICSR[NMIPENDSET] - Setting this bit will activate an NMI */
#define ICSR_NMIPENDSET_MSK            (0x1   << 31 )
#define ICSR_NMIPENDSET                (0x1   << 31 )
#define ICSR_NMIPENDSET_DIS            (0x0   << 31 ) /* DIS                      */
#define ICSR_NMIPENDSET_EN             (0x1   << 31 ) /* EN                       */

/* ICSR[PENDSVSET] - Set a pending PendSV interrupt */
#define ICSR_PENDSVSET_MSK             (0x1   << 28 )
#define ICSR_PENDSVSET                 (0x1   << 28 )
#define ICSR_PENDSVSET_DIS             (0x0   << 28 ) /* DIS                      */
#define ICSR_PENDSVSET_EN              (0x1   << 28 ) /* EN                       */

/* ICSR[PENDSVCLR] - Clear a pending PendSV interrupt */
#define ICSR_PENDSVCLR_MSK             (0x1   << 27 )
#define ICSR_PENDSVCLR                 (0x1   << 27 )
#define ICSR_PENDSVCLR_DIS             (0x0   << 27 ) /* DIS                      */
#define ICSR_PENDSVCLR_EN              (0x1   << 27 ) /* EN                       */

/* ICSR[PENDSTSET] - Set a pending SysTick. Reads back with current state */
#define ICSR_PENDSTSET_MSK             (0x1   << 26 )
#define ICSR_PENDSTSET                 (0x1   << 26 )
#define ICSR_PENDSTSET_DIS             (0x0   << 26 ) /* DIS                      */
#define ICSR_PENDSTSET_EN              (0x1   << 26 ) /* EN                       */

/* ICSR[PENDSTCLR] - Clear a pending SysTick */
#define ICSR_PENDSTCLR_MSK             (0x1   << 25 )
#define ICSR_PENDSTCLR                 (0x1   << 25 )
#define ICSR_PENDSTCLR_DIS             (0x0   << 25 ) /* DIS                      */
#define ICSR_PENDSTCLR_EN              (0x1   << 25 ) /* EN                       */

/* ICSR[ISRPREEMPT] - If set, a pending exception will be serviced on exit from the debug halt state */
#define ICSR_ISRPREEMPT_MSK            (0x1   << 23 )
#define ICSR_ISRPREEMPT                (0x1   << 23 )
#define ICSR_ISRPREEMPT_DIS            (0x0   << 23 ) /* DIS                      */
#define ICSR_ISRPREEMPT_EN             (0x1   << 23 ) /* EN                       */

/* ICSR[ISRPENDING] - Indicates if an external configurable is pending */
#define ICSR_ISRPENDING_MSK            (0x1   << 22 )
#define ICSR_ISRPENDING                (0x1   << 22 )
#define ICSR_ISRPENDING_DIS            (0x0   << 22 ) /* DIS                      */
#define ICSR_ISRPENDING_EN             (0x1   << 22 ) /* EN                       */

/* ICSR[VECTPENDING] - Indicates the exception number for the highest priority pending exception */
#define ICSR_VECTPENDING_MSK           (0x1FF << 12 )

/* ICSR[RETTOBASE] -  */
#define ICSR_RETTOBASE_MSK             (0x1   << 11 )
#define ICSR_RETTOBASE                 (0x1   << 11 )
#define ICSR_RETTOBASE_DIS             (0x0   << 11 ) /* DIS                      */
#define ICSR_RETTOBASE_EN              (0x1   << 11 ) /* EN                       */

/* ICSR[VECTACTIVE] - Thread mode, or exception number */
#define ICSR_VECTACTIVE_MSK            (0x1FF << 0  )

/* Reset Value for VTOR*/
#define VTOR_RVAL                      0x0

/* VTOR[TBLBASE] -  */
#define VTOR_TBLBASE_MSK               (0x1   << 29 )
#define VTOR_TBLBASE                   (0x1   << 29 )
#define VTOR_TBLBASE_DIS               (0x0   << 29 ) /* DIS                      */
#define VTOR_TBLBASE_EN                (0x1   << 29 ) /* EN                       */

/* VTOR[TBLOFF] -  */
#define VTOR_TBLOFF_MSK                (0x3FFFFF << 7  )

/* Reset Value for AIRCR*/
#define AIRCR_RVAL                     0xFA050000

/* AIRCR[VECTKEYSTAT] - Reads as 0xFA05 */
#define AIRCR_VECTKEYSTAT_MSK          (0xFFFF << 16 )

/* AIRCR[ENDIANESS] - This bit is static or configured by a hardware input on reset */
#define AIRCR_ENDIANESS_MSK            (0x1   << 15 )
#define AIRCR_ENDIANESS                (0x1   << 15 )
#define AIRCR_ENDIANESS_DIS            (0x0   << 15 ) /* DIS                      */
#define AIRCR_ENDIANESS_EN             (0x1   << 15 ) /* EN                       */

/* AIRCR[PRIGROUP] - Priority grouping position */
#define AIRCR_PRIGROUP_MSK             (0x7   << 8  )

/* AIRCR[SYSRESETREQ] - System Reset Request */
#define AIRCR_SYSRESETREQ_MSK          (0x1   << 2  )
#define AIRCR_SYSRESETREQ              (0x1   << 2  )
#define AIRCR_SYSRESETREQ_DIS          (0x0   << 2  ) /* DIS                      */
#define AIRCR_SYSRESETREQ_EN           (0x1   << 2  ) /* EN                       */

/* AIRCR[VECTCLRACTIVE] - Clears all active state information for fixed and configurable exceptions */
#define AIRCR_VECTCLRACTIVE_MSK        (0x1   << 1  )
#define AIRCR_VECTCLRACTIVE            (0x1   << 1  )
#define AIRCR_VECTCLRACTIVE_DIS        (0x0   << 1  ) /* DIS                      */
#define AIRCR_VECTCLRACTIVE_EN         (0x1   << 1  ) /* EN                       */

/* AIRCR[VECTRESET] - Local system reset */
#define AIRCR_VECTRESET_MSK            (0x1   << 0  )
#define AIRCR_VECTRESET                (0x1   << 0  )
#define AIRCR_VECTRESET_DIS            (0x0   << 0  ) /* DIS                      */
#define AIRCR_VECTRESET_EN             (0x1   << 0  ) /* EN                       */

/* Reset Value for SCR*/
#define SCR_RVAL                       0x0

/* SCR[SEVONPEND] -  */
#define SCR_SEVONPEND_MSK              (0x1   << 4  )
#define SCR_SEVONPEND                  (0x1   << 4  )
#define SCR_SEVONPEND_DIS              (0x0   << 4  ) /* DIS                      */
#define SCR_SEVONPEND_EN               (0x1   << 4  ) /* EN                       */

/* SCR[SLEEPDEEP] - Sleep deep bit */
#define SCR_SLEEPDEEP_MSK              (0x1   << 2  )
#define SCR_SLEEPDEEP                  (0x1   << 2  )
#define SCR_SLEEPDEEP_DIS              (0x0   << 2  ) /* DIS                      */
#define SCR_SLEEPDEEP_EN               (0x1   << 2  ) /* EN                       */

/* SCR[SLEEPONEXIT] - Sleep on exit when returning from handler mode to thread mode */
#define SCR_SLEEPONEXIT_MSK            (0x1   << 1  )
#define SCR_SLEEPONEXIT                (0x1   << 1  )
#define SCR_SLEEPONEXIT_DIS            (0x0   << 1  ) /* DIS                      */
#define SCR_SLEEPONEXIT_EN             (0x1   << 1  ) /* EN                       */

/* Reset Value for CCR*/
#define CCR_RVAL                       0x200

/* CCR[STKALIGN] -  */
#define CCR_STKALIGN_MSK               (0x1   << 9  )
#define CCR_STKALIGN                   (0x1   << 9  )
#define CCR_STKALIGN_DIS               (0x0   << 9  ) /* DIS                      */
#define CCR_STKALIGN_EN                (0x1   << 9  ) /* EN                       */

/* CCR[BFHFNMIGN] -  */
#define CCR_BFHFNMIGN_MSK              (0x1   << 8  )
#define CCR_BFHFNMIGN                  (0x1   << 8  )
#define CCR_BFHFNMIGN_DIS              (0x0   << 8  ) /* DIS                      */
#define CCR_BFHFNMIGN_EN               (0x1   << 8  ) /* EN                       */

/* CCR[DIV0TRP] -  */
#define CCR_DIV0TRP_MSK                (0x1   << 4  )
#define CCR_DIV0TRP                    (0x1   << 4  )
#define CCR_DIV0TRP_DIS                (0x0   << 4  ) /* DIS                      */
#define CCR_DIV0TRP_EN                 (0x1   << 4  ) /* EN                       */

/* CCR[UNALIGNTRP] -  */
#define CCR_UNALIGNTRP_MSK             (0x1   << 3  )
#define CCR_UNALIGNTRP                 (0x1   << 3  )
#define CCR_UNALIGNTRP_DIS             (0x0   << 3  ) /* DIS                      */
#define CCR_UNALIGNTRP_EN              (0x1   << 3  ) /* EN                       */

/* CCR[USERSETMPEND] -  */
#define CCR_USERSETMPEND_MSK           (0x1   << 1  )
#define CCR_USERSETMPEND               (0x1   << 1  )
#define CCR_USERSETMPEND_DIS           (0x0   << 1  ) /* DIS                      */
#define CCR_USERSETMPEND_EN            (0x1   << 1  ) /* EN                       */

/* CCR[NONBASETHRDENA] -  */
#define CCR_NONBASETHRDENA_MSK         (0x1   << 0  )
#define CCR_NONBASETHRDENA             (0x1   << 0  )
#define CCR_NONBASETHRDENA_DIS         (0x0   << 0  ) /* DIS                      */
#define CCR_NONBASETHRDENA_EN          (0x1   << 0  ) /* EN                       */

/* Reset Value for SHPR1*/
#define SHPR1_RVAL                     0x0

/* SHPR1[PRI7] - Priority of system handler 7 - reserved */
#define SHPR1_PRI7_MSK                 (0xFF  << 24 )

/* SHPR1[PRI6] - Priority of system handler 6 - UsageFault */
#define SHPR1_PRI6_MSK                 (0xFF  << 16 )

/* SHPR1[PRI5] - Priority of system handler 5 - BusFault */
#define SHPR1_PRI5_MSK                 (0xFF  << 8  )

/* SHPR1[PRI4] - Priority of system handler 4 - MemManage */
#define SHPR1_PRI4_MSK                 (0xFF  << 0  )

/* Reset Value for SHPR2*/
#define SHPR2_RVAL                     0x0

/* SHPR2[PRI11] - Priority of system handler 11 - SVCall */
#define SHPR2_PRI11_MSK                (0xFF  << 24 )

/* SHPR2[PRI10] - Priority of system handler 10 - reserved */
#define SHPR2_PRI10_MSK                (0xFF  << 16 )

/* SHPR2[PRI9] - Priority of system handler 9 - reserved */
#define SHPR2_PRI9_MSK                 (0xFF  << 8  )

/* SHPR2[PRI8] - Priority of system handler 8 - reserved */
#define SHPR2_PRI8_MSK                 (0xFF  << 0  )

/* Reset Value for SHPR3*/
#define SHPR3_RVAL                     0x0

/* SHPR3[PRI15] - Priority of system handler 15 - SysTick */
#define SHPR3_PRI15_MSK                (0xFF  << 24 )

/* SHPR3[PRI14] - Priority of system handler 14 - PendSV */
#define SHPR3_PRI14_MSK                (0xFF  << 16 )

/* SHPR3[PRI13] - Priority of system handler 13 - reserved */
#define SHPR3_PRI13_MSK                (0xFF  << 8  )

/* SHPR3[PRI12] - Priority of system handler 12 - DebugMonitor */
#define SHPR3_PRI12_MSK                (0xFF  << 0  )

/* Reset Value for SHCSR*/
#define SHCSR_RVAL                     0x0

/* SHCSR[USGFAULTENA] - Enable for UsageFault */
#define SHCSR_USGFAULTENA_MSK          (0x1   << 18 )
#define SHCSR_USGFAULTENA              (0x1   << 18 )
#define SHCSR_USGFAULTENA_DIS          (0x0   << 18 ) /* DIS                      */
#define SHCSR_USGFAULTENA_EN           (0x1   << 18 ) /* EN                       */

/* SHCSR[BUSFAULTENA] - Enable for BusFault. */
#define SHCSR_BUSFAULTENA_MSK          (0x1   << 17 )
#define SHCSR_BUSFAULTENA              (0x1   << 17 )
#define SHCSR_BUSFAULTENA_DIS          (0x0   << 17 ) /* DIS                      */
#define SHCSR_BUSFAULTENA_EN           (0x1   << 17 ) /* EN                       */

/* SHCSR[MEMFAULTENA] - Enable for MemManage fault. */
#define SHCSR_MEMFAULTENA_MSK          (0x1   << 16 )
#define SHCSR_MEMFAULTENA              (0x1   << 16 )
#define SHCSR_MEMFAULTENA_DIS          (0x0   << 16 ) /* DIS                      */
#define SHCSR_MEMFAULTENA_EN           (0x1   << 16 ) /* EN                       */

/* SHCSR[SVCALLPENDED] - Reads as 1 if SVCall is Pending */
#define SHCSR_SVCALLPENDED_MSK         (0x1   << 15 )
#define SHCSR_SVCALLPENDED             (0x1   << 15 )
#define SHCSR_SVCALLPENDED_DIS         (0x0   << 15 ) /* DIS                      */
#define SHCSR_SVCALLPENDED_EN          (0x1   << 15 ) /* EN                       */

/* SHCSR[BUSFAULTPENDED] - Reads as 1 if BusFault is Pending */
#define SHCSR_BUSFAULTPENDED_MSK       (0x1   << 14 )
#define SHCSR_BUSFAULTPENDED           (0x1   << 14 )
#define SHCSR_BUSFAULTPENDED_DIS       (0x0   << 14 ) /* DIS                      */
#define SHCSR_BUSFAULTPENDED_EN        (0x1   << 14 ) /* EN                       */

/* SHCSR[MEMFAULTPENDED] - Reads as 1 if MemManage is Pending */
#define SHCSR_MEMFAULTPENDED_MSK       (0x1   << 13 )
#define SHCSR_MEMFAULTPENDED           (0x1   << 13 )
#define SHCSR_MEMFAULTPENDED_DIS       (0x0   << 13 ) /* DIS                      */
#define SHCSR_MEMFAULTPENDED_EN        (0x1   << 13 ) /* EN                       */

/* SHCSR[USGFAULTPENDED] - Reads as 1 if UsageFault is Pending */
#define SHCSR_USGFAULTPENDED_MSK       (0x1   << 12 )
#define SHCSR_USGFAULTPENDED           (0x1   << 12 )
#define SHCSR_USGFAULTPENDED_DIS       (0x0   << 12 ) /* DIS                      */
#define SHCSR_USGFAULTPENDED_EN        (0x1   << 12 ) /* EN                       */

/* SHCSR[SYSTICKACT] - Reads as 1 if SysTick is Active */
#define SHCSR_SYSTICKACT_MSK           (0x1   << 11 )
#define SHCSR_SYSTICKACT               (0x1   << 11 )
#define SHCSR_SYSTICKACT_DIS           (0x0   << 11 ) /* DIS                      */
#define SHCSR_SYSTICKACT_EN            (0x1   << 11 ) /* EN                       */

/* SHCSR[PENDSVACT] - Reads as 1 if PendSV is Active */
#define SHCSR_PENDSVACT_MSK            (0x1   << 10 )
#define SHCSR_PENDSVACT                (0x1   << 10 )
#define SHCSR_PENDSVACT_DIS            (0x0   << 10 ) /* DIS                      */
#define SHCSR_PENDSVACT_EN             (0x1   << 10 ) /* EN                       */

/* SHCSR[MONITORACT] - Reads as 1 if the Monitor is Active */
#define SHCSR_MONITORACT_MSK           (0x1   << 8  )
#define SHCSR_MONITORACT               (0x1   << 8  )
#define SHCSR_MONITORACT_DIS           (0x0   << 8  ) /* DIS                      */
#define SHCSR_MONITORACT_EN            (0x1   << 8  ) /* EN                       */

/* SHCSR[SVCALLACT] - Reads as 1 if SVCall is Active */
#define SHCSR_SVCALLACT_MSK            (0x1   << 7  )
#define SHCSR_SVCALLACT                (0x1   << 7  )
#define SHCSR_SVCALLACT_DIS            (0x0   << 7  ) /* DIS                      */
#define SHCSR_SVCALLACT_EN             (0x1   << 7  ) /* EN                       */

/* SHCSR[USGFAULTACT] - Reads as 1 if UsageFault is Active. */
#define SHCSR_USGFAULTACT_MSK          (0x1   << 3  )
#define SHCSR_USGFAULTACT              (0x1   << 3  )
#define SHCSR_USGFAULTACT_DIS          (0x0   << 3  ) /* DIS                      */
#define SHCSR_USGFAULTACT_EN           (0x1   << 3  ) /* EN                       */

/* SHCSR[BUSFAULTACT] - Reads as 1 if BusFault is Active. */
#define SHCSR_BUSFAULTACT_MSK          (0x1   << 1  )
#define SHCSR_BUSFAULTACT              (0x1   << 1  )
#define SHCSR_BUSFAULTACT_DIS          (0x0   << 1  ) /* DIS                      */
#define SHCSR_BUSFAULTACT_EN           (0x1   << 1  ) /* EN                       */

/* SHCSR[MEMFAULTACT] - Reads as 1 if MemManage is Active */
#define SHCSR_MEMFAULTACT_MSK          (0x1   << 0  )
#define SHCSR_MEMFAULTACT              (0x1   << 0  )
#define SHCSR_MEMFAULTACT_DIS          (0x0   << 0  ) /* DIS                      */
#define SHCSR_MEMFAULTACT_EN           (0x1   << 0  ) /* EN                       */

/* Reset Value for CFSR*/
#define CFSR_RVAL                      0x0

/* CFSR[DIVBYZERO] - Divide by zero error */
#define CFSR_DIVBYZERO_MSK             (0x1   << 25 )
#define CFSR_DIVBYZERO                 (0x1   << 25 )
#define CFSR_DIVBYZERO_DIS             (0x0   << 25 ) /* DIS                      */
#define CFSR_DIVBYZERO_EN              (0x1   << 25 ) /* EN                       */

/* CFSR[UNALIGNED] - Unaligned access error */
#define CFSR_UNALIGNED_MSK             (0x1   << 24 )
#define CFSR_UNALIGNED                 (0x1   << 24 )
#define CFSR_UNALIGNED_DIS             (0x0   << 24 ) /* DIS                      */
#define CFSR_UNALIGNED_EN              (0x1   << 24 ) /* EN                       */

/* CFSR[NOCP] - Coprocessor access error */
#define CFSR_NOCP_MSK                  (0x1   << 19 )
#define CFSR_NOCP                      (0x1   << 19 )
#define CFSR_NOCP_DIS                  (0x0   << 19 ) /* DIS                      */
#define CFSR_NOCP_EN                   (0x1   << 19 ) /* EN                       */

/* CFSR[INVPC] - Integrity check error on EXC_RETURN */
#define CFSR_INVPC_MSK                 (0x1   << 18 )
#define CFSR_INVPC                     (0x1   << 18 )
#define CFSR_INVPC_DIS                 (0x0   << 18 ) /* DIS                      */
#define CFSR_INVPC_EN                  (0x1   << 18 ) /* EN                       */

/* CFSR[INVSTATE] - Invalid EPSR.T bit or illegal EPSR.IT bits for executing */
#define CFSR_INVSTATE_MSK              (0x1   << 17 )
#define CFSR_INVSTATE                  (0x1   << 17 )
#define CFSR_INVSTATE_DIS              (0x0   << 17 ) /* DIS                      */
#define CFSR_INVSTATE_EN               (0x1   << 17 ) /* EN                       */

/* CFSR[UNDEFINSTR] - Undefined instruction executed */
#define CFSR_UNDEFINSTR_MSK            (0x1   << 16 )
#define CFSR_UNDEFINSTR                (0x1   << 16 )
#define CFSR_UNDEFINSTR_DIS            (0x0   << 16 ) /* DIS                      */
#define CFSR_UNDEFINSTR_EN             (0x1   << 16 ) /* EN                       */

/* CFSR[BFARVALID] - This bit is set if the BFAR register has valid contents */
#define CFSR_BFARVALID_MSK             (0x1   << 15 )
#define CFSR_BFARVALID                 (0x1   << 15 )
#define CFSR_BFARVALID_DIS             (0x0   << 15 ) /* DIS                      */
#define CFSR_BFARVALID_EN              (0x1   << 15 ) /* EN                       */

/* CFSR[STKERR] - This bit indicates a derived bus fault has occurred on exception entry */
#define CFSR_STKERR_MSK                (0x1   << 12 )
#define CFSR_STKERR                    (0x1   << 12 )
#define CFSR_STKERR_DIS                (0x0   << 12 ) /* DIS                      */
#define CFSR_STKERR_EN                 (0x1   << 12 ) /* EN                       */

/* CFSR[UNSTKERR] - This bit indicates a derived bus fault has occurred on exception return */
#define CFSR_UNSTKERR_MSK              (0x1   << 11 )
#define CFSR_UNSTKERR                  (0x1   << 11 )
#define CFSR_UNSTKERR_DIS              (0x0   << 11 ) /* DIS                      */
#define CFSR_UNSTKERR_EN               (0x1   << 11 ) /* EN                       */

/* CFSR[IMPRECISERR] - Imprecise data access error */
#define CFSR_IMPRECISERR_MSK           (0x1   << 10 )
#define CFSR_IMPRECISERR               (0x1   << 10 )
#define CFSR_IMPRECISERR_DIS           (0x0   << 10 ) /* DIS                      */
#define CFSR_IMPRECISERR_EN            (0x1   << 10 ) /* EN                       */

/* CFSR[PRECISERR] - Precise data access error. The BFAR is written with the faulting address */
#define CFSR_PRECISERR_MSK             (0x1   << 9  )
#define CFSR_PRECISERR                 (0x1   << 9  )
#define CFSR_PRECISERR_DIS             (0x0   << 9  ) /* DIS                      */
#define CFSR_PRECISERR_EN              (0x1   << 9  ) /* EN                       */

/* CFSR[IBUSERR] - This bit indicates a bus fault on an instruction prefetch */
#define CFSR_IBUSERR_MSK               (0x1   << 8  )
#define CFSR_IBUSERR                   (0x1   << 8  )
#define CFSR_IBUSERR_DIS               (0x0   << 8  ) /* DIS                      */
#define CFSR_IBUSERR_EN                (0x1   << 8  ) /* EN                       */

/* CFSR[MMARVALID] - This bit is set if the MMAR register has valid contents. */
#define CFSR_MMARVALID_MSK             (0x1   << 7  )
#define CFSR_MMARVALID                 (0x1   << 7  )
#define CFSR_MMARVALID_DIS             (0x0   << 7  ) /* DIS                      */
#define CFSR_MMARVALID_EN              (0x1   << 7  ) /* EN                       */

/* CFSR[MSTKERR] - A derived MemManage fault has occurred on exception entry */
#define CFSR_MSTKERR_MSK               (0x1   << 4  )
#define CFSR_MSTKERR                   (0x1   << 4  )
#define CFSR_MSTKERR_DIS               (0x0   << 4  ) /* DIS                      */
#define CFSR_MSTKERR_EN                (0x1   << 4  ) /* EN                       */

/* CFSR[MUNSTKERR] - A derived MemManage fault has occurred on exception return */
#define CFSR_MUNSTKERR_MSK             (0x1   << 3  )
#define CFSR_MUNSTKERR                 (0x1   << 3  )
#define CFSR_MUNSTKERR_DIS             (0x0   << 3  ) /* DIS                      */
#define CFSR_MUNSTKERR_EN              (0x1   << 3  ) /* EN                       */

/* CFSR[DACCVIOL] - Data access violation. The MMAR is set to the data address which the load store tried to access. */
#define CFSR_DACCVIOL_MSK              (0x1   << 1  )
#define CFSR_DACCVIOL                  (0x1   << 1  )
#define CFSR_DACCVIOL_DIS              (0x0   << 1  ) /* DIS                      */
#define CFSR_DACCVIOL_EN               (0x1   << 1  ) /* EN                       */

/* CFSR[IACCVIOL] - violation on an instruction fetch. */
#define CFSR_IACCVIOL_MSK              (0x1   << 0  )
#define CFSR_IACCVIOL                  (0x1   << 0  )
#define CFSR_IACCVIOL_DIS              (0x0   << 0  ) /* DIS                      */
#define CFSR_IACCVIOL_EN               (0x1   << 0  ) /* EN                       */

/* Reset Value for HFSR*/
#define HFSR_RVAL                      0x0

/* HFSR[DEBUGEVT] - Debug event, and the Debug Fault Status Register has been updated. */
#define HFSR_DEBUGEVT_MSK              (0x1   << 31 )
#define HFSR_DEBUGEVT                  (0x1   << 31 )
#define HFSR_DEBUGEVT_DIS              (0x0   << 31 ) /* DIS                      */
#define HFSR_DEBUGEVT_EN               (0x1   << 31 ) /* EN                       */

/* HFSR[FORCED] - Configurable fault cannot be activated due to priority or it was disabled. Priority escalated to a HardFault. */
#define HFSR_FORCED_MSK                (0x1   << 30 )
#define HFSR_FORCED                    (0x1   << 30 )
#define HFSR_FORCED_DIS                (0x0   << 30 ) /* DIS                      */
#define HFSR_FORCED_EN                 (0x1   << 30 ) /* EN                       */

/* HFSR[VECTTBL] - Fault was due to vector table read on exception processing */
#define HFSR_VECTTBL_MSK               (0x1   << 1  )
#define HFSR_VECTTBL                   (0x1   << 1  )
#define HFSR_VECTTBL_DIS               (0x0   << 1  ) /* DIS                      */
#define HFSR_VECTTBL_EN                (0x1   << 1  ) /* EN                       */

/* Reset Value for MMFAR*/
#define MMFAR_RVAL                     0x0

/* MMFAR[ADDRESS] - Data address MPU faulted. */
#define MMFAR_ADDRESS_MSK              (0xFFFFFFFF << 0  )

/* Reset Value for BFAR*/
#define BFAR_RVAL                      0x0

/* BFAR[ADDRESS] - Updated on precise data access faults */
#define BFAR_ADDRESS_MSK               (0xFFFFFFFF << 0  )

/* Reset Value for STIR*/
#define STIR_RVAL                      0x0

/* STIR[INTID] - The value written in this field is the interrupt to be triggered. */
#define STIR_INTID_MSK                 (0x3FF << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        PWRCTL                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Power Management Unit (pADI_PWRCTL)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_PWRCTL Structure                  */
  __IO uint16_t  PWRMOD;                    /*!< Power Modes Register                  */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  PWRKEY;                    /*!< Key Protection for the PWRMOD Register. */
  __I  uint16_t  RESERVED1;
  __IO uint8_t   PSMCON;                    /*!< Power Supply Monitor Control and Status */
  __I  uint8_t   RESERVED2[111];
  __IO uint8_t   SRAMRET;                   /*!< SRAM Retention Register               */
  __I  uint8_t   RESERVED3[3];
  __IO uint8_t   SHUTDOWN;                  /*!< Shutdown Acknowledge Register         */
} ADI_PWRCTL_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          PWRMOD                                     (*(volatile unsigned short int *) 0x40002400)
#define          PWRKEY                                     (*(volatile unsigned short int *) 0x40002404)
#define          PSMCON                                     (*(volatile unsigned char      *) 0x40002408)
#define          SRAMRET                                    (*(volatile unsigned char      *) 0x40002478)
#define          SHUTDOWN                                   (*(volatile unsigned char      *) 0x4000247C)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for PWRMOD*/
#define PWRMOD_RVAL                    0x100

/* PWRMOD[WICENACK] - WIC Acknowledge, for cortex M3 deep sleep mode */
#define PWRMOD_WICENACK_BBA            (*(volatile unsigned long *) 0x4204800C)
#define PWRMOD_WICENACK_MSK            (0x1   << 3  )
#define PWRMOD_WICENACK                (0x1   << 3  )
#define PWRMOD_WICENACK_CLR            (0x0   << 3  ) /* CLR. Cleared automatically by hardware when the cortex M3 processor is not ready to enter deep sleep mode including if serial wire activity is detected. */
#define PWRMOD_WICENACK_SET            (0x1   << 3  ) /* SET. Set automatically by the cortex M3 processor when ready to enter sleep deep mode. */

/* PWRMOD[MOD] - Low Power Mode */
#define PWRMOD_MOD_MSK                 (0x7   << 0  )
#define PWRMOD_MOD_FLEXI               (0x0   << 0  ) /* FLEXI.                   */
#define PWRMOD_MOD_HIBERNATE           (0x5   << 0  ) /* HIBERNATE.               */
#define PWRMOD_MOD_SHUTDOWN            (0x6   << 0  ) /* SHUTDOWN.                */

/* Reset Value for PWRKEY*/
#define PWRKEY_RVAL                    0x0

/* PWRKEY[VALUE] -  */
#define PWRKEY_VALUE_MSK               (0xFFFF << 0  )
#define PWRKEY_VALUE_KEY1              (0x4859 << 0  ) /* KEY1                     */
#define PWRKEY_VALUE_KEY2              (0xF27B << 0  ) /* KEY2                     */

/* Reset Value for PSMCON*/
#define PSMCON_RVAL                    0x3

/* PSMCON[PD] - Power Supply Monitor power down bit. */
#define PSMCON_PD_BBA                  (*(volatile unsigned long *) 0x42048104)
#define PSMCON_PD_MSK                  (0x1   << 1  )
#define PSMCON_PD                      (0x1   << 1  )
#define PSMCON_PD_DIS                  (0x0   << 1  ) /* DIS. Power up the PSM.   */
#define PSMCON_PD_EN                   (0x1   << 1  ) /* EN. Power down the PSM.  */

/* Reset Value for SRAMRET*/
#define SRAMRET_RVAL                   0x1

/* SRAMRET[RETAIN] - SRAM retention enable bit */
#define SRAMRET_RETAIN_BBA             (*(volatile unsigned long *) 0x42048F00)
#define SRAMRET_RETAIN_MSK             (0x1   << 0  )
#define SRAMRET_RETAIN                 (0x1   << 0  )
#define SRAMRET_RETAIN_DIS             (0x0   << 0  ) /* DIS. To retain contents of the bottom 8 kB of SRAM only */
#define SRAMRET_RETAIN_EN              (0x1   << 0  ) /* EN. To retain contents of the entire 16 kB of SRAM */

/* Reset Value for SHUTDOWN*/
#define SHUTDOWN_RVAL                  0x0

/* SHUTDOWN[EINT8] - External Interrupt 8 detected during SHUTDOWN mode */
#define SHUTDOWN_EINT8_BBA             (*(volatile unsigned long *) 0x42048F88)
#define SHUTDOWN_EINT8_MSK             (0x1   << 2  )
#define SHUTDOWN_EINT8                 (0x1   << 2  )
#define SHUTDOWN_EINT8_CLR             (0x0   << 2  ) /* CLR. Cleared automatically by hardware when clearing IRQ8 in EICLR. */
#define SHUTDOWN_EINT8_SET             (0x1   << 2  ) /* SET Indicates the interrupt was detected */

/* SHUTDOWN[EINT1] - External Interrupt 1 detected during SHUTDOWN mode */
#define SHUTDOWN_EINT1_BBA             (*(volatile unsigned long *) 0x42048F84)
#define SHUTDOWN_EINT1_MSK             (0x1   << 1  )
#define SHUTDOWN_EINT1                 (0x1   << 1  )
#define SHUTDOWN_EINT1_CLR             (0x0   << 1  ) /* CLR. Cleared automatically by hardware when clearing IRQ1 in EICLR. */
#define SHUTDOWN_EINT1_SET             (0x1   << 1  ) /* SET Indicates the interrupt was detected */

/* SHUTDOWN[EINT0] - External Interrupt 0 detected during SHUTDOWN mode */
#define SHUTDOWN_EINT0_BBA             (*(volatile unsigned long *) 0x42048F80)
#define SHUTDOWN_EINT0_MSK             (0x1   << 0  )
#define SHUTDOWN_EINT0                 (0x1   << 0  )
#define SHUTDOWN_EINT0_CLR             (0x0   << 0  ) /* CLR. Cleared automatically by hardware when clearing IRQ0 in EICLR. */
#define SHUTDOWN_EINT0_SET             (0x1   << 0  ) /* SET Indicates the interrupt was detected */
// ------------------------------------------------------------------------------------------------
// -----                                        PWM                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Pulse Width Modulation (pADI_PWM)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_PWM Structure                     */
  __IO uint16_t  PWMCON0;                   /*!< PWM Control Register                  */
  __I  uint16_t  RESERVED0;
  __IO uint8_t   PWMCON1;                   /*!< Trip Control Register                 */
  __I  uint8_t   RESERVED1[3];
  __IO uint16_t  PWMCLRI;                   /*!< PWM Interrupt Clear                   */
  __I  uint16_t  RESERVED2[3];
  __IO uint16_t  PWM0COM0;                  /*!< Compare Register 0 for Pair 0         */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  PWM0COM1;                  /*!< Compare Register 1 for Pair 0         */
  __I  uint16_t  RESERVED4;
  __IO uint16_t  PWM0COM2;                  /*!< Compare Register 2 for Pair 0         */
  __I  uint16_t  RESERVED5;
  __IO uint16_t  PWM0LEN;                   /*!< Period Value Register for Pair 0      */
  __I  uint16_t  RESERVED6;
  __IO uint16_t  PWM1COM0;                  /*!< Compare Register 0 for Pair 1         */
  __I  uint16_t  RESERVED7;
  __IO uint16_t  PWM1COM1;                  /*!< Compare Register 1 for Pair 1         */
  __I  uint16_t  RESERVED8;
  __IO uint16_t  PWM1COM2;                  /*!< Compare Register 2 for Pair 1         */
  __I  uint16_t  RESERVED9;
  __IO uint16_t  PWM1LEN;                   /*!< Period Value Register for Pair 1      */
  __I  uint16_t  RESERVED10;
  __IO uint16_t  PWM2COM0;                  /*!< Compare Register 0 for Pair 2         */
  __I  uint16_t  RESERVED11;
  __IO uint16_t  PWM2COM1;                  /*!< Compare Register 1 for Pair 2         */
  __I  uint16_t  RESERVED12;
  __IO uint16_t  PWM2COM2;                  /*!< Compare Register 2 for Pair 2         */
  __I  uint16_t  RESERVED13;
  __IO uint16_t  PWM2LEN;                   /*!< Period Value Register for Pair 2      */
  __I  uint16_t  RESERVED14;
  __IO uint16_t  PWM3COM0;                  /*!< Compare Register 0 for Pair 3         */
  __I  uint16_t  RESERVED15;
  __IO uint16_t  PWM3COM1;                  /*!< Compare Register 1 for Pair 3         */
  __I  uint16_t  RESERVED16;
  __IO uint16_t  PWM3COM2;                  /*!< Compare Register 2 for Pair 3         */
  __I  uint16_t  RESERVED17;
  __IO uint16_t  PWM3LEN;                   /*!< Period Value Register for Pair 3      */
} ADI_PWM_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          PWMCON0                                    (*(volatile unsigned short int *) 0x40001000)
#define          PWMCON1                                    (*(volatile unsigned char      *) 0x40001004)
#define          PWMCLRI                                    (*(volatile unsigned short int *) 0x40001008)
#define          PWM0COM0                                   (*(volatile unsigned short int *) 0x40001010)
#define          PWM0COM1                                   (*(volatile unsigned short int *) 0x40001014)
#define          PWM0COM2                                   (*(volatile unsigned short int *) 0x40001018)
#define          PWM0LEN                                    (*(volatile unsigned short int *) 0x4000101C)
#define          PWM1COM0                                   (*(volatile unsigned short int *) 0x40001020)
#define          PWM1COM1                                   (*(volatile unsigned short int *) 0x40001024)
#define          PWM1COM2                                   (*(volatile unsigned short int *) 0x40001028)
#define          PWM1LEN                                    (*(volatile unsigned short int *) 0x4000102C)
#define          PWM2COM0                                   (*(volatile unsigned short int *) 0x40001030)
#define          PWM2COM1                                   (*(volatile unsigned short int *) 0x40001034)
#define          PWM2COM2                                   (*(volatile unsigned short int *) 0x40001038)
#define          PWM2LEN                                    (*(volatile unsigned short int *) 0x4000103C)
#define          PWM3COM0                                   (*(volatile unsigned short int *) 0x40001040)
#define          PWM3COM1                                   (*(volatile unsigned short int *) 0x40001044)
#define          PWM3COM2                                   (*(volatile unsigned short int *) 0x40001048)
#define          PWM3LEN                                    (*(volatile unsigned short int *) 0x4000104C)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for PWMCON0*/
#define PWMCON0_RVAL                   0x12

/* PWMCON0[SYNC] - PWM Synchronization. */
#define PWMCON0_SYNC_BBA               (*(volatile unsigned long *) 0x4202003C)
#define PWMCON0_SYNC_MSK               (0x1   << 15 )
#define PWMCON0_SYNC                   (0x1   << 15 )
#define PWMCON0_SYNC_DIS               (0x0   << 15 ) /* DIS. Ignore transitions on the PWMSYNC pin. */
#define PWMCON0_SYNC_EN                (0x1   << 15 ) /* EN. All PWM counters are reset on the next clock edge after the detection of a falling edge on the PWMSYNC pin. */

/* PWMCON0[PWM7INV] - Inversion of PWM output. Available in standard mode only. */
#define PWMCON0_PWM7INV_BBA            (*(volatile unsigned long *) 0x42020038)
#define PWMCON0_PWM7INV_MSK            (0x1   << 14 )
#define PWMCON0_PWM7INV                (0x1   << 14 )
#define PWMCON0_PWM7INV_DIS            (0x0   << 14 ) /* DIS. PWM7 is normal.     */
#define PWMCON0_PWM7INV_EN             (0x1   << 14 ) /* EN. Invert PWM7.         */

/* PWMCON0[PWM5INV] - Inversion of PWM output. Available in standard mode only. */
#define PWMCON0_PWM5INV_BBA            (*(volatile unsigned long *) 0x42020034)
#define PWMCON0_PWM5INV_MSK            (0x1   << 13 )
#define PWMCON0_PWM5INV                (0x1   << 13 )
#define PWMCON0_PWM5INV_DIS            (0x0   << 13 ) /* DIS. PWM5 is normal.     */
#define PWMCON0_PWM5INV_EN             (0x1   << 13 ) /* EN. Invert PWM5.         */

/* PWMCON0[PWM3INV] - Inversion of PWM output. Available in standard mode only. */
#define PWMCON0_PWM3INV_BBA            (*(volatile unsigned long *) 0x42020030)
#define PWMCON0_PWM3INV_MSK            (0x1   << 12 )
#define PWMCON0_PWM3INV                (0x1   << 12 )
#define PWMCON0_PWM3INV_DIS            (0x0   << 12 ) /* DIS. PWM3 is normal.     */
#define PWMCON0_PWM3INV_EN             (0x1   << 12 ) /* EN. Invert PWM3.         */

/* PWMCON0[PWM1INV] - Inversion of PWM output. Available in standard mode only. */
#define PWMCON0_PWM1INV_BBA            (*(volatile unsigned long *) 0x4202002C)
#define PWMCON0_PWM1INV_MSK            (0x1   << 11 )
#define PWMCON0_PWM1INV                (0x1   << 11 )
#define PWMCON0_PWM1INV_DIS            (0x0   << 11 ) /* DIS.  PWM1 is normal.    */
#define PWMCON0_PWM1INV_EN             (0x1   << 11 ) /* EN. Invert PWM1.         */

/* PWMCON0[PWMIEN] - Enable PWM interrupts. */
#define PWMCON0_PWMIEN_BBA             (*(volatile unsigned long *) 0x42020028)
#define PWMCON0_PWMIEN_MSK             (0x1   << 10 )
#define PWMCON0_PWMIEN                 (0x1   << 10 )
#define PWMCON0_PWMIEN_DIS             (0x0   << 10 ) /* DIS. Disable PWM interrupts. */
#define PWMCON0_PWMIEN_EN              (0x1   << 10 ) /* EN.  Enable PWM interrupts. */

/* PWMCON0[ENA] - Enable PWM outputs. Available in H-Bridge mode only. */
#define PWMCON0_ENA_BBA                (*(volatile unsigned long *) 0x42020024)
#define PWMCON0_ENA_MSK                (0x1   << 9  )
#define PWMCON0_ENA                    (0x1   << 9  )
#define PWMCON0_ENA_DIS                (0x0   << 9  ) /* DIS. Disable PWM outputs. */
#define PWMCON0_ENA_EN                 (0x1   << 9  ) /* EN. Enable PWM outputs.  */

/* PWMCON0[PWMCP] - PWM Clock Prescaler. Sets UCLK divider. */
#define PWMCON0_PWMCP_MSK              (0x7   << 6  )
#define PWMCON0_PWMCP_UCLKDIV2         (0x0   << 6  ) /* UCLK/2.                  */
#define PWMCON0_PWMCP_UCLKDIV4         (0x1   << 6  ) /* UCLK/4.                  */
#define PWMCON0_PWMCP_UCLKDIV8         (0x2   << 6  ) /* UCLK/8.                  */
#define PWMCON0_PWMCP_UCLKDIV16        (0x3   << 6  ) /* UCLK/16.                 */
#define PWMCON0_PWMCP_UCLKDIV32        (0x4   << 6  ) /* UCLK/32.                 */
#define PWMCON0_PWMCP_UCLKDIV64        (0x5   << 6  ) /* UCLK/64.                 */
#define PWMCON0_PWMCP_UCLKDIV128       (0x6   << 6  ) /* UCLK/128.                */
#define PWMCON0_PWMCP_UCLKDIV256       (0x7   << 6  ) /* UCLK/256.                */

/* PWMCON0[POINV] - Invert all PWM outputs. Available in H-Bridge mode only. */
#define PWMCON0_POINV_BBA              (*(volatile unsigned long *) 0x42020014)
#define PWMCON0_POINV_MSK              (0x1   << 5  )
#define PWMCON0_POINV                  (0x1   << 5  )
#define PWMCON0_POINV_DIS              (0x0   << 5  ) /* DIS. PWM outputs as normal. */
#define PWMCON0_POINV_EN               (0x1   << 5  ) /* EN. Invert all PWM outputs. */

/* PWMCON0[HOFF] - High Side Off. Available in H-Bridge mode only. */
#define PWMCON0_HOFF_BBA               (*(volatile unsigned long *) 0x42020010)
#define PWMCON0_HOFF_MSK               (0x1   << 4  )
#define PWMCON0_HOFF                   (0x1   << 4  )
#define PWMCON0_HOFF_DIS               (0x0   << 4  ) /* DIS. PWM outputs as normal. */
#define PWMCON0_HOFF_EN                (0x1   << 4  ) /* EN. Force PWM0 and PWM2 outputs high and PWM1 and PWM3 low. */

/* PWMCON0[LCOMP] - Load Compare Registers. */
#define PWMCON0_LCOMP_BBA              (*(volatile unsigned long *) 0x4202000C)
#define PWMCON0_LCOMP_MSK              (0x1   << 3  )
#define PWMCON0_LCOMP                  (0x1   << 3  )
#define PWMCON0_LCOMP_DIS              (0x0   << 3  ) /* DIS. Use the values previously stored in the internal compare registers. */
#define PWMCON0_LCOMP_EN               (0x1   << 3  ) /* EN. Load the internal compare registers with the values in PWMxCOMx on the next transition of the PWM timer from 0x00 to 0x01. */

/* PWMCON0[DIR] - Direction Control. Available in H-Bridge mode only. */
#define PWMCON0_DIR_BBA                (*(volatile unsigned long *) 0x42020008)
#define PWMCON0_DIR_MSK                (0x1   << 2  )
#define PWMCON0_DIR                    (0x1   << 2  )
#define PWMCON0_DIR_DIS                (0x0   << 2  ) /* DIS. Enable PWM2 and PWM3 as the output signals while PWM0 and PWM1 are held low. */
#define PWMCON0_DIR_EN                 (0x1   << 2  ) /* EN. Enable PWM0 and PWM1 as the output signals while PWM2 and PWM3 are held low. */

/* PWMCON0[HMODE] - Enable H-Bridge Mode. */
#define PWMCON0_HMODE_BBA              (*(volatile unsigned long *) 0x42020004)
#define PWMCON0_HMODE_MSK              (0x1   << 1  )
#define PWMCON0_HMODE                  (0x1   << 1  )
#define PWMCON0_HMODE_DIS              (0x0   << 1  ) /* DIS. The PWM operates in standard mode. */
#define PWMCON0_HMODE_EN               (0x1   << 1  ) /* EN. The PWM is configured in H-Bridge mode. */

/* PWMCON0[PWMEN] - Enable all PWM outputs. */
#define PWMCON0_PWMEN_BBA              (*(volatile unsigned long *) 0x42020000)
#define PWMCON0_PWMEN_MSK              (0x1   << 0  )
#define PWMCON0_PWMEN                  (0x1   << 0  )
#define PWMCON0_PWMEN_DIS              (0x0   << 0  ) /* DIS. Disables all PWM outputs. */
#define PWMCON0_PWMEN_EN               (0x1   << 0  ) /* EN. Enables all PWM outputs. */

/* Reset Value for PWMCON1*/
#define PWMCON1_RVAL                   0x0

/* PWMCON1[TRIPEN] - Enable PWM trip functionality. */
#define PWMCON1_TRIPEN_BBA             (*(volatile unsigned long *) 0x42020098)
#define PWMCON1_TRIPEN_MSK             (0x1   << 6  )
#define PWMCON1_TRIPEN                 (0x1   << 6  )
#define PWMCON1_TRIPEN_DIS             (0x0   << 6  ) /* DIS. Disable PWM trip functionality. */
#define PWMCON1_TRIPEN_EN              (0x1   << 6  ) /* EN. Enable PWM trip functionality. */

/* Reset Value for PWMCLRI*/
#define PWMCLRI_RVAL                   0x0

/* PWMCLRI[TRIP] - Clear the latched trip interrupt. This bit always reads 0. */
#define PWMCLRI_TRIP_BBA               (*(volatile unsigned long *) 0x42020110)
#define PWMCLRI_TRIP_MSK               (0x1   << 4  )
#define PWMCLRI_TRIP                   (0x1   << 4  )
#define PWMCLRI_TRIP_EN                (0x1   << 4  ) /* EN. Clear the latched PWMTRIP interrupt. */

/* PWMCLRI[IRQPWM3] - Clear the latched PWM3 interrupt. This bit always reads 0. */
#define PWMCLRI_IRQPWM3_BBA            (*(volatile unsigned long *) 0x4202010C)
#define PWMCLRI_IRQPWM3_MSK            (0x1   << 3  )
#define PWMCLRI_IRQPWM3                (0x1   << 3  )
#define PWMCLRI_IRQPWM3_EN             (0x1   << 3  ) /* EN. Clear the latched IRQPWM3 interrupt. */

/* PWMCLRI[IRQPWM2] - Clear the latched PWM2 interrupt. This bit always reads 0. */
#define PWMCLRI_IRQPWM2_BBA            (*(volatile unsigned long *) 0x42020108)
#define PWMCLRI_IRQPWM2_MSK            (0x1   << 2  )
#define PWMCLRI_IRQPWM2                (0x1   << 2  )
#define PWMCLRI_IRQPWM2_EN             (0x1   << 2  ) /* EN. Clear the latched IRQPWM2 interrupt. */

/* PWMCLRI[IRQPWM1] - Clear the latched PWM1 interrupt. This bit always reads 0. */
#define PWMCLRI_IRQPWM1_BBA            (*(volatile unsigned long *) 0x42020104)
#define PWMCLRI_IRQPWM1_MSK            (0x1   << 1  )
#define PWMCLRI_IRQPWM1                (0x1   << 1  )
#define PWMCLRI_IRQPWM1_EN             (0x1   << 1  ) /* EN. Clear the latched IRQPWM1 interrupt. */

/* PWMCLRI[IRQPWM0] - Clear the latched PWM0 interrupt. This bit always reads 0. */
#define PWMCLRI_IRQPWM0_BBA            (*(volatile unsigned long *) 0x42020100)
#define PWMCLRI_IRQPWM0_MSK            (0x1   << 0  )
#define PWMCLRI_IRQPWM0                (0x1   << 0  )
#define PWMCLRI_IRQPWM0_EN             (0x1   << 0  ) /* EN. Clear the latched IRQPWM0 interrupt. */

/* Reset Value for PWM0COM0*/
#define PWM0COM0_RVAL                  0x0

/* PWM0COM0[VALUE] -  */
#define PWM0COM0_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM0COM1*/
#define PWM0COM1_RVAL                  0x0

/* PWM0COM1[VALUE] -  */
#define PWM0COM1_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM0COM2*/
#define PWM0COM2_RVAL                  0x0

/* PWM0COM2[VALUE] -  */
#define PWM0COM2_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM0LEN*/
#define PWM0LEN_RVAL                   0x0

/* PWM0LEN[VALUE] -  */
#define PWM0LEN_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for PWM1COM0*/
#define PWM1COM0_RVAL                  0x0

/* PWM1COM0[VALUE] -  */
#define PWM1COM0_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM1COM1*/
#define PWM1COM1_RVAL                  0x0

/* PWM1COM1[VALUE] -  */
#define PWM1COM1_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM1COM2*/
#define PWM1COM2_RVAL                  0x0

/* PWM1COM2[VALUE] -  */
#define PWM1COM2_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM1LEN*/
#define PWM1LEN_RVAL                   0x0

/* PWM1LEN[VALUE] -  */
#define PWM1LEN_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for PWM2COM0*/
#define PWM2COM0_RVAL                  0x0

/* PWM2COM0[VALUE] -  */
#define PWM2COM0_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM2COM1*/
#define PWM2COM1_RVAL                  0x0

/* PWM2COM1[VALUE] -  */
#define PWM2COM1_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM2COM2*/
#define PWM2COM2_RVAL                  0x0

/* PWM2COM2[VALUE] -  */
#define PWM2COM2_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM2LEN*/
#define PWM2LEN_RVAL                   0x0

/* PWM2LEN[VALUE] -  */
#define PWM2LEN_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for PWM3COM0*/
#define PWM3COM0_RVAL                  0x0

/* PWM3COM0[VALUE] -  */
#define PWM3COM0_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM3COM1*/
#define PWM3COM1_RVAL                  0x0

/* PWM3COM1[VALUE] -  */
#define PWM3COM1_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM3COM2*/
#define PWM3COM2_RVAL                  0x0

/* PWM3COM2[VALUE] -  */
#define PWM3COM2_VALUE_MSK             (0xFFFF << 0  )

/* Reset Value for PWM3LEN*/
#define PWM3LEN_RVAL                   0x0

/* PWM3LEN[VALUE] -  */
#define PWM3LEN_VALUE_MSK              (0xFFFF << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        RESET                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Reset (pADI_RESET)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_RESET Structure                   */

  union {
    __IO uint8_t   RSTSTA;                  /*!< Reset Status                          */
    __IO uint8_t   RSTCLR;                  /*!< Reset Status Clear                    */
  } ;
} ADI_RESET_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          RSTSTA                                     (*(volatile unsigned char      *) 0x40002440)
#define          RSTCLR                                     (*(volatile unsigned char      *) 0x40002440)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for RSTSTA*/
#define RSTSTA_RVAL                    0x3

/* RSTSTA[SWRST] - Software reset status bit */
#define RSTSTA_SWRST_BBA               (*(volatile unsigned long *) 0x42048810)
#define RSTSTA_SWRST_MSK               (0x1   << 4  )
#define RSTSTA_SWRST                   (0x1   << 4  )
#define RSTSTA_SWRST_CLR               (0x0   << 4  ) /* CLR. Indicates that no software reset has occurred. */
#define RSTSTA_SWRST_SET               (0x1   << 4  ) /* SET. Indicates that a software reset has occurred. */

/* RSTSTA[WDRST] - Watchdog reset status bit */
#define RSTSTA_WDRST_BBA               (*(volatile unsigned long *) 0x4204880C)
#define RSTSTA_WDRST_MSK               (0x1   << 3  )
#define RSTSTA_WDRST                   (0x1   << 3  )
#define RSTSTA_WDRST_CLR               (0x0   << 3  ) /* CLR. Indicates that no watchdog reset has occurred. */
#define RSTSTA_WDRST_SET               (0x1   << 3  ) /* SET. Indicates that a Watchdog Reset has occurred. */

/* RSTSTA[EXTRST] - External reset status bit */
#define RSTSTA_EXTRST_BBA              (*(volatile unsigned long *) 0x42048808)
#define RSTSTA_EXTRST_MSK              (0x1   << 2  )
#define RSTSTA_EXTRST                  (0x1   << 2  )
#define RSTSTA_EXTRST_CLR              (0x0   << 2  ) /* CLR. Indicates that no external reset has occurred. */
#define RSTSTA_EXTRST_SET              (0x1   << 2  ) /* SET. Indicates an external reset has occurred. */

/* RSTSTA[PORHV] - Power-on reset status bit HV */
#define RSTSTA_PORHV_BBA               (*(volatile unsigned long *) 0x42048804)
#define RSTSTA_PORHV_MSK               (0x1   << 1  )
#define RSTSTA_PORHV                   (0x1   << 1  )
#define RSTSTA_PORHV_CLR               (0x0   << 1  ) /* CLR. Indicates a POR or wake up from SHUTDOWN has not occurred. */
#define RSTSTA_PORHV_SET               (0x1   << 1  ) /* SET. This bit indicates that the AVDD supply has dropped below the POR trip point, causing a Power On Reset. It is also set when waking up from SHUTDOWN mode. */

/* RSTSTA[PORLV] - Power-on reset status bit LV */
#define RSTSTA_PORLV_BBA               (*(volatile unsigned long *) 0x42048800)
#define RSTSTA_PORLV_MSK               (0x1   << 0  )
#define RSTSTA_PORLV                   (0x1   << 0  )
#define RSTSTA_PORLV_CLR               (0x0   << 0  ) /* CLR. Indicates a POR or wake up from SHUTDOWN has not occurred. */
#define RSTSTA_PORLV_SET               (0x1   << 0  ) /* SET. This bit indicates that the AVDD supply has dropped below the POR trip point, causing a Power On Reset. It is also set when waking up from SHUTDOWN mode. */

/* Reset Value for RSTCLR*/
#define RSTCLR_RVAL                    0x3

/* RSTCLR[SWRST] - Software reset clear status bit */
#define RSTCLR_SWRST_BBA               (*(volatile unsigned long *) 0x42048810)
#define RSTCLR_SWRST_MSK               (0x1   << 4  )
#define RSTCLR_SWRST                   (0x1   << 4  )
#define RSTCLR_SWRST_DIS               (0x0   << 4  ) /* DIS. Has no effect.      */
#define RSTCLR_SWRST_EN                (0x1   << 4  ) /* EN. Clears the SWRST status bit in RSTSTA. */

/* RSTCLR[WDRST] - Watchdog reset clear status bit */
#define RSTCLR_WDRST_BBA               (*(volatile unsigned long *) 0x4204880C)
#define RSTCLR_WDRST_MSK               (0x1   << 3  )
#define RSTCLR_WDRST                   (0x1   << 3  )
#define RSTCLR_WDRST_DIS               (0x0   << 3  ) /* DIS. Has no effect.      */
#define RSTCLR_WDRST_EN                (0x1   << 3  ) /* EN. Clears the WDRST status bit in RSTSTA. */

/* RSTCLR[EXTRST] - External reset clear status bit */
#define RSTCLR_EXTRST_BBA              (*(volatile unsigned long *) 0x42048808)
#define RSTCLR_EXTRST_MSK              (0x1   << 2  )
#define RSTCLR_EXTRST                  (0x1   << 2  )
#define RSTCLR_EXTRST_DIS              (0x0   << 2  ) /* DIS. Has no effect.      */
#define RSTCLR_EXTRST_EN               (0x1   << 2  ) /* EN. Clears the EXTRST status bit in RSTSTA. */

/* RSTCLR[PORHV] - Power on reset clear status bit */
#define RSTCLR_PORHV_BBA               (*(volatile unsigned long *) 0x42048804)
#define RSTCLR_PORHV_MSK               (0x1   << 1  )
#define RSTCLR_PORHV                   (0x1   << 1  )
#define RSTCLR_PORHV_DIS               (0x0   << 1  ) /* DIS. Has no effect.      */
#define RSTCLR_PORHV_EN                (0x1   << 1  ) /* EN. Clears PORLV status bit in RSTSTA. */

/* RSTCLR[PORLV] - Power-on reset clear status bit LV */
#define RSTCLR_PORLV_BBA               (*(volatile unsigned long *) 0x42048800)
#define RSTCLR_PORLV_MSK               (0x1   << 0  )
#define RSTCLR_PORLV                   (0x1   << 0  )
#define RSTCLR_PORLV_DIS               (0x0   << 0  ) /* DIS. Has no effect.      */
#define RSTCLR_PORLV_EN                (0x1   << 0  ) /* EN. Clears the PORLV status bit in RSTSTA. */
// ------------------------------------------------------------------------------------------------
// -----                                        SPI0                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Serial Peripheral Interface (pADI_SPI0)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_SPI0 Structure                    */
  __IO uint16_t  SPISTA;                    /*!< SPI0 Status Register                  */
  __I  uint16_t  RESERVED0;
  __IO uint8_t   SPIRX;                     /*!< SPI0 Receive Register                 */
  __I  uint8_t   RESERVED1[3];
  __IO uint8_t   SPITX;                     /*!< SPI0 Transmit Register                */
  __I  uint8_t   RESERVED2[3];
  __IO uint16_t  SPIDIV;                    /*!< SPI0 Bit Rate Selection Register      */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  SPICON;                    /*!< SPI0 Configuration Register           */
  __I  uint16_t  RESERVED4;
  __IO uint16_t  SPIDMA;                    /*!< SPI0 DMA Enable Register              */
  __I  uint16_t  RESERVED5;
  __IO uint16_t  SPICNT;                    /*!< SPI0 DMA Master Received Byte Count Register */
} ADI_SPI_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          SPI0STA                                    (*(volatile unsigned short int *) 0x40004000)
#define          SPI0RX                                     (*(volatile unsigned char      *) 0x40004004)
#define          SPI0TX                                     (*(volatile unsigned char      *) 0x40004008)
#define          SPI0DIV                                    (*(volatile unsigned short int *) 0x4000400C)
#define          SPI0CON                                    (*(volatile unsigned short int *) 0x40004010)
#define          SPI0DMA                                    (*(volatile unsigned short int *) 0x40004014)
#define          SPI0CNT                                    (*(volatile unsigned short int *) 0x40004018)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for SPI0STA*/
#define SPI0STA_RVAL                   0x0

/* SPI0STA[CSERR] - CS error status bit. This bit generates an interrupt when detecting an abrupt CS desassertion before the full byte of data is transmitted completely. */
#define SPI0STA_CSERR_BBA              (*(volatile unsigned long *) 0x42080030)
#define SPI0STA_CSERR_MSK              (0x1   << 12 )
#define SPI0STA_CSERR                  (0x1   << 12 )
#define SPI0STA_CSERR_CLR              (0x0   << 12 ) /* CLR: Cleared when no CS error is detected. Cleared to 0 on a read of SPI0STA register. */
#define SPI0STA_CSERR_SET              (0x1   << 12 ) /* SET: Set when the CS line is deasserted abruptly. */

/* SPI0STA[RXS] - Rx FIFO excess bytes present. */
#define SPI0STA_RXS_BBA                (*(volatile unsigned long *) 0x4208002C)
#define SPI0STA_RXS_MSK                (0x1   << 11 )
#define SPI0STA_RXS                    (0x1   << 11 )
#define SPI0STA_RXS_CLR                (0x0   << 11 ) /* CLR. When the number of bytes in the FIFO is equal or less than the number in SPI0CON[15:14]. This bit is not cleared on a read of SPI0STA register. */
#define SPI0STA_RXS_SET                (0x1   << 11 ) /* SET. When there are more bytes in the Rx FIFO than configured in MOD (SPI0CON[15:14]). For example if MOD = TX1RX1, RXS is set when there are 2 or more bytes in the Rx FIFO. This bit does not dependent on SPI0CON[6] and does not cause an interrupt. */

/* SPI0STA[RXFSTA] - Rx FIFO status bits, indicates how many valid bytes are in the Rx FIFO. */
#define SPI0STA_RXFSTA_MSK             (0x7   << 8  )
#define SPI0STA_RXFSTA_EMPTY           (0x0   << 8  ) /* EMPTY. When Rx FIFO is empty. */
#define SPI0STA_RXFSTA_ONEBYTE         (0x1   << 8  ) /* ONEBYTE. When 1 valid byte is in the FIFO. */
#define SPI0STA_RXFSTA_TWOBYTES        (0x2   << 8  ) /* TWOBYTES. When 2 valid bytes are in the FIFO. */
#define SPI0STA_RXFSTA_THREEBYTES      (0x3   << 8  ) /* THREEBYTES. When 3 valid bytes are in the FIFO. */
#define SPI0STA_RXFSTA_FOURBYTES       (0x4   << 8  ) /* FOURBYTES. When 4 valid bytes are in the FIFO. */

/* SPI0STA[RXOF] - Rx FIFO overflow status bit. This bit generates an interrupt. */
#define SPI0STA_RXOF_BBA               (*(volatile unsigned long *) 0x4208001C)
#define SPI0STA_RXOF_MSK               (0x1   << 7  )
#define SPI0STA_RXOF                   (0x1   << 7  )
#define SPI0STA_RXOF_CLR               (0x0   << 7  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPI0STA_RXOF_SET               (0x1   << 7  ) /* SET. Set when the Rx FIFO is already full when new data is loaded to the FIFO. This bit generates an interrupt except when RFLUSH is set. (SPI0CON[12]). */

/* SPI0STA[RX] - Rx interrupt status bit. This bit generates an interrupt, except when DMA transfer is enabled. */
#define SPI0STA_RX_BBA                 (*(volatile unsigned long *) 0x42080018)
#define SPI0STA_RX_MSK                 (0x1   << 6  )
#define SPI0STA_RX                     (0x1   << 6  )
#define SPI0STA_RX_CLR                 (0x0   << 6  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPI0STA_RX_SET                 (0x1   << 6  ) /* SET. Set when a receive interrupt occurs. This bit is set when TIM (SPI0CON[6]) is cleared and the required number of bytes have been received. */

/* SPI0STA[TX] - Tx interrupt status bit. This bit generates an interrupt, except when DMA transfer is enabled. */
#define SPI0STA_TX_BBA                 (*(volatile unsigned long *) 0x42080014)
#define SPI0STA_TX_MSK                 (0x1   << 5  )
#define SPI0STA_TX                     (0x1   << 5  )
#define SPI0STA_TX_CLR                 (0x0   << 5  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPI0STA_TX_SET                 (0x1   << 5  ) /* SET. Set when a transmit interrupt occurs. This bit is set when TIM (SPI0CON[6]) set and the required number of bytes have been transmitted. */

/* SPI0STA[TXUR] - Tx FIFO Underrun. This bit generates an interrupt. */
#define SPI0STA_TXUR_BBA               (*(volatile unsigned long *) 0x42080010)
#define SPI0STA_TXUR_MSK               (0x1   << 4  )
#define SPI0STA_TXUR                   (0x1   << 4  )
#define SPI0STA_TXUR_CLR               (0x0   << 4  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPI0STA_TXUR_SET               (0x1   << 4  ) /* SET. Set when a transmit is initiated without any valid data in the Tx FIFO. This bit generates an interrupt except when TFLUSH is set in SPI0CON. */

/* SPI0STA[TXFSTA] - Tx FIFO status bits, indicates how many valid bytes are in the Tx FIFO. */
#define SPI0STA_TXFSTA_MSK             (0x7   << 1  )
#define SPI0STA_TXFSTA_EMPTY           (0x0   << 1  ) /* EMPTY. Tx FIFO is empty. */
#define SPI0STA_TXFSTA_ONEBYTE         (0x1   << 1  ) /* ONEBYTE. 1 valid byte is in the FIFO. */
#define SPI0STA_TXFSTA_TWOBYTES        (0x2   << 1  ) /* TWOBYTES. 2 valid bytes are in the FIFO. */
#define SPI0STA_TXFSTA_THREEBYTES      (0x3   << 1  ) /* THREEBYTES. 3 valid bytes are in the FIFO. */
#define SPI0STA_TXFSTA_FOURBYTES       (0x4   << 1  ) /* FOURBYTES . 4 valid bytes are in the FIFO. */

/* SPI0STA[IRQ] - Interrupt status bit. */
#define SPI0STA_IRQ_BBA                (*(volatile unsigned long *) 0x42080000)
#define SPI0STA_IRQ_MSK                (0x1   << 0  )
#define SPI0STA_IRQ                    (0x1   << 0  )
#define SPI0STA_IRQ_CLR                (0x0   << 0  ) /* CLR. Cleared to 0 on a read of SPI0STA register. */
#define SPI0STA_IRQ_SET                (0x1   << 0  ) /* SET. Set to 1 when an SPI0 based interrupt occurs. */

/* Reset Value for SPI0RX*/
#define SPI0RX_RVAL                    0x0

/* SPI0RX[VALUE] - 8-bit receive register. A read of the RX FIFO returns the next byte to be read from the FIFO. A read of the FIFO when it is empty returns zero. */
#define SPI0RX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for SPI0TX*/
#define SPI0TX_RVAL                    0x0

/* SPI0TX[VALUE] - 8-bit transmit register. A write to the Tx FIFO address space writes data to the next available location in the Tx FIFO. If the FIFO is full, the oldest byte of data in the FIFO is overwritten. A read from this address location return zero. */
#define SPI0TX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for SPI0DIV*/
#define SPI0DIV_RVAL                   0x0

/* SPI0DIV[BCRST] - Configures the behavior of SPI communication after an abrupt deassertion of CS.  This bit should be set in slave and master mode. */
#define SPI0DIV_BCRST_BBA              (*(volatile unsigned long *) 0x4208019C)
#define SPI0DIV_BCRST_MSK              (0x1   << 7  )
#define SPI0DIV_BCRST                  (0x1   << 7  )
#define SPI0DIV_BCRST_DIS              (0x0   << 7  ) /* DIS. Resumes communication from where it stopped when the CS is deasserted. The rest of the bits are then received/ transmitted when CS returns low. User code should ignore the CSERR interrupt. */
#define SPI0DIV_BCRST_EN               (0x1   << 7  ) /* EN. Enabled for a clean restart of SPI transfer after a CSERR condition. User code must also clear the SPI enable bit in SPI0CON during the CSERR interrupt. */

/* SPI0DIV[DIV] - Factor used to divide UCLK in the generation of the master mode serial clock. */
#define SPI0DIV_DIV_MSK                (0x3F  << 0  )

/* Reset Value for SPI0CON*/
#define SPI0CON_RVAL                   0x0

/* SPI0CON[MOD] - IRQ mode bits. When TIM is set these bits configure when the Tx/Rx interrupts occur in a transfer. For a DMA Rx transfer, these bits should be 00. */
#define SPI0CON_MOD_MSK                (0x3   << 14 )
#define SPI0CON_MOD_TX1RX1             (0x0   << 14 ) /* TX1RX1. Tx/Rx interrupt occurs when 1 byte has been transmitted/received from/into the FIFO. */
#define SPI0CON_MOD_TX2RX2             (0x1   << 14 ) /* TX2RX2. Tx/Rx interrupt occurs when 2 bytes have been transmitted/received from/into the FIFO. */
#define SPI0CON_MOD_TX3RX3             (0x2   << 14 ) /* TX3RX3. Tx/Rx interrupt occurs when 3 bytes have been transmitted/received from/into the FIFO. */
#define SPI0CON_MOD_TX4RX4             (0x3   << 14 ) /* TX4RX4. Tx/Rx interrupt occurs when 4 bytes have been transmitted/received from/into the FIFO. */

/* SPI0CON[TFLUSH] - Tx FIFO flush enable bit. */
#define SPI0CON_TFLUSH_BBA             (*(volatile unsigned long *) 0x42080234)
#define SPI0CON_TFLUSH_MSK             (0x1   << 13 )
#define SPI0CON_TFLUSH                 (0x1   << 13 )
#define SPI0CON_TFLUSH_DIS             (0x0   << 13 ) /* DIS. Disable Tx FIFO flushing. */
#define SPI0CON_TFLUSH_EN              (0x1   << 13 ) /* EN. Flush the Tx FIFO. This bit does not clear itself and should be toggled if a single flush is required. If this bit is left high, then either the last transmitted value or 0x00 is transmitted depending on the ZEN bit (SPI0CON[7]). Any writes to the Tx FIFO are ignored while this bit is set. */

/* SPI0CON[RFLUSH] - Rx FIFO flush enable bit. */
#define SPI0CON_RFLUSH_BBA             (*(volatile unsigned long *) 0x42080230)
#define SPI0CON_RFLUSH_MSK             (0x1   << 12 )
#define SPI0CON_RFLUSH                 (0x1   << 12 )
#define SPI0CON_RFLUSH_DIS             (0x0   << 12 ) /* DIS. Disable Rx FIFO flushing. */
#define SPI0CON_RFLUSH_EN              (0x1   << 12 ) /* EN. If this bit is set, all incoming data is ignored and no interrupts are generated. If set and TIM = 0  (SPI0CON[6]), a read of the Rx FIFO initiates a transfer. */

/* SPI0CON[CON] - Continuous transfer enable bit. */
#define SPI0CON_CON_BBA                (*(volatile unsigned long *) 0x4208022C)
#define SPI0CON_CON_MSK                (0x1   << 11 )
#define SPI0CON_CON                    (0x1   << 11 )
#define SPI0CON_CON_DIS                (0x0   << 11 ) /* DIS. Disable continuous transfer. Each transfer consists of a single 8-bit serial transfer. If valid data exists in the SPIxTX register, then a new transfer is initiated after a stall period of one serial clock cycle. The CS line is deactivated for this one serial clock cycle. */
#define SPI0CON_CON_EN                 (0x1   << 11 ) /* EN. Enable continuous transfer. In master mode, the transfer continues until no valid data is available in the Tx register. CS is asserted and remains asserted for the duration of each 8-bit serial transfer until Tx is empty. */

/* SPI0CON[LOOPBACK] - Loopback enable bit. */
#define SPI0CON_LOOPBACK_BBA           (*(volatile unsigned long *) 0x42080228)
#define SPI0CON_LOOPBACK_MSK           (0x1   << 10 )
#define SPI0CON_LOOPBACK               (0x1   << 10 )
#define SPI0CON_LOOPBACK_DIS           (0x0   << 10 ) /* DIS. Normal mode.        */
#define SPI0CON_LOOPBACK_EN            (0x1   << 10 ) /* EN. Connect MISO to MOSI, thus, data transmitted from Tx register is looped back to the Rx register. SPI must be configured in master mode. */

/* SPI0CON[SOEN] - Slave output enable bit. */
#define SPI0CON_SOEN_BBA               (*(volatile unsigned long *) 0x42080224)
#define SPI0CON_SOEN_MSK               (0x1   << 9  )
#define SPI0CON_SOEN                   (0x1   << 9  )
#define SPI0CON_SOEN_DIS               (0x0   << 9  ) /* DIS. Disable the output driver on the MISO pin. The MISO pin is open-circuit when this bit is clear. */
#define SPI0CON_SOEN_EN                (0x1   << 9  ) /* EN. MISO operates as normal. */

/* SPI0CON[RXOF] - RX overflow overwrite enable bit. */
#define SPI0CON_RXOF_BBA               (*(volatile unsigned long *) 0x42080220)
#define SPI0CON_RXOF_MSK               (0x1   << 8  )
#define SPI0CON_RXOF                   (0x1   << 8  )
#define SPI0CON_RXOF_DIS               (0x0   << 8  ) /* DIS. The new serial byte received is discarded when there is no space left in the FIFO */
#define SPI0CON_RXOF_EN                (0x1   << 8  ) /* EN. The valid data in the Rx register is overwritten by the new serial byte received when there is no space left in the FIFO. */

/* SPI0CON[ZEN] - Transmit underrun: Transmit 0s when the Tx FIFO is empty */
#define SPI0CON_ZEN_BBA                (*(volatile unsigned long *) 0x4208021C)
#define SPI0CON_ZEN_MSK                (0x1   << 7  )
#define SPI0CON_ZEN                    (0x1   << 7  )
#define SPI0CON_ZEN_DIS                (0x0   << 7  ) /* DIS. The last byte from the previous transmission is shifted out when a transfer is initiated with no valid data in the FIFO. */
#define SPI0CON_ZEN_EN                 (0x1   << 7  ) /* EN. Transmit 0x00 when a transfer is initiated with no valid data in the FIFO. */

/* SPI0CON[TIM] - Transfer and interrupt mode bit. */
#define SPI0CON_TIM_BBA                (*(volatile unsigned long *) 0x42080218)
#define SPI0CON_TIM_MSK                (0x1   << 6  )
#define SPI0CON_TIM                    (0x1   << 6  )
#define SPI0CON_TIM_TXWR               (0x1   << 6  ) /* TXWR. Initiate transfer with a write to the SPIxTX register.  Interrupt only occurs when Tx is empty. */
#define SPI0CON_TIM_RXRD               (0x0   << 6  ) /* RXRD. Initiate transfer with a read of the SPIxRX register.  The read must be done while the SPI interface is idle. Interrupt only occurs when Rx is full. */

/* SPI0CON[LSB] - LSB first transfer enable bit. */
#define SPI0CON_LSB_BBA                (*(volatile unsigned long *) 0x42080214)
#define SPI0CON_LSB_MSK                (0x1   << 5  )
#define SPI0CON_LSB                    (0x1   << 5  )
#define SPI0CON_LSB_DIS                (0x0   << 5  ) /* DIS. MSB is transmitted first. */
#define SPI0CON_LSB_EN                 (0x1   << 5  ) /* EN. LSB is transmitted first. */

/* SPI0CON[WOM] - Wired OR enable bit. */
#define SPI0CON_WOM_BBA                (*(volatile unsigned long *) 0x42080210)
#define SPI0CON_WOM_MSK                (0x1   << 4  )
#define SPI0CON_WOM                    (0x1   << 4  )
#define SPI0CON_WOM_DIS                (0x0   << 4  ) /* DIS. Normal driver output operation. */
#define SPI0CON_WOM_EN                 (0x1   << 4  ) /* EN. Enable open circuit data output for multimaster/multislave configuration. */

/* SPI0CON[CPOL] - Serial clock polarity mode bit. */
#define SPI0CON_CPOL_BBA               (*(volatile unsigned long *) 0x4208020C)
#define SPI0CON_CPOL_MSK               (0x1   << 3  )
#define SPI0CON_CPOL                   (0x1   << 3  )
#define SPI0CON_CPOL_LOW               (0x0   << 3  ) /* LOW. Serial clock idles low. */
#define SPI0CON_CPOL_HIGH              (0x1   << 3  ) /* HIGH. Serial clock idles high. */

/* SPI0CON[CPHA] - Serial clock phase mode bit. */
#define SPI0CON_CPHA_BBA               (*(volatile unsigned long *) 0x42080208)
#define SPI0CON_CPHA_MSK               (0x1   << 2  )
#define SPI0CON_CPHA                   (0x1   << 2  )
#define SPI0CON_CPHA_SAMPLELEADING     (0x0   << 2  ) /* SAMPLELEADING. Serial clock pulses at the middle of the first data bit transfer. */
#define SPI0CON_CPHA_SAMPLETRAILING    (0x1   << 2  ) /* SAMPLETRAILING. Serial clock pulses at the start of the first data bit. */

/* SPI0CON[MASEN] - Master mode enable bit. */
#define SPI0CON_MASEN_BBA              (*(volatile unsigned long *) 0x42080204)
#define SPI0CON_MASEN_MSK              (0x1   << 1  )
#define SPI0CON_MASEN                  (0x1   << 1  )
#define SPI0CON_MASEN_DIS              (0x0   << 1  ) /* DIS. Configure in slave mode. */
#define SPI0CON_MASEN_EN               (0x1   << 1  ) /* EN. Configure in master mode. */

/* SPI0CON[ENABLE] - SPI enable bit. */
#define SPI0CON_ENABLE_BBA             (*(volatile unsigned long *) 0x42080200)
#define SPI0CON_ENABLE_MSK             (0x1   << 0  )
#define SPI0CON_ENABLE                 (0x1   << 0  )
#define SPI0CON_ENABLE_DIS             (0x0   << 0  ) /* DIS. Disable the SPI. Clearing this bit will also reset all the FIFO related logic to enable a clean start. */
#define SPI0CON_ENABLE_EN              (0x1   << 0  ) /* EN. Enable the SPI.      */

/* Reset Value for SPI0DMA*/
#define SPI0DMA_RVAL                   0x0

/* SPI0DMA[IENRXDMA] - Receive DMA request enable bit. */
#define SPI0DMA_IENRXDMA_BBA           (*(volatile unsigned long *) 0x42080288)
#define SPI0DMA_IENRXDMA_MSK           (0x1   << 2  )
#define SPI0DMA_IENRXDMA               (0x1   << 2  )
#define SPI0DMA_IENRXDMA_DIS           (0x0   << 2  ) /* DIS. Disable Rx DMA requests. */
#define SPI0DMA_IENRXDMA_EN            (0x1   << 2  ) /* EN. Enable Rx DMA requests. */

/* SPI0DMA[IENTXDMA] - Transmit DMA request enable bit. */
#define SPI0DMA_IENTXDMA_BBA           (*(volatile unsigned long *) 0x42080284)
#define SPI0DMA_IENTXDMA_MSK           (0x1   << 1  )
#define SPI0DMA_IENTXDMA               (0x1   << 1  )
#define SPI0DMA_IENTXDMA_DIS           (0x0   << 1  ) /* DIS. Disable Tx DMA requests. */
#define SPI0DMA_IENTXDMA_EN            (0x1   << 1  ) /* EN. Enable Tx DMA requests. */

/* SPI0DMA[ENABLE] - DMA data transfer enable bit. */
#define SPI0DMA_ENABLE_BBA             (*(volatile unsigned long *) 0x42080280)
#define SPI0DMA_ENABLE_MSK             (0x1   << 0  )
#define SPI0DMA_ENABLE                 (0x1   << 0  )
#define SPI0DMA_ENABLE_DIS             (0x0   << 0  ) /* DIS. Disable DMA transfer. This bit needs to be cleared to prevent extra DMA request to the µDMA controller. */
#define SPI0DMA_ENABLE_EN              (0x1   << 0  ) /* EN. Enable a DMA transfer. Starts the transfer of a master configured to initiate transfer on transmit. */

/* Reset Value for SPI0CNT*/
#define SPI0CNT_RVAL                   0x0

/* SPI0CNT[VALUE] - Number of bytes requested by the SPI master during DMA transfer, when configured to initiate a transfer on a read of SPI0RX. This register is only used in DMA, master, Rx mode. */
#define SPI0CNT_VALUE_MSK              (0xFF  << 0  )
#if (__NO_MMR_STRUCTS__==1)

#define          SPI1STA                                    (*(volatile unsigned short int *) 0x40004400)
#define          SPI1RX                                     (*(volatile unsigned char      *) 0x40004404)
#define          SPI1TX                                     (*(volatile unsigned char      *) 0x40004408)
#define          SPI1DIV                                    (*(volatile unsigned short int *) 0x4000440C)
#define          SPI1CON                                    (*(volatile unsigned short int *) 0x40004410)
#define          SPI1DMA                                    (*(volatile unsigned short int *) 0x40004414)
#define          SPI1CNT                                    (*(volatile unsigned short int *) 0x40004418)
#endif // (__NO_MMR_STRUCTS__==1)

/* Reset Value for SPI1STA*/
#define SPI1STA_RVAL                   0x0

/* SPI1STA[CSERR] - CS error status bit. This bit generates an interrupt when detecting an abrupt CS desassertion before the full byte of data is transmitted completely. */
#define SPI1STA_CSERR_BBA              (*(volatile unsigned long *) 0x42088030)
#define SPI1STA_CSERR_MSK              (0x1   << 12 )
#define SPI1STA_CSERR                  (0x1   << 12 )
#define SPI1STA_CSERR_CLR              (0x0   << 12 ) /* CLR: Cleared when no CS error is detected. Cleared to 0 on a read of SPI1STA register. */
#define SPI1STA_CSERR_SET              (0x1   << 12 ) /* SET. Set when the CS line is deasserted abruptly. */

/* SPI1STA[RXS] - Rx FIFO excess bytes present. */
#define SPI1STA_RXS_BBA                (*(volatile unsigned long *) 0x4208802C)
#define SPI1STA_RXS_MSK                (0x1   << 11 )
#define SPI1STA_RXS                    (0x1   << 11 )
#define SPI1STA_RXS_CLR                (0x0   << 11 ) /* CLR. When the number of bytes in the FIFO is equal or less than the number in SPI0CON[15:14]. This bit is not cleared on a read of SPI0STA register. */
#define SPI1STA_RXS_SET                (0x1   << 11 ) /* SET. When there are more bytes in the Rx FIFO than configured in MOD (SPI1CON[15:14]). For example if MOD = TX1RX1, RXS is set when there are 2 or more bytes in the Rx FIFO. This bit does not dependent on SPI1CON[6] and does not cause an interrupt. */

/* SPI1STA[RXFSTA] - Rx FIFO status bits, indicates how many valid bytes are in the Rx FIFO. */
#define SPI1STA_RXFSTA_MSK             (0x7   << 8  )
#define SPI1STA_RXFSTA_EMPTY           (0x0   << 8  ) /* EMPTY. When Rx FIFO is empty. */
#define SPI1STA_RXFSTA_ONEBYTE         (0x1   << 8  ) /* ONEBYTE. When 1 valid byte is in the FIFO. */
#define SPI1STA_RXFSTA_TWOBYTES        (0x2   << 8  ) /* TWOBYTES. When 2 valid bytes are in the FIFO. */
#define SPI1STA_RXFSTA_THREEBYTES      (0x3   << 8  ) /* THREEBYTES. When 3 valid bytes are in the FIFO. */
#define SPI1STA_RXFSTA_FOURBYTES       (0x4   << 8  ) /* FOURBYTES. When 4 valid bytes are in the FIFO. */

/* SPI1STA[RXOF] - Rx FIFO overflow status bit. This bit generates an interrupt. */
#define SPI1STA_RXOF_BBA               (*(volatile unsigned long *) 0x4208801C)
#define SPI1STA_RXOF_MSK               (0x1   << 7  )
#define SPI1STA_RXOF                   (0x1   << 7  )
#define SPI1STA_RXOF_CLR               (0x0   << 7  ) /* CLR.Cleared to 0 on a read of SPI1STA register. */
#define SPI1STA_RXOF_SET               (0x1   << 7  ) /* SET. Set when the Rx FIFO is already full when new data is loaded to the FIFO. This bit generates an interrupt except when RFLUSH is set. (SPI1CON[12]). */

/* SPI1STA[RX] - Rx interrupt status bit. This bit generates an interrupt, except when DMA transfer is enabled. */
#define SPI1STA_RX_BBA                 (*(volatile unsigned long *) 0x42088018)
#define SPI1STA_RX_MSK                 (0x1   << 6  )
#define SPI1STA_RX                     (0x1   << 6  )
#define SPI1STA_RX_CLR                 (0x0   << 6  ) /* CLR. Cleared to 0 on a read of SPI1STA register. */
#define SPI1STA_RX_SET                 (0x1   << 6  ) /* SET. Set when a receive interrupt occurs. This bit is set when TIM (SPI1CON[6]) is cleared and the required number of bytes have been received. */

/* SPI1STA[TX] - Tx interrupt status bit. This bit generates an interrupt, except when DMA transfer is enabled. */
#define SPI1STA_TX_BBA                 (*(volatile unsigned long *) 0x42088014)
#define SPI1STA_TX_MSK                 (0x1   << 5  )
#define SPI1STA_TX                     (0x1   << 5  )
#define SPI1STA_TX_CLR                 (0x0   << 5  ) /* CLR. Cleared to 0 on a read of SPI1STA register. */
#define SPI1STA_TX_SET                 (0x1   << 5  ) /* SET. Set when a transmit interrupt occurs. This bit is set when TIM (SPI1CON[6]) is set and the required number of bytes have been transmitted. */

/* SPI1STA[TXUR] - Tx FIFO Underrun. This bit generates an interrupt. */
#define SPI1STA_TXUR_BBA               (*(volatile unsigned long *) 0x42088010)
#define SPI1STA_TXUR_MSK               (0x1   << 4  )
#define SPI1STA_TXUR                   (0x1   << 4  )
#define SPI1STA_TXUR_CLR               (0x0   << 4  ) /* CLR. Cleared to 0 on a read of SPI1STA register. */
#define SPI1STA_TXUR_SET               (0x1   << 4  ) /* SET. Set when a transmit is initiated without any valid data in the Tx FIFO. This bit generates an interrupt except when TFLUSH is set in SPI1CON. */

/* SPI1STA[TXFSTA] - Tx FIFO status bits, indicates how many valid bytes are in the Tx FIFO. */
#define SPI1STA_TXFSTA_MSK             (0x7   << 1  )
#define SPI1STA_TXFSTA_EMPTY           (0x0   << 1  ) /* EMPTY. When Tx FIFO is empty. */
#define SPI1STA_TXFSTA_ONEBYTE         (0x1   << 1  ) /* ONEBYTE. 1 valid byte is in the FIFO. */
#define SPI1STA_TXFSTA_TWOBYTES        (0x2   << 1  ) /* TWOBYTES. 2 valid bytes are in the FIFO. */
#define SPI1STA_TXFSTA_THREEBYTES      (0x3   << 1  ) /* THREEBYTES. 3 valid bytes are in the FIFO. */
#define SPI1STA_TXFSTA_FOURBYTES       (0x4   << 1  ) /* FOURBYTES. 4 valid bytes are in the FIFO. */

/* SPI1STA[IRQ] - Interrupt status bit. */
#define SPI1STA_IRQ_BBA                (*(volatile unsigned long *) 0x42088000)
#define SPI1STA_IRQ_MSK                (0x1   << 0  )
#define SPI1STA_IRQ                    (0x1   << 0  )
#define SPI1STA_IRQ_CLR                (0x0   << 0  ) /* CLR. Cleared to 0 on a read of SPI1STA register. */
#define SPI1STA_IRQ_SET                (0x1   << 0  ) /* SET. Set to 1 when an SPI1 based interrupt occurs. */

/* Reset Value for SPI1RX*/
#define SPI1RX_RVAL                    0x0

/* SPI1RX[VALUE] - 8-bit receive register. A read of the RX FIFO returns the next byte to be read from the FIFO. A read of the FIFO when it is empty returns zero. */
#define SPI1RX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for SPI1TX*/
#define SPI1TX_RVAL                    0x0

/* SPI1TX[VALUE] - 8-bit transmit register. A write to the Tx FIFO address space writes data to the next available location in the Tx FIFO. If the FIFO is full, the oldest byte of data in the FIFO is overwritten. A read from this address location return zero. */
#define SPI1TX_VALUE_MSK               (0xFF  << 0  )

/* Reset Value for SPI1DIV*/
#define SPI1DIV_RVAL                   0x0

/* SPI1DIV[BCRST] - Configures the behavior of SPI communication after an abrupt deassertion of CS.  This bit should be set in slave and master mode. */
#define SPI1DIV_BCRST_BBA              (*(volatile unsigned long *) 0x4208819C)
#define SPI1DIV_BCRST_MSK              (0x1   << 7  )
#define SPI1DIV_BCRST                  (0x1   << 7  )
#define SPI1DIV_BCRST_DIS              (0x0   << 7  ) /* DIS. Resumes communication from where it stopped when the CS is deasserted. The rest of the bits are then received/ transmitted when CS returns low. User code should ignore the CSERR interrupt. */
#define SPI1DIV_BCRST_EN               (0x1   << 7  ) /* EN. Enabled for a clean restart of SPI transfer after a CSERR condition. User code must also clear the SPI enable bit in SPI1CON during the CSERR interrupt. */

/* SPI1DIV[DIV] - Factor used to divide UCLK in the generation of the master mode serial clock. */
#define SPI1DIV_DIV_MSK                (0x3F  << 0  )

/* Reset Value for SPI1CON*/
#define SPI1CON_RVAL                   0x0

/* SPI1CON[MOD] - IRQ mode bits. When TIM is set these bits configure when the Tx/Rx interrupts occur in a transfer. For a DMA Rx transfer, these bits should be 00. */
#define SPI1CON_MOD_MSK                (0x3   << 14 )
#define SPI1CON_MOD_TX1RX1             (0x0   << 14 ) /* TX1RX1.  Tx/Rx interrupt occurs when 1 byte has been transmitted/received from/into the FIFO. */
#define SPI1CON_MOD_TX2RX2             (0x1   << 14 ) /* TX2RX2.  Tx/Rx interrupt occurs when 2 bytes have been transmitted/received from/into the FIFO. */
#define SPI1CON_MOD_TX3RX3             (0x2   << 14 ) /* TX3RX3.  Tx/Rx interrupt occurs when 3 bytes have been transmitted/received from/into the FIFO. */
#define SPI1CON_MOD_TX4RX4             (0x3   << 14 ) /* TX4RX4.  Tx/Rx interrupt occurs when 4 bytes have been transmitted/received from/into the FIFO. */

/* SPI1CON[TFLUSH] - Tx FIFO flush enable bit. */
#define SPI1CON_TFLUSH_BBA             (*(volatile unsigned long *) 0x42088234)
#define SPI1CON_TFLUSH_MSK             (0x1   << 13 )
#define SPI1CON_TFLUSH                 (0x1   << 13 )
#define SPI1CON_TFLUSH_DIS             (0x0   << 13 ) /* DIS. Disable Tx FIFO flushing. */
#define SPI1CON_TFLUSH_EN              (0x1   << 13 ) /* EN. Flush the Tx FIFO. This bit does not clear itself and should be toggled if a single flush is required. If this bit is left high, then either the last transmitted value or 0x00 is transmitted depending on the ZEN bit (SPI1CON[7]). Any writes to the Tx FIFO are ignored while this bit is set. */

/* SPI1CON[RFLUSH] - Rx FIFO flush enable bit. */
#define SPI1CON_RFLUSH_BBA             (*(volatile unsigned long *) 0x42088230)
#define SPI1CON_RFLUSH_MSK             (0x1   << 12 )
#define SPI1CON_RFLUSH                 (0x1   << 12 )
#define SPI1CON_RFLUSH_DIS             (0x0   << 12 ) /* DIS. Disable Rx FIFO flushing. */
#define SPI1CON_RFLUSH_EN              (0x1   << 12 ) /* EN.  If this bit is set, all incoming data is ignored and no interrupts are generated. If set and TIM = 0  (SPI1CON[6]), a read of the Rx FIFO initiates a transfer. */

/* SPI1CON[CON] - Continuous transfer enable bit. */
#define SPI1CON_CON_BBA                (*(volatile unsigned long *) 0x4208822C)
#define SPI1CON_CON_MSK                (0x1   << 11 )
#define SPI1CON_CON                    (0x1   << 11 )
#define SPI1CON_CON_DIS                (0x0   << 11 ) /* DIS. Disable continuous transfer. Each transfer consists of a single 8-bit serial transfer. If valid data exists in the SPIxTX register, then a new transfer is initiated after a stall period of one serial clock cycle. The CS line is deactivated for this one serial clock cycle. */
#define SPI1CON_CON_EN                 (0x1   << 11 ) /* EN. Enable continuous transfer. In master mode, the transfer continues until no valid data is available in the Tx register. CS is asserted and remains asserted for the duration of each 8-bit serial transfer until Tx is empty. */

/* SPI1CON[LOOPBACK] - Loopback enable bit. */
#define SPI1CON_LOOPBACK_BBA           (*(volatile unsigned long *) 0x42088228)
#define SPI1CON_LOOPBACK_MSK           (0x1   << 10 )
#define SPI1CON_LOOPBACK               (0x1   << 10 )
#define SPI1CON_LOOPBACK_DIS           (0x0   << 10 ) /* DIS. Normal mode.        */
#define SPI1CON_LOOPBACK_EN            (0x1   << 10 ) /* EN. Connect MISO to MOSI, thus, data transmitted from Tx register is looped back to the Rx register. SPI must be configured in master mode. */

/* SPI1CON[SOEN] - Slave output enable bit. */
#define SPI1CON_SOEN_BBA               (*(volatile unsigned long *) 0x42088224)
#define SPI1CON_SOEN_MSK               (0x1   << 9  )
#define SPI1CON_SOEN                   (0x1   << 9  )
#define SPI1CON_SOEN_DIS               (0x0   << 9  ) /* DIS.  Disable the output driver on the MISO pin. The MISO pin is open-circuit when this bit is clear. */
#define SPI1CON_SOEN_EN                (0x1   << 9  ) /* EN. MISO operates as normal. */

/* SPI1CON[RXOF] - RX overflow overwrite enable bit. */
#define SPI1CON_RXOF_BBA               (*(volatile unsigned long *) 0x42088220)
#define SPI1CON_RXOF_MSK               (0x1   << 8  )
#define SPI1CON_RXOF                   (0x1   << 8  )
#define SPI1CON_RXOF_DIS               (0x0   << 8  ) /* DIS. The new serial byte received is discarded when there is no space left in the FIFO */
#define SPI1CON_RXOF_EN                (0x1   << 8  ) /* EN. The valid data in the Rx register is overwritten by the new serial byte received when there is no space left in the FIFO. */

/* SPI1CON[ZEN] - TX underrun: Transmit 0s when Tx FIFO is empty. */
#define SPI1CON_ZEN_BBA                (*(volatile unsigned long *) 0x4208821C)
#define SPI1CON_ZEN_MSK                (0x1   << 7  )
#define SPI1CON_ZEN                    (0x1   << 7  )
#define SPI1CON_ZEN_DIS                (0x0   << 7  ) /* DIS. The last byte from the previous transmission is shifted out when a transfer is initiated with no valid data in the FIFO. */
#define SPI1CON_ZEN_EN                 (0x1   << 7  ) /* EN. Transmit 0x00 when a transfer is initiated with no valid data in the FIFO. */

/* SPI1CON[TIM] - Transfer and interrupt mode bit. */
#define SPI1CON_TIM_BBA                (*(volatile unsigned long *) 0x42088218)
#define SPI1CON_TIM_MSK                (0x1   << 6  )
#define SPI1CON_TIM                    (0x1   << 6  )
#define SPI1CON_TIM_TXWR               (0x1   << 6  ) /* TXWR. Initiate transfer with a write to the SPIxTX register.  Interrupt only occurs when Tx is empty. */
#define SPI1CON_TIM_RXRD               (0x0   << 6  ) /* RXRD. Initiate transfer with a read of the SPIxRX register.  The read must be done while the SPI interface is idle. Interrupt only occurs when Rx is full. */

/* SPI1CON[LSB] - LSB first transfer enable bit. */
#define SPI1CON_LSB_BBA                (*(volatile unsigned long *) 0x42088214)
#define SPI1CON_LSB_MSK                (0x1   << 5  )
#define SPI1CON_LSB                    (0x1   << 5  )
#define SPI1CON_LSB_DIS                (0x0   << 5  ) /* DIS. MSB is transmitted first. */
#define SPI1CON_LSB_EN                 (0x1   << 5  ) /* EN. LSB is transmitted first. */

/* SPI1CON[WOM] - Wired OR enable bit. */
#define SPI1CON_WOM_BBA                (*(volatile unsigned long *) 0x42088210)
#define SPI1CON_WOM_MSK                (0x1   << 4  )
#define SPI1CON_WOM                    (0x1   << 4  )
#define SPI1CON_WOM_DIS                (0x0   << 4  ) /* DIS. Normal driver output operation. */
#define SPI1CON_WOM_EN                 (0x1   << 4  ) /* EN. Enable open circuit data output for multimaster/multislave configuration. */

/* SPI1CON[CPOL] - Serial clock polarity mode bit. */
#define SPI1CON_CPOL_BBA               (*(volatile unsigned long *) 0x4208820C)
#define SPI1CON_CPOL_MSK               (0x1   << 3  )
#define SPI1CON_CPOL                   (0x1   << 3  )
#define SPI1CON_CPOL_LOW               (0x0   << 3  ) /* LOW. Serial clock idles low. */
#define SPI1CON_CPOL_HIGH              (0x1   << 3  ) /* HIGH. Serial clock idles high. */

/* SPI1CON[CPHA] - Serial clock phase mode bit. */
#define SPI1CON_CPHA_BBA               (*(volatile unsigned long *) 0x42088208)
#define SPI1CON_CPHA_MSK               (0x1   << 2  )
#define SPI1CON_CPHA                   (0x1   << 2  )
#define SPI1CON_CPHA_SAMPLELEADING     (0x0   << 2  ) /* SAMPLELEADING. Serial clock pulses at the middle of the first data bit transfer. */
#define SPI1CON_CPHA_SAMPLETRAILING    (0x1   << 2  ) /* SAMPLETRAILING. Serial clock pulses at the start of the first data bit. */

/* SPI1CON[MASEN] - Master mode enable bit. */
#define SPI1CON_MASEN_BBA              (*(volatile unsigned long *) 0x42088204)
#define SPI1CON_MASEN_MSK              (0x1   << 1  )
#define SPI1CON_MASEN                  (0x1   << 1  )
#define SPI1CON_MASEN_DIS              (0x0   << 1  ) /* DIS. Configure in slave mode. */
#define SPI1CON_MASEN_EN               (0x1   << 1  ) /* EN. Configure in master mode. */

/* SPI1CON[ENABLE] - SPI enable bit. */
#define SPI1CON_ENABLE_BBA             (*(volatile unsigned long *) 0x42088200)
#define SPI1CON_ENABLE_MSK             (0x1   << 0  )
#define SPI1CON_ENABLE                 (0x1   << 0  )
#define SPI1CON_ENABLE_DIS             (0x0   << 0  ) /* DIS. Disable the SPI. Clearing this bit will also reset all the FIFO related logic to enable a clean start. */
#define SPI1CON_ENABLE_EN              (0x1   << 0  ) /* EN. Enable the SPI.      */

/* Reset Value for SPI1DMA*/
#define SPI1DMA_RVAL                   0x0

/* SPI1DMA[IENRXDMA] - Receive DMA request enable bit. */
#define SPI1DMA_IENRXDMA_BBA           (*(volatile unsigned long *) 0x42088288)
#define SPI1DMA_IENRXDMA_MSK           (0x1   << 2  )
#define SPI1DMA_IENRXDMA               (0x1   << 2  )
#define SPI1DMA_IENRXDMA_DIS           (0x0   << 2  ) /* DIS. Disable Rx DMA requests. */
#define SPI1DMA_IENRXDMA_EN            (0x1   << 2  ) /* EN. Enable Rx DMA requests. */

/* SPI1DMA[IENTXDMA] - Transmit DMA request enable bit. */
#define SPI1DMA_IENTXDMA_BBA           (*(volatile unsigned long *) 0x42088284)
#define SPI1DMA_IENTXDMA_MSK           (0x1   << 1  )
#define SPI1DMA_IENTXDMA               (0x1   << 1  )
#define SPI1DMA_IENTXDMA_DIS           (0x0   << 1  ) /* DIS. Disable Tx DMA requests. */
#define SPI1DMA_IENTXDMA_EN            (0x1   << 1  ) /* EN. Enable Tx DMA requests. */

/* SPI1DMA[ENABLE] - DMA data transfer enable bit. */
#define SPI1DMA_ENABLE_BBA             (*(volatile unsigned long *) 0x42088280)
#define SPI1DMA_ENABLE_MSK             (0x1   << 0  )
#define SPI1DMA_ENABLE                 (0x1   << 0  )
#define SPI1DMA_ENABLE_DIS             (0x0   << 0  ) /* DIS. Disable DMA transfer. This bit needs to be cleared to prevent extra DMA request to the µDMA controller. */
#define SPI1DMA_ENABLE_EN              (0x1   << 0  ) /* EN. Enable a DMA transfer. Starts the transfer of a master configured to initiate transfer on transmit. */

/* Reset Value for SPI1CNT*/
#define SPI1CNT_RVAL                   0x0

/* SPI1CNT[VALUE] - Number of bytes requested by the SPI master during DMA transfer, when configured to initiate a transfer on a read of SPI0RX. This register is only used in DMA, master, Rx mode.. */
#define SPI1CNT_VALUE_MSK              (0xFF  << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        T0                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Timer 0 (pADI_TM0)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_TM0 Structure                     */
  __IO uint16_t  LD;                        /*!< 16-bit Load Value                     */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  VAL;                       /*!< 16-bit Timer Value                    */
  __I  uint16_t  RESERVED1;
  __IO uint16_t  CON;                       /*!< Control Register                      */
  __I  uint16_t  RESERVED2;
  __IO uint16_t  CLRI;                      /*!< Clear Interrupt Register              */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  CAP;                       /*!< Capture Register                      */
  __I  uint16_t  RESERVED4[5];
  __IO uint16_t  STA;                       /*!< Status Register                       */
} ADI_TIMER_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          T0LD                                       (*(volatile unsigned short int *) 0x40000000)
#define          T0VAL                                      (*(volatile unsigned short int *) 0x40000004)
#define          T0CON                                      (*(volatile unsigned short int *) 0x40000008)
#define          T0CLRI                                     (*(volatile unsigned short int *) 0x4000000C)
#define          T0CAP                                      (*(volatile unsigned short int *) 0x40000010)
#define          T0STA                                      (*(volatile unsigned short int *) 0x4000001C)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for T0LD*/
#define T0LD_RVAL                      0x0

/* T0LD[VALUE] - Load value. */
#define T0LD_VALUE_MSK                 (0xFFFF << 0  )

/* Reset Value for T0VAL*/
#define T0VAL_RVAL                     0x0

/* T0VAL[VALUE] - Current counter value. */
#define T0VAL_VALUE_MSK                (0xFFFF << 0  )

/* Reset Value for T0CON*/
#define T0CON_RVAL                     0xA

/* T0CON[EVENTEN] - Enable event bit. */
#define T0CON_EVENTEN_BBA              (*(volatile unsigned long *) 0x42000130)
#define T0CON_EVENTEN_MSK              (0x1   << 12 )
#define T0CON_EVENTEN                  (0x1   << 12 )
#define T0CON_EVENTEN_DIS              (0x0   << 12 ) /* DIS. Cleared by user.    */
#define T0CON_EVENTEN_EN               (0x1   << 12 ) /* EN. Enable time capture of an event. */

/* T0CON[EVENT] - Event select bits. Settings not described are reserved and should not be used. */
#define T0CON_EVENT_MSK                (0xF   << 8  )
#define T0CON_EVENT_T2                 (0x0   << 8  ) /* T2. Wakeup Timer.        */
#define T0CON_EVENT_EXT0               (0x1   << 8  ) /* EXT0. External interrupt 0. */
#define T0CON_EVENT_EXT1               (0x2   << 8  ) /* EXT1. External interrupt 1. */
#define T0CON_EVENT_EXT2               (0x3   << 8  ) /* EXT2. External interrupt 2. */
#define T0CON_EVENT_EXT3               (0x4   << 8  ) /* EXT3. External interrupt 3. */
#define T0CON_EVENT_EXT4               (0x5   << 8  ) /* EXT4. External interrupt 4. */
#define T0CON_EVENT_EXT5               (0x6   << 8  ) /* EXT5. External interrupt 5. */
#define T0CON_EVENT_EXT6               (0x7   << 8  ) /* EXT6. External interrupt 6. */
#define T0CON_EVENT_EXT7               (0x8   << 8  ) /* EXT7. External interrupt 7. */
#define T0CON_EVENT_EXT8               (0x9   << 8  ) /* EXT8. External interrupt 8. */
#define T0CON_EVENT_T3                 (0xA   << 8  ) /* T3. Watchdog timer.      */
#define T0CON_EVENT_T1                 (0xC   << 8  ) /* T1. Timer 1.             */
#define T0CON_EVENT_ADC                (0xD   << 8  ) /* ADC. Analog to Digital Converter. */
#define T0CON_EVENT_FEE                (0xE   << 8  ) /* FEE. Flash Controller.   */
#define T0CON_EVENT_COM                (0xF   << 8  ) /* COM. UART Peripheral.    */

/* T0CON[RLD] - Reload control bit for periodic mode. */
#define T0CON_RLD_BBA                  (*(volatile unsigned long *) 0x4200011C)
#define T0CON_RLD_MSK                  (0x1   << 7  )
#define T0CON_RLD                      (0x1   << 7  )
#define T0CON_RLD_DIS                  (0x0   << 7  ) /* DIS. Reload on a time out. */
#define T0CON_RLD_EN                   (0x1   << 7  ) /* EN. Reload the counter on a write to T0CLRI. */

/* T0CON[CLK] - Clock select. */
#define T0CON_CLK_MSK                  (0x3   << 5  )
#define T0CON_CLK_UCLK                 (0x0   << 5  ) /* UCLK. Undivided system clock. */
#define T0CON_CLK_PCLK                 (0x1   << 5  ) /* PCLK. Peripheral clock.  */
#define T0CON_CLK_LFOSC                (0x2   << 5  ) /* LFOSC. 32 kHz internal oscillator. */
#define T0CON_CLK_LFXTAL               (0x3   << 5  ) /* LFXTAL. 32 kHz external crystal. */

/* T0CON[ENABLE] - Timer enable bit. */
#define T0CON_ENABLE_BBA               (*(volatile unsigned long *) 0x42000110)
#define T0CON_ENABLE_MSK               (0x1   << 4  )
#define T0CON_ENABLE                   (0x1   << 4  )
#define T0CON_ENABLE_DIS               (0x0   << 4  ) /* DIS. Disable the timer. Clearing this bit resets the timer, including the T0VAL register. */
#define T0CON_ENABLE_EN                (0x1   << 4  ) /* EN. Enable the timer.  The timer starts counting from its initial value, 0 if count-up mode or 0xFFFF if count-down mode. */

/* T0CON[MOD] - Timer mode. */
#define T0CON_MOD_BBA                  (*(volatile unsigned long *) 0x4200010C)
#define T0CON_MOD_MSK                  (0x1   << 3  )
#define T0CON_MOD                      (0x1   << 3  )
#define T0CON_MOD_FREERUN              (0x0   << 3  ) /* FREERUN. Operate in free running mode. */
#define T0CON_MOD_PERIODIC             (0x1   << 3  ) /* PERIODIC. Operate in periodic mode. */

/* T0CON[UP] - Count down/up. */
#define T0CON_UP_BBA                   (*(volatile unsigned long *) 0x42000108)
#define T0CON_UP_MSK                   (0x1   << 2  )
#define T0CON_UP                       (0x1   << 2  )
#define T0CON_UP_DIS                   (0x0   << 2  ) /* DIS. Timer to count down. */
#define T0CON_UP_EN                    (0x1   << 2  ) /* EN. Timer to count up.   */

/* T0CON[PRE] - Prescaler. */
#define T0CON_PRE_MSK                  (0x3   << 0  )
#define T0CON_PRE_DIV1                 (0x0   << 0  ) /* DIV1. Source clock/1.If the selected clock source is UCLK or PCLK this setting results in a prescaler of 4. */
#define T0CON_PRE_DIV16                (0x1   << 0  ) /* DIV16. Source clock/16.  */
#define T0CON_PRE_DIV256               (0x2   << 0  ) /* DIV256. Source clock/256. */
#define T0CON_PRE_DIV32768             (0x3   << 0  ) /* DIV32768. Source clock/32768. */

/* Reset Value for T0CLRI*/
#define T0CLRI_RVAL                    0x0

/* T0CLRI[CAP] - Clear captured event interrupt. */
#define T0CLRI_CAP_BBA                 (*(volatile unsigned long *) 0x42000184)
#define T0CLRI_CAP_MSK                 (0x1   << 1  )
#define T0CLRI_CAP                     (0x1   << 1  )
#define T0CLRI_CAP_CLR                 (0x1   << 1  ) /* CLR. Clear a captured event interrupt. This bit always reads 0. */

/* T0CLRI[TMOUT] - Clear timeout interrupt. */
#define T0CLRI_TMOUT_BBA               (*(volatile unsigned long *) 0x42000180)
#define T0CLRI_TMOUT_MSK               (0x1   << 0  )
#define T0CLRI_TMOUT                   (0x1   << 0  )
#define T0CLRI_TMOUT_CLR               (0x1   << 0  ) /* CLR. Clear a timeout interrupt. This bit always reads 0. */

/* Reset Value for T0CAP*/
#define T0CAP_RVAL                     0x0

/* T0CAP[VALUE] - Capture value. */
#define T0CAP_VALUE_MSK                (0xFFFF << 0  )

/* Reset Value for T0STA*/
#define T0STA_RVAL                     0x0

/* T0STA[CLRI] - T0CLRI write sync in progress.. */
#define T0STA_CLRI_BBA                 (*(volatile unsigned long *) 0x4200039C)
#define T0STA_CLRI_MSK                 (0x1   << 7  )
#define T0STA_CLRI                     (0x1   << 7  )
#define T0STA_CLRI_CLR                 (0x0   << 7  ) /* CLR. Cleared when the interrupt is cleared in the timer clock domain. */
#define T0STA_CLRI_SET                 (0x1   << 7  ) /* SET. Set automatically when the T0CLRI value is being updated in the timer clock domain, indicating that the timers configuration is not yet valid. */

/* T0STA[CON] - T0CON write sync in progress. */
#define T0STA_CON_BBA                  (*(volatile unsigned long *) 0x42000398)
#define T0STA_CON_MSK                  (0x1   << 6  )
#define T0STA_CON                      (0x1   << 6  )
#define T0STA_CON_CLR                  (0x0   << 6  ) /* CLR. Timer ready to receive commands to T0CON. The previous change of T0CON has been synchronized in the timer clock domain. */
#define T0STA_CON_SET                  (0x1   << 6  ) /* SET. Timer not ready to receive commands to T0CON. Previous change of the T0CON value has not been synchronized in the timer clock domain. */

/* T0STA[CAP] - Capture event pending. */
#define T0STA_CAP_BBA                  (*(volatile unsigned long *) 0x42000384)
#define T0STA_CAP_MSK                  (0x1   << 1  )
#define T0STA_CAP                      (0x1   << 1  )
#define T0STA_CAP_CLR                  (0x0   << 1  ) /* CLR. No capture event is pending. */
#define T0STA_CAP_SET                  (0x1   << 1  ) /* SET. Capture event is pending. */

/* T0STA[TMOUT] - Time out event occurred. */
#define T0STA_TMOUT_BBA                (*(volatile unsigned long *) 0x42000380)
#define T0STA_TMOUT_MSK                (0x1   << 0  )
#define T0STA_TMOUT                    (0x1   << 0  )
#define T0STA_TMOUT_CLR                (0x0   << 0  ) /* CLR. No timeout event has occurred. */
#define T0STA_TMOUT_SET                (0x1   << 0  ) /* SET. Timeout event has occurred.  For count-up mode, this is when the counter reaches full scale. For count-down mode, this is when the counter reaches 0. */
#if (__NO_MMR_STRUCTS__==1)

#define          T1LD                                       (*(volatile unsigned short int *) 0x40000400)
#define          T1VAL                                      (*(volatile unsigned short int *) 0x40000404)
#define          T1CON                                      (*(volatile unsigned short int *) 0x40000408)
#define          T1CLRI                                     (*(volatile unsigned short int *) 0x4000040C)
#define          T1CAP                                      (*(volatile unsigned short int *) 0x40000410)
#define          T1STA                                      (*(volatile unsigned short int *) 0x4000041C)
#endif // (__NO_MMR_STRUCTS__==1)

/* Reset Value for T1LD*/
#define T1LD_RVAL                      0x0

/* T1LD[VALUE] - Load value. */
#define T1LD_VALUE_MSK                 (0xFFFF << 0  )

/* Reset Value for T1VAL*/
#define T1VAL_RVAL                     0x0

/* T1VAL[VALUE] - Current counter value. */
#define T1VAL_VALUE_MSK                (0xFFFF << 0  )

/* Reset Value for T1CON*/
#define T1CON_RVAL                     0xA

/* T1CON[EVENTEN] - Enable event bit. */
#define T1CON_EVENTEN_BBA              (*(volatile unsigned long *) 0x42008130)
#define T1CON_EVENTEN_MSK              (0x1   << 12 )
#define T1CON_EVENTEN                  (0x1   << 12 )
#define T1CON_EVENTEN_DIS              (0x0   << 12 ) /* DIS. Cleared by user.    */
#define T1CON_EVENTEN_EN               (0x1   << 12 ) /* EN. Enable time capture of an event. */

/* T1CON[EVENT] - Event select bits. Settings not described are reserved and should not be used. */
#define T1CON_EVENT_MSK                (0xF   << 8  )
#define T1CON_EVENT_T0                 (0x0   << 8  ) /* T0. Timer 0.             */
#define T1CON_EVENT_SPI0               (0x1   << 8  ) /* SPI0. SPI0 Peripheral.   */
#define T1CON_EVENT_SPI1               (0x2   << 8  ) /* SPI1. SPI1 Peripheral.   */
#define T1CON_EVENT_I2CS               (0x3   << 8  ) /* I2CS. I2C slave peripheral. */
#define T1CON_EVENT_I2CM               (0x4   << 8  ) /* I2CM. I2C master peripheral. */
#define T1CON_EVENT_DMAERR             (0x6   << 8  ) /* DMAERR. DMA error        */
#define T1CON_EVENT_DMADONE            (0x7   << 8  ) /* DMADONE. Completion of transfer on any of the DMA channel. */
#define T1CON_EVENT_EXT1               (0x8   << 8  ) /* EXT1. External interrupt 1. */
#define T1CON_EVENT_EXT2               (0x9   << 8  ) /* EXT2. External interrupt 2. */
#define T1CON_EVENT_EXT3               (0xA   << 8  ) /* EXT3. External interrupt 3. */
#define T1CON_EVENT_PWMTRIP            (0xB   << 8  ) /* PWMTRIP.                 */
#define T1CON_EVENT_PWM0               (0xC   << 8  ) /* PWM0. PWM pair 0.        */
#define T1CON_EVENT_PWM1               (0xD   << 8  ) /* PWM1. PWM pair 1.        */
#define T1CON_EVENT_PWM2               (0xE   << 8  ) /* PWM2. PWM pair 2.        */
#define T1CON_EVENT_PWM3               (0xF   << 8  ) /* PWM3. PWM pair 3.        */

/* T1CON[RLD] - Reload control bit for periodic mode. */
#define T1CON_RLD_BBA                  (*(volatile unsigned long *) 0x4200811C)
#define T1CON_RLD_MSK                  (0x1   << 7  )
#define T1CON_RLD                      (0x1   << 7  )
#define T1CON_RLD_DIS                  (0x0   << 7  ) /* DIS. Reload on a time out. */
#define T1CON_RLD_EN                   (0x1   << 7  ) /* EN. Reload the counter on a write to T1CLRI. */

/* T1CON[CLK] - Clock select. */
#define T1CON_CLK_MSK                  (0x3   << 5  )
#define T1CON_CLK_UCLK                 (0x0   << 5  ) /* UCLK. Undivided system clock. */
#define T1CON_CLK_PCLK                 (0x1   << 5  ) /* PCLK. Peripheral clock.  */
#define T1CON_CLK_LFOSC                (0x2   << 5  ) /* LFOSC. 32 kHz internal oscillator. */
#define T1CON_CLK_LFXTAL               (0x3   << 5  ) /* LFXTAL. 32 kHz external crystal. */

/* T1CON[ENABLE] - Timer enable bit. */
#define T1CON_ENABLE_BBA               (*(volatile unsigned long *) 0x42008110)
#define T1CON_ENABLE_MSK               (0x1   << 4  )
#define T1CON_ENABLE                   (0x1   << 4  )
#define T1CON_ENABLE_DIS               (0x0   << 4  ) /* DIS. Disable the timer. Clearing this bit resets the timer, including the T1VAL register. */
#define T1CON_ENABLE_EN                (0x1   << 4  ) /* EN. Enable the timer.  The timer starts counting from its initial value, 0 if count-up mode or 0xFFFF if count-down mode. */

/* T1CON[MOD] - Timer mode. */
#define T1CON_MOD_BBA                  (*(volatile unsigned long *) 0x4200810C)
#define T1CON_MOD_MSK                  (0x1   << 3  )
#define T1CON_MOD                      (0x1   << 3  )
#define T1CON_MOD_FREERUN              (0x0   << 3  ) /* FREERUN. Operate in free running mode. */
#define T1CON_MOD_PERIODIC             (0x1   << 3  ) /* PERIODIC. Operate in periodic mode. */

/* T1CON[UP] - Count down/up. */
#define T1CON_UP_BBA                   (*(volatile unsigned long *) 0x42008108)
#define T1CON_UP_MSK                   (0x1   << 2  )
#define T1CON_UP                       (0x1   << 2  )
#define T1CON_UP_DIS                   (0x0   << 2  ) /* DIS. Timer to count down. */
#define T1CON_UP_EN                    (0x1   << 2  ) /* EN. Timer to count up.   */

/* T1CON[PRE] - Prescaler. */
#define T1CON_PRE_MSK                  (0x3   << 0  )
#define T1CON_PRE_DIV1                 (0x0   << 0  ) /* DIV1. Source clock/1.If the selected clock source is UCLK or PCLK this setting results in a prescaler of 4. */
#define T1CON_PRE_DIV16                (0x1   << 0  ) /* DIV16. Source clock/16.  */
#define T1CON_PRE_DIV256               (0x2   << 0  ) /* DIV256. Source clock/256. */
#define T1CON_PRE_DIV32768             (0x3   << 0  ) /* DIV32768. Source clock/32768. */

/* Reset Value for T1CLRI*/
#define T1CLRI_RVAL                    0x0

/* T1CLRI[CAP] - Clear captured event interrupt. */
#define T1CLRI_CAP_BBA                 (*(volatile unsigned long *) 0x42008184)
#define T1CLRI_CAP_MSK                 (0x1   << 1  )
#define T1CLRI_CAP                     (0x1   << 1  )
#define T1CLRI_CAP_CLR                 (0x1   << 1  ) /* CLR. Clear a captured event interrupt. This bit always reads 0. */

/* T1CLRI[TMOUT] - Clear timeout interrupt. */
#define T1CLRI_TMOUT_BBA               (*(volatile unsigned long *) 0x42008180)
#define T1CLRI_TMOUT_MSK               (0x1   << 0  )
#define T1CLRI_TMOUT                   (0x1   << 0  )
#define T1CLRI_TMOUT_CLR               (0x1   << 0  ) /* CLR. Clear a timeout interrupt. This bit always reads 0. */

/* Reset Value for T1CAP*/
#define T1CAP_RVAL                     0x0

/* T1CAP[VALUE] - Capture value. */
#define T1CAP_VALUE_MSK                (0xFFFF << 0  )

/* Reset Value for T1STA*/
#define T1STA_RVAL                     0x0

/* T1STA[CLRI] - T1CLRI write sync in progress. */
#define T1STA_CLRI_BBA                 (*(volatile unsigned long *) 0x4200839C)
#define T1STA_CLRI_MSK                 (0x1   << 7  )
#define T1STA_CLRI                     (0x1   << 7  )
#define T1STA_CLRI_CLR                 (0x0   << 7  ) /* CLR. Cleared when the interrupt is cleared in the timer clock domain. */
#define T1STA_CLRI_SET                 (0x1   << 7  ) /* SET. Set automatically when the T1CLRI value is being updated in the timer clock domain, indicating that the timers configuration is not yet valid. */

/* T1STA[CON] - T1CON write sync in progress. */
#define T1STA_CON_BBA                  (*(volatile unsigned long *) 0x42008398)
#define T1STA_CON_MSK                  (0x1   << 6  )
#define T1STA_CON                      (0x1   << 6  )
#define T1STA_CON_CLR                  (0x0   << 6  ) /* CLR. Timer ready to receive commands to T1CON. The previous change of T1CON has been synchronized in the timer clock domain. */
#define T1STA_CON_SET                  (0x1   << 6  ) /* SET. Timer not ready to receive commands to T1CON. Previous change of the T1CON value has not been synchronized in the timer clock domain. */

/* T1STA[CAP] - Capture event pending. */
#define T1STA_CAP_BBA                  (*(volatile unsigned long *) 0x42008384)
#define T1STA_CAP_MSK                  (0x1   << 1  )
#define T1STA_CAP                      (0x1   << 1  )
#define T1STA_CAP_CLR                  (0x0   << 1  ) /* CLR. No capture event is pending. */
#define T1STA_CAP_SET                  (0x1   << 1  ) /* SET. Capture event is pending. */

/* T1STA[TMOUT] - Time out event occurred. */
#define T1STA_TMOUT_BBA                (*(volatile unsigned long *) 0x42008380)
#define T1STA_TMOUT_MSK                (0x1   << 0  )
#define T1STA_TMOUT                    (0x1   << 0  )
#define T1STA_TMOUT_CLR                (0x0   << 0  ) /* CLR. No timeout event has occurred. */
#define T1STA_TMOUT_SET                (0x1   << 0  ) /* SET. Timeout event has occurred.  For count-up mode, this is when the counter reaches full scale. For count-down mode, this is when the counter reaches 0. */
// ------------------------------------------------------------------------------------------------
// -----                                        UART                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief UART (pADI_UART)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_UART Structure                    */

  union {
    __IO uint8_t   COMTX;                   /*!< Transmit Holding Register             */
    __IO uint8_t   COMRX;                   /*!< Receive Buffer Register               */
  } ;
  __I  uint8_t   RESERVED0[3];
  __IO uint8_t   COMIEN;                    /*!< Interrupt Enable Register             */
  __I  uint8_t   RESERVED1[3];
  __IO uint8_t   COMIIR;                    /*!< Interrupt Identification Register     */
  __I  uint8_t   RESERVED2[3];
  __IO uint8_t   COMLCR;                    /*!< Line Control Register                 */
  __I  uint8_t   RESERVED3[3];
  __IO uint8_t   COMMCR;                    /*!< Module Control Register               */
  __I  uint8_t   RESERVED4[3];
  __IO uint8_t   COMLSR;                    /*!< Line Status Register                  */
  __I  uint8_t   RESERVED5[3];
  __IO uint8_t   COMMSR;                    /*!< Modem Status Register                 */
  __I  uint8_t   RESERVED6[11];
  __IO uint16_t  COMFBR;                    /*!< Fractional Baud Rate Register.        */
  __I  uint16_t  RESERVED7;
  __IO uint16_t  COMDIV;                    /*!< Baud Rate Divider Register            */
} ADI_UART_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          COMTX                                      (*(volatile unsigned char      *) 0x40005000)
#define          COMRX                                      (*(volatile unsigned char      *) 0x40005000)
#define          COMIEN                                     (*(volatile unsigned char      *) 0x40005004)
#define          COMIIR                                     (*(volatile unsigned char      *) 0x40005008)
#define          COMLCR                                     (*(volatile unsigned char      *) 0x4000500C)
#define          COMMCR                                     (*(volatile unsigned char      *) 0x40005010)
#define          COMLSR                                     (*(volatile unsigned char      *) 0x40005014)
#define          COMMSR                                     (*(volatile unsigned char      *) 0x40005018)
#define          COMFBR                                     (*(volatile unsigned short int *) 0x40005024)
#define          COMDIV                                     (*(volatile unsigned short int *) 0x40005028)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for COMTX*/
#define COMTX_RVAL                     0x0

/* COMTX[VALUE] - Transmit Holding Register */
#define COMTX_VALUE_MSK                (0xFF  << 0  )

/* Reset Value for COMRX*/
#define COMRX_RVAL                     0x0

/* COMRX[VALUE] - Receive Buffer Register */
#define COMRX_VALUE_MSK                (0xFF  << 0  )

/* Reset Value for COMIEN*/
#define COMIEN_RVAL                    0x0

/* COMIEN[EDMAR] - DMA requests in transmit mode */
#define COMIEN_EDMAR_BBA               (*(volatile unsigned long *) 0x420A0094)
#define COMIEN_EDMAR_MSK               (0x1   << 5  )
#define COMIEN_EDMAR                   (0x1   << 5  )
#define COMIEN_EDMAR_DIS               (0x0   << 5  ) /* DIS. Disable.            */
#define COMIEN_EDMAR_EN                (0x1   << 5  ) /* EN. Enable.              */

/* COMIEN[EDMAT] - DMA requests in receive mode */
#define COMIEN_EDMAT_BBA               (*(volatile unsigned long *) 0x420A0090)
#define COMIEN_EDMAT_MSK               (0x1   << 4  )
#define COMIEN_EDMAT                   (0x1   << 4  )
#define COMIEN_EDMAT_DIS               (0x0   << 4  ) /* DIS. Disable.            */
#define COMIEN_EDMAT_EN                (0x1   << 4  ) /* EN. Enable.              */

/* COMIEN[EDSSI] - Modem status interrupt */
#define COMIEN_EDSSI_BBA               (*(volatile unsigned long *) 0x420A008C)
#define COMIEN_EDSSI_MSK               (0x1   << 3  )
#define COMIEN_EDSSI                   (0x1   << 3  )
#define COMIEN_EDSSI_DIS               (0x0   << 3  ) /* DIS. Disable.            */
#define COMIEN_EDSSI_EN                (0x1   << 3  ) /* EN. Enable.              */

/* COMIEN[ELSI] - Rx status interrupt */
#define COMIEN_ELSI_BBA                (*(volatile unsigned long *) 0x420A0088)
#define COMIEN_ELSI_MSK                (0x1   << 2  )
#define COMIEN_ELSI                    (0x1   << 2  )
#define COMIEN_ELSI_DIS                (0x0   << 2  ) /* DIS. Disable.            */
#define COMIEN_ELSI_EN                 (0x1   << 2  ) /* EN. Enable.              */

/* COMIEN[ETBEI] - Transmit buffer empty interrupt */
#define COMIEN_ETBEI_BBA               (*(volatile unsigned long *) 0x420A0084)
#define COMIEN_ETBEI_MSK               (0x1   << 1  )
#define COMIEN_ETBEI                   (0x1   << 1  )
#define COMIEN_ETBEI_DIS               (0x0   << 1  ) /* DIS. Disable.            */
#define COMIEN_ETBEI_EN                (0x1   << 1  ) /* EN. Enable the transmit interrupt. An interrupt is generated when the COMTX register is empty. Note that if the COMTX is already empty when enabling this bit, an interrupt is generated immediately. */

/* COMIEN[ERBFI] - Receive buffer full interrupt */
#define COMIEN_ERBFI_BBA               (*(volatile unsigned long *) 0x420A0080)
#define COMIEN_ERBFI_MSK               (0x1   << 0  )
#define COMIEN_ERBFI                   (0x1   << 0  )
#define COMIEN_ERBFI_DIS               (0x0   << 0  ) /* DIS. Disable.            */
#define COMIEN_ERBFI_EN                (0x1   << 0  ) /* EN. Enable the receive interrupt. An interrupt is generated when the COMRX register is loaded with the received data. Note that if the COMRX is already full when enabling this bit, an interrupt is generated immediately. */

/* Reset Value for COMIIR*/
#define COMIIR_RVAL                    0x1

/* COMIIR[STA] - Status bits. */
#define COMIIR_STA_MSK                 (0x3   << 1  )
#define COMIIR_STA_MODEMSTATUS         (0x0   << 1  ) /* MODEMSTATUS.  Modem status interrupt. */
#define COMIIR_STA_TXBUFEMPTY          (0x1   << 1  ) /* TXBUFEMPTY. Transmit buffer empty interrupt. */
#define COMIIR_STA_RXBUFFULL           (0x2   << 1  ) /* RXBUFFULL. Receive buffer full interrupt. Read COMRX register to clear. */
#define COMIIR_STA_RXLINESTATUS        (0x3   << 1  ) /* RXLINESTATUS. Receive line status interrupt. Read COMLSR register to clear. */

/* COMIIR[NINT] - Interrupt flag. */
#define COMIIR_NINT_BBA                (*(volatile unsigned long *) 0x420A0100)
#define COMIIR_NINT_MSK                (0x1   << 0  )
#define COMIIR_NINT                    (0x1   << 0  )
#define COMIIR_NINT_CLR                (0x0   << 0  ) /* CLR. Indicates any of the following: receive buffer full, transmit buffer empty, line status, or modem status interrupt occurred. */
#define COMIIR_NINT_SET                (0x1   << 0  ) /* SET. There is no interrupt (default). */

/* Reset Value for COMLCR*/
#define COMLCR_RVAL                    0x0

/* COMLCR[BRK] - Set Break. */
#define COMLCR_BRK_BBA                 (*(volatile unsigned long *) 0x420A0198)
#define COMLCR_BRK_MSK                 (0x1   << 6  )
#define COMLCR_BRK                     (0x1   << 6  )
#define COMLCR_BRK_DIS                 (0x0   << 6  ) /* DIS to operate in normal mode. */
#define COMLCR_BRK_EN                  (0x1   << 6  ) /* EN to force TxD to 0.    */

/* COMLCR[SP] - Stick Parity. */
#define COMLCR_SP_BBA                  (*(volatile unsigned long *) 0x420A0194)
#define COMLCR_SP_MSK                  (0x1   << 5  )
#define COMLCR_SP                      (0x1   << 5  )
#define COMLCR_SP_DIS                  (0x0   << 5  ) /* DIS. Parity is not forced based on EPS and PEN values. */
#define COMLCR_SP_EN                   (0x1   << 5  ) /* EN. Force parity to defined values based on EPS and PEN values. EPS = 1 and PEN = 1, parity forced to 1  EPS = 0 and PEN = 1, parity forced to 0 EPS = X and PEN = 0, no parity transmitted. */

/* COMLCR[EPS] - Even Parity Select Bit. */
#define COMLCR_EPS_BBA                 (*(volatile unsigned long *) 0x420A0190)
#define COMLCR_EPS_MSK                 (0x1   << 4  )
#define COMLCR_EPS                     (0x1   << 4  )
#define COMLCR_EPS_DIS                 (0x0   << 4  ) /* DIS. Odd parity.         */
#define COMLCR_EPS_EN                  (0x1   << 4  ) /* EN. Even parity.         */

/* COMLCR[PEN] - Parity Enable Bit. */
#define COMLCR_PEN_BBA                 (*(volatile unsigned long *) 0x420A018C)
#define COMLCR_PEN_MSK                 (0x1   << 3  )
#define COMLCR_PEN                     (0x1   << 3  )
#define COMLCR_PEN_DIS                 (0x0   << 3  ) /* DIS. No parity transmission or checking. */
#define COMLCR_PEN_EN                  (0x1   << 3  ) /* EN. Transmit and check the parity bit. */

/* COMLCR[STOP] - Stop Bit. */
#define COMLCR_STOP_BBA                (*(volatile unsigned long *) 0x420A0188)
#define COMLCR_STOP_MSK                (0x1   << 2  )
#define COMLCR_STOP                    (0x1   << 2  )
#define COMLCR_STOP_DIS                (0x0   << 2  ) /* DIS. Generate one stop bit in the transmitted data. */
#define COMLCR_STOP_EN                 (0x1   << 2  ) /* EN. Transmit 1.5 stop bits if the word length is 5 bits, or 2 stop bits if the word length is 6, 7, or 8 bits. The receiver checks the first stop bit only, regardless of the number of stop bits selected. */

/* COMLCR[WLS] - Word Length Select Bits. */
#define COMLCR_WLS_MSK                 (0x3   << 0  )
#define COMLCR_WLS_5BITS               (0x0   << 0  ) /* 5BITS. 5 bits.           */
#define COMLCR_WLS_6BITS               (0x1   << 0  ) /* 6BITS. 6 bits.           */
#define COMLCR_WLS_7BITS               (0x2   << 0  ) /* 7BITS. 7 bits.           */
#define COMLCR_WLS_8BITS               (0x3   << 0  ) /* 8BITS. 8 bits.           */

/* Reset Value for COMMCR*/
#define COMMCR_RVAL                    0x0

/* COMMCR[LOOPBACK] - Loop Back. */
#define COMMCR_LOOPBACK_BBA            (*(volatile unsigned long *) 0x420A0210)
#define COMMCR_LOOPBACK_MSK            (0x1   << 4  )
#define COMMCR_LOOPBACK                (0x1   << 4  )
#define COMMCR_LOOPBACK_DIS            (0x0   << 4  ) /* DIS. Normal mode.        */
#define COMMCR_LOOPBACK_EN             (0x1   << 4  ) /* EN. Enable loopback mode. */

/* COMMCR[RTS] - Request To Send output control bit. */
#define COMMCR_RTS_BBA                 (*(volatile unsigned long *) 0x420A0204)
#define COMMCR_RTS_MSK                 (0x1   << 1  )
#define COMMCR_RTS                     (0x1   << 1  )
#define COMMCR_RTS_DIS                 (0x0   << 1  ) /* DIS. Force the RTS output to 1. */
#define COMMCR_RTS_EN                  (0x1   << 1  ) /* EN. Force the RTS output to 0. */

/* Reset Value for COMLSR*/
#define COMLSR_RVAL                    0x60

/* COMLSR[TEMT] - COMTX and Shift Register Empty Status Bit. */
#define COMLSR_TEMT_BBA                (*(volatile unsigned long *) 0x420A0298)
#define COMLSR_TEMT_MSK                (0x1   << 6  )
#define COMLSR_TEMT                    (0x1   << 6  )
#define COMLSR_TEMT_CLR                (0x0   << 6  ) /* CLR. Cleared when writing to COMTX. */
#define COMLSR_TEMT_SET                (0x1   << 6  ) /* SET. If COMTX and the shift register are empty, this bit indicates that the data has been transmitted, that is, it is no longer present in the shift register (default). */

/* COMLSR[THRE] - COMTX Empty Status Bit. */
#define COMLSR_THRE_BBA                (*(volatile unsigned long *) 0x420A0294)
#define COMLSR_THRE_MSK                (0x1   << 5  )
#define COMLSR_THRE                    (0x1   << 5  )
#define COMLSR_THRE_CLR                (0x0   << 5  ) /* CLR. Cleared when writing to COMTX. */
#define COMLSR_THRE_SET                (0x1   << 5  ) /* SET. If COMTX is empty, COMTX can be written as soon as this bit is set. The previous data may not have been transmitted yet and can still be present in the shift register (default). */

/* COMLSR[BI] - Break Indicator. */
#define COMLSR_BI_BBA                  (*(volatile unsigned long *) 0x420A0290)
#define COMLSR_BI_MSK                  (0x1   << 4  )
#define COMLSR_BI                      (0x1   << 4  )
#define COMLSR_BI_CLR                  (0x0   << 4  ) /* CLR. Cleared automatically. */
#define COMLSR_BI_SET                  (0x1   << 4  ) /* SET. Set when UART RXD is held low for more than the maximum word length. */

/* COMLSR[FE] - Framing Error. */
#define COMLSR_FE_BBA                  (*(volatile unsigned long *) 0x420A028C)
#define COMLSR_FE_MSK                  (0x1   << 3  )
#define COMLSR_FE                      (0x1   << 3  )
#define COMLSR_FE_CLR                  (0x0   << 3  ) /* CLR. Cleared automatically. */
#define COMLSR_FE_SET                  (0x1   << 3  ) /* SET. Set when the stop bit is invalid. */

/* COMLSR[PE] - Parity Error. */
#define COMLSR_PE_BBA                  (*(volatile unsigned long *) 0x420A0288)
#define COMLSR_PE_MSK                  (0x1   << 2  )
#define COMLSR_PE                      (0x1   << 2  )
#define COMLSR_PE_CLR                  (0x0   << 2  ) /* CLR. Cleared automatically. */
#define COMLSR_PE_SET                  (0x1   << 2  ) /* SET. Set when a parity error occurs. */

/* COMLSR[OE] - Overrun Error. */
#define COMLSR_OE_BBA                  (*(volatile unsigned long *) 0x420A0284)
#define COMLSR_OE_MSK                  (0x1   << 1  )
#define COMLSR_OE                      (0x1   << 1  )
#define COMLSR_OE_CLR                  (0x0   << 1  ) /* CLR. Cleared automatically. */
#define COMLSR_OE_SET                  (0x1   << 1  ) /* SET. Set automatically if data is overwritten before being read. */

/* COMLSR[DR] - Data Ready. */
#define COMLSR_DR_BBA                  (*(volatile unsigned long *) 0x420A0280)
#define COMLSR_DR_MSK                  (0x1   << 0  )
#define COMLSR_DR                      (0x1   << 0  )
#define COMLSR_DR_CLR                  (0x0   << 0  ) /* CLR. Cleared by reading COMRX. */
#define COMLSR_DR_SET                  (0x1   << 0  ) /* SET. Set automatically when COMRX is full. */

/* Reset Value for COMMSR*/
#define COMMSR_RVAL                    0x0

/* COMMSR[CTS] - Clear To Send signal status bit. */
#define COMMSR_CTS_BBA                 (*(volatile unsigned long *) 0x420A0310)
#define COMMSR_CTS_MSK                 (0x1   << 4  )
#define COMMSR_CTS                     (0x1   << 4  )
#define COMMSR_CTS_CLR                 (0x0   << 4  ) /* CLR. Cleared to 0 when CTS input is logic high. */
#define COMMSR_CTS_SET                 (0x1   << 4  ) /* SET. Set to 1 when CTS input is logic low. */

/* COMMSR[DCTS] - Delta CTS */
#define COMMSR_DCTS_BBA                (*(volatile unsigned long *) 0x420A0300)
#define COMMSR_DCTS_MSK                (0x1   << 0  )
#define COMMSR_DCTS                    (0x1   << 0  )
#define COMMSR_DCTS_DIS                (0x0   << 0  ) /* DIS. Cleared automatically by reading COMMSR. */
#define COMMSR_DCTS_EN                 (0x1   << 0  ) /* EN. Set automatically if CTS changed state since COMMSR last read. */

/* Reset Value for COMFBR*/
#define COMFBR_RVAL                    0x0

/* COMFBR[ENABLE] - Fractional baud rate generator enable bit. Used for more accurate baud rate generation. */
#define COMFBR_ENABLE_BBA              (*(volatile unsigned long *) 0x420A04BC)
#define COMFBR_ENABLE_MSK              (0x1   << 15 )
#define COMFBR_ENABLE                  (0x1   << 15 )
#define COMFBR_ENABLE_DIS              (0x0   << 15 ) /* DIS. Disable.            */
#define COMFBR_ENABLE_EN               (0x1   << 15 ) /* EN. Enable.              */

/* COMFBR[DIVM] - Fractional baud rate M divide bits (1 to 3). These bits should not be set to 0. */
#define COMFBR_DIVM_MSK                (0x3   << 11 )

/* COMFBR[DIVN] - Fractional baud rate N divide bits (0 to 2047). */
#define COMFBR_DIVN_MSK                (0x7FF << 0  )

/* Reset Value for COMDIV*/
#define COMDIV_RVAL                    0x1

/* COMDIV[VALUE] - Sets the baud rate. The COMDIV register should not be 0. */
#define COMDIV_VALUE_MSK               (0xFFFF << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        WUT                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief WakeUp Timer (pADI_WUT)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_WUT Structure                     */
  __IO uint16_t  T2VAL0;                    /*!< Current Wake-Up Timer Value LSB       */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  T2VAL1;                    /*!< Current Wake-Up Timer Value MSB       */
  __I  uint16_t  RESERVED1;
  __IO uint16_t  T2CON;                     /*!< Control Register                      */
  __I  uint16_t  RESERVED2;
  __IO uint16_t  T2INC;                     /*!< 12-bit Interval Register for Wake-Up Field A */
  __I  uint16_t  RESERVED3;
  __IO uint16_t  T2WUFB0;                   /*!< Wake-Up Field B  LSB                  */
  __I  uint16_t  RESERVED4;
  __IO uint16_t  T2WUFB1;                   /*!< Wake-Up Field B  MSB                  */
  __I  uint16_t  RESERVED5;
  __IO uint16_t  T2WUFC0;                   /*!< Wake-Up Field C  LSB                  */
  __I  uint16_t  RESERVED6;
  __IO uint16_t  T2WUFC1;                   /*!< Wake-Up Field C  MSB                  */
  __I  uint16_t  RESERVED7;
  __IO uint16_t  T2WUFD0;                   /*!< Wake-UpField D  LSB                   */
  __I  uint16_t  RESERVED8;
  __IO uint16_t  T2WUFD1;                   /*!< Wake-Up Field D  MSB                  */
  __I  uint16_t  RESERVED9;
  __IO uint16_t  T2IEN;                     /*!< Interrupt Enable                      */
  __I  uint16_t  RESERVED10;
  __IO uint16_t  T2STA;                     /*!< Status                                */
  __I  uint16_t  RESERVED11;
  __IO uint16_t  T2CLRI;                    /*!< Clear Interrupts                      */
  __I  uint16_t  RESERVED12[5];
  __IO uint16_t  T2WUFA0;                   /*!< Wake-Up Field A  LSB                  */
  __I  uint16_t  RESERVED13;
  __IO uint16_t  T2WUFA1;                   /*!< Wake-Up Field A MSB                   */
} ADI_WUT_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          T2VAL0                                     (*(volatile unsigned short int *) 0x40002500)
#define          T2VAL1                                     (*(volatile unsigned short int *) 0x40002504)
#define          T2CON                                      (*(volatile unsigned short int *) 0x40002508)
#define          T2INC                                      (*(volatile unsigned short int *) 0x4000250C)
#define          T2WUFB0                                    (*(volatile unsigned short int *) 0x40002510)
#define          T2WUFB1                                    (*(volatile unsigned short int *) 0x40002514)
#define          T2WUFC0                                    (*(volatile unsigned short int *) 0x40002518)
#define          T2WUFC1                                    (*(volatile unsigned short int *) 0x4000251C)
#define          T2WUFD0                                    (*(volatile unsigned short int *) 0x40002520)
#define          T2WUFD1                                    (*(volatile unsigned short int *) 0x40002524)
#define          T2IEN                                      (*(volatile unsigned short int *) 0x40002528)
#define          T2STA                                      (*(volatile unsigned short int *) 0x4000252C)
#define          T2CLRI                                     (*(volatile unsigned short int *) 0x40002530)
#define          T2WUFA0                                    (*(volatile unsigned short int *) 0x4000253C)
#define          T2WUFA1                                    (*(volatile unsigned short int *) 0x40002540)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for T2VAL0*/
#define T2VAL0_RVAL                    0x0

/* T2VAL0[VALUE] - Current Wake-Up timer value (bits 15 to 0). */
#define T2VAL0_VALUE_MSK               (0xFFFF << 0  )

/* Reset Value for T2VAL1*/
#define T2VAL1_RVAL                    0x0

/* T2VAL1[VALUE] - Current Wake-Up timer value (bits 31 to 16). */
#define T2VAL1_VALUE_MSK               (0xFFFF << 0  )

/* Reset Value for T2CON*/
#define T2CON_RVAL                     0x40

/* T2CON[STOPINC] - Allows the user to update the interval register safely. */
#define T2CON_STOPINC_BBA              (*(volatile unsigned long *) 0x4204A12C)
#define T2CON_STOPINC_MSK              (0x1   << 11 )
#define T2CON_STOPINC                  (0x1   << 11 )
#define T2CON_STOPINC_DIS              (0x0   << 11 ) /* DIS. Allows the wake-up field A to be updated by hardware. */
#define T2CON_STOPINC_EN               (0x1   << 11 ) /* EN. Prevents wake-up field A being automatically updated by hardware.This allows user software to update the T2INC register value. */

/* T2CON[CLK] - Clock select. */
#define T2CON_CLK_MSK                  (0x3   << 9  )
#define T2CON_CLK_PCLK                 (0x0   << 9  ) /* PCLK. Peripheral clock.  */
#define T2CON_CLK_LFXTAL               (0x1   << 9  ) /* LFXTAL. 32 kHz external crystal. */
#define T2CON_CLK_LFOSC                (0x2   << 9  ) /* LFOSC. 32 kHz internal oscillator. */
#define T2CON_CLK_EXTCLK               (0x3   << 9  ) /* EXTCLK. External clock applied on P0.5. */

/* T2CON[WUEN] - Wake-up enable bits for time field values. */
#define T2CON_WUEN_BBA                 (*(volatile unsigned long *) 0x4204A120)
#define T2CON_WUEN_MSK                 (0x1   << 8  )
#define T2CON_WUEN                     (0x1   << 8  )
#define T2CON_WUEN_DIS                 (0x0   << 8  ) /* DIS. Disable asynchronous Wake-Up timer. Interrupt conditions will not wake-up the part from sleep mode. */
#define T2CON_WUEN_EN                  (0x1   << 8  ) /* EN. Enable asynchronous Wake-Up timer even when the core clock is off. Once the timer value equals any of the interrupt enabled compare field, a wake-up signal is generated. */

/* T2CON[ENABLE] - Timer enable bit. */
#define T2CON_ENABLE_BBA               (*(volatile unsigned long *) 0x4204A11C)
#define T2CON_ENABLE_MSK               (0x1   << 7  )
#define T2CON_ENABLE                   (0x1   << 7  )
#define T2CON_ENABLE_DIS               (0x0   << 7  ) /* DIS. Disable the timer.  */
#define T2CON_ENABLE_EN                (0x1   << 7  ) /* EN. Enable the timer. When enabled wait for T2STA[8] to clear before continuing. */

/* T2CON[MOD] - Timer free run enable. */
#define T2CON_MOD_BBA                  (*(volatile unsigned long *) 0x4204A118)
#define T2CON_MOD_MSK                  (0x1   << 6  )
#define T2CON_MOD                      (0x1   << 6  )
#define T2CON_MOD_PERIODIC             (0x0   << 6  ) /* PERIODIC. Operate in periodic mode.  Counts up to the value in T2WUFD */
#define T2CON_MOD_FREERUN              (0x1   << 6  ) /* FREERUN. Operate in free running mode (default). Counts from 0 to FFFF FFFF and starts again at 0. */

/* T2CON[FREEZE] - Freeze enable bit. */
#define T2CON_FREEZE_BBA               (*(volatile unsigned long *) 0x4204A10C)
#define T2CON_FREEZE_MSK               (0x1   << 3  )
#define T2CON_FREEZE                   (0x1   << 3  )
#define T2CON_FREEZE_DIS               (0x0   << 3  ) /* DIS. Disable this feature. */
#define T2CON_FREEZE_EN                (0x1   << 3  ) /* EN. Enable the freeze of the high 16 bits after the lower bits have been read from T2VAL0. This ensures that the software reads an atomic shot of the timer. The entire T2VAL register unfreezes after the high bits (T2VAL1) have been read. */

/* T2CON[PRE] - Prescaler. */
#define T2CON_PRE_MSK                  (0x3   << 0  )
#define T2CON_PRE_DIV1                 (0x0   << 0  ) /* DIV1. Source clock/1. If the selected clock source is PCLK this setting results in a prescaler of 4. */
#define T2CON_PRE_DIV16                (0x1   << 0  ) /* DIV16. Source clock/16.  */
#define T2CON_PRE_DIV256               (0x2   << 0  ) /* DIV256. Source clock/256. */
#define T2CON_PRE_DIV32768             (0x3   << 0  ) /* DIV32768. Source clock/32768. */

/* Reset Value for T2INC*/
#define T2INC_RVAL                     0xC8

/* T2INC[VALUE] - Wake-up interval */
#define T2INC_VALUE_MSK                (0xFFF << 0  )

/* Reset Value for T2WUFB0*/
#define T2WUFB0_RVAL                   0x1FFF

/* T2WUFB0[VALUE] - Lower 16 bits of Wake-Up Field B */
#define T2WUFB0_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2WUFB1*/
#define T2WUFB1_RVAL                   0x0

/* T2WUFB1[VALUE] - Upper 16 bits of Wake-Up Field B */
#define T2WUFB1_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2WUFC0*/
#define T2WUFC0_RVAL                   0x2FFF

/* T2WUFC0[VALUE] - Lower 16 bits of Wake-Up Field C */
#define T2WUFC0_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2WUFC1*/
#define T2WUFC1_RVAL                   0x0

/* T2WUFC1[VALUE] - Upper 16 bits of Wake-Up Field C */
#define T2WUFC1_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2WUFD0*/
#define T2WUFD0_RVAL                   0x3FFF

/* T2WUFD0[VALUE] - Lower 16 bits of Wake-Up Field D */
#define T2WUFD0_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2WUFD1*/
#define T2WUFD1_RVAL                   0x0

/* T2WUFD1[VALUE] - Upper 16 bits of Wake-Up Field D */
#define T2WUFD1_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2IEN*/
#define T2IEN_RVAL                     0x0

/* T2IEN[ROLL] - Interrupt enable bit when the counter rolls over. Only occurs in free running mode. */
#define T2IEN_ROLL_BBA                 (*(volatile unsigned long *) 0x4204A510)
#define T2IEN_ROLL_MSK                 (0x1   << 4  )
#define T2IEN_ROLL                     (0x1   << 4  )
#define T2IEN_ROLL_DIS                 (0x0   << 4  ) /* DIS. Disable the roll over interrupt. */
#define T2IEN_ROLL_EN                  (0x1   << 4  ) /* EN. Generate an interrupt when Timer2 rolls over. */

/* T2IEN[WUFD] - T2WUFD interrupt enable */
#define T2IEN_WUFD_BBA                 (*(volatile unsigned long *) 0x4204A50C)
#define T2IEN_WUFD_MSK                 (0x1   << 3  )
#define T2IEN_WUFD                     (0x1   << 3  )
#define T2IEN_WUFD_DIS                 (0x0   << 3  ) /* DIS. Disable T2WUFD interrupt. */
#define T2IEN_WUFD_EN                  (0x1   << 3  ) /* EN. Generate an interrupt when T2VAL reaches T2WUFD. */

/* T2IEN[WUFC] - T2WUFC interrupt enable */
#define T2IEN_WUFC_BBA                 (*(volatile unsigned long *) 0x4204A508)
#define T2IEN_WUFC_MSK                 (0x1   << 2  )
#define T2IEN_WUFC                     (0x1   << 2  )
#define T2IEN_WUFC_DIS                 (0x0   << 2  ) /* DIS. Disable T2WUFC interrupt. */
#define T2IEN_WUFC_EN                  (0x1   << 2  ) /* EN. Generate an interrupt when T2VAL reaches T2WUFC. */

/* T2IEN[WUFB] - T2WUFB interrupt enable */
#define T2IEN_WUFB_BBA                 (*(volatile unsigned long *) 0x4204A504)
#define T2IEN_WUFB_MSK                 (0x1   << 1  )
#define T2IEN_WUFB                     (0x1   << 1  )
#define T2IEN_WUFB_DIS                 (0x0   << 1  ) /* DIS. Disable T2WUFB interrupt. */
#define T2IEN_WUFB_EN                  (0x1   << 1  ) /* EN. Generate an interrupt when T2VAL reaches T2WUFB. */

/* T2IEN[WUFA] - T2WUFA interrupt enable */
#define T2IEN_WUFA_BBA                 (*(volatile unsigned long *) 0x4204A500)
#define T2IEN_WUFA_MSK                 (0x1   << 0  )
#define T2IEN_WUFA                     (0x1   << 0  )
#define T2IEN_WUFA_DIS                 (0x0   << 0  ) /* DIS. Disable T2WUFA interrupt. */
#define T2IEN_WUFA_EN                  (0x1   << 0  ) /* EN. Generate an interrupt when T2VAL reaches T2WUFA. */

/* Reset Value for T2STA*/
#define T2STA_RVAL                     0x0

/* T2STA[CON] - Indicates when a change in the enable bit is synchronized to the 32 kHz clock domain (Done automatically) */
#define T2STA_CON_BBA                  (*(volatile unsigned long *) 0x4204A5A0)
#define T2STA_CON_MSK                  (0x1   << 8  )
#define T2STA_CON                      (0x1   << 8  )
#define T2STA_CON_CLR                  (0x0   << 8  ) /* CLR. It returns low when the change in the Enable bit has been synchronised to the 32 kHz clock domain. */
#define T2STA_CON_SET                  (0x1   << 8  ) /* SET. This bit is set high when the Enable bit (bit 5) in the Control register is set or cleared and it is not synchronised to tthe 32 kHz clock. */

/* T2STA[FREEZE] - Status of T2VAL freeze */
#define T2STA_FREEZE_BBA               (*(volatile unsigned long *) 0x4204A59C)
#define T2STA_FREEZE_MSK               (0x1   << 7  )
#define T2STA_FREEZE                   (0x1   << 7  )
#define T2STA_FREEZE_CLR               (0x0   << 7  ) /* CLR.  Reset low when T2VAL1 is read, indicating T2VAL is unfrozen. */
#define T2STA_FREEZE_SET               (0x1   << 7  ) /* SET. Set high when the T2VAL0 is read, indicating T2VAL is frozen. */

/* T2STA[ROLL] - Interrupt status bit for instances when counter rolls over. Only occurs in free running mode. */
#define T2STA_ROLL_BBA                 (*(volatile unsigned long *) 0x4204A590)
#define T2STA_ROLL_MSK                 (0x1   << 4  )
#define T2STA_ROLL                     (0x1   << 4  )
#define T2STA_ROLL_CLR                 (0x0   << 4  ) /* CLR. Indicate that the timer has not rolled over. */
#define T2STA_ROLL_SET                 (0x1   << 4  ) /* SET. Set high when enabled in the interrupt enable register and the T2VALS counter register is equal to all 1s */

/* T2STA[WUFD] - T2WUFD interrupt flag */
#define T2STA_WUFD_BBA                 (*(volatile unsigned long *) 0x4204A58C)
#define T2STA_WUFD_MSK                 (0x1   << 3  )
#define T2STA_WUFD                     (0x1   << 3  )
#define T2STA_WUFD_CLR                 (0x0   << 3  ) /* CLR. Cleared after a write to the corresponding bit in T2CLRI. */
#define T2STA_WUFD_SET                 (0x1   << 3  ) /* SET. Indicates that a comparator interrupt has occurred. */

/* T2STA[WUFC] - T2WUFC interrupt flag */
#define T2STA_WUFC_BBA                 (*(volatile unsigned long *) 0x4204A588)
#define T2STA_WUFC_MSK                 (0x1   << 2  )
#define T2STA_WUFC                     (0x1   << 2  )
#define T2STA_WUFC_CLR                 (0x0   << 2  ) /* CLR. Cleared after a write to the corresponding bit in T2CLRI. */
#define T2STA_WUFC_SET                 (0x1   << 2  ) /* SET. Indicates that a comparator interrupt has occurred. */

/* T2STA[WUFB] - T2WUFB interrupt flag */
#define T2STA_WUFB_BBA                 (*(volatile unsigned long *) 0x4204A584)
#define T2STA_WUFB_MSK                 (0x1   << 1  )
#define T2STA_WUFB                     (0x1   << 1  )
#define T2STA_WUFB_CLR                 (0x0   << 1  ) /* CLR. Cleared after a write to the corresponding bit in T2CLRI. */
#define T2STA_WUFB_SET                 (0x1   << 1  ) /* SET. Indicates that a comparator interrupt has occurred. */

/* T2STA[WUFA] - T2WUFA interrupt flag */
#define T2STA_WUFA_BBA                 (*(volatile unsigned long *) 0x4204A580)
#define T2STA_WUFA_MSK                 (0x1   << 0  )
#define T2STA_WUFA                     (0x1   << 0  )
#define T2STA_WUFA_CLR                 (0x0   << 0  ) /* CLR. Cleared after a write to the corresponding bit in T2CLRI. */
#define T2STA_WUFA_SET                 (0x1   << 0  ) /* SET. Indicates that a comparator interrupt has occurred. */

/* Reset Value for T2CLRI*/
#define T2CLRI_RVAL                    0x0

/* T2CLRI[ROLL] - Clear interrupt on Rollover.  Only occurs in free running mode. */
#define T2CLRI_ROLL_BBA                (*(volatile unsigned long *) 0x4204A610)
#define T2CLRI_ROLL_MSK                (0x1   << 4  )
#define T2CLRI_ROLL                    (0x1   << 4  )
#define T2CLRI_ROLL_CLR                (0x1   << 4  ) /* CLR. Interrupt clear bit for when counter rolls over. */

/* T2CLRI[WUFD] - T2WUFD interrupt flag. Cleared automatically after synchronization. */
#define T2CLRI_WUFD_BBA                (*(volatile unsigned long *) 0x4204A60C)
#define T2CLRI_WUFD_MSK                (0x1   << 3  )
#define T2CLRI_WUFD                    (0x1   << 3  )
#define T2CLRI_WUFD_CLR                (0x1   << 3  ) /* CLR. Clear the T2WUFD interrupt flag. */

/* T2CLRI[WUFC] - T2WUFC interrupt flag. Cleared automatically after synchronization. */
#define T2CLRI_WUFC_BBA                (*(volatile unsigned long *) 0x4204A608)
#define T2CLRI_WUFC_MSK                (0x1   << 2  )
#define T2CLRI_WUFC                    (0x1   << 2  )
#define T2CLRI_WUFC_CLR                (0x1   << 2  ) /* CLR. Clear the T2WUFC interrupt flag. */

/* T2CLRI[WUFB] - T2WUFB interrupt flag. Cleared automatically after synchronization. */
#define T2CLRI_WUFB_BBA                (*(volatile unsigned long *) 0x4204A604)
#define T2CLRI_WUFB_MSK                (0x1   << 1  )
#define T2CLRI_WUFB                    (0x1   << 1  )
#define T2CLRI_WUFB_CLR                (0x1   << 1  ) /* CLR. Clear the T2WUFB interrupt flag. */

/* T2CLRI[WUFA] - T2WUFA interrupt flag. Cleared automatically after synchronization. */
#define T2CLRI_WUFA_BBA                (*(volatile unsigned long *) 0x4204A600)
#define T2CLRI_WUFA_MSK                (0x1   << 0  )
#define T2CLRI_WUFA                    (0x1   << 0  )
#define T2CLRI_WUFA_CLR                (0x1   << 0  ) /* CLR. Clear the T2WUFA interrupt flag. */

/* Reset Value for T2WUFA0*/
#define T2WUFA0_RVAL                   0x1900

/* T2WUFA0[VALUE] - Lower 16 bits of Compare Register A */
#define T2WUFA0_VALUE_MSK              (0xFFFF << 0  )

/* Reset Value for T2WUFA1*/
#define T2WUFA1_RVAL                   0x0

/* T2WUFA1[VALUE] - Upper 16 bits of Compare Register A */
#define T2WUFA1_VALUE_MSK              (0xFFFF << 0  )
// ------------------------------------------------------------------------------------------------
// -----                                        WDT                                        -----
// ------------------------------------------------------------------------------------------------


/**
  * @brief Watchdog Timer (pADI_WDT)
  */

#if (__NO_MMR_STRUCTS__==0)
typedef struct {                            /*!< pADI_WDT Structure                     */
  __IO uint16_t  T3LD;                      /*!< 16-bit Load Value                     */
  __I  uint16_t  RESERVED0;
  __IO uint16_t  T3VAL;                     /*!< 16-bit Timer Value                    */
  __I  uint16_t  RESERVED1;
  __IO uint16_t  T3CON;                     /*!< Control Register                      */
  __I  uint16_t  RESERVED2;
  __IO uint16_t  T3CLRI;                    /*!< Clear Interrupt Register              */
  __I  uint16_t  RESERVED3[5];
  __IO uint16_t  T3STA;                     /*!< Status Register                       */
} ADI_WDT_TypeDef;
#else // (__NO_MMR_STRUCTS__==0)
#define          T3LD                                       (*(volatile unsigned short int *) 0x40002580)
#define          T3VAL                                      (*(volatile unsigned short int *) 0x40002584)
#define          T3CON                                      (*(volatile unsigned short int *) 0x40002588)
#define          T3CLRI                                     (*(volatile unsigned short int *) 0x4000258C)
#define          T3STA                                      (*(volatile unsigned short int *) 0x40002598)
#endif // (__NO_MMR_STRUCTS__==0)

/* Reset Value for T3LD*/
#define T3LD_RVAL                      0x1000

/* T3LD[VALUE] - Load value. */
#define T3LD_VALUE_MSK                 (0xFFFF << 0  )

/* Reset Value for T3VAL*/
#define T3VAL_RVAL                     0x1000

/* T3VAL[VALUE] - Current counter value. */
#define T3VAL_VALUE_MSK                (0xFFFF << 0  )

/* Reset Value for T3CON*/
#define T3CON_RVAL                     0xE9

/* T3CON[MOD] - Timer Mode */
#define T3CON_MOD_BBA                  (*(volatile unsigned long *) 0x4204B118)
#define T3CON_MOD_MSK                  (0x1   << 6  )
#define T3CON_MOD                      (0x1   << 6  )
#define T3CON_MOD_Reserved             (0x0   << 6  ) /* Reserved                 */
#define T3CON_MOD_PERIODIC             (0x1   << 6  ) /* PERIODIC: Operate in periodic mode. */

/* T3CON[ENABLE] - Timer enable bit. */
#define T3CON_ENABLE_BBA               (*(volatile unsigned long *) 0x4204B114)
#define T3CON_ENABLE_MSK               (0x1   << 5  )
#define T3CON_ENABLE                   (0x1   << 5  )
#define T3CON_ENABLE_DIS               (0x0   << 5  ) /* DIS. Disable the timer. Clearing this bit resets the timer, including the T0VAL register. */
#define T3CON_ENABLE_EN                (0x1   << 5  ) /* EN. Enable the timer.  The timer starts counting from its initial value. */

/* T3CON[PRE] - Prescaler. */
#define T3CON_PRE_MSK                  (0x3   << 2  )
#define T3CON_PRE_DIV1                 (0x0   << 2  ) /* DIV1. Source clock/1.    */
#define T3CON_PRE_DIV16                (0x1   << 2  ) /* DIV16. Source clock/16.  */
#define T3CON_PRE_DIV256               (0x2   << 2  ) /* DIV256. Source clock/256. */
#define T3CON_PRE_DIV4096              (0x3   << 2  ) /* DIV4096. Source clock/4096. */

/* T3CON[IRQ] - Timer interrupt. */
#define T3CON_IRQ_BBA                  (*(volatile unsigned long *) 0x4204B104)
#define T3CON_IRQ_MSK                  (0x1   << 1  )
#define T3CON_IRQ                      (0x1   << 1  )
#define T3CON_IRQ_DIS                  (0x0   << 1  ) /* DIS. Generate a reset on a timeout. */
#define T3CON_IRQ_EN                   (0x1   << 1  ) /* EN. Generate an interrupt when the timer times out. This feature is available in active mode only and can be used to debug the watchdog timeout events. */

/* T3CON[PD] - Stop count in hibernate mode. */
#define T3CON_PD_BBA                   (*(volatile unsigned long *) 0x4204B100)
#define T3CON_PD_MSK                   (0x1   << 0  )
#define T3CON_PD                       (0x1   << 0  )
#define T3CON_PD_DIS                   (0x0   << 0  ) /* DIS. The timer continues to count when in hibernate mode. */
#define T3CON_PD_EN                    (0x1   << 0  ) /* EN. The timer stops counting when in hibernate mode. */

/* Reset Value for T3CLRI*/
#define T3CLRI_RVAL                    0x0

/* T3CLRI[VALUE] - Clear watchdog. */
#define T3CLRI_VALUE_MSK               (0xFFFF << 0  )

/* Reset Value for T3STA*/
#define T3STA_RVAL                     0x20

/* T3STA[LOCK] - Lock status bit. */
#define T3STA_LOCK_BBA                 (*(volatile unsigned long *) 0x4204B310)
#define T3STA_LOCK_MSK                 (0x1   << 4  )
#define T3STA_LOCK                     (0x1   << 4  )
#define T3STA_LOCK_CLR                 (0x0   << 4  ) /* CLR. Cleared after any reset and until user code sets T3CON[5]. */
#define T3STA_LOCK_SET                 (0x1   << 4  ) /* SET. Set automatically in hardware when user code sets T3CON[5]. */

/* T3STA[CON] - T3CON write sync in progress. */
#define T3STA_CON_BBA                  (*(volatile unsigned long *) 0x4204B30C)
#define T3STA_CON_MSK                  (0x1   << 3  )
#define T3STA_CON                      (0x1   << 3  )
#define T3STA_CON_CLR                  (0x0   << 3  ) /* CLR. Timer ready to receive commands to T3CON. The previous change of T3CON has been synchronized in the timer clock domain. */
#define T3STA_CON_SET                  (0x1   << 3  ) /* SET. Timer not ready to receive commands to T3CON. Previous change of the T3CON value has not been synchronized in the timer clock domain. */

/* T3STA[LD] - T3LD write sync in progress. */
#define T3STA_LD_BBA                   (*(volatile unsigned long *) 0x4204B308)
#define T3STA_LD_MSK                   (0x1   << 2  )
#define T3STA_LD                       (0x1   << 2  )
#define T3STA_LD_CLR                   (0x0   << 2  ) /* CLR. The previous change of T3LD has been synchronized in the timer clock domain. */
#define T3STA_LD_SET                   (0x1   << 2  ) /* SET. Previous change of the T3LD value has not been synchronized in the timer clock domain. */

/* T3STA[CLRI] - T3CLRI write sync in progress. */
#define T3STA_CLRI_BBA                 (*(volatile unsigned long *) 0x4204B304)
#define T3STA_CLRI_MSK                 (0x1   << 1  )
#define T3STA_CLRI                     (0x1   << 1  )
#define T3STA_CLRI_CLR                 (0x0   << 1  ) /* CLR. Cleared when the interrupt is cleared in the timer clock domain. */
#define T3STA_CLRI_SET                 (0x1   << 1  ) /* SET. Set automatically when the T3CLRI value is being updated in the timer clock domain, indicating that the timers configuration is not yet valid. */

/* T3STA[IRQ] - Interrupt pending. */
#define T3STA_IRQ_BBA                  (*(volatile unsigned long *) 0x4204B300)
#define T3STA_IRQ_MSK                  (0x1   << 0  )
#define T3STA_IRQ                      (0x1   << 0  )
#define T3STA_IRQ_CLR                  (0x0   << 0  ) /* CLR. No timeout event has occurred. */
#define T3STA_IRQ_SET                  (0x1   << 0  ) /* SET. A timeout event has occurred. */


/* --------------------  End of section using anonymous unions  ------------------- */
#if defined(__CC_ARM)
  #pragma pop
#elif defined(__ICCARM__)
  /* leave anonymous unions enabled */
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined(__TASKING__)
  #pragma warning restore
#else
  #warning Not supported compiler type
#endif
/********************************************
** Miscellaneous Definitions               **
*********************************************/

//iEiNr in EiCfg()
#define EXTINT0   0x0
#define EXTINT1   0x1
#define EXTINT2   0x2
#define EXTINT3   0x3
#define EXTINT4   0x4
#define EXTINT5   0x5
#define EXTINT6   0x6
#define EXTINT7   0x7
#define EXTINT8   0x8

//iEnable in EiCfg()
#define INT_DIS   0x0
#define INT_EN    0x1

//iMode in EiCfg()
#define INT_RISE  0x0
#define INT_FALL  0x1
#define INT_EDGES 0x2
#define INT_HIGH  0x3
#define INT_LOW   0x4

//Bit values.
#define BIT0      1
#define BIT1      2
#define BIT2      4
#define BIT3      8
#define BIT4      0x10
#define BIT5      0x20
#define BIT6      0x40
#define BIT7      0x80


/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */

#define ADI_ADC0_ADDR                            0x40050000UL
#define ADI_CLKCTL_ADDR                          0x40002000UL
#define ADI_DMA_ADDR                             0x40010000UL
#define ADI_FEE_ADDR                             0x40002800UL
#define ADI_GP0_ADDR                             0x40006000UL
#define ADI_GP1_ADDR                             0x40006030UL
#define ADI_GP2_ADDR                             0x40006060UL
#define ADI_GP3_ADDR                             0x40006090UL
#define ADI_GP4_ADDR                             0x400060C0UL
#define ADI_GPIOCMN_ADDR                         0x400060F0UL
#define ADI_MISC_ADDR                            0x40008820UL
#define ADI_I2C_ADDR                             0x40003000UL
#define ADI_INTERRUPT_ADDR                       0x40002420UL
#define ADI_IDENT_ADDR                           0x40002020UL
#define ADI_NVIC_ADDR                            0xE000E000UL
#define ADI_PWRCTL_ADDR                          0x40002400UL
#define ADI_PWM_ADDR                             0x40001000UL
#define ADI_RESET_ADDR                           0x40002440UL
#define ADI_SPI0_ADDR                            0x40004000UL
#define ADI_SPI1_ADDR                            0x40004400UL
#define ADI_TM0_ADDR                             0x40000000UL
#define ADI_TM1_ADDR                             0x40000400UL
#define ADI_UART_ADDR                            0x40005000UL
#define ADI_WUT_ADDR                             0x40002500UL
#define ADI_WDT_ADDR                             0x40002580UL


/* ================================================================================ */
/* ================             Peripheral declaration             ================ */
/* ================================================================================ */

#define pADI_ADC0                     ((ADI_ADC_TypeDef                *)ADI_ADC0_ADDR)
#define pADI_CLKCTL                   ((ADI_CLKCTL_TypeDef             *)ADI_CLKCTL_ADDR)
#define pADI_DMA                      ((ADI_DMA_TypeDef                *)ADI_DMA_ADDR)
#define pADI_FEE                      ((ADI_FEE_TypeDef                *)ADI_FEE_ADDR)
#define pADI_GP0                      ((ADI_GPIO_TypeDef               *)ADI_GP0_ADDR)
#define pADI_GP1                      ((ADI_GPIO_TypeDef               *)ADI_GP1_ADDR)
#define pADI_GP2                      ((ADI_GPIO_TypeDef               *)ADI_GP2_ADDR)
#define pADI_GP3                      ((ADI_GPIO_TypeDef               *)ADI_GP3_ADDR)
#define pADI_GP4                      ((ADI_GPIO_TypeDef               *)ADI_GP4_ADDR)
#define pADI_GPIOCMN                  ((ADI_GPIOCMN_TypeDef            *)ADI_GPIOCMN_ADDR)
#define pADI_MISC                     ((ADI_MISC_TypeDef               *)ADI_MISC_ADDR)
#define pADI_I2C                      ((ADI_I2C_TypeDef                *)ADI_I2C_ADDR)
#define pADI_INTERRUPT                ((ADI_INTERRUPT_TypeDef          *)ADI_INTERRUPT_ADDR)
#define pADI_PWRCTL                   ((ADI_PWRCTL_TypeDef             *)ADI_PWRCTL_ADDR)
#define pADI_PWM                      ((ADI_PWM_TypeDef                *)ADI_PWM_ADDR)
#define pADI_RESET                    ((ADI_RESET_TypeDef              *)ADI_RESET_ADDR)
#define pADI_SPI0                     ((ADI_SPI_TypeDef                *)ADI_SPI0_ADDR)
#define pADI_SPI1                     ((ADI_SPI_TypeDef                *)ADI_SPI1_ADDR)
#define pADI_TM0                      ((ADI_TIMER_TypeDef              *)ADI_TM0_ADDR)
#define pADI_TM1                      ((ADI_TIMER_TypeDef              *)ADI_TM1_ADDR)
#define pADI_UART                     ((ADI_UART_TypeDef               *)ADI_UART_ADDR)
#define pADI_WUT                      ((ADI_WUT_TypeDef                *)ADI_WUT_ADDR)
#define pADI_WDT                      ((ADI_WDT_TypeDef                *)ADI_WDT_ADDR)

/** @} */ /* End of group Device_Peripheral_Registers *//** @} */ /* End of group ADUCRF101 */
/** @} */ /* End of group CMSIS */

#ifdef __cplusplus
}
#endif


#endif  // __ADUCRF101_H__

