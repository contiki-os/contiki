/**
******************************************************************************
* @file    radio_gpio.c
* @author  System Lab - NOIDA
* @version V1.0.0
* @date    15-May-2014 
* @brief   This file provides code for the configuration of all used GPIO pins 
           for Radio inetrface.
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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
#include "radio_gpio.h"

/**
 * @addtogroup BSP
 * @{
 */


/**
 * @addtogroup X-NUCLEO-IDS02Ax
 * @{
 */


/**
 * @defgroup Radio_Gpio_Private_TypesDefinitions       Radio_Gpio Private Types Definitions
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Radio_Gpio_Private_Defines                Radio_Gpio Private Defines
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Radio_Gpio_Private_Macros                 Radio_Gpio Private Macros
 * @{
 */
#define POR_TIME ((uint16_t)0x1E00)

/**
 * @}
 */


/**
 * @defgroup Radio_Gpio_Private_Variables              Radio_Gpio Private Variables
 * @{
 */

/**
* @brief  Radio_Gpio Port array
*/
GPIO_TypeDef* aRADIO_GPIO_PORT[RADIO_GPIO_NUMBER] = {
  RADIO_GPIO_0_PORT,
  RADIO_GPIO_1_PORT,
  RADIO_GPIO_2_PORT,
  RADIO_GPIO_3_PORT,
  RADIO_GPIO_SDN_PORT
};


/**
* @brief  Radio_Gpio Pin array
*/
static const uint16_t aRADIO_GPIO_PIN[RADIO_GPIO_NUMBER] = {
  RADIO_GPIO_0_PIN,
  RADIO_GPIO_1_PIN,
  RADIO_GPIO_2_PIN,
  RADIO_GPIO_3_PIN,
  RADIO_GPIO_SDN_PIN   
};


/**
* @brief  Radio_Gpio Speed array
*/
static const uint32_t aRADIO_GPIO_SPEED[RADIO_GPIO_NUMBER] = { 
  RADIO_GPIO_0_SPEED,
  RADIO_GPIO_1_SPEED,
  RADIO_GPIO_2_SPEED,
  RADIO_GPIO_3_SPEED,
  RADIO_GPIO_SDN_SPEED    
};


/**
* @brief  Radio_Gpio PuPd array
*/
static const uint32_t aRADIO_GPIO_PUPD[RADIO_GPIO_NUMBER] = {
  RADIO_GPIO_0_PUPD,
  RADIO_GPIO_1_PUPD,
  RADIO_GPIO_2_PUPD,
  RADIO_GPIO_3_PUPD,
  RADIO_GPIO_SDN_PUPD   
};


/**
* @brief  Exti Mode array
*/
static const uint32_t aRADIO_GPIO_EXTI_MODE[RADIO_GPIO_NUMBER-1] = {
  RADIO_GPIO_0_EXTI_MODE,
  RADIO_GPIO_1_EXTI_MODE,
  RADIO_GPIO_2_EXTI_MODE,
  RADIO_GPIO_3_EXTI_MODE     
};


/**
* @brief  Exti IRQn array
*/
static const uint8_t aRADIO_GPIO_IRQn[RADIO_GPIO_NUMBER-1] = {
  RADIO_GPIO_0_EXTI_IRQN,
  RADIO_GPIO_1_EXTI_IRQN,
  RADIO_GPIO_2_EXTI_IRQN,
  RADIO_GPIO_3_EXTI_IRQN  
};


/**
 * @}
 */


/**
 * @defgroup Radio_Gpio_Private_FunctionPrototypes     Radio_Gpio Private Function Prototypes
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Radio_Gpio_Private_Functions              Radio_Gpio Private Functions
 * @{
 */
 

/**
* @brief  Configures MCU GPIO and EXTI Line for GPIOs.
* @param  xGpio Specifies the GPIO to be configured.
*         This parameter can be one of following parameters:
*         @arg GPIO_0
*         @arg GPIO_1
*         @arg GPIO_2
*         @arg GPIO_3
* @param  xGpioMode Specifies GPIO mode.
*         This parameter can be one of following parameters:
*         @arg RADIO_MODE_GPIO_IN: MCU GPIO will be used as simple input.
*         @argRADIO_MODE_GPIO_OUT: MCU GPIO will be used as simple output.
*         @arg RADIO_MODE_EXTI_IN: MCU GPIO will be connected to EXTI line with interrupt
*         generation capability.
* @retval None.
*/
void RadioGpioInit(RadioGpioPin xGpio, RadioGpioMode xGpioMode)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* Check the parameters */
  assert_param(IS_RADIO_GPIO_PIN(xGpio));
  assert_param(IS_RADIO_GPIO_MODE(xGpioMode));
  
  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  
  /* Configures MCU GPIO */
  if (xGpioMode == RADIO_MODE_GPIO_OUT)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  }
  else
  {
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  }
  
  GPIO_InitStruct.Pin = aRADIO_GPIO_PIN[xGpio];
  GPIO_InitStruct.Pull = aRADIO_GPIO_PUPD[xGpio];
  GPIO_InitStruct.Speed = aRADIO_GPIO_SPEED[xGpio];
  HAL_GPIO_Init(aRADIO_GPIO_PORT[xGpio], &GPIO_InitStruct);
  
  if (xGpioMode == RADIO_MODE_EXTI_IN)
  {
    GPIO_InitStruct.Pin = aRADIO_GPIO_PIN[xGpio];
    GPIO_InitStruct.Pull = aRADIO_GPIO_PUPD[xGpio];
    GPIO_InitStruct.Speed = aRADIO_GPIO_SPEED[xGpio];
    GPIO_InitStruct.Mode = aRADIO_GPIO_EXTI_MODE[xGpio];
    HAL_GPIO_Init(aRADIO_GPIO_PORT[xGpio], &GPIO_InitStruct);
    
  /* Enable and set Button EXTI Interrupt to the lowest priority */
  /*  NVIC_SetPriority((IRQn_Type)(aRADIO_GPIO_IRQn[xGpio]), 0x02); */
  /*  HAL_NVIC_EnableIRQ((IRQn_Type)(aRADIO_GPIO_IRQn[xGpio]));     */
  } 
}


/**
* @brief  Enables or disables the interrupt on GPIO .
* @param  xGpio Specifies the GPIO whose priority shall be changed.
*         This parameter can be one of following parameters:
*         @arg GPIO_0
*         @arg GPIO_1
*         @arg GPIO_2
*         @arg GPIO_3
* @param  nPreemption Specifies Preemption Priority.
* @param  nSubpriority Specifies Subgroup Priority.
* @param  xNewState Specifies the State.
*         This parameter can be one of following parameters:
*         @arg ENABLE: Interrupt is enabled
*         @arg DISABLE: Interrupt is disabled
* @retval None.
*/
void RadioGpioInterruptCmd(RadioGpioPin xGpio, uint8_t nPreemption, uint8_t nSubpriority, FunctionalState xNewState)
{
  HAL_NVIC_SetPriority((IRQn_Type) (aRADIO_GPIO_IRQn[xGpio]), nPreemption, nSubpriority);
  if (!xNewState)
  {
       HAL_NVIC_DisableIRQ((IRQn_Type)(aRADIO_GPIO_IRQn[xGpio])); 
  }
  else
  {
        HAL_NVIC_EnableIRQ((IRQn_Type)(aRADIO_GPIO_IRQn[xGpio]));
  }
}


/**
* @brief  Returns the level of a specified GPIO.
* @param  xGpio Specifies the GPIO to be read.
*         This parameter can be one of following parameters:
*         @arg GPIO_0
*         @arg GPIO_1
*         @arg GPIO_2
*         @arg GPIO_3
* @retval FlagStatus Level of the GPIO. This parameter can be:
*         SET or RESET.
*/
FlagStatus RadioGpioGetLevel(RadioGpioPin xGpio)
{
  /* Gets the GPIO level */
  uint16_t nDataPort = HAL_GPIO_ReadPin(aRADIO_GPIO_PORT[xGpio], aRADIO_GPIO_PIN[xGpio]);
  if (nDataPort & aRADIO_GPIO_PIN[xGpio])
  {
    return SET;
  }
  else
  {
    return RESET; 
  }
}


/**
* @brief  Sets the level of a specified GPIO.
* @param  xGpio Specifies the GPIO to be set.
*         This parameter can be one of following parameters:
*         @arg GPIO_0
*         @arg GPIO_1
*         @arg GPIO_2
*         @arg GPIO_3
* @param  GPIO_PinState Level of the GPIO. This parameter can be:
*         GPIO_PIN_SET or GPIO_PIN_RESET.
* @retval None.
*/
void RadioGpioSetLevel(RadioGpioPin xGpio, GPIO_PinState xState)
{
  /* Sets the GPIO level */
  HAL_GPIO_WritePin(aRADIO_GPIO_PORT[xGpio], aRADIO_GPIO_PIN[xGpio], xState); 
}


/**
* @brief  Puts at logic 1 the SDN pin.
* @param  None.
* @retval None.
*/
void SdkEvalEnterShutdown(void)
{
  /* Puts high the GPIO connected to shutdown pin */
  /* Check the parameters */ 
  RadioGpioSetLevel(RADIO_GPIO_SDN, GPIO_PIN_SET);
}


/**
* @brief  Put at logic 0 the SDN pin.
* @param  None.
* @retval None.
*/
void SdkEvalExitShutdown(void)
{
  /* Puts low the GPIO connected to shutdown pin */
  RadioGpioSetLevel(RADIO_GPIO_SDN, GPIO_PIN_RESET);

  /* Delay to allow the circuit POR, about 700 us */
  for (volatile uint32_t Index = 0; Index < POR_TIME; Index++);
}


/**
* @brief  check the logic(0 or 1) at the SDN pin.
* @param  None.
* @retval FlagStatus.
*/
SpiritFlagStatus SdkEvalCheckShutdown(void)
{
  return RadioGpioGetLevel(RADIO_GPIO_SDN);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
