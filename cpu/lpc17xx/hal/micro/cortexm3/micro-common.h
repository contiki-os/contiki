/** @file hal/micro/cortexm3/micro-common.h
 * @brief Utility and convenience functions for STM32W108 microcontroller,
 *        common to both the full and minimal hal.
 * See @ref micro for documentation.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

/** @addtogroup micro
 * See also hal/micro/cortexm3/micro.h for source code.
 *@{
 */

#ifndef __STM32W108XX_MICRO_COMMON_H__
#define __STM32W108XX_MICRO_COMMON_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifndef __STSTATUS_TYPE__
#define __STSTATUS_TYPE__
  //This is necessary here because halSleepForQsWithOptions returns an
  //StStatus and not adding this typedef to this file breaks a
  //whole lot of builds.
  typedef int8u StStatus;
#endif //__STSTATUS_TYPE__
#endif // DOXYGEN_SHOULD_SKIP_THIS

#define PORTA (0 << 3)
#define PORTB (1 << 3)
#define PORTC (2 << 3)

/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port A pins into a single number.
 */
#define PORTA_PIN(y) (PORTA|y)
/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port B pins into a single number.
 */
#define PORTB_PIN(y) (PORTB|y)
/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port C pins into a single number.
 */
#define PORTC_PIN(y) (PORTC|y)

/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port C pins into a single number.
 */
#define PORTx_PIN(x, y) (x|y)

/**
 * @brief Resets the watchdog timer.  This function is pointed
 * to by the macro ::halResetWatchdog(). 
 * @warning Be very careful when using this as you can easily get into an 
 * infinite loop.
 */
void halInternalResetWatchDog( void );


/**
 * @brief Configure an IO pin's operating mode
 *
 * @param io  The io pin to use, can be specified with the convenience macros
 *            PORTA_PIN(), PORTB_PIN(), PORTC_PIN()
 * @param config   The configuration mode to use.
 *
 */
void halGpioConfig(int32u io, int32u config);

/**
 * @brief Set/Clear single GPIO bit
 *
 * @param io  The io pin to use, can be specified with the convenience macros
 *            PORTA_PIN(), PORTB_PIN(), PORTC_PIN()
 * @param value   A flag indicating whether to set or clear the io.
 *
 */
void halGpioSet(int32u io, boolean value);


/**
 * @brief Calibrates the internal SlowRC to generate a 1024 Hz (1kHz) clock.
 */
void halInternalCalibrateSlowRc( void );

/**
 * @brief Calibrates the internal FastRC to generate a 12Mhz clock.
 */
void halInternalCalibrateFastRc(void);


/**
 * @brief Sets the trim values for the 1.8V and 1.2V regulators based upon
 * manufacturing configuration.
 *
 * @param boostMode  Alter the regulator trim based upon the state
 * of boost mode.  TRUE if boost mode is active, FALSE otherwise.
 */
void halInternalSetRegTrim(boolean boostMode);

/** @brief Takes a slow ADC measurement of VDD_PADS in millivolts.  Due to
 * the conversions performed, this function takes slightly under 3.2ms with a
 * variation across successive conversions approximately +/-2mv of the average
 * conversion.
 *
 * @return A slow measurement of VDD_PADS in millivolts.
 */
int16u stMeasureVddSlow(void);


/** @brief Takes a fast ADC measurement of VDD_PADS in millivolts.
 * Due to the conversions performed, this function takes slightly under 150us
 * with a variation across successive conversions approximately +/-20mv of
 * the average conversion.
 *
 * @return A fast measurement of VDD_PADS in millivolts.
 */
int16u stMeasureVddFast(void);


/**
 * @brief Calibrates the GPIO pads.  This function is called from within
 * the stack and HAL at appropriate times.
 */
void halCommonCalibratePads(void);

/**
 * @brief This function is intended to be called periodically, from the
 * stack and application, to check the XTAL bias trim is within
 * appropriate levels and adjust if not.  This function is *not* designed
 * to be used before halInternalSwitchToXtal() has been called.
 */
void halCommonCheckXtalBiasTrim(void);

/**
 * @brief Switches to running off of the 24MHz crystal, including changing
 * the CPU to be 24MHz (FCLK sourced from SYSCLK).  The switch function
 * will respect the BIASTRIM HI and LO flags and adjust bias trim until
 * appropriate crystal biasing is used.  This function is called from
 * within the stack and HAL at appropriate times.
 */
void halInternalSwitchToXtal(void);

/**
 * @brief Search for optimal 24MHz crystal bias trim, assuming no valid
 * prior value.  This function is typically called during initialization
 * of the microcontroller.
 */
void halInternalSearchForBiasTrim(void);

/** @brief Blocks the current thread of execution for the specified
 * amount of time, in milliseconds.
 *
 * The function is implemented with cycle-counted busy loops
 * and is intended to create the short delays required when interfacing with
 * hardware peripherals.  This function works by simply adding another
 * layer on top of halCommonDelayMicroseconds().
 *
 * @param ms  The specified time, in milliseconds. 
 */
void halCommonDelayMilliseconds(int16u ms);


/** @brief Puts the microcontroller to sleep in a specified mode, allows
 * the GPIO wake sources to be determined at runtime.  This function 
 * requires the GPIO wake sources to be defined at compile time in the board
 * file.
 *
 * @note This routine always enables interrupts.
 *
 * @param sleepMode  A microcontroller sleep mode.
 *
 * @param gpioWakeBitMask  A bit mask of the GPIO that are allowed to wake
 * the chip from deep sleep.  A high bit in the mask will enable waking
 * the chip if the corresponding GPIO changes state.  bit0 is PA0, bit1 is
 * PA1, bit8 is PB0, bit16 is PCO, bit23 is PC7, bits[31:24] are ignored.
 * 
 * @sa ::SleepModes
 */
void halSleepWithOptions(SleepModes sleepMode, int32u gpioWakeBitMask);


/**
 * @brief Uses the system timer to enter ::SLEEPMODE_WAKETIMER for
 * approximately the specified amount of time (provided in quarter seconds),
 * the GPIO wake sources can be provided at runtime.
 *
 * This function returns ::ST_SUCCESS and the duration parameter is
 * decremented to 0 after sleeping for the specified amount of time.  If an
 * interrupt occurs that brings the chip out of sleep, the function returns
 * ::ST_SLEEP_INTERRUPTED and the duration parameter reports the amount of
 * time remaining out of the original request.
 *
 * @note This routine always enables interrupts.
 *
 * @note The maximum sleep time of the hardware is limited on STM32W108 platforms
 * to 48.5 days.  Any sleep duration greater than this limit will wake up
 * briefly (e.g. 16 microseconds) to reenable another sleep cycle.
 *
 * @nostackusage
 *
 * @param duration The amount of time, expressed in quarter seconds, that the
 * micro should be placed into ::SLEEPMODE_WAKETIMER.  When the function returns,
 * this parameter provides the amount of time remaining out of the original
 * sleep time request (normally the return value will be 0).
 * 
 * @param gpioWakeBitMask  A bit mask of the GPIO that are allowed to wake
 * the chip from deep sleep.  A high bit in the mask will enable waking
 * the chip if the corresponding GPIO changes state.  bit0 is PA0, bit1 is
 * PA1, bit8 is PB0, bit16 is PCO, bit23 is PC7, bits[31:24] are ignored.
 * 
 * @return An StStatus value indicating the success or
 *  failure of the command.
 */
StStatus halSleepForQsWithOptions(int32u *duration, int32u gpioWakeBitMask);

/**
 * @brief Provides access to assembly code which triggers idle sleep.
 */
void halInternalIdleSleep(void);

/** @brief Puts the microcontroller to sleep in a specified mode.  This
 *  internal function performs the actual sleep operation.  This function
 *  assumes all of the wake source registers are configured properly.
 *
 * @note This routine always enables interrupts.
 *
 * @param sleepMode  A microcontroller sleep mode
 */
void halInternalSleep(SleepModes sleepMode);


/**
 * @brief Obtains the events that caused the last wake from sleep.  The
 * meaning of each bit is as follows:
 * - [31] = WakeInfoValid
 * - [30] = SleepSkipped
 * - [29] = CSYSPWRUPREQ
 * - [28] = CDBGPWRUPREQ
 * - [27] = PWRUP_WAKECORE
 * - [26] = PWRUP_SLEEPTMRWRAP
 * - [25] = PWRUP_SLEEPTMRCOMPB
 * - [24] = PWRUP_SLEEPTMRCOMPA
 * - [23:0] = corresponding GPIO activity
 *  
 * WakeInfoValid means that ::halSleepWithOptions (::halInternalSleep) has been called
 * at least once.  Since the power on state clears the wake event info,
 * this bit says the sleep code has been called since power on.
 *
 * SleepSkipped means that the chip never left the running state.  Sleep can
 * be skipped if any wake event occurs between going ::ATOMIC and transferring
 * control from the CPU to the power management state machine.  Sleep can
 * also be skipped if the debugger is connected (JTAG/SerialWire CSYSPWRUPREQ
 * signal is set).  The net affect of skipping sleep is the Low Voltage
 * domain never goes through a power/reset cycle.
 *
 * @return The events that caused the last wake from sleep. 
 */
int32u halGetWakeInfo(void);


/** @brief Seeds the ::halCommonGetRandom() pseudorandom number
 * generator.
 *
 * It should be called by the application during initialization with a seed
 * from the radio randon number generator.
 *
 * @param seed  A seed for the pseudorandom number generator.
 */
void halCommonSeedRandom(int32u seed);

/** @brief Runs a standard LFSR to generate pseudorandom numbers.
 *
 * Called by the MAC in the stack to choose random backoff slots.
 *
 * Complicated implementations may improve the MAC's
 * ability to avoid collisions in large networks, but it is \b critical to
 * implement this function to return quickly.
 */
int16u halCommonGetRandom(void);

#endif //__STM32W108XX_MICRO_COMMON_H__

/**@} // END micro group
 */

