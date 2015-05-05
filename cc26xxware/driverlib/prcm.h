/******************************************************************************
*  Filename:       prcm.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the PRCM
*
*  Copyright (c) 2015, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

//*****************************************************************************
//
//! \addtogroup prcm_api
//! @{
//
//*****************************************************************************

#ifndef __PRCM_H__
#define __PRCM_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <inc/hw_prcm.h>
#include <inc/hw_nvic.h>
#include <inc/hw_aon_rtc.h>
#include <driverlib/interrupt.h>
#include <driverlib/debug.h>
#include <driverlib/cpu.h>

//*****************************************************************************
//
// Support for DriverLib in ROM:
// This section renames all functions that are not "static inline", so that
// calling these functions will default to implementation in flash. At the end
// of this file a second renaming will change the defaults to implementation in
// ROM for available functions.
//
// To force use of the implementation in flash, e.g. for debugging:
// - Globally: Define DRIVERLIB_NOROM at project level
// - Per function: Use prefix "NOROM_" when calling the function
//
// Do not define DRIVERLIB_GENERATE_ROM!
//
//*****************************************************************************
#ifndef DRIVERLIB_GENERATE_ROM
    #define PRCMInfClockConfigureSet        NOROM_PRCMInfClockConfigureSet
    #define PRCMInfClockConfigureGet        NOROM_PRCMInfClockConfigureGet
    #define PRCMClockConfigureSet           NOROM_PRCMClockConfigureSet
    #define PRCMClockConfigureGet           NOROM_PRCMClockConfigureGet
    #define PRCMAudioClockConfigSet         NOROM_PRCMAudioClockConfigSet
    #define PRCMAudioClockConfigSetOverride NOROM_PRCMAudioClockConfigSetOverride
    #define PRCMPowerDomainOn               NOROM_PRCMPowerDomainOn
    #define PRCMPowerDomainOff              NOROM_PRCMPowerDomainOff
    #define PRCMPeripheralRunEnable         NOROM_PRCMPeripheralRunEnable
    #define PRCMPeripheralRunDisable        NOROM_PRCMPeripheralRunDisable
    #define PRCMPeripheralSleepEnable       NOROM_PRCMPeripheralSleepEnable
    #define PRCMPeripheralSleepDisable      NOROM_PRCMPeripheralSleepDisable
    #define PRCMPeripheralDeepSleepEnable   NOROM_PRCMPeripheralDeepSleepEnable
    #define PRCMPeripheralDeepSleepDisable  NOROM_PRCMPeripheralDeepSleepDisable
    #define PRCMPowerDomainStatus           NOROM_PRCMPowerDomainStatus
    #define PRCMDeepSleep                   NOROM_PRCMDeepSleep
    #define PRCMRetentionEnable             NOROM_PRCMRetentionEnable
    #define PRCMRetentionDisable            NOROM_PRCMRetentionDisable
#endif

//*****************************************************************************
//
// Defines for the different Cortex M3 power modes.
//
//*****************************************************************************
#define PRCM_RUN_MODE           0x00000001
#define PRCM_SLEEP_MODE         0x00000002
#define PRCM_DEEP_SLEEP_MODE    0x00000004

//*****************************************************************************
//
// Defines used for setting the clock divison factors
//
//*****************************************************************************
#define PRCM_CLOCK_DIV_1        0x0000000
#define PRCM_CLOCK_DIV_2        0x0000001
#define PRCM_CLOCK_DIV_4        0x0000002
#define PRCM_CLOCK_DIV_8        0x0000003
#define PRCM_CLOCK_DIV_16       0x0000004
#define PRCM_CLOCK_DIV_32       0x0000005
#define PRCM_CLOCK_DIV_64       0x0000006
#define PRCM_CLOCK_DIV_128      0x0000007
#define PRCM_CLOCK_DIV_256      0x0000008

//*****************************************************************************
//
// Defines used for enabling and disabling domains and memories in the MCU
// domain
//
//*****************************************************************************
#define PRCM_DOMAIN_RFCORE      0x00000001  // RF Core domain ID for
                                            // clock/power control.
#define PRCM_DOMAIN_SERIAL      0x00000002  // Serial domain ID for
                                            // clock/power control.
#define PRCM_DOMAIN_PERIPH      0x00000004  // Peripheral domain ID for
                                            // clock/power control.
#define PRCM_DOMAIN_SYSBUS      0x00000008  // Bus domain ID for clock/power
                                            // control.
#define PRCM_DOMAIN_VIMS        0x00000010  // VIMS domain ID for clock/power
                                            // control.
#define PRCM_DOMAIN_CPU         0x00000020  // CPU domain ID for clock/power
                                            // control.
#define PRCM_DOMAIN_TIMER       0x00000040  // GPT domain ID for clock
                                            // control.
#define PRCM_DOMAIN_CLKCTRL     0x00000080  // Clock Control domain ID for
                                            // clock/power control.
#define PRCM_DOMAIN_MCU         0x00000100  // Reset control for entire MCU
                                            // domain.
#define PRCM_DOMAIN_POWER_OFF   0x00000002  // The domain is powered off
#define PRCM_DOMAIN_POWER_ON    0x00000001  // The domain is powered on
#define PRCM_DOMAIN_POWER_DOWN_READY                                          \
                                0x00000000  // The domain is ready to be
                                            // powered down.

//*****************************************************************************
//
// Defines for setting up the audio interface in the I2S module.
//
//*****************************************************************************
#define PRCM_WCLK_NEG_EDGE      0x00000008
#define PRCM_WCLK_POS_EDGE      0x00000000
#define PRCM_WCLK_SINGLE_PHASE  0x00000000
#define PRCM_WCLK_DUAL_PHASE    0x00000002
#define PRCM_WCLK_USER_DEF      0x00000004

#define I2S_SAMPLE_RATE_16K     0x00000001
#define I2S_SAMPLE_RATE_24K     0x00000002
#define I2S_SAMPLE_RATE_32K     0x00000004
#define I2S_SAMPLE_RATE_48K     0x00000008

//*****************************************************************************
//
// Defines used for enabling and disabling peripheral modules in the MCU
// domain
//
//*****************************************************************************
#define PRCM_PERIPH_TIMER0      0x00000000  // Peripheral ID for GPT module 0
#define PRCM_PERIPH_TIMER1      0x00000001  // Peripheral ID for GPT module 1
#define PRCM_PERIPH_TIMER2      0x00000002  // Peripheral ID for GPT module 2
#define PRCM_PERIPH_TIMER3      0x00000003  // Peripheral ID for GPT module 3
#define PRCM_PERIPH_SSI0        0x00000100  // Peripheral ID for SSI module 0
#define PRCM_PERIPH_SSI1        0x00000101  // Peripheral ID for SSI module 1
#define PRCM_PERIPH_UART0       0x00000200  // Peripheral ID for UART module 0
#define PRCM_PERIPH_UART1       0x00000201  // Peripheral ID for UART module 1
#define PRCM_PERIPH_I2C0        0x00000300  // Peripheral ID for I2C module 0
#define PRCM_PERIPH_I2C1        0x00000301  // Peripheral ID for I2C module 1
#define PRCM_PERIPH_CRYPTO      0x00000400  // Peripheral ID for CRYPTO module
#define PRCM_PERIPH_TRNG        0x00000401  // Peripheral ID for TRNG module
#define PRCM_PERIPH_UDMA        0x00000408  // Peripheral ID for UDMA module
#define PRCM_PERIPH_GPIO        0x00000500  // Peripheral ID for GPIO module
#define PRCM_PERIPH_I2S         0x00000600  // Peripheral ID for I2S module

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

#ifdef DRIVERLIB_DEBUG
//*****************************************************************************
//
//! \brief Checks a peripheral identifier.
//!
//! This function determines if a peripheral identifier is valid.
//!
//! \param ui32Peripheral is the peripheral identifier.
//!
//! \return Returns status of peripheral identifier:
//! - \b true  : Peripheral identifier is valid.
//! - \b false : Peripheral identifier is invalid.
//
//*****************************************************************************
static bool
PRCMPeripheralValid(uint32_t ui32Peripheral)
{
    return((ui32Peripheral == PRCM_PERIPH_TIMER0)   ||
           (ui32Peripheral == PRCM_PERIPH_TIMER1)   ||
           (ui32Peripheral == PRCM_PERIPH_TIMER2)   ||
           (ui32Peripheral == PRCM_PERIPH_TIMER3)   ||
           (ui32Peripheral == PRCM_PERIPH_SSI0)     ||
           (ui32Peripheral == PRCM_PERIPH_SSI1)     ||
           (ui32Peripheral == PRCM_PERIPH_UART0)    ||
           (ui32Peripheral == PRCM_PERIPH_UART1)    ||
           (ui32Peripheral == PRCM_PERIPH_I2C0)     ||
           (ui32Peripheral == PRCM_PERIPH_I2C1)     ||
           (ui32Peripheral == PRCM_PERIPH_UDMA)     ||
           (ui32Peripheral == PRCM_PERIPH_TRNG)     ||
           (ui32Peripheral == PRCM_PERIPH_CRYPTO)   ||
           (ui32Peripheral == PRCM_PERIPH_GPIO)     ||
           (ui32Peripheral == PRCM_PERIPH_I2S));
}
#endif

//*****************************************************************************
//
//! \brief Configure the infrastructure clock.
//!
//! Each CM3 power mode has its own infrastructure clock divison factor. This
//! function can be used for setting up the division factor for the
//! infrastructure clock in the available power modes for the CM3. The
//! infrastructure clock is used for all internal logic in the PRCM, and is
//! always running as long as power is on in the MCU voltage domain.
//! This can be enabled and disabled from the AON Wake Up Controller.
//!
//! \note If source clock is 48 MHz, minimum clock divider is \ref PRCM_CLOCK_DIV_2.
//!
//! \param ui32ClkDiv determines the division ratio for the infrastructure
//! clock when the device is in the specified mode.
//! Allowed division factors for all three Cortex M3 power modes are:
//! - \ref PRCM_CLOCK_DIV_1
//! - \ref PRCM_CLOCK_DIV_2
//! - \ref PRCM_CLOCK_DIV_8
//! - \ref PRCM_CLOCK_DIV_32
//! \param ui32PowerMode determines the Cortex M3 operation mode for which to
//! modify the clock divison factor.
//! The three allowed power modes are:
//! - \ref PRCM_RUN_MODE
//! - \ref PRCM_SLEEP_MODE
//! - \ref PRCM_DEEP_SLEEP_MODE
//!
//! \return None
//
//*****************************************************************************
extern void PRCMInfClockConfigureSet(uint32_t ui32ClkDiv,
                                     uint32_t ui32PowerMode);

//*****************************************************************************
//
//! \brief Use this function to retreive the set infrastructure clock configuration.
//!
//! \param ui32PowerMode determines which Cortex M3 power mode to return the
//! infrastructure clock divison ratio for.
//! The three allowed power modes are:
//! - \ref PRCM_RUN_MODE
//! - \ref PRCM_SLEEP_MODE
//! - \ref PRCM_DEEP_SLEEP_MODE
//!
//! \return Returns the infrastructure clock divison factor for the specified
//! power mode.
//! - \ref PRCM_CLOCK_DIV_1
//! - \ref PRCM_CLOCK_DIV_2
//! - \ref PRCM_CLOCK_DIV_8
//! - \ref PRCM_CLOCK_DIV_32
//!
//! \sa \ref PRCMInfClockConfigureSet().
//
//*****************************************************************************
extern uint32_t PRCMInfClockConfigureGet(uint32_t ui32PowerMode);

//*****************************************************************************
//
//! \brief Request a power off of the MCU voltage domain.
//!
//! Use this function to request a power off of the entire MCU voltage domain.
//! This request will have no affect until deepsleep mode is requested.
//!
//! \return None
//!
//! \sa \ref PRCMDeepSleep(), \ref PRCMMcuPowerOffCancel()
//
//*****************************************************************************
__STATIC_INLINE void
PRCMMcuPowerOff(void)
{
    //
    // Assert the power off request signal.
    //
    HWREG(PRCM_BASE + PRCM_O_VDCTL) |= PRCM_VDCTL_MCU_VD;
}

//*****************************************************************************
//
//! \brief Cancel a request for a power off of the MCU voltage domain.
//!
//! Use this function to cancel a request for power off of the entire MCU
//! voltage domain. This could be relevant if a transition to power down is
//! regretted and an application must backtrack.
//!
//! \return None
//!
//! \sa \ref PRCMDeepSleep(), \ref PRCMMcuPowerOff()
//
//*****************************************************************************
__STATIC_INLINE void
PRCMMcuPowerOffCancel(void)
{
    //
    // Assert the power off request signal.
    //
    HWREG(PRCM_BASE + PRCM_O_VDCTL) &= ~PRCM_VDCTL_MCU_VD;
}

//*****************************************************************************
//
//! \brief Assert or deassert a request for the uLDO.
//!
//! Use this function to request to switch to the micro Low Voltage Dropout
//! regulator (uLDO). The uLDO has a much lower capacicty for supplying power
//! to the system. It is therefore imperative and solely the programmers
//! responsibility to ensure that a sufficient amount of peripheral modules
//! have been turned of before requesting a switch to the uLDO.
//!
//! \note Asserting this bit has no effect until:
//! 1. FLASH has accepted to be powered down
//! 2. Deepsleep must be asserted
//!
//! \return None
//!
//! \sa \ref PRCMDeepSleep()
//
//*****************************************************************************
__STATIC_INLINE void
PRCMMcuUldoConfigure(uint32_t ui32Enable)
{
    uint32_t ui32Val;

    //
    // Enable/disable the uLDO request signal.
    //
    ui32Val = HWREG(PRCM_BASE + PRCM_O_VDCTL);
    if(ui32Enable)
    {
        ui32Val |= PRCM_VDCTL_ULDO;
    }
    else
    {
        ui32Val &= ~PRCM_VDCTL_ULDO;
    }
    HWREG(PRCM_BASE + PRCM_O_VDCTL) = ui32Val;
}

//*****************************************************************************
//
//! \brief Setup the clock division factor for a subsystem in the MCU voltage
//! domain.
//!
//! Use this function to set up the clock division factor on a specific clock
//! domain inside the MCU voltage domain.
//!
//! The \ref PRCM_DOMAIN_TIMER division rate will be constant and ungated for Run
//! / Sleep / DeepSleep mode when it is slower than the \ref PRCM_DOMAIN_PERIPH
//! setting. When set faster than \ref PRCM_DOMAIN_PERIPH setting the
//! \ref PRCM_DOMAIN_PERIPH will be used.
//!
//! The \ref PRCM_DOMAIN_CPU can only run with 48 or 24 MHz. The only two valid
//! clock division factors are therefore 1 and 2 and it is always sourced from
//! the HF source.
//!
//! The \ref PRCM_DOMAIN_SERIAL clock can run independently of the
//! \ref PRCM_DOMAIN_SYSBUS clock. This function can be used to setup the clock
//! divison factor but this is only valid when the \ref PRCM_DOMAIN_SYSBUS clock
//! is disabled. When the \ref PRCM_DOMAIN_SYSBUS clock is enabled, the clock
//! division ratio for this clock will follow the \ref PRCM_DOMAIN_SYSBUS clock.
//!
//! The \ref PRCM_DOMAIN_SERIAL clock sets the serial interface clock for SSI
//! and UART. The clock source is the MCU domain root clock.
//! The \ref PRCM_DOMAIN_SERIAL clock is the only asynchronous clock in the MCU
//! Voltage domain and is configured independently from all other clocks.
//! It is the programmers responsibility to ensure that the
//! \ref PRCM_DOMAIN_SERIAL clock frequency setting is slower than 5/3 of the
//! \ref PRCM_DOMAIN_SYSBUS clock for UART operation and less than or equal to
//! the \ref PRCM_DOMAIN_SYSBUS clock for SSI operation. If both modules are
//! active the lowest value of the two must be selected.
//!
//! \note Not all sub-systems support all division factors. Please ensure
//! that a given clock domain will support the chosen division factor before
//! trying to configure it.
//!
//! \param ui32Domains defines the clock domains to configure the clock on.
//! The 5 clock domains are:
//! - \ref PRCM_DOMAIN_SYSBUS
//! - \ref PRCM_DOMAIN_CPU
//! - \ref PRCM_DOMAIN_PERIPH
//! - \ref PRCM_DOMAIN_SERIAL
//! - \ref PRCM_DOMAIN_TIMER
//! \param ui32ClkDiv is the division factor to configure.
//! The argument must be only one of the following values:
//! - \ref PRCM_CLOCK_DIV_1
//! - \ref PRCM_CLOCK_DIV_2
//! - \ref PRCM_CLOCK_DIV_4
//! - \ref PRCM_CLOCK_DIV_8
//! - \ref PRCM_CLOCK_DIV_16
//! - \ref PRCM_CLOCK_DIV_32
//! - \ref PRCM_CLOCK_DIV_64
//! - \ref PRCM_CLOCK_DIV_128
//! - \ref PRCM_CLOCK_DIV_256
//!
//! \return None
//
//*****************************************************************************
extern void PRCMClockConfigureSet(uint32_t ui32Domains, uint32_t ui32ClkDiv);

//*****************************************************************************
//
//! \brief Get the clock configuration for a specific sub system in the MCU Voltage
//! Domain.
//!
//! Use this function to retrieve the clock division factor on a specific clock
//! domain inside the MCU voltage domain.
//!
//! \param ui32Domain defines the sub system for which to return the clock
//! configuration.
//! The 5 clock domains are:
//! - \ref PRCM_DOMAIN_SYSBUS
//! - \ref PRCM_DOMAIN_CPU
//! - \ref PRCM_DOMAIN_PERIPH
//! - \ref PRCM_DOMAIN_SERIAL
//! - \ref PRCM_DOMAIN_TIMER
//!
//! \return Returns the sub system clock configuration
//
//*****************************************************************************
extern uint32_t PRCMClockConfigureGet(uint32_t ui32Domain);

//*****************************************************************************
//
//! \brief Enable the audio clock generation.
//!
//! Use this function to enable the audio clock generation.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
PRCMAudioClockEnable(void)
{
    //
    // Enable the audio clock generation.
    //
    HWREG(PRCM_BASE + PRCM_O_I2SCLKCTL) |= PRCM_I2SCLKCTL_EN;
}

//*****************************************************************************
//
//! \brief Disable the audio clock generation.
//!
//! Use this function to disable the audio clock generation.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
PRCMAudioClockDisable(void)
{
    //
    // Disable the audio clock generation
    //
    HWREG(PRCM_BASE + PRCM_O_I2SCLKCTL) &= ~PRCM_I2SCLKCTL_EN;
}

//*****************************************************************************
//
//! \brief Configure the audio clock generation.
//!
//! Use this function to set the sample rate when using internal audio clock
//! generation for the I2S module.
//!
//! \note While other clocks are possible, the stability of the four sample
//! rates defined here are only guaranteed if the clock input to the I2S module
//! is 48MHz. This can be configured using \ref PRCMClockConfigureSet().
//!
//! \param ui32ClkConfig is the audio clock configuration.
//! The parameter is a bitwise OR'ed value consisting of:
//! - Phase
//!   - \ref PRCM_WCLK_SINGLE_PHASE
//!   - \ref PRCM_WCLK_DUAL_PHASE
//! - Clock polarity
//!   - \ref PRCM_WCLK_NEG_EDGE
//!   - \ref PRCM_WCLK_POS_EDGE
//! \param ui32SampleRate is the desired audio clock sample rate.
//! The supported sample rate configurations are:
//! - \ref I2S_SAMPLE_RATE_16K
//! - \ref I2S_SAMPLE_RATE_24K
//! - \ref I2S_SAMPLE_RATE_32K
//! - \ref I2S_SAMPLE_RATE_48K
//!
//! \return None
//!
//! \sa \ref PRCMAudioClockConfigSetOverride()
//
//*****************************************************************************
extern void PRCMAudioClockConfigSet(uint32_t ui32ClkConfig,
                                    uint32_t ui32SampleRate);

//*****************************************************************************
//
//! \brief Configure the audio clock generation with manual setting of clock divider.
//!
//! Use this function to set the audio clock divider values manually.
//!
//! \note See hardware documentation before setting audio clock dividers manually.
//!
//! \param ui32ClkConfig is the audio clock configuration.
//! The parameter is a bitwise OR'ed value consisting of:
//! - Phase
//!   - \ref PRCM_WCLK_SINGLE_PHASE
//!   - \ref PRCM_WCLK_DUAL_PHASE
//! - Clock polarity
//!   - \ref PRCM_WCLK_NEG_EDGE
//!   - \ref PRCM_WCLK_POS_EDGE
//! \param ui32MstDiv is the desired master clock divider.
//! \param ui32WordDiv is the desired word clock divider.
//! \param ui32BitDiv is the desired bit clock divider.
//!
//! \return None
//!
//! \sa \ref PRCMAudioClockConfigSet()
//
//*****************************************************************************
extern void PRCMAudioClockConfigSetOverride(uint32_t ui32ClkConfig, uint32_t ui32MstDiv,
                        uint32_t ui32BitDiv, uint32_t ui32WordDiv);

//*****************************************************************************
//
//! \brief Use this function to synchronize the load settings.
//!
//! Most of the clock settings in the PRCM module should be updated
//! synchronously. This is ensured by the implementation of a load registers
//! that, when written to, will let the previous written update values for all
//! the relevant registers propagate through to hardware.
//!
//! The functions that require a synchronization of the clock settings are:
//! - \ref PRCMAudioClockConfigSet()
//! - \ref PRCMAudioClockConfigSetOverride()
//! - \ref PRCMAudioClockEnable()
//! - \ref PRCMAudioClockDisable()
//! - \ref PRCMClockConfigureSet()
//! - \ref PRCMDomainEnable()
//! - \ref PRCMDomainDisable()
//! - \ref PRCMPeripheralRunEnable()
//! - \ref PRCMPeripheralRunEnable()
//! - \ref PRCMPeripheralSleepEnable()
//! - \ref PRCMPeripheralSleepEnable()
//! - \ref PRCMPeripheralDeepSleepEnable()
//! - \ref PRCMPeripheralDeepSleepEnable()
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
PRCMLoadSet(void)
{
    //
    // Enable the update of all load related registers.
    //
    HWREG(PRCM_NONBUF_BASE + PRCM_O_CLKLOADCTL) = PRCM_CLKLOADCTL_LOAD;
}

//*****************************************************************************
//
//! \brief Check if any of the load sensitive register has been updated.
//!
//! \return Returns status of the load sensitive register:
//! - \c true  : No registers have changed since the last load.
//! - \c false : Any register has changed.
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
__STATIC_INLINE bool
PRCMLoadGet(void)
{
    //
    // Return the load status.
    //
    return ((HWREG(PRCM_BASE + PRCM_O_CLKLOADCTL) & PRCM_CLKLOADCTL_LOAD_DONE) ?
            true : false);
}

//*****************************************************************************
//
//! \brief Enable clock domains in the MCU voltage domain.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \note Clocks will only be running if the domain is powered.
//!
//! \param ui32Domains is a bitmask containing the clock domains to enable.
//! The independent clock domains inside the MCU voltage domain which can be
//! configured are:
//! - \ref PRCM_DOMAIN_RFCORE
//! - \ref PRCM_DOMAIN_VIMS
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
PRCMDomainEnable(uint32_t ui32Domains)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32Domains & PRCM_DOMAIN_RFCORE) ||
           (ui32Domains & PRCM_DOMAIN_VIMS));

    //
    // Enable the clock domain(s).
    //
    if(ui32Domains & PRCM_DOMAIN_RFCORE)
    {
        HWREG(PRCM_BASE + PRCM_O_RFCCLKG) = PRCM_RFCCLKG_CLK_EN;
    }
    if(ui32Domains & PRCM_DOMAIN_VIMS)
    {
        HWREG(PRCM_BASE + PRCM_O_VIMSCLKG) = PRCM_VIMSCLKG_CLK_EN_M;
    }
}

//*****************************************************************************
//
//! \brief Disable clock domains in the MCU voltage domain.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \note Clocks will only be running if the domain is powered.
//!
//! \param ui32Domains is a bitmask containing the clock domains to disable.
//! The independent clock domains inside the MCU voltage domain are:
//! - \ref PRCM_DOMAIN_RFCORE
//! - \ref PRCM_DOMAIN_VIMS
//!
//! \return None
//!
//! \sa PRCMDomainEnable()
//
//*****************************************************************************
__STATIC_INLINE void
PRCMDomainDisable(uint32_t ui32Domains)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32Domains & PRCM_DOMAIN_RFCORE) ||
           (ui32Domains & PRCM_DOMAIN_VIMS));

    //
    // Disable the power domains.
    //
    if(ui32Domains & PRCM_DOMAIN_RFCORE)
    {
        HWREG(PRCM_BASE + PRCM_O_RFCCLKG) = 0x0;
    }
    if(ui32Domains & PRCM_DOMAIN_VIMS)
    {
        HWREG(PRCM_BASE + PRCM_O_VIMSCLKG) = 0x0;
    }
}

//*****************************************************************************
//
//! \brief Turn power on in power domains in the MCU domain.
//!
//! Use this function to turn on power domains inside the MCU voltage
//! domain.
//!
//! Power on and power off request has different implications for the
//! different power domains.
//! - RF Core power domain:
//!   - Power On : Domain is on or in the process of turning on.
//!   - Power Off: Domain is powered down when CM3 is in deep sleep. The third
//!                option for the RF Core is to power down when the it is idle.
//!                This can be set using \b PRCMRfPowerDownWhenIdle()
//! - SERIAL power domain:
//!   - Power on : Domain is powered on.
//!   - Power off : Domain is powered off.
//! - PERIPHERIAL power domain:
//!   - Power on : Domain is powered on.
//!   - Power off : Domain is powered off.
//! - VIMS power domain:
//!   - Power On: Domain is powered if Bus domain is powered.
//!   - Power Off: Domain is only powered when CPU domain is on.
//! - BUS power domain:
//!   - Power On: Domain is on.
//!   - Power Off: Domain is on if requested by RF Core or if CPU domian is on.
//! - CPU power domain:
//!   - Power On: Domain is on.
//!   - Power Off: Domain is powering down if Cortex M3 is idle. This will also
//!                initiate a power down of the SRAM and BUS power domains, unless
//!                RF Core is requesting them to be on.
//!
//! \note After a call to this function the status of the power domain should
//! be checked using either \ref PRCMPowerDomainStatus().
//! Any write operation to a power domain which is still not operational can
//! result in unexpected behaviour.
//!
//! \param ui32Domains determines which power domains to turn on.
//! The domains that can be turned on/off are:
//! - \b PRCM_DOMAIN_RFCORE : RF Core
//! - \b PRCM_DOMAIN_SERIAL : SSI0, UART0, I2C0
//! - \b PRCM_DOMAIN_PERIPH : GPT0, GPT1, GPT2, GPT3, GPIO, SSI1, I2S,
//!                           DMA, UART1, I2C1
//! - \b PRCM_DOMAIN_VIMS   : SRAM, FLASH, ROM
//! - \b PRCM_DOMAIN_SYSBUS
//! - \b PRCM_DOMAIN_CPU
//!
//! \return None
//
//*****************************************************************************
extern void PRCMPowerDomainOn(uint32_t ui32Domains);

//*****************************************************************************
//
//! \brief Turn off a specific power domain.
//!
//! Use this function to power down domains inside the MCU voltage
//! domain.
//!
//! \note For specifics regarding on/off configuration please see
//! \ref PRCMPowerDomainOn().
//!
//! \param ui32Domains determines which domain to request a power down for.
//! The domains that can be turned on/off are:
//! - \b PRCM_DOMAIN_RFCORE : RF Core
//! - \b PRCM_DOMAIN_SERIAL : SSI0, UART0, I2C0
//! - \b PRCM_DOMAIN_PERIPH : GPT0, GPT1, GPT2, GPT3, GPIO, SSI1, I2S,
//!                           DMA, UART1, I2C1
//! - \b PRCM_DOMAIN_VIMS   : SRAM, FLASH, ROM
//! - \b PRCM_DOMAIN_SYSBUS
//! - \b PRCM_DOMAIN_CPU
//!
//! \return None
//
//*****************************************************************************
extern void PRCMPowerDomainOff(uint32_t ui32Domains);

//*****************************************************************************
//
//! \brief Configure RF core to power down when idle.
//!
//! Use this function to configure the RF core to power down when Idle. This
//! is handled automatically in hardware if the RF Core reports that it is
//! idle.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
PRCMRfPowerDownWhenIdle(void)
{
    //
    // Configure the RF power domain.
    //
    HWREG(PRCM_BASE + PRCM_O_PDCTL0RFC) &= ~PRCM_PDCTL0RFC_ON;
}

//*****************************************************************************
//
//! \brief Enables a peripheral in Run mode.
//!
//! Peripherals are enabled with this function.  At power-up, some peripherals
//! are disabled; they must be enabled in order to operate or respond to
//! register reads/writes.
//!
//! \note The actual enabling of the peripheral may be delayed until some
//! time after this function returns. Care should be taken to ensure that the
//! peripheral is not accessed until it is enabled.
//! When enabling Timers always make sure that the division factor for the
//! \b PERBUSCPUCLK is set. This will guarantee that the timers run at a
//! continuous rate even if the \b SYSBUSCLK is gated.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \param ui32Peripheral is the peripheral to enable.
//! The parameter must be one of the following:
//! - \ref PRCM_PERIPH_TIMER0
//! - \ref PRCM_PERIPH_TIMER1
//! - \ref PRCM_PERIPH_TIMER2
//! - \ref PRCM_PERIPH_TIMER3
//! - \ref PRCM_PERIPH_SSI0
//! - \ref PRCM_PERIPH_SSI1
//! - \ref PRCM_PERIPH_UART0
//! - \ref PRCM_PERIPH_UART1
//! - \ref PRCM_PERIPH_I2C0
//! - \ref PRCM_PERIPH_I2C1
//! - \ref PRCM_PERIPH_CRYPTO
//! - \ref PRCM_PERIPH_TRNG
//! - \ref PRCM_PERIPH_UDMA
//! - \ref PRCM_PERIPH_GPIO
//! - \ref PRCM_PERIPH_I2S
//!
//! \return None
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
extern void PRCMPeripheralRunEnable(uint32_t ui32Peripheral);

//*****************************************************************************
//
//! Disables a peripheral in Run mode
//!
//! Peripherals are disabled with this function. Once disabled, they will not
//! operate or respond to register reads/writes.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \note The actual disabling of the peripheral may be delayed until some
//! time after this function returns. Care should be taken by the user to
//! ensure that the peripheral is not accessed in this interval as this might
//! cause the system to hang.
//!
//! \param ui32Peripheral is the peripheral to disable.
//! The parameter must be one of the following:
//! - \ref PRCM_PERIPH_TIMER0
//! - \ref PRCM_PERIPH_TIMER1
//! - \ref PRCM_PERIPH_TIMER2
//! - \ref PRCM_PERIPH_TIMER3
//! - \ref PRCM_PERIPH_SSI0
//! - \ref PRCM_PERIPH_SSI1
//! - \ref PRCM_PERIPH_UART0
//! - \ref PRCM_PERIPH_UART1
//! - \ref PRCM_PERIPH_I2C0
//! - \ref PRCM_PERIPH_I2C1
//! - \ref PRCM_PERIPH_CRYPTO
//! - \ref PRCM_PERIPH_TRNG
//! - \ref PRCM_PERIPH_UDMA
//! - \ref PRCM_PERIPH_GPIO
//! - \ref PRCM_PERIPH_I2S
//!
//! \return None
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
extern void PRCMPeripheralRunDisable(uint32_t ui32Peripheral);

//*****************************************************************************
//
//! \brief Enables a peripheral in sleep mode.
//!
//! This function allows a peripheral to continue operating when the processor
//! goes into sleep mode. Since the clocking configuration of the device does
//! not change, any peripheral can safely continue operating while the
//! processor is in sleep mode, and can therefore wake the processor from sleep
//! mode.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \param ui32Peripheral is the peripheral to enable in sleep mode.
//! The parameter must be one of the following:
//! - \ref PRCM_PERIPH_TIMER0
//! - \ref PRCM_PERIPH_TIMER1
//! - \ref PRCM_PERIPH_TIMER2
//! - \ref PRCM_PERIPH_TIMER3
//! - \ref PRCM_PERIPH_SSI0
//! - \ref PRCM_PERIPH_SSI1
//! - \ref PRCM_PERIPH_UART0
//! - \ref PRCM_PERIPH_UART1
//! - \ref PRCM_PERIPH_I2C0
//! - \ref PRCM_PERIPH_I2C1
//! - \ref PRCM_PERIPH_CRYPTO
//! - \ref PRCM_PERIPH_TRNG
//! - \ref PRCM_PERIPH_UDMA
//! - \ref PRCM_PERIPH_GPIO
//! - \ref PRCM_PERIPH_I2S
//!
//! \return None
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
extern void PRCMPeripheralSleepEnable(uint32_t ui32Peripheral);

//*****************************************************************************
//
//! \brief Disables a peripheral in sleep mode.
//!
//! This function causes a peripheral to stop operating when the processor goes
//! into sleep mode. Disabling peripherals while in sleep mode helps to lower
//! the current draw of the device. If enabled (via \ref PRCMPeripheralRunEnable()),
//! the peripheral will automatically resume operation when the processor
//! leaves sleep mode, maintaining its entire state from before sleep mode was
//! entered.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \param ui32Peripheral is the peripheral to disable in sleep mode.
//! The parameter must be one of the following:
//! - \ref PRCM_PERIPH_TIMER0
//! - \ref PRCM_PERIPH_TIMER1
//! - \ref PRCM_PERIPH_TIMER2
//! - \ref PRCM_PERIPH_TIMER3
//! - \ref PRCM_PERIPH_SSI0
//! - \ref PRCM_PERIPH_SSI1
//! - \ref PRCM_PERIPH_UART0
//! - \ref PRCM_PERIPH_UART1
//! - \ref PRCM_PERIPH_I2C0
//! - \ref PRCM_PERIPH_I2C1
//! - \ref PRCM_PERIPH_CRYPTO
//! - \ref PRCM_PERIPH_TRNG
//! - \ref PRCM_PERIPH_UDMA
//! - \ref PRCM_PERIPH_GPIO
//! - \ref PRCM_PERIPH_I2S
//!
//! \return None
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
extern void PRCMPeripheralSleepDisable(uint32_t ui32Peripheral);

//*****************************************************************************
//
//! \brief Enables a peripheral in deep-sleep mode.
//!
//! This function allows a peripheral to continue operating when the processor
//! goes into deep-sleep mode.  Since the clocking configuration of the device
//! may change, not all peripherals can safely continue operating while the
//! processor is in sleep mode. This in turn depends on the chosen power mode.
//! It is the responsibility of the caller to make sensible choices.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \param ui32Peripheral is the peripheral to enable in deep-sleep mode.
//! The parameter must be one of the following:
//! - \ref PRCM_PERIPH_TIMER0
//! - \ref PRCM_PERIPH_TIMER1
//! - \ref PRCM_PERIPH_TIMER2
//! - \ref PRCM_PERIPH_TIMER3
//! - \ref PRCM_PERIPH_SSI0
//! - \ref PRCM_PERIPH_SSI1
//! - \ref PRCM_PERIPH_UART0
//! - \ref PRCM_PERIPH_UART1
//! - \ref PRCM_PERIPH_I2C0
//! - \ref PRCM_PERIPH_I2C1
//! - \ref PRCM_PERIPH_CRYPTO
//! - \ref PRCM_PERIPH_TRNG
//! - \ref PRCM_PERIPH_UDMA
//! - \ref PRCM_PERIPH_GPIO
//! - \ref PRCM_PERIPH_I2S
//!
//! \return None
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
extern void PRCMPeripheralDeepSleepEnable(uint32_t ui32Peripheral);

//*****************************************************************************
//
//! \brief Disables a peripheral in deep-sleep mode.
//!
//! This function causes a peripheral to stop operating when the processor goes
//! into deep-sleep mode.  Disabling peripherals while in deep-sleep mode helps
//! to lower the current draw of the device, and can keep peripherals that
//! require a particular clock frequency from operating when the clock changes
//! as a result of entering deep-sleep mode.  If enabled (via
//! \ref PRCMPeripheralRunEnable()), the peripheral will automatically resume
//! operation when the processor leaves deep-sleep mode, maintaining its entire
//! state from before deep-sleep mode was entered.
//!
//! \note A call to this function will only setup the shadow registers in the
//! MCU domain for the PRCM module. For the changes to propagate to the system
//! controller in the AON domain a call to this function should always be
//! followed by a call to \ref PRCMLoadSet().
//!
//! \param ui32Peripheral is the peripheral to disable in deep-sleep mode.
//! The parameter must be one of the following:
//! - \ref PRCM_PERIPH_TIMER0
//! - \ref PRCM_PERIPH_TIMER1
//! - \ref PRCM_PERIPH_TIMER2
//! - \ref PRCM_PERIPH_TIMER3
//! - \ref PRCM_PERIPH_SSI0
//! - \ref PRCM_PERIPH_SSI1
//! - \ref PRCM_PERIPH_UART0
//! - \ref PRCM_PERIPH_UART1
//! - \ref PRCM_PERIPH_I2C0
//! - \ref PRCM_PERIPH_I2C1
//! - \ref PRCM_PERIPH_CRYPTO
//! - \ref PRCM_PERIPH_TRNG
//! - \ref PRCM_PERIPH_UDMA
//! - \ref PRCM_PERIPH_GPIO
//! - \ref PRCM_PERIPH_I2S
//!
//! \return None
//!
//! \sa \ref PRCMLoadSet()
//
//*****************************************************************************
extern void PRCMPeripheralDeepSleepDisable(uint32_t ui32Peripheral);

//*****************************************************************************
//
//! \brief Get the status for a specific power domain.
//!
//! Use this function to retrieve the current power status of one or more
//! power domains.
//!
//! \param ui32Domains determines which domain to get the power status for.
//! The parameter must be an OR'ed combination of one or several of:
//! - \ref PRCM_DOMAIN_RFCORE : RF Core.
//! - \ref PRCM_DOMAIN_SERIAL : SSI0, UART0, I2C0
//! - \ref PRCM_DOMAIN_PERIPH : GPT0, GPT1, GPT2, GPT3, GPIO, SSI1, I2S, DMA, UART1, I2C1
//!
//! \return Returns status of the requested domains:
//! - \ref PRCM_DOMAIN_POWER_ON  : The specified domains are \b all powered up.
//! This status is unconditional and the powered up status is guaranteed.
//! - \ref PRCM_DOMAIN_POWER_OFF : Any of the domains are powered down.
//
//*****************************************************************************
extern uint32_t PRCMPowerDomainStatus(uint32_t ui32Domains);

//*****************************************************************************
//
//! \brief Return the access status of the RF Core.
//!
//! Use this function to check if the RF Core is on and ready to be accessed.
//! Accessing register or memories that are not powered and clocked will
//! cause a bus fault.
//!
//! \return Returns access status of the RF Core.
//! - \c true  : RF Core can be accessed.
//! - \c false : RF Core domain is not ready for access.
//
//*****************************************************************************
__STATIC_INLINE bool
PRCMRfReady(void)
{
    //
    // Return the ready status of the RF Core.
    //
    return ((HWREG(PRCM_BASE + PRCM_O_PDSTAT1RFC) &
             PRCM_PDSTAT1RFC_ON) ? true : false);
}

//*****************************************************************************
//
//! \brief Read reset status for WatchDog Timer.
//!
//! WDT reset is the only reset status available through the PRCM module.
//! This function can be used to check whether or not a WDT reset has
//! occured since last time this bit was cleared.
//!
//! \note This function will automatically clear the WDT reset status bit
//! if asserted. If the reset bit is not asserted then nothing happens.
//!
//! \return Returns reset status of Watchdog Timer.
//! - \c true  : A WDT reset occured since last time the bit was cleared.
//! - \c false : A WDT reset has not occured since last clear.
//
//*****************************************************************************
__STATIC_INLINE bool
PRCMWdtResetStatus(void)
{
    //
    // Return the WDT reset status.
    //
    return ((HWREG(PRCM_BASE + PRCM_O_WARMRESET) & PRCM_WARMRESET_WDT_STAT)
            ? true : false);
}

//*****************************************************************************
//
//! \brief Put the processor into sleep mode.
//!
//! This function places the processor into sleep mode; it does not return
//! until the processor returns to run mode.  The peripherals that are enabled
//! via \brief PRCMPeripheralSleepEnable() continue to operate and can wake up the
//! processor.
//!
//! \return None
//!
//! \sa \ref PRCMPeripheralSleepEnable()
//
//*****************************************************************************
__STATIC_INLINE void
PRCMSleep(void)
{
    //
    // Wait for an interrupt.
    //
    CPUwfi();
}

//*****************************************************************************
//
//! \brief Put the processor into deep-sleep mode.
//!
//! This function places the processor into deep-sleep mode; it does not return
//! until the processor returns to run mode.  The peripherals that are enabled
//! via \ref PRCMPeripheralDeepSleepEnable() continue to operate and can wake up
//! the processor.
//!
//! \return None
//!
//! \sa \ref PRCMPeripheralDeepSleepEnable()
//
//*****************************************************************************
extern void PRCMDeepSleep(void);

//*****************************************************************************
//
//! \brief Enable retention on specific power domains.
//!
//! Determines if power domain should have retention enabled.
//!
//! \param ui32PowerDomain is a bit mask of the domains in question.
//! The parameter must be a bitwise OR'ed value of:
//! - \ref PRCM_DOMAIN_PERIPH
//! - \ref PRCM_DOMAIN_CPU
//! - \ref PRCM_DOMAIN_VIMS
//! - \ref PRCM_DOMAIN_RFCORE
//!
//! \return None
//
//*****************************************************************************
extern void PRCMRetentionEnable(uint32_t ui32PowerDomain);

//*****************************************************************************
//
//! \brief Disable retention on power domains.
//!
//! Determines if a power domain should have retention disabled.
//!
//! \param ui32PowerDomain is a bit maks of the domains in question.
//! The parameter must be a bitwise OR'ed value of:
//! - \ref PRCM_DOMAIN_PERIPH
//! - \ref PRCM_DOMAIN_CPU
//! - \ref PRCM_DOMAIN_VIMS
//! - \ref PRCM_DOMAIN_RFCORE
//!
//! \return None
//
//*****************************************************************************
extern void PRCMRetentionDisable(uint32_t ui32PowerDomain);

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_PRCMInfClockConfigureSet
        #undef  PRCMInfClockConfigureSet
        #define PRCMInfClockConfigureSet        ROM_PRCMInfClockConfigureSet
    #endif
    #ifdef ROM_PRCMInfClockConfigureGet
        #undef  PRCMInfClockConfigureGet
        #define PRCMInfClockConfigureGet        ROM_PRCMInfClockConfigureGet
    #endif
    #ifdef ROM_PRCMClockConfigureSet
        #undef  PRCMClockConfigureSet
        #define PRCMClockConfigureSet           ROM_PRCMClockConfigureSet
    #endif
    #ifdef ROM_PRCMClockConfigureGet
        #undef  PRCMClockConfigureGet
        #define PRCMClockConfigureGet           ROM_PRCMClockConfigureGet
    #endif
    #ifdef ROM_PRCMAudioClockConfigSet
        #undef  PRCMAudioClockConfigSet
        #define PRCMAudioClockConfigSet         ROM_PRCMAudioClockConfigSet
    #endif
    #ifdef ROM_PRCMAudioClockConfigSetOverride
        #undef  PRCMAudioClockConfigSetOverride
        #define PRCMAudioClockConfigSetOverride ROM_PRCMAudioClockConfigSetOverride
    #endif
    #ifdef ROM_PRCMPowerDomainOn
        #undef  PRCMPowerDomainOn
        #define PRCMPowerDomainOn               ROM_PRCMPowerDomainOn
    #endif
    #ifdef ROM_PRCMPowerDomainOff
        #undef  PRCMPowerDomainOff
        #define PRCMPowerDomainOff              ROM_PRCMPowerDomainOff
    #endif
    #ifdef ROM_PRCMPeripheralRunEnable
        #undef  PRCMPeripheralRunEnable
        #define PRCMPeripheralRunEnable         ROM_PRCMPeripheralRunEnable
    #endif
    #ifdef ROM_PRCMPeripheralRunDisable
        #undef  PRCMPeripheralRunDisable
        #define PRCMPeripheralRunDisable        ROM_PRCMPeripheralRunDisable
    #endif
    #ifdef ROM_PRCMPeripheralSleepEnable
        #undef  PRCMPeripheralSleepEnable
        #define PRCMPeripheralSleepEnable       ROM_PRCMPeripheralSleepEnable
    #endif
    #ifdef ROM_PRCMPeripheralSleepDisable
        #undef  PRCMPeripheralSleepDisable
        #define PRCMPeripheralSleepDisable      ROM_PRCMPeripheralSleepDisable
    #endif
    #ifdef ROM_PRCMPeripheralDeepSleepEnable
        #undef  PRCMPeripheralDeepSleepEnable
        #define PRCMPeripheralDeepSleepEnable   ROM_PRCMPeripheralDeepSleepEnable
    #endif
    #ifdef ROM_PRCMPeripheralDeepSleepDisable
        #undef  PRCMPeripheralDeepSleepDisable
        #define PRCMPeripheralDeepSleepDisable  ROM_PRCMPeripheralDeepSleepDisable
    #endif
    #ifdef ROM_PRCMPowerDomainStatus
        #undef  PRCMPowerDomainStatus
        #define PRCMPowerDomainStatus           ROM_PRCMPowerDomainStatus
    #endif
    #ifdef ROM_PRCMDeepSleep
        #undef  PRCMDeepSleep
        #define PRCMDeepSleep                   ROM_PRCMDeepSleep
    #endif
    #ifdef ROM_PRCMRetentionEnable
        #undef  PRCMRetentionEnable
        #define PRCMRetentionEnable             ROM_PRCMRetentionEnable
    #endif
    #ifdef ROM_PRCMRetentionDisable
        #undef  PRCMRetentionDisable
        #define PRCMRetentionDisable            ROM_PRCMRetentionDisable
    #endif
#endif

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __PRCM_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
