/**
  ******************************************************************************
  * @file    SPIRIT_Gpio.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   This file provides all the low level API to manage SPIRIT GPIO.
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
#include "SPIRIT_Gpio.h"
#include "MCU_Interface.h"


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @addtogroup SPIRIT_Gpio
 * @{
 */


/** @defgroup Gpio_Private_TypesDefinitions     GPIO Private Types Definitions
 * @{
 */


/**
 * @}
 */


/** @defgroup Gpio_Private_Defines              GPIO Private Defines
 * @{
 */


/**
 * @}
 */



/** @defgroup Gpio_Private_Macros               GPIO Private Macros
 * @{
 */


/**
 * @}
 */



/** @defgroup Gpio_Private_Variables            GPIO Private Variables
 * @{
 */


/**
 * @}
 */



/** @defgroup Gpio_Private_FunctionPrototypes   GPIO Private Function Prototypes
 * @{
 */


/**
 * @}
 */



/** @defgroup Gpio_Private_Functions            GPIO Private Functions
 * @{
 */

/**
 * @brief  Initializes the SPIRIT GPIOx according to the specified
 *         parameters in the pxGpioInitStruct.
 * @param  pxGpioInitStruct pointer to a SGpioInit structure that
 *         contains the configuration information for the specified SPIRIT GPIO.
 * @retval None.
 */
void SpiritGpioInit(SGpioInit* pxGpioInitStruct)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_GPIO(pxGpioInitStruct->xSpiritGpioPin));
  s_assert_param(IS_SPIRIT_GPIO_MODE(pxGpioInitStruct->xSpiritGpioMode));
  s_assert_param(IS_SPIRIT_GPIO_IO(pxGpioInitStruct->xSpiritGpioIO));

  tempRegValue = ((uint8_t)(pxGpioInitStruct->xSpiritGpioMode) | (uint8_t)(pxGpioInitStruct->xSpiritGpioIO));

  g_xStatus = SpiritSpiWriteRegisters(pxGpioInitStruct->xSpiritGpioPin, 1, &tempRegValue);

}


/**
 * @brief  Enables or Disables the output of temperature sensor on SPIRIT GPIO_0.
 * @param  xNewState new state for temperature sensor.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritGpioTemperatureSensor(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue = 0x00;
  uint8_t gpio0tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the ANA_FUNC_CONF0 register and mask the result to enable or disable the
     temperature sensor */
  g_xStatus = SpiritSpiReadRegisters(ANA_FUNC_CONF0_BASE, 1, &tempRegValue);
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= TEMPERATURE_SENSOR_MASK;
  }
  else
  {
    tempRegValue &= (~TEMPERATURE_SENSOR_MASK);
    gpio0tempRegValue = 0x0A; /* Default value */
  }
  g_xStatus = SpiritSpiWriteRegisters(ANA_FUNC_CONF0_BASE, 1, &tempRegValue);

  /* Sets the SPIRIT GPIO_0 according to input request */
  g_xStatus = SpiritSpiWriteRegisters(GPIO0_CONF_BASE, 1, &gpio0tempRegValue);

}


/**
 * @brief  Forces SPIRIT GPIO_x configured as digital output, to VDD or GND.
 * @param  xGpioX Specifies the GPIO to be configured.
 *   This parameter can be one of following parameters:
 *     @arg SPIRIT_GPIO_0: SPIRIT GPIO_0
 *     @arg SPIRIT_GPIO_1: SPIRIT GPIO_1
 *     @arg SPIRIT_GPIO_2: SPIRIT GPIO_2
 *     @arg SPIRIT_GPIO_3: SPIRIT GPIO_3
 * @param  xLevel Specifies the level.
 *   This parameter can be: HIGH or LOW.
 * @retval None.
 */
void SpiritGpioSetLevel(SpiritGpioPin xGpioX, OutputLevel xLevel)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_GPIO(xGpioX));
  s_assert_param(IS_SPIRIT_GPIO_LEVEL(xLevel));

  /* Reads the SPIRIT_GPIOx register and mask the GPIO_SELECT field */
  g_xStatus = SpiritSpiReadRegisters(xGpioX, 1, &tempRegValue);
  tempRegValue &= 0x04;

  /* Sets the value of the SPIRIT GPIO register according to the specified level */
  if(xLevel == HIGH)
  {
    tempRegValue |= (uint8_t)SPIRIT_GPIO_DIG_OUT_VDD | (uint8_t)SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP;
  }
  else
  {
    tempRegValue |= (uint8_t)SPIRIT_GPIO_DIG_OUT_GND | (uint8_t)SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP;
  }

  /* Writes the SPIRIT GPIO register */
  g_xStatus = SpiritSpiWriteRegisters(xGpioX, 1, &tempRegValue);

}


/**
 * @brief  Returns output value (VDD or GND) of SPIRIT GPIO_x, when it is configured as digital output.
 * @param  xGpioX Specifies the GPIO to be read.
 *         This parameter can be one of following parameters:
 *         @arg SPIRIT_GPIO_0: SPIRIT GPIO_0
 *         @arg SPIRIT_GPIO_1: SPIRIT GPIO_1
 *         @arg SPIRIT_GPIO_2: SPIRIT GPIO_2
 *         @arg SPIRIT_GPIO_3: SPIRIT GPIO_3
 * @retval OutputLevel Logical level of selected GPIO configured as digital output.
 *         This parameter can be: HIGH or LOW.
 */
OutputLevel SpiritGpioGetLevel(SpiritGpioPin xGpioX)
{
  uint8_t tempRegValue = 0x00;
  OutputLevel level;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_GPIO(xGpioX));

  /* Reads the SPIRIT_GPIOx register */
  g_xStatus = SpiritSpiReadRegisters(xGpioX, 1, &tempRegValue);

  /* Mask the GPIO_SELECT field and returns the value according */
  tempRegValue &= 0xF8;
  if(tempRegValue == SPIRIT_GPIO_DIG_OUT_VDD)
  {
    level = HIGH;
  }
  else
  {
    level = LOW;
  }

  return level;

}


/**
 * @brief  Enables or Disables the MCU clock output.
 * @param  xNewState new state for the MCU clock output.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritGpioClockOutput(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Reads the MCU_CK_CONF register and mask the result to enable or disable the clock output */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  if(xNewState)
  {
    tempRegValue |= MCU_CK_ENABLE;
  }
  else
  {
    tempRegValue &= (~MCU_CK_ENABLE);
  }

  /* Writes the MCU_CK_CONF register */
  g_xStatus = SpiritSpiWriteRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

}


/**
 * @brief  Initializes the SPIRIT Clock Output according to the specified
 *         parameters in the xClockOutputInitStruct.
 * @param  pxClockOutputInitStruct pointer to a ClockOutputInit structure that
 *         contains the configuration information for the SPIRIT Clock Output.
 * @retval None.
 * @note   The function SpiritGpioClockOutput() must be called in order to enable
 *         or disable the MCU clock dividers.
 */
void SpiritGpioClockOutputInit(ClockOutputInit* pxClockOutputInitStruct)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_CLOCK_OUTPUT_XO(pxClockOutputInitStruct->xClockOutputXOPrescaler));
  s_assert_param(IS_SPIRIT_CLOCK_OUTPUT_RCO(pxClockOutputInitStruct->xClockOutputRCOPrescaler));
  s_assert_param(IS_SPIRIT_CLOCK_OUTPUT_EXTRA_CYCLES(pxClockOutputInitStruct->xExtraClockCycles));

  /* Calculates the register value to write according to the specified configuration */
  tempRegValue = ((uint8_t)(pxClockOutputInitStruct->xClockOutputXOPrescaler) | (uint8_t)(pxClockOutputInitStruct->xClockOutputRCOPrescaler) | \
           (uint8_t)(pxClockOutputInitStruct->xExtraClockCycles));

  /* Writes the MCU_CLOCK register */
  g_xStatus = SpiritSpiWriteRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

}


/**
 * @brief  Sets the XO ratio as clock output.
 * @param  xXOPrescaler the XO prescaler to be used as clock output.
 *         This parameter can be any value of @ref ClockOutputXOPrescaler .
 * @retval None
 */
void SpiritGpioSetXOPrescaler(ClockOutputXOPrescaler xXOPrescaler)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_CLOCK_OUTPUT_XO(xXOPrescaler));

  /* Reads the MCU_CLK_CONFIG register */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  /* Mask the XO_RATIO field and writes the new value */
  tempRegValue &= 0x61;
  tempRegValue |= ((uint8_t)xXOPrescaler);

  /* Writes the new XO prescaler in the MCU_CLOCK register */
  g_xStatus = SpiritSpiWriteRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the settled XO prescaler as clock output.
 * @param  None.
 * @retval ClockOutputXOPrescaler Settled XO prescaler used for clock
 *         output. This parameter can be a value of @ref ClockOutputXOPrescaler .
 */
ClockOutputXOPrescaler SpiritGpioGetXOPrescaler(void)
{
  uint8_t tempRegValue = 0x00;

  /* Reads the MCU_CLK_CONFIG register */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  /* Mask the XO_RATIO field and return the value */
  return ((ClockOutputXOPrescaler)(tempRegValue & 0x1E));

}


/**
 * @brief  Sets the RCO ratio as clock output
 * @param  xRCOPrescaler the RCO prescaler to be used as clock output.
 *         This parameter can be any value of @ref ClockOutputRCOPrescaler .
 * @retval None.
 */
void SpiritGpioSetRCOPrescaler(ClockOutputRCOPrescaler xRCOPrescaler)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_CLOCK_OUTPUT_RCO(xRCOPrescaler));

  /* Reads the MCU_CLK_CONFIG register */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  /* Mask the RCO_RATIO field and writes the new value */
  tempRegValue &= 0xFE;
  tempRegValue |= ((uint8_t)xRCOPrescaler);

  /* Writes the new RCO prescaler in the MCU_CLOCK register */
  g_xStatus = SpiritSpiWriteRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the settled RCO prescaler as clock output.
 * @param  None.
 * @retval ClockOutputRCOPrescaler Settled RCO prescaler used for clock
 *         output. This parameter can be a value of @ref ClockOutputRCOPrescaler.
 */
ClockOutputRCOPrescaler SpiritGpioGetRCOPrescaler(void)
{
  uint8_t tempRegValue = 0x00;

  /* Reads the MCU_CLK_CONFIG register */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  /* Mask the RCO_RATIO field and returns the value */
  return ((ClockOutputRCOPrescaler)(tempRegValue & 0x01));

}


/**
 * @brief  Sets the RCO ratio as clock output.
 * @param  xExtraCycles the number of extra clock cycles provided before switching
 *         to STANDBY state. This parameter can be any value of @ref ExtraClockCycles .
 * @retval None.
 */
void SpiritGpioSetExtraClockCycles(ExtraClockCycles xExtraCycles)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_CLOCK_OUTPUT_EXTRA_CYCLES(xExtraCycles));

  /* Reads the MCU_CLK_CONFIG register */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  /* Mask the CLOCK_TAIL field and writes the new value */
  tempRegValue &= 0x9F;
  tempRegValue |= ((uint8_t)xExtraCycles);

  /* Writes the new number of extra clock cycles in the MCU_CLOCK register */
  g_xStatus = SpiritSpiWriteRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the settled RCO prescaler as clock output.
 * @param  None.
 * @retval ExtraClockCycles Settled number of extra clock cycles
 *         provided before switching to STANDBY state. This parameter can be
 *         any value of @ref ExtraClockCycles .
 */
ExtraClockCycles SpiritGpioGetExtraClockCycles(void)
{
  uint8_t tempRegValue = 0x00;

  /* Reads the MCU_CLK_CONFIG register */
  g_xStatus = SpiritSpiReadRegisters(MCU_CK_CONF_BASE, 1, &tempRegValue);

  /* Mask the CLOCK_TAIL field and returns the value */
  return ((ExtraClockCycles)(tempRegValue & 0x60));

}


/**
 * @}
 */


/**
 * @}
 */


/**
 * @}
 */



/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
