/** @file /hal/micro/button.h
 * @brief Header for button driver
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef BUTTON_H_
#define BUTTON_H_

/* button status */
#define BUTTON_PRESSED   0
#define BUTTON_RELEASED  1
#define BUTTON_UNKNOWN   3


typedef uint8_t HalBoardButton;

/* Functions -----------------------------------------------------------------*/

/** @brief Init buttons */
void halInitButton(void);

/** @brief Get button status */
uint8_t halGetButtonStatus(HalBoardButton button);

#endif /*BUTTON_H_ */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
