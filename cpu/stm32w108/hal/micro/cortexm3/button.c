/** @file /hal/micro/cortexm3/button.c
 * @brief button APIs
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include BOARD_HEADER
#include "hal/micro/button.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

void halInitButton(void)
{
   /* Set GPIO pin to PUD (input pull-up or pull-down) for button S1 */
  halGpioConfig(BUTTON_S1,GPIOCFG_IN_PUD);
 /* Set the button S1 gpio pin to pull-up */
  BUTTON_S1_OUTPUT_GPIO |= GPIOOUT_PULLUP << BUTTON_S1_GPIO_PIN;
}/* end halInitButton() */


int8u halGetButtonStatus(HalBoardButton button)
{
  if (button == BUTTON_S1)
    return (BUTTON_S1_INPUT_GPIO & (1<<BUTTON_S1_GPIO_PIN)) ? BUTTON_RELEASED : BUTTON_PRESSED;
  else 
    return BUTTON_UNKNOWN;
}/* end halGetButtonStatus()*/

