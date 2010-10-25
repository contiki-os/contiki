/** @file hal/micro/cortexm3/led.c
 *  @brief LED manipulation routines; stack and example APIs
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include BOARD_HEADER
#include "hal/micro/led.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

#define GPIO_PxCLR_BASE (GPIO_PACLR_ADDR)
#define GPIO_PxSET_BASE (GPIO_PASET_ADDR)
#define GPIO_PxOUT_BASE (GPIO_PAOUT_ADDR)
// Each port is offset from the previous port by the same amount
#define GPIO_Px_OFFSET  (GPIO_PBCFGL_ADDR-GPIO_PACFGL_ADDR)

void halInitLed(void)
{
  /* Set GPIO pins for Led D1 and Led D3  */
  halGpioConfig(LED_D1, GPIOCFG_OUT);
  halGpioConfig(LED_D3, GPIOCFG_OUT);
  /* Switch off Led D1,D3 */
  halClearLed(LED_D1);
  halClearLed(LED_D3);
}

void halSetLed(HalBoardLed led)
{
  if(led/8 < 3) {
    *((volatile int32u *)(GPIO_PxCLR_BASE+(GPIO_Px_OFFSET*(led/8)))) = BIT(led&7);
  }
}

void halClearLed(HalBoardLed led)
{
  if(led/8 < 3) {
    *((volatile int32u *)(GPIO_PxSET_BASE+(GPIO_Px_OFFSET*(led/8)))) = BIT(led&7);
  }
}

void halToggleLed(HalBoardLed led)
{
  //to avoid contention with other code using the other pins for other
  //purposes, we disable interrupts since this is a read-modify-write
  ATOMIC(
    if(led/8 < 3) {
      *((volatile int32u *)(GPIO_PxOUT_BASE+(GPIO_Px_OFFSET*(led/8)))) ^= BIT(led&7);
    }
  )
}
