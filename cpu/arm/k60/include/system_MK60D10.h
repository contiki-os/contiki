

/**
 * \file
 * \brief Device specific configuration file for MK60D10 (header file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device and initializes the oscillator
 * (PLL) that is part of the microcontroller device.
 */

#ifndef SYSTEM_MK60D10_H_
#define SYSTEM_MK60D10_H_                        /**< Symbol preventing repeated inclusion */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * \brief Current core clock frequency
 *
 * MCGOUTCLK divided by OUTDIV1 clocks the ARM Cortex-M4 core.
 */
extern uint32_t SystemCoreClock;

/**
 * \brief Current system clock frequency
 *
 * MCGOUTCLK divided by OUTDIV1 clocks the crossbar switch and bus masters
 * directly connected to the crossbar. In addition, this clock is used for UART0
 * and UART1.
 */
extern uint32_t SystemSysClock;

/**
 * \brief Current bus clock frequency
 *
 * MCGOUTCLK divided by OUTDIV2 clocks the bus slaves and peripheral (excluding
 * memories).
 */
extern uint32_t SystemBusClock;

/**
 * \brief Current FlexBus clock frequency
 *
 * MCGOUTCLK divided by OUTDIV3 clocks the external FlexBus interface.
 */
extern uint32_t SystemFlexBusClock;

/**
 * \brief Current flash clock frequency
 *
 * MCGOUTCLK divided by OUTDIV4 clocks the flash memory.
 */
extern uint32_t SystemFlashClock;


/**
 * \brief Setup the microcontroller system.
 *
 * Typically this function configures the oscillator (PLL) that is part of the
 * microcontroller device. For systems with variable clock speed it also updates
 * the variable SystemCoreClock. SystemInit is called from startup_device file.
 */
void SystemInit(void);

/**
 * \brief Updates all of the SystemCoreClock variables.
 *
 * It must be called whenever the core clock is changed during program
 * execution. SystemCoreClockUpdate() evaluates the clock register settings and
 * calculates the current core clock.
 */
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif  /* #if !defined(SYSTEM_MK60D10_H_) */
