/** @file /hal/micro/button.h
 * @brief Header for button driver
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef _BUTTON_H_
#define _BUTTON_H_

/* button status */
#define BUTTON_PRESSED   0
#define BUTTON_RELEASED  1
#define BUTTON_UNKNOWN   3


typedef int8u HalBoardButton;

/* Functions -----------------------------------------------------------------*/

/** @brief Init buttons */
void halInitButton(void);

/** @brief Get button status */
int8u halGetButtonStatus(HalBoardButton button);

#endif /* _BUTTON_H_ */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
