/** @file hal/micro/led.h
 *  @brief Header for led APIs
 *  
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

/** @addtogroup led 
 *  @brief  Sample API funtions for controlling LEDs.
 *
 * When specifying an LED to use, always use the BOARDLEDx definitions that
 * are defined within the BOARD_HEADER.
 * 
 * See led.h for source code.
 *@{
 */

 
/** @brief Configures GPIOs pertaining to the control of LEDs.
 */
void halInitLed(void);

/** @brief Ensures that the definitions from the BOARD_HEADER
 *  are always used as parameters to the LED functions.
 */
  typedef int8u HalBoardLed;
// Note: Even though many compilers will use 16 bits for an enum instead of 8, 
//  we choose to use an enum here.  The possible compiler inefficiency does not 
//  affect stack-based parameters and local variables, which is the
//  general case for led paramters.

/** @brief Atomically wraps an XOR or similar operation for a single GPIO
 *  pin attached to an LED.
 * 
 *  @param led  Identifier (from BOARD_HEADER) for the LED to be toggled.
 */
void halToggleLed(HalBoardLed led);

/** @brief Turns on (sets) a GPIO pin connected to an LED so that the LED 
 *  turns on.
 * 
 *  @param led  Identifier (from BOARD_HEADER) for the LED to turn on.
 */
void halSetLed(HalBoardLed led);

/** @brief Turns off (clears) a GPIO pin connected to an LED, which turns 
 *  off the LED.
 *  
 *  @param led  Identifier (from BOARD_HEADER) for the LED to turn off.
 */
void halClearLed(HalBoardLed led);

/** @brief Called by the stack to indicate activity over the radio (for 
 *  both transmission and reception). It is called once with \c turnOn TRUE and 
 *  shortly thereafter with \c turnOn FALSE.
 *
 *  Typically does something interesting, such as change the state of
 *  an LED.
 *
 *  @param turnOn  See Usage.
 */
void halStackIndicateActivity(boolean turnOn);

/** @} // END addtogroup 
 */

