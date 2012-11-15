/** @file micro-common.h
 * @brief Minimal Hal functions common across all microcontroller-specific files.
 * See @ref micro for documentation.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
 
/** @addtogroup micro
 * Many of the supplied example applications use these microcontroller functions.
 * See hal/micro/micro-common.h for source code.
 *
 *@{
 */

#ifndef __MICRO_COMMON_H__
#define __MICRO_COMMON_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifndef __STSTATUS_TYPE__
#define __STSTATUS_TYPE__
  //This is necessary here because halSleepForQsWithOptions returns an
  //StStatus and not adding this typedef to this file breaks a
  //whole lot of builds.
  typedef int8u StStatus;
#endif //__STSTATUS_TYPE__
#endif // DOXYGEN_SHOULD_SKIP_THIS

/** @brief Initializes microcontroller-specific peripherals.
*/
void halInit(void);

/** @brief Restarts the microcontroller and therefore everything else.
*/
void halReboot(void);

/** @brief Powers up microcontroller peripherals and board peripherals.
*/
void halPowerUp(void);

/** @brief Powers down microcontroller peripherals and board peripherals.
*/
void halPowerDown(void);

/** @brief The value that must be passed as the single parameter to 
 *  ::halInternalDisableWatchDog() in order to sucessfully disable the watchdog 
 *  timer.
 */ 
#define MICRO_DISABLE_WATCH_DOG_KEY 0xA5

/** @brief Enables the watchdog timer.
 */
void halInternalEnableWatchDog(void);

/** @brief Disables the watchdog timer.
 *
 * @note To prevent the watchdog from being disabled accidentally, 
 * a magic key must be provided.
 * 
 * @param magicKey  A value (::MICRO_DISABLE_WATCH_DOG_KEY) that enables the function.
 */
void halInternalDisableWatchDog(int8u magicKey);

/** @brief Determines whether the watchdog has been enabled or disabled.
 *
 * @return A boolean value indicating if the watchdog is current enabled.
 */
boolean halInternalWatchDogEnabled( void );

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Enumerations for the possible microcontroller sleep modes.
 * - SLEEPMODE_RUNNING
 *     Everything is active and running.  In practice this mode is not
 *     used, but it is defined for completeness of information.
 * - SLEEPMODE_IDLE
 *     Only the CPU is idled.  The rest of the chip continues runing
 *     normally.  The chip will wake from any interrupt.
 * - SLEEPMODE_WAKETIMER
 *     The sleep timer clock sources remain running.  The RC is always
 *     running and the 32kHz XTAL depends on the board header.  Wakeup
 *     is possible from both GPIO and the sleep timer.  System time
 *     is maintained.  The sleep timer is assumed to be configured
 *     properly for wake events.
 * - SLEEPMODE_MAINTAINTIMER
 *     The sleep timer clock sources remain running.  The RC is always
 *     running and the 32kHz XTAL depends on the board header.  Wakeup
 *     is possible from only GPIO.  System time is maintained.
 * - SLEEPMODE_NOTIMER
 *     The sleep timer clock sources (both RC and XTAL) are turned off.
 *     Wakeup is possible from only GPIO.  System time is lost.
 */
enum SleepModes
#else
typedef int8u SleepModes;
enum
#endif
{
  SLEEPMODE_RUNNING = 0,
  SLEEPMODE_IDLE = 1,
  SLEEPMODE_WAKETIMER = 2,
  SLEEPMODE_MAINTAINTIMER = 3,
  SLEEPMODE_NOTIMER = 4,
};

/** @brief Blocks the current thread of execution for the specified
 * amount of time, in microseconds.
 *
 * The function is implemented with cycle-counted busy loops
 * and is intended to create the short delays required when interfacing with
 * hardware peripherals.
 *
 * The accuracy of the timing provided by this function is not specified,
 * but a general rule is that when running off of a crystal oscillator it will
 * be within 10us.  If the micro is running off of another type of oscillator
 * (e.g. RC) the timing accuracy will potentially be much worse.
 *
 * @param us  The specified time, in microseconds. 
              Values should be between 1 and 65535 microseconds.
 */
void halCommonDelayMicroseconds(int16u us);

/** @brief Request the appplication to enter in bootloader mode
 *
 * This function will check whwther the user flash contains the bootloader
 * and if yes it will jump into it according to the user parameters.
 * 
 *
 * @param mode  The bootloader mode, 0 UART mode, 1 RF mode. All other
 * values are reserved
 * @param channel  The channel where the booloader will operate. 0 means
 * default channel (only vaild for RF mode).
 * @param panID  The panID where the booloader will operate. 0xFFFF means
 * default panID (only vaild for RF mode).
 * @return An error code or it will never return.
 */
StStatus halBootloaderStart(int8u mode, int8u channel, int16u panId);

#ifdef CORTEXM3_STM32F103
#include "micro/cortexm3/stm32f103ret/micro-specific.h"
#endif
#ifdef CORTEXM3_STM32W108
#include "micro/cortexm3/micro-common.h"
#endif

#endif //__MICRO_COMMON_H__

/** @} END micro group  */
  
