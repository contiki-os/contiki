/** @file hal/micro/system-timer.h
 *  @brief Header file for system_timer APIs
 * 
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->

 */

/** @addtogroup system_timer
 * @brief Functions that provide access to the system clock.
 *
 * A single system tick (as returned by ::halCommonGetInt16uMillisecondTick() and
 * ::halCommonGetInt32uMillisecondTick() ) is approximately 1 millisecond.
 *
 * - When used with a 32.768kHz crystal, the system tick is 0.976 milliseconds.
 *
 * - When used with a 3.6864MHz crystal, the system tick is 1.111 milliseconds.
 *
 * A single quarter-second tick (as returned by
 * ::halCommonGetInt16uQuarterSecondTick() ) is approximately 0.25 seconds.
 *
 * The values used by the time support functions will wrap after an interval.
 * The length of the interval depends on the length of the tick and the number
 * of bits in the value. However, there is no issue when comparing time deltas
 * of less than half this interval with a subtraction, if all data types are the
 * same.
 *
 * See system-timer.h for source code.
 *@{
 */

#ifndef SYSTEM_TIMER_H_
#define SYSTEM_TIMER_H_

/**
 * @brief Initializes the system tick.
 *
 * @return Time to update the async registers after RTC is started (units of 100 
 * microseconds).
 */
uint16_t halInternalStartSystemTimer(void);


/**
 * @brief Returns the current system time in system ticks, as a 16-bit
 * value.
 *
 * @return The least significant 16 bits of the current system time, in system
 * ticks.
 */
uint16_t halCommonGetInt16uMillisecondTick(void);

/**
 * @brief Returns the current system time in system ticks, as a 32-bit
 * value.
 *
 * @nostackusage
 *
 * @return The least significant 32 bits of the current system time, in 
 * system ticks.
 */
uint32_t halCommonGetInt32uMillisecondTick(void);

/**
 * @brief Returns the current system time in quarter second ticks, as a
 * 16-bit value.
 *
 * @nostackusage
 *
 * @return The least significant 16 bits of the current system time, in system
 * ticks multiplied by 256.
 */
uint16_t halCommonGetInt16uQuarterSecondTick(void);

#endif //SYSTEM_TIMER_H_

/**@} //END addtogroup 
 */




