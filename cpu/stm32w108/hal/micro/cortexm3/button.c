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
  uint8_t i;
  /* Configure GPIO for BUTTONSs */
  ButtonResourceType *buttons = (ButtonResourceType *) boardDescription->io->buttons;
  for (i = 0; i < boardDescription->buttons; i++) {
    halGpioConfig(PORTx_PIN(buttons[i].gpioPort, buttons[i].gpioPin), GPIOCFG_IN_PUD);
    halGpioSet(PORTx_PIN(buttons[i].gpioPort, buttons[i].gpioPin), GPIOOUT_PULLUP);
  }
}/* end halInitButton() */


uint8_t halGetButtonStatus(HalBoardButton button)
{
  uint8_t port = (button >> 3) & 0xf;
  uint8_t pin = button & 0x7;

  if (button != DUMMY_BUTTON)
  {
    return (BUTTON_INPUT_GPIO(port) & (1 << pin)) ? BUTTON_RELEASED : BUTTON_PRESSED;
  }
  return BUTTON_UNKNOWN;
}/* end halGetButtonStatus()*/

