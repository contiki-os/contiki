/**
******************************************************************************
* @file    stm32l1xx_it.c
* @author  System LAB
* @version V1.0.0
* @date    17-June-2015
* @brief   Main Interrupt Service Routines
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
#include "stm32cube_hal_init.h"
#include "stm32l1xx_it.h"
#include "stm32l1xx_nucleo.h"
#include "radio_gpio.h"
#include "spirit1.h"    


extern UART_HandleTypeDef UartHandle;    
/** @addtogroup STM32L1xx_HAL_Examples
* @{
*/

/** @addtogroup Templates
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
extern I2C_HandleTypeDef I2cHandle;
/**
* @brief   This function handles NMI exception.
* @param  None
* @retval None
*/
void NMI_Handler(void)
{
}
void WWDG_IRQHandler(void)
{
  while(1);
}
/**
* @brief  This function handles Hard Fault exception.
* @param  None
* @retval None
*/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles Memory Manage exception.
* @param  None
* @retval None
*/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles Bus Fault exception.
* @param  None
* @retval None
*/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles Usage Fault exception.
* @param  None
* @retval None
*/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles SVCall exception.
* @param  None
* @retval None
*/
void SVC_Handler(void)
{
}

/**
* @brief  This function handles Debug Monitor exception.
* @param  None
* @retval None
*/
void DebugMon_Handler(void)
{
}

/**
* @brief  This function handles PendSVC exception.
* @param  None
* @retval None
*/
void PendSV_Handler(void)
{
}


/******************************************************************************/
/*                 STM32L1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx.s).                                               */
/******************************************************************************/
/**
* @brief  This function handles I2C event interrupt request.  
* @param  None
* @retval None
* @Note   This function is redefined in "stm32cube_hal_init.h" and related to I2C data transmission     
*/
void I2Cx_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(& I2cHandle);
}

/**
* @brief  This function handles I2C error interrupt request.
* @param  None
* @retval None
* @Note   This function is redefined in "stm32cube_hal_init.h" and related to I2C error
*/
void I2Cx_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(& I2cHandle);
}


/**
* @brief  This function handles External lines 15 to 4 interrupt request.
* @param  None
* @retval None
*/
void EXTI0_IRQHandler(void)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) 
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
  }
  while(1);
}

void EXTI1_IRQHandler(void)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET) 
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
  }  
  while(1);
}

void EXTI2_IRQHandler(void)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET) 
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
  }
  while(1);
}

void EXTI3_IRQHandler(void)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET) 
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
  }
  while(1);
}


/**
* @brief  This function handles External lines 15 to 4 interrupt request.
* @param  None
* @retval None
*/
void EXTI9_5_IRQHandler(void)
{
  /* EXTI line 7 interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(RADIO_GPIO_3_EXTI_LINE))
  {
    __HAL_GPIO_EXTI_CLEAR_IT(RADIO_GPIO_3_EXTI_LINE);
    
    HAL_GPIO_EXTI_Callback(RADIO_GPIO_3_EXTI_LINE);
    
    spirit1_interrupt_callback();
  }
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
  
  
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
  
#ifndef  LPM_ENABLE
  
  //  if(__HAL_GPIO_EXTI_GET_IT(KEY_BUTTON_EXTI_LINE) != RESET) 
  //  { 
  //    __HAL_GPIO_EXTI_CLEAR_IT(KEY_BUTTON_EXTI_LINE);
  //    
  // Set_KeyStatus(SET);
  //  }
  //  
#else /*Low Power mode enabled*/ 
  
#if defined(RF_STANDBY)/*if spirit1 is in standby*/
  
  if(EXTI->PR & KEY_BUTTON_EXTI_LINE)
  {
    HAL_GPIO_EXTI_Callback(KEY_BUTTON_EXTI_LINE);
    /* EXTI line 13 interrupt detected */
    if(HAL_GPIO_ReadPin(KEY_BUTTON_GPIO_PORT, KEY_BUTTON_PIN) == 0x01) //0x00
    {
      HAL_GPIO_EXTI_Callback(KEY_BUTTON_EXTI_LINE);
      
      PushButtonStatusWakeup = SET;
      PushButtonStatusData = RESET;
      wakeupCounter = LPM_WAKEUP_TIME; 
      dataSendCounter = DATA_SEND_TIME;
      dataSendCounter++;
    }
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_BUTTON_EXTI_LINE);
  } 
#else /*if spirit1 is not in standby or sleep mode but MCU is in LPM*/
  
  if(__HAL_GPIO_EXTI_GET_IT(KEY_BUTTON_EXTI_LINE) != RESET) 
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_BUTTON_EXTI_LINE);
    
    HAL_GPIO_EXTI_Callback(KEY_BUTTON_EXTI_LINE);
    
    Set_KeyStatus(SET);
  }
#endif
#endif
}



/**
* @brief  This function handles EXTI15_10_IRQHandler
* @param  None
* @retval None
*/
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(USER_BUTTON_PIN);
}


/**
* @brief  This function handles UART interrupt request.  
* @param  None
* @retval None
* @Note   This function is redefined in "stm32cube_hal_init.h" and related to DMA  
*         used for USART data transmission     
*/

void USART2_IRQHandler()
{
	UART_HandleTypeDef *huart = &UartHandle;

	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_PE)){
		__HAL_UART_CLEAR_PEFLAG(huart);
	}
	
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_FE)){
		__HAL_UART_CLEAR_FEFLAG(huart);
	}
	
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_NE)){
		__HAL_UART_CLEAR_NEFLAG(huart);  
	}
	
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)){
		__HAL_UART_CLEAR_OREFLAG(&UartHandle);
	}
	
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE)){
		slip_input_byte(UartHandle.Instance->DR);
		__HAL_UART_GET_IT_SOURCE(&UartHandle, UART_IT_RXNE);
	}
}

/**
* @brief  UART error callbacks
* @param  UartHandle: UART handle
* @note   This example shows a simple way to report transfer error, and you can
*         add your own implementation.
* @retval None
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  // Error_Handler();
}


/**
* @brief  Tx Transfer completed callback
* @param  UartHandle: UART handle. 
* @note   This example shows a simple way to report end of IT Tx transfer, and 
*         you can add your own implementation. 
* @retval None
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  
  
}


/**
* @}
*/ 

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
