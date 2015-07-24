/**
  ******************************************************************************
 * @file    SPIRIT_Gpio.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   This file provides all the low level API to manage SPIRIT GPIO.
  * 
 * @details
 *
 * This module can be used to configure the Spirit GPIO pins to perform
 * specific functions.
 * The structure <i>@ref gpioIRQ</i> can be used to specify these features for
 * one of the four Spirit Gpio pin.
 * The following example shows how to configure a pin (GPIO 3) to be used as an IRQ source
 * for a microcontroller using the <i>@ref SpiritGpioInit()</i> function.
 *
 * <b>Example:</b>
 * @code
 *
 * SGpioInit gpioIRQ={
 *   SPIRIT_GPIO_3,
 *   SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP,
 *   SPIRIT_GPIO_DIG_OUT_IRQ
 * };
 *
 * ...
 *
 * SpiritGpioInit(&gpioIRQ);
 *
 * @endcode
 *
 * @note Please read the functions documentation for the other GPIO features.
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
#ifndef __SPIRIT_GPIO_H
#define __SPIRIT_GPIO_H


/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Regs.h"
#include "SPIRIT_Types.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @defgroup SPIRIT_Gpio       GPIO
 * @brief Configuration and management of SPIRIT GPIO.
 * @details See the file <i>@ref SPIRIT_Gpio.h</i> for more details.
 * @{
 */



/** @defgroup Gpio_Exported_Types       GPIO Exported Types
 * @{
 */

/**
 * @brief  SPIRIT GPIO pin enumeration.
 */
typedef enum
{
  SPIRIT_GPIO_0  = GPIO0_CONF_BASE, /*!< GPIO_0 selected */
  SPIRIT_GPIO_1  = GPIO1_CONF_BASE, /*!< GPIO_1 selected */
  SPIRIT_GPIO_2  = GPIO2_CONF_BASE, /*!< GPIO_2 selected */
  SPIRIT_GPIO_3  = GPIO3_CONF_BASE  /*!< GPIO_3 selected */
}SpiritGpioPin;


#define IS_SPIRIT_GPIO(PIN)  ((PIN == SPIRIT_GPIO_0) || \
                              (PIN == SPIRIT_GPIO_1) || \
                              (PIN == SPIRIT_GPIO_2) || \
                              (PIN == SPIRIT_GPIO_3))


/**
 * @brief  SPIRIT GPIO mode enumeration.
 */
typedef enum
{
  SPIRIT_GPIO_MODE_DIGITAL_INPUT      = 0x01, /*!< Digital Input on GPIO */
  SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP  = 0x02, /*!< Digital Output on GPIO (low current) */
  SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP  = 0x03  /*!< Digital Output on GPIO (high current) */
}SpiritGpioMode;

#define IS_SPIRIT_GPIO_MODE(MODE)   ((MODE == SPIRIT_GPIO_MODE_DIGITAL_INPUT) || \
                                     (MODE == SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP) || \
                                     (MODE == SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP))



/**
 * @brief  SPIRIT I/O selection enumeration.
 */
typedef enum
{
  SPIRIT_GPIO_DIG_OUT_IRQ                               = 0x00, /*!< nIRQ (Interrupt Request, active low) , default configuration after POR */
  SPIRIT_GPIO_DIG_OUT_POR_INV                           = 0x08, /*!< POR inverted (active low) */
  SPIRIT_GPIO_DIG_OUT_WUT_EXP                           = 0x10, /*!< Wake-Up Timer expiration: "1" when WUT has expired */
  SPIRIT_GPIO_DIG_OUT_LBD                               = 0x18, /*!< Low battery detection: "1" when battery is below threshold setting */
  SPIRIT_GPIO_DIG_OUT_TX_DATA                           = 0x20, /*!< TX data internal clock output (TX data are sampled on the rising edge of it) */
  SPIRIT_GPIO_DIG_OUT_TX_STATE                          = 0x28, /*!< TX state indication: "1" when Spirit1 is passing in the TX state */
  SPIRIT_GPIO_DIG_OUT_TX_FIFO_ALMOST_EMPTY              = 0x30, /*!< TX FIFO Almost Empty Flag */
  SPIRIT_GPIO_DIG_OUT_TX_FIFO_ALMOST_FULL               = 0x38, /*!< TX FIFO Almost Full Flag */
  SPIRIT_GPIO_DIG_OUT_RX_DATA                           = 0x40, /*!< RX data output */
  SPIRIT_GPIO_DIG_OUT_RX_CLOCK                          = 0x48, /*!< RX clock output (recovered from received data) */
  SPIRIT_GPIO_DIG_OUT_RX_STATE                          = 0x50, /*!< RX state indication: "1" when Spirit1 is passing in the RX state */
  SPIRIT_GPIO_DIG_OUT_RX_FIFO_ALMOST_FULL               = 0x58, /*!< RX FIFO Almost Full Flag */
  SPIRIT_GPIO_DIG_OUT_RX_FIFO_ALMOST_EMPTY              = 0x60, /*!< RX FIFO Almost Empty Flag */
  SPIRIT_GPIO_DIG_OUT_ANTENNA_SWITCH                    = 0x68, /*!< Antenna switch used for antenna diversity  */
  SPIRIT_GPIO_DIG_OUT_VALID_PREAMBLE                    = 0x70, /*!< Valid Preamble Detected Flag */
  SPIRIT_GPIO_DIG_OUT_SYNC_DETECTED                     = 0x78, /*!< Sync WordSync Word Detected Flag */
  SPIRIT_GPIO_DIG_OUT_RSSI_THRESHOLD                    = 0x80, /*!< RSSI above threshold */
  SPIRIT_GPIO_DIG_OUT_MCU_CLOCK                         = 0x88, /*!< MCU Clock */
  SPIRIT_GPIO_DIG_OUT_TX_RX_MODE                        = 0x90, /*!< TX or RX mode indicator (to enable an external range extender) */
  SPIRIT_GPIO_DIG_OUT_VDD                               = 0x98, /*!< VDD (to emulate an additional GPIO of the MCU, programmable by SPI) */
  SPIRIT_GPIO_DIG_OUT_GND                               = 0xA0, /*!< GND (to emulate an additional GPIO of the MCU, programmable by SPI) */
  SPIRIT_GPIO_DIG_OUT_SMPS_EXT                          = 0xA8, /*!< External SMPS enable signal (active high) */
  SPIRIT_GPIO_DIG_OUT_SLEEP_OR_STANDBY                  = 0xB0,
  SPIRIT_GPIO_DIG_OUT_READY                             = 0xB8,
  SPIRIT_GPIO_DIG_OUT_LOCK                              = 0xC0,
  SPIRIT_GPIO_DIG_OUT_WAIT_FOR_LOCK_SIG                 = 0xC8,
  SPIRIT_GPIO_DIG_OUT_WAIT_FOR_TIMER_FOR_LOCK           = 0xD0,
  SPIRIT_GPIO_DIG_OUT_WAIT_FOR_READY2_SIG               = 0xD8,
  SPIRIT_GPIO_DIG_OUT_WAIT_FOR_TIMER_FOR_PM_SET         = 0xE0,
  SPIRIT_GPIO_DIG_OUT_WAIT_VCO_CALIBRATION              = 0xE8,
  SPIRIT_GPIO_DIG_OUT_ENABLE_SYNTH_FULL_CIRCUIT         = 0xF0,
  SPIRIT_GPIO_DIG_OUT_WAIT_FOR_RCCAL_OK_SIG             = 0xFF,

  SPIRIT_GPIO_DIG_IN_TX_COMMAND                         = 0x00,
  SPIRIT_GPIO_DIG_IN_RX_COMMAND                         = 0x08,
  SPIRIT_GPIO_DIG_IN_TX_DATA_INPUT_FOR_DIRECTRF         = 0x10,
  SPIRIT_GPIO_DIG_IN_DATA_WAKEUP                        = 0x18,
  SPIRIT_GPIO_DIG_IN_EXT_CLOCK_AT_34_7KHZ               = 0x20

}SpiritGpioIO;

#define IS_SPIRIT_GPIO_IO(IO_SEL)        ((IO_SEL == SPIRIT_GPIO_DIG_OUT_IRQ) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_POR_INV) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WUT_EXP) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_LBD) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_TX_DATA) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_TX_STATE) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_TX_FIFO_ALMOST_EMPTY) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_TX_FIFO_ALMOST_FULL) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_RX_DATA) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_RX_CLOCK) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_RX_STATE) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_RX_FIFO_ALMOST_FULL) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_RX_FIFO_ALMOST_EMPTY) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_ANTENNA_SWITCH) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_VALID_PREAMBLE) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_SYNC_DETECTED) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_RSSI_THRESHOLD) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_MCU_CLOCK) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_TX_RX_MODE) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_VDD) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_GND) || \
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_SMPS_EXT) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_SLEEP_OR_STANDBY) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_READY) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_LOCK) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WAIT_FOR_LOCK_SIG) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WAIT_FOR_TIMER_FOR_LOCK) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WAIT_FOR_READY2_SIG) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WAIT_FOR_TIMER_FOR_PM_SET) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WAIT_VCO_CALIBRATION) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_ENABLE_SYNTH_FULL_CIRCUIT) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_OUT_WAIT_FOR_RCCAL_OK_SIG) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_IN_TX_COMMAND) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_IN_RX_COMMAND) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_IN_TX_DATA_INPUT_FOR_DIRECTRF) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_IN_DATA_WAKEUP) ||\
                                          (IO_SEL == SPIRIT_GPIO_DIG_IN_EXT_CLOCK_AT_34_7KHZ))

/**
 * @brief  SPIRIT OutputLevel enumeration.
 */

typedef enum
{
  LOW = 0,
  HIGH = !LOW
}OutputLevel;

#define IS_SPIRIT_GPIO_LEVEL(LEVEL)      ((LEVEL == LOW) || \
                                          (LEVEL == HIGH))


/**
 * @brief  SPIRIT GPIO Init structure definition.
 */
typedef struct
{
  SpiritGpioPin xSpiritGpioPin;    /*!< Specifies the GPIO pins to be configured.
                                        This parameter can be any value of @ref SpiritGpioPin */

  SpiritGpioMode xSpiritGpioMode;  /*!< Specifies the operating mode for the selected pins.
                                        This parameter can be a value of @ref SpiritGpioMode */

  SpiritGpioIO xSpiritGpioIO;      /*!< Specifies the I/O selection for the selected pins.
                                        This parameter can be a value of @ref SpiritGpioIO */

}SGpioInit;



/**
 * @brief  SPIRIT clock output XO prescaler enumeration.
 */

typedef enum
{
 XO_RATIO_1     = 0x00, /*!< XO Clock signal available on the GPIO divided by 1     */
 XO_RATIO_2_3	= 0x02, /*!< XO Clock signal available on the GPIO divided by 2/3   */
 XO_RATIO_1_2	= 0x04, /*!< XO Clock signal available on the GPIO divided by 1/2   */
 XO_RATIO_1_3	= 0x06, /*!< XO Clock signal available on the GPIO divided by 1/3   */
 XO_RATIO_1_4	= 0x08, /*!< XO Clock signal available on the GPIO divided by 1/4   */
 XO_RATIO_1_6	= 0x0A, /*!< XO Clock signal available on the GPIO divided by 1/6   */
 XO_RATIO_1_8	= 0x0C, /*!< XO Clock signal available on the GPIO divided by 1/8   */
 XO_RATIO_1_12	= 0x0E, /*!< XO Clock signal available on the GPIO divided by 1/12  */
 XO_RATIO_1_16	= 0x10, /*!< XO Clock signal available on the GPIO divided by 1/16  */
 XO_RATIO_1_24	= 0x12, /*!< XO Clock signal available on the GPIO divided by 1/24  */
 XO_RATIO_1_36	= 0x14, /*!< XO Clock signal available on the GPIO divided by 1/36  */
 XO_RATIO_1_48	= 0x16, /*!< XO Clock signal available on the GPIO divided by 1/48  */
 XO_RATIO_1_64	= 0x18, /*!< XO Clock signal available on the GPIO divided by 1/64  */
 XO_RATIO_1_96	= 0x1A, /*!< XO Clock signal available on the GPIO divided by 1/96  */
 XO_RATIO_1_128	= 0x1C, /*!< XO Clock signal available on the GPIO divided by 1/128 */
 XO_RATIO_1_192	= 0x1E  /*!< XO Clock signal available on the GPIO divided by 1/196 */
}ClockOutputXOPrescaler;

#define IS_SPIRIT_CLOCK_OUTPUT_XO(RATIO) ((RATIO == XO_RATIO_1) || \
                                           (RATIO == XO_RATIO_2_3) || \
                                           (RATIO == XO_RATIO_1_2) || \
                                           (RATIO == XO_RATIO_1_3) || \
                                           (RATIO == XO_RATIO_1_4) || \
                                           (RATIO == XO_RATIO_1_6) || \
                                           (RATIO == XO_RATIO_1_8) || \
                                           (RATIO == XO_RATIO_1_12) || \
                                           (RATIO == XO_RATIO_1_16) || \
                                           (RATIO == XO_RATIO_1_24) || \
                                           (RATIO == XO_RATIO_1_36) || \
                                           (RATIO == XO_RATIO_1_48) || \
                                           (RATIO == XO_RATIO_1_64) || \
                                           (RATIO == XO_RATIO_1_96) || \
                                           (RATIO == XO_RATIO_1_128) || \
                                           (RATIO == XO_RATIO_1_192))

/**
 * @brief  SPIRIT Clock Output RCO prescaler enumeration.
 */

typedef enum
{
 RCO_RATIO_1              = 0x00, /*!< RCO Clock signal available on the GPIO divided by 1     */
 RCO_RATIO_1_128	  = 0x01  /*!< RCO Clock signal available on the GPIO divided by 1/128   */
}ClockOutputRCOPrescaler;

#define IS_SPIRIT_CLOCK_OUTPUT_RCO(RATIO) ((RATIO == RCO_RATIO_1) || \
                                           (RATIO == RCO_RATIO_1_128))

/**
 * @brief  SPIRIT ExtraClockCycles enumeration.
 */

typedef enum
{
EXTRA_CLOCK_CYCLES_0	= 0x00, /*!< 0   extra clock cycles provided to the MCU before switching to STANDBY state */
EXTRA_CLOCK_CYCLES_64	= 0x20, /*!< 64  extra clock cycles provided to the MCU before switching to STANDBY state */
EXTRA_CLOCK_CYCLES_256	= 0x40, /*!< 256 extra clock cycles provided to the MCU before switching to STANDBY state */
EXTRA_CLOCK_CYCLES_512	= 0x60  /*!< 512 extra clock cycles provided to the MCU before switching to STANDBY state */
}ExtraClockCycles;

#define IS_SPIRIT_CLOCK_OUTPUT_EXTRA_CYCLES(CYCLES) ((CYCLES == EXTRA_CLOCK_CYCLES_0) || \
                                                      (CYCLES == EXTRA_CLOCK_CYCLES_64) || \
                                                      (CYCLES == EXTRA_CLOCK_CYCLES_256) || \
                                                      (CYCLES == EXTRA_CLOCK_CYCLES_512))


/**
 * @brief  SPIRIT Clock Output initialization structure definition.
 */
typedef struct
{
  ClockOutputXOPrescaler   xClockOutputXOPrescaler;  /*!< Specifies the XO Ratio as clock output.
                                                          This parameter can be any value of @ref ClockOutputXOPrescaler */

  ClockOutputRCOPrescaler  xClockOutputRCOPrescaler; /*!< Specifies the RCO Ratio as clock output.
                                                          This parameter can be a value of @ref ClockOutputRCOPrescaler */

  ExtraClockCycles         xExtraClockCycles;       /*!< Specifies the Extra Clock Cycles provided before entering in Standby State.
                                                          This parameter can be a value of @ref ExtraClockCycles */

}ClockOutputInit;



/**
 * @}
 */



/** @defgroup Gpio_Exported_Constants   GPIO Exported Constants
 * @{
 */


/**
 * @}
 */



/** @defgroup Gpio_Exported_Macros      GPIO Exported Macros
 * @{
 */


/**
 * @}
 */



/** @defgroup Gpio_Exported_Functions   GPIO Exported Functions
 * @{
 */

void SpiritGpioInit(SGpioInit* pxGpioInitStruct);
void SpiritGpioTemperatureSensor(SpiritFunctionalState xNewState);
void SpiritGpioSetLevel(SpiritGpioPin xGpioX, OutputLevel xLevel);
OutputLevel SpiritGpioGetLevel(SpiritGpioPin xGpioX);
void SpiritGpioClockOutput(SpiritFunctionalState xNewState);
void SpiritGpioClockOutputInit(ClockOutputInit* pxClockOutputInitStruct);
void SpiritGpioSetXOPrescaler(ClockOutputXOPrescaler xXOPrescaler);
ClockOutputXOPrescaler SpiritGpioGetXOPrescaler(void);
void SpiritGpioSetRCOPrescaler(ClockOutputRCOPrescaler xRCOPrescaler);
ClockOutputRCOPrescaler SpiritGpioGetRCOPrescaler(void);
void SpiritGpioSetExtraClockCycles(ExtraClockCycles xExtraCycles);
ExtraClockCycles SpiritGpioGetExtraClockCycles(void);


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
