/**
 * \addtogroup stm32w-cpu
 *
 * @{
 */

/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32w108_systick.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file provides all the SysTick firmware functions.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include <stdint.h>
#include "stm32w-systick.h"

/* SysTick registers bit mask */
/* CTRL TICKINT Mask */
#define CTRL_TICKINT_Set      ((uint32_t)0x00000002)
#define CTRL_TICKINT_Reset    ((uint32_t)0xFFFFFFFD)
/*--------------------------------------------------------------------------*/
/**
 * \brief      Configures the SysTick clock source.
 * \param SysTick_CLKSource specifies the SysTick clock source.
 *             This parameter can be one of the following values:
 *                    SysTick_CLKSource_HCLK_Div8: AHB clock divided by 8
 *                    selected as SysTick clock source.
 *                    SysTick_CLKSource_HCLK: AHB clock selected as
 *                    SysTick clock source.
 */
void
SysTick_CLKSourceConfig(uint32_t SysTick_CLKSource)
{
  /* Check the parameters */
  assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));

  if(SysTick_CLKSource == SysTick_CLKSource_HCLK) {
    SysTick->CTRL |= SysTick_CLKSource_HCLK;
  } else {
    SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;
  }
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Sets SysTick Reload value.
 * \param Reload SysTick Reload new value. Must be between 1 and 0xFFFFFF.
 */
void
SysTick_SetReload(uint32_t Reload)
{
  /* Check the parameters */
  assert_param(IS_SYSTICK_RELOAD(Reload));
  SysTick->LOAD = Reload;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Enables or disables the SysTick counter.
 * \param SysTick_Counter new state of the SysTick counter.
*                    This parameter can be one of the following values:
*                       - SysTick_Counter_Disable: Disable counter
*                       - SysTick_Counter_Enable: Enable counter
*                       - SysTick_Counter_Clear: Clear counter value to 0
 */
void
SysTick_CounterCmd(uint32_t SysTick_Counter)
{
  /* Check the parameters */
  assert_param(IS_SYSTICK_COUNTER(SysTick_Counter));

  if(SysTick_Counter == SysTick_Counter_Enable) {
    SysTick->CTRL |= SysTick_Counter_Enable;
  } else if(SysTick_Counter == SysTick_Counter_Disable) {
    SysTick->CTRL &= SysTick_Counter_Disable;
  } else {                      /* SysTick_Counter == SysTick_Counter_Clear */
    SysTick->VAL = SysTick_Counter_Clear;
  }
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Enables or disables the SysTick Interrupt.
 * \param NewState new state of the SysTick Interrupt.
*                    This parameter can be: ENABLE or DISABLE.
 */
void
SysTick_ITConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if(NewState != DISABLE) {
    SysTick->CTRL |= CTRL_TICKINT_Set;
  } else {
    SysTick->CTRL &= CTRL_TICKINT_Reset;
  }
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Gets SysTick counter value.
 * \return     SysTick current value
 */
uint32_t
SysTick_GetCounter(void)
{
  return (SysTick->VAL);
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Checks whether the specified SysTick flag is set or not.
 * \param SysTick_FLAG specifies the flag to check.
*                    This parameter can be one of the following values:
*                       - SysTick_FLAG_COUNT
*                       - SysTick_FLAG_SKEW
*                       - SysTick_FLAG_NOREF
 */
FlagStatus
SysTick_GetFlagStatus(uint8_t SysTick_FLAG)
{
  uint32_t statusreg = 0, tmp = 0;
  FlagStatus bitstatus = RESET;

  /* Check the parameters */
  assert_param(IS_SYSTICK_FLAG(SysTick_FLAG));

  /* Get the SysTick register index */
  tmp = SysTick_FLAG >> 3;

  if(tmp == 2) {                /* The flag to check is in CTRL register */
    statusreg = SysTick->CTRL;
  } else {                      /* The flag to check is in CALIB register */

    statusreg = SysTick->CALIB;
  }

  if((statusreg & ((uint32_t) 1 << SysTick_FLAG)) != (uint32_t) RESET) {
    bitstatus = SET;
  } else {
    bitstatus = RESET;
  }
  return bitstatus;
}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
/** @} */
