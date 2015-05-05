/******************************************************************************
*  Filename:       sys_ctrl.h
*  Revised:        2015-01-15 14:12:37 +0100 (to, 15 jan 2015)
*  Revision:       42392
*
*  Description:    Defines and prototypes for the System Controller.
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
//! \addtogroup sysctrl_api
//! @{
//
//*****************************************************************************

#ifndef __SYSCTRL_H__
#define __SYSCTRL_H__

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
#include <inc/hw_sysctl.h>
#include <inc/hw_prcm.h>
#include <inc/hw_nvic.h>
#include <inc/hw_aon_wuc.h>
#include <inc/hw_aux_wuc.h>
#include <inc/hw_aon_ioc.h>
#include <inc/hw_ddi_0_osc.h>
#include <inc/hw_rfc_pwr.h>
#include <inc/hw_prcm.h>
#include <inc/hw_adi_3_refsys.h>
#include <inc/hw_aon_sysctl.h>
#include <inc/hw_aon_rtc.h>
#include <inc/hw_fcfg1.h>
#include <driverlib/interrupt.h>
#include <driverlib/debug.h>
#include <driverlib/pwr_ctrl.h>
#include <driverlib/osc.h>
#include <driverlib/prcm.h>
#include <driverlib/aux_wuc.h>
#include <driverlib/aon_wuc.h>
#include <driverlib/adi.h>
#include <driverlib/ddi.h>
#include <driverlib/cpu.h>
#include <driverlib/vims.h>

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
    #define SysCtrlPowerEverything          NOROM_SysCtrlPowerEverything
    #define SysCtrlStandby                  NOROM_SysCtrlStandby
    #define SysCtrlPowerdown                NOROM_SysCtrlPowerdown
    #define SysCtrlShutdown                 NOROM_SysCtrlShutdown
    #define SysCtrlClockGet                 NOROM_SysCtrlClockGet
    #define SysCtrlPeripheralClockGet       NOROM_SysCtrlPeripheralClockGet
    #define SysCtrlResetSourceGet           NOROM_SysCtrlResetSourceGet
#endif

//*****************************************************************************
//
// Defines for the settings of the main XOSC
//
//*****************************************************************************
#define SYSCTRL_SYSBUS_ON       0x00000001
#define SYSCTRL_SYSBUS_OFF      0x00000000

//*****************************************************************************
//
// Defines for the different power modes of the Cortex M3
//
//*****************************************************************************
#define CPU_RUN                 0x00000000
#define CPU_SLEEP               0x00000001
#define CPU_DEEP_SLEEP          0x00000002

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Power up everything.
//!
//! \note The sequencing in this function is not necessarily how you would
//! want to sequence active mode in a real application. There might be
//! application specific prerequisites or hardware restrictions you would want
//! to consider which deviate from this specific implementation.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtrlPowerEverything(void);

//*****************************************************************************
//
//! \brief Force the system into standby mode.
//!
//! \note The sequencing in this function is not necessarily how you would
//! want to sequence the standby in a real application. There might be
//! application specific prerequisites you would want to do before entering
//! standby which deviate from this specific implementation.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtrlStandby(void);

//*****************************************************************************
//
//! \brief Force the system into power down.
//!
//! \note The sequencing in this function is not necessarily how you would
//! want to sequence the powerdown in a real application. There might be
//! application specific prerequisites you would want to do before entering
//! powerdown which deviate from this specific implementation.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtrlPowerdown(void);

//*****************************************************************************
//
//! \brief Force the system into shutdown.
//!
//! \note It is solely the programmer's responsibility to properly configure an
//! interrupt that will enable the device to wakeup from the shutdown mode,
//! before calling this function. In shutdown the only possible wakeup action
//! is an IO interrupt.
//!
//! \note The sequencing in this function is not necessarily how you would
//! want to sequence the shutdown in a real application. There might be
//! application specific prerequisites you would want to do before entering
//! shutdown which deviate from this specific implementation.
//!
//! \return This function does \b not return.
//
//*****************************************************************************
extern void SysCtrlShutdown(void);

//*****************************************************************************
//
//! \brief Get the CPU core clock frequency.
//!
//! Use this function to retreive the current clock frequency for the CPU.
//!
//! The CPU can run from 48 MHz and down to 750kHz. The frequency is defined
//! by the combined division factor of the SYSBUS and the CPU clock divider.
//!
//! \return Returns the current CPU core clock frequency.
//
//*****************************************************************************
extern uint32_t SysCtrlClockGet(void);

//*****************************************************************************
//
//! \brief Get the clock for a peripheral.
//!
//! Use this function for retrieving the current clock frequency for a specific
//! MCU domain peripheral. The clock source for many of the peripheral changes
//! if the System Bus is gated. The \c ui32Mode parameter specifies for which
//! state of the System Bus the peripheral clock should be calculated.
//!
//! \note No checks are made for peripheral clock gating. It is left to the
//! programmer to ensure that the clock for a peripheral is not gated.
//!
//! \param ui32Peripheral identifies the peripheral for which to return the clock
//! frequency.
//! \param ui32BusMode is the mode of the System Bus.
//! - \ref SYSCTRL_SYSBUS_OFF
//! - \ref SYSCTRL_SYSBUS_ON
//!
//! \return Returns the clock speed of the selected peripheral.
//
//*****************************************************************************
extern uint32_t SysCtrlPeripheralClockGet(uint32_t ui32Peripheral,
                                          uint32_t ui32BusMode);

//*****************************************************************************
//
//! \brief Sync all accesses to the AON register interface.
//!
//! When this function returns, all writes to the AON register interface is
//! guaranteed to have progressed to hardware.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SysCtrlAonSync(void)
{
    //
    // Sync the AON interface
    //
    HWREG(AON_RTC_BASE + AON_RTC_O_SYNC);
}

//*****************************************************************************
//
//! \brief Update all interfaces to AON.
//!
//! When this function returns, at least 1 clock cycle has progressed on the
//! AON domain, so that any outstanding updates to and from the AON interface
//! is guaranteed to be in sync.
//!
//! \note This function should primarily be used after wakeup from sleep modes,
//! as it will guarantee that all shadow registers on the interface between MCU
//! and AON are updated. If a write has been done to the AON interface it is
//! sufficient to call the \ref SysCtrlAonSync().
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SysCtrlAonUpdate(void)
{
    //
    // Force a clock cycle on the AON interface to guarantee all registers are
    // in sync.
    //
    HWREG(AON_RTC_BASE + AON_RTC_O_SYNC) = 1;
    HWREG(AON_RTC_BASE + AON_RTC_O_SYNC);
}


//*****************************************************************************
//
//! \brief Enumeration describing possible input options to
//! \ref SysCtrlSetRechargeBeforePowerDown().
//!
//! Inform the \ref SysCtrlSetRechargeBeforePowerDown() function whether the XOSC is
//! in High or Low power mode. This is typically set by calling the function
//! \ref OSCXHfPowerModeSet().
//
//*****************************************************************************
typedef enum {
   XoscInHighPowerMode = 0, //!< When xosc_hf is in HIGH_POWER_XOSC.
   XoscInLowPowerMode       //!< When xosc_hf is in LOW_POWER_XOSC.
} XoscPowerMode_t;

//*****************************************************************************
//
//! \brief Set Recharge values before entering Power Down.
//!
//! This function shall be called just before entering Power Down.
//! It calculates an optimal and safe recharge setting of the adaptive recharge
//! controller. The results of previous setting are also taken into account.
//!
//! \note In order to make sure that the register writes are completed, \ref SysCtrlAonSync()
//! must be called before entering standby/power down. This is not done internally
//! in this function due to two reasons:
//! - 1) There might be other register writes that must be synchronized as well.
//! - 2) It is possible to save some time by doing other things before calling
//! \ref SysCtrlAonSync() since this call will not return before there are no
//! outstanding write requests between MCU and AON.
//!
//! \param xoscPowerMode (typically running in XoscInHighPowerMode all the time).
//!
//! \return None
//
//*****************************************************************************
void
SysCtrlSetRechargeBeforePowerDown( XoscPowerMode_t xoscPowerMode );


//*****************************************************************************
//
//! \brief Adjust Recharge calculations to be used next.
//!
//! This function shall be called just after returning from Power Down.
//!
//! Reads the results from the adaptive recharge controller and current chip
//! temperature. This is used as additional information when calculating
//! optimal recharge controller settings next time (When
//! \ref SysCtrlSetRechargeBeforePowerDown() is called next time).
//!
//! \note
//! Special care must be taken to make sure that the AON registers read are
//! updated after the wakeup. Writing to an AON register and then calling
//! \ref SysCtrlAonSync() will handle this. Typically this must be done anyway,
//! for example by calling \ref AONWUCAuxWakeupEvent() and then later on calling
//! \ref SysCtrlAonSync() just before calling \ref SysCtrlSetRechargeBeforePowerDown().
//!
//! \return None
//
//*****************************************************************************
void
SysCtrlAdjustRechargeAfterPowerDown( void );


//*****************************************************************************
//
//! \brief Turns DCDC on or off depending of what�s considered to be optimal usage.
//!
//! This function controls the DCDC only if both the following CCFG settings are TRUE:
//! 1) DCDC is configured to be used
//! 2) Alternative DCDC settings are defined and enabled.
//! The DCDC is configured in accordance to the CCFG settings when turned on.
//! This function should be called periodically.
//!
//! \return None
//
//*****************************************************************************
void
SysCtrl_DCDC_VoltageConditionalControl( void );


//*****************************************************************************
// \name Return values from calling SysCtrlResetSourceGet()
//@{
//*****************************************************************************
#define RSTSRC_PWR_ON               (( AON_SYSCTL_RESETCTL_RESET_SRC_PWR_ON    >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_PIN_RESET            (( AON_SYSCTL_RESETCTL_RESET_SRC_PIN_RESET >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_VDDS_LOSS            (( AON_SYSCTL_RESETCTL_RESET_SRC_VDDS_LOSS >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_VDD_LOSS             (( AON_SYSCTL_RESETCTL_RESET_SRC_VDD_LOSS  >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_VDDR_LOSS            (( AON_SYSCTL_RESETCTL_RESET_SRC_VDDR_LOSS >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_CLK_LOSS             (( AON_SYSCTL_RESETCTL_RESET_SRC_CLK_LOSS  >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_SYSRESET             (( AON_SYSCTL_RESETCTL_RESET_SRC_SYSRESET  >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_WARMRESET            (( AON_SYSCTL_RESETCTL_RESET_SRC_WARMRESET >> AON_SYSCTL_RESETCTL_RESET_SRC_S ))
#define RSTSRC_WAKEUP_FROM_SHUTDOWN (( AON_SYSCTL_RESETCTL_RESET_SRC_M         >> AON_SYSCTL_RESETCTL_RESET_SRC_S ) + 1 )
//@}

//*****************************************************************************
//
//! \brief Returns last reset source (including "wakeup from shutdown")
//!
//! \return Returns on of the RSTSRC_.. defines
//
//*****************************************************************************
extern uint32_t SysCtrlResetSourceGet( void );


//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_SysCtrlPowerEverything
        #undef  SysCtrlPowerEverything
        #define SysCtrlPowerEverything          ROM_SysCtrlPowerEverything
    #endif
    #ifdef ROM_SysCtrlStandby
        #undef  SysCtrlStandby
        #define SysCtrlStandby                  ROM_SysCtrlStandby
    #endif
    #ifdef ROM_SysCtrlPowerdown
        #undef  SysCtrlPowerdown
        #define SysCtrlPowerdown                ROM_SysCtrlPowerdown
    #endif
    #ifdef ROM_SysCtrlShutdown
        #undef  SysCtrlShutdown
        #define SysCtrlShutdown                 ROM_SysCtrlShutdown
    #endif
    #ifdef ROM_SysCtrlClockGet
        #undef  SysCtrlClockGet
        #define SysCtrlClockGet                 ROM_SysCtrlClockGet
    #endif
    #ifdef ROM_SysCtrlPeripheralClockGet
        #undef  SysCtrlPeripheralClockGet
        #define SysCtrlPeripheralClockGet       ROM_SysCtrlPeripheralClockGet
    #endif
    #ifdef ROM_SysCtrlResetSourceGet
        #undef  SysCtrlResetSourceGet
        #define SysCtrlResetSourceGet           ROM_SysCtrlResetSourceGet
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

#endif //  __SYSCTRL_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
