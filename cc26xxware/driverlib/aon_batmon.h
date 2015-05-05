/******************************************************************************
*  Filename:       aon_batmon.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the AON Battery and Temperature
*                  Monitor
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
//! \addtogroup aonbatmon_api
//! @{
//
//*****************************************************************************

#ifndef __AON_BATMON_H__
#define __AON_BATMON_H__

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
#include <inc/hw_aon_batmon.h>
#include <driverlib/debug.h>

//*****************************************************************************
//
// Values that can be passed to the AONBatMonMeasurementCycleSet() for setting
// the measurement cycle.
//
//*****************************************************************************
#define AON_BATMON_CYCLE_32     AON_BATMON_MEASCFG_PER_32CYC  // 32 Clock cycles between
                                                              // each measurement.
#define AON_BATMON_CYCLE_16     AON_BATMON_MEASCFG_PER_16CYC  // 16 Clock cycles between
                                                              // each measurement.
#define AON_BATMON_CYCLE_8      AON_BATMON_MEASCFG_PER_8CYC   // 8 Clock cycles between
                                                              // each measurement.
#define AON_BATMON_CYCLE_CONT   AON_BATMON_MEASCFG_PER_CONT   // Continouos measurement

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Enable the temperatur and battery monitoring.
//!
//! This function will enable the measurements of the temperature and the
//! battery voltage.
//!
//! To speed up the measurement of the levels the measurement can be enabled
//! before configuring the battery and temperatur settings. When all of the
//! AON_BATMON registers are configured, the calculation of the voltage and
//! temperature values can be enabled (the measurement will now take
//! effect/propagate to other blocks).
//!
//! It is possible to enable both at the same time, after the AON_BATMON
//! registers are configured, but then the first values will be ready at a
//! later point compared to the scenario above.
//!
//! \note Temperature and battery voltage measurements are not done in
//! parallel. The measurement cycle is controlled by a hardware Finite State
//! Machine. First the temperature and then the battery voltage each taking
//! one cycle to complete. However, if the comparator measuring the battery
//! voltage detects a change on the reference value, a new measurement of the
//! battery voltage only is performed immediately after. This has no impact on
//! the cycle count.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
AONBatMonEnable(void)
{
    //
    // Enable the measurements.
    //
    HWREG(AON_BATMON_BASE + AON_BATMON_O_CTL) =
        AON_BATMON_CTL_CALC_EN |
        AON_BATMON_CTL_MEAS_EN;
}

//*****************************************************************************
//
//! \brief Disable the temperatur and battery monitoring.
//!
//! This function will disable the measurements of the temperature and the
//! battery voltage.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
AONBatMonDisable(void)
{
    //
    // Disable the measurements.
    //
    HWREG(AON_BATMON_BASE + AON_BATMON_O_CTL) = 0;
}

//*****************************************************************************
//
//! \brief Adjust the measurement cycle for the temperature and the battery voltage.
//!
//! The AON battery and temperature monitor can measure continously or wait
//! a predefined number of BATMON clock cycles between each measurement. This function
//! is used to adjust the length of the measurement cycle in number of BATMON clock
//! cycles.
//!
//! \param ui32Cycle defines the number of clock cycles between each measurement.
//! - \ref AON_BATMON_CYCLE_CONT : Measure continuously.
//! - \ref AON_BATMON_CYCLE_8 : Measure every 8 cycle.
//! - \ref AON_BATMON_CYCLE_16 : Measure every 16 cycle.
//! - \ref AON_BATMON_CYCLE_32 : Measure every 32 cycle.
//!
//! \return None
//!
//! \sa AONBatMonMeasurementCycleGet()
//
//*****************************************************************************
__STATIC_INLINE void
AONBatMonMeasurementCycleSet(uint32_t ui32Cycle)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32Cycle == AON_BATMON_CYCLE_32 ||
           ui32Cycle == AON_BATMON_CYCLE_16 ||
           ui32Cycle == AON_BATMON_CYCLE_8 ||
           ui32Cycle == AON_BATMON_CYCLE_CONT);

    //
    // Set the measurement cycle.
    //
    HWREG(AON_BATMON_BASE + AON_BATMON_O_MEASCFG) = ui32Cycle;
}

//*****************************************************************************
//
//! \brief Get the measurement cycle for the temperature and the battery voltage.
//!
//! This function returns the value of the register AON_BATMON_O_MEASCFG which
//! contains the settings for the measurement interval.
//!
//! \return Returns the current setting of the measurement period configuration.
//!
//! \sa AONBatMonMeasurementCycleSet()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
AONBatMonMeasurementCycleGet(void)
{
    //
    // Return the current measurement cycle.
    //
    return (HWREG(AON_BATMON_BASE + AON_BATMON_O_MEASCFG));
}

//*****************************************************************************
//
//! \brief Set the trim values for the battery voltage calculation.
//!
//! The trim value must be setup correctly before enabling the battery voltage
//! measurement, to ensure correct calculation and system behaviour. The
//! output from the battery monitor measurement is a 10 bit vector with a 2 bit
//! integer part and an 8 bit fractional part in the range 0 to 3,9961 volt.
//!
/*!
\verbatim
          (352 + ui32TrimP1)    (128 + ui32TrimP0) x Vbmon
  Vout = ------------------- + -----------------------------
                2^8                      2^12
\endverbatim
*/
//!
//! \param ui32TrimP0 is a 6 bit trim value.
//! \param ui32TrimP1 is a 6 bit trim value.
//!
//! \return None
//!
//! \sa AONBatMonTemperatureTrimSet()
//
//*****************************************************************************
__STATIC_INLINE void
AONBatMonBatteryTrimSet(uint32_t ui32TrimP0, uint32_t ui32TrimP1)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32TrimP0 <= AON_BATMON_BATMONP0_CFG_M ||
           ui32TrimP1 <= AON_BATMON_BATMONP1_CFG_M);

    //
    // Set the trim values.
    //
    HWREG(AON_BATMON_BASE + AON_BATMON_O_BATMONP0) = ui32TrimP0;
    HWREG(AON_BATMON_BASE + AON_BATMON_O_BATMONP1) = ui32TrimP1;
}

//*****************************************************************************
//
//! \brief Set the trim values for the temperature calculation.
//!
//! The trim value must be setup correctly before enabling the temperature
//! measurement, to ensure correct calculation and system behaviour.The
//! output from the temperature monitor measurement is a signed 10 bit vector
//! with an 8 bit integer part and a 2 bit fractional part in the range -128 C to
//! +127,75 C with a resolution of 0,25 C.
//!
/*!
\verbatim
          (-384 + ui32TrimP2)    (256 + ui32TrimP1) x Temp     (64 + ui32TrimP0) x Temp^2
  Vout = ------------------- + ---------------------------- + ----------------------------
                2^2                       2^8                            2^12
\endverbatim
*/
//!
//! \param ui32TrimP0 is an 8 bit trim value.
//! \param ui32TrimP1 is a 6 bit trim value.
//! \param ui32TrimP2 is a 5 bit trim value.
//!
//! \return None
//!
//! \sa AONBatMonBatteryTrimSet()
//
//*****************************************************************************
__STATIC_INLINE void
AONBatMonTemperatureTrimSet(uint32_t ui32TrimP0, uint32_t ui32TrimP1,
                            uint32_t ui32TrimP2)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32TrimP0 < AON_BATMON_TEMPP0_CFG_M ||
           ui32TrimP1 < AON_BATMON_TEMPP1_CFG_M ||
           ui32TrimP2 < AON_BATMON_TEMPP2_CFG_M);

    //
    // Set the trim values
    //
    HWREG(AON_BATMON_BASE + AON_BATMON_O_TEMPP0) = ui32TrimP0;
    HWREG(AON_BATMON_BASE + AON_BATMON_O_TEMPP1) = ui32TrimP1;
    HWREG(AON_BATMON_BASE + AON_BATMON_O_TEMPP2) = ui32TrimP2;
}

//*****************************************************************************
//
//! \brief Get the current temperature measurement.
//!
//! This function will return the current temperature measurement.
//! The temperature measurements are updated every cycle as defined
//! using AONBatMonMeasurementCycleSet().
//!
//! \note The returned value is NOT sign-extended!
//!
//! \note The temperature drifts slightly depending on the battery voltage.
//! This function is a raw read and does not compensate for this drift.
//! Use function \ref AON_BatmonTempGetDegC() to get a calibrated value.
//!
//! \note Use the function AONBatMonNewTempMeasureReady() to test
//! for a change in measurement.
//!
//! \return Returns the current value of the temperature measurement in a
//! <int.frac> format size <9.2>.
//!
//! \sa AONBatMonNewTempMeasureReady(), AONBatMonMeasurementCycleSet(), AON_BatmonTempGetDegC()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
AONBatMonTemperatureGet(void)
{
    uint32_t ui32CurrentTemp;

    ui32CurrentTemp = HWREG(AON_BATMON_BASE + AON_BATMON_O_TEMP);

    //
    // Return the current temperature measurement.
    //
    return (ui32CurrentTemp >> AON_BATMON_TEMP_FRAC_S);
}

//*****************************************************************************
//
//! \brief Get the current temperature measurement as a signed value in Deg Celsius.
//!
//! This function returns an calibrated and rounded value in degree Celsius.
//! The temperature measurements are updated every cycle as defined using AONBatMonMeasurementCycleSet().
//!
//! \note The temperature drifts slightly depending on the battery voltage.
//! This function compensates for this drift and returns a calibrated temperature.
//!
//! \note Use the function AONBatMonNewTempMeasureReady() to test for a new measurement.
//!
//! \return Returns signed integer part of temperature in Deg C (-256 .. +255)
//!
//! \sa AONBatMonNewTempMeasureReady(), AONBatMonMeasurementCycleSet(), AONBatMonTemperatureGet()
//
//*****************************************************************************
int32_t
AON_BatmonTempGetDegC( void );

//*****************************************************************************
//
//! \brief Get the battery monitor measurement.
//!
//! This function will return the current battery monitor measurement.
//! The battery voltage measurements are updated every cycle as defined
//! using AONBatMonMeasurementCycleSet().
//!
//! \note The returned value is NOT sign-extended!
//!
//! \note Use the function \ref AONBatMonNewBatteryMeasureReady() to test for
//! a change in measurement.
//!
//! \return Returns the current battery monitor value of the battery voltage
//! measurement in a <int.frac> format size <3.8> in units of volt.
//!
//! \sa AONBatMonMeasurementCycleSet(), AONBatMonNewBatteryMeasureReady()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
AONBatMonBatteryVoltageGet(void)
{
    uint32_t ui32CurrentBattery;

    ui32CurrentBattery = HWREG(AON_BATMON_BASE + AON_BATMON_O_BAT);

    //
    // Return the current battery voltage measurement.
    //
    return (ui32CurrentBattery >> AON_BATMON_BAT_FRAC_S);
}

//*****************************************************************************
//
//! \brief Check if battery monitor measurement has changed.
//!
//! This function checks if a new battery monitor value is available. If the
//! measurement value has \b changed since last clear the function returns \c true.
//!
//! If the measurement has changed the function will automatically clear the
//! status bit.
//!
//! \note It is always possible to read out the current value of the
//! battery level using AONBatMonBatteryVoltageGet() but this function can be
//! used to check if the measurement has changed.
//!
//! \return Returns \c true if the measurement value has changed and \c false
//! otherwise.
//!
//! \sa AONBatMonNewTempMeasureReady(), AONBatMonBatteryVoltageGet()
//
//*****************************************************************************
__STATIC_INLINE bool
AONBatMonNewBatteryMeasureReady(void)
{
    bool bStatus;

    //
    // Check the status bit.
    //
    bStatus = HWREG(AON_BATMON_BASE + AON_BATMON_O_BATUPD) &
              AON_BATMON_BATUPD_STAT ? true : false;

    //
    // Clear status bit if set.
    //
    if(bStatus)
    {
        HWREG(AON_BATMON_BASE + AON_BATMON_O_BATUPD) = 1;
    }

    //
    // Return status.
    //
    return (bStatus);
}

//*****************************************************************************
//
//! \brief Check if temperature monitor measurement has changed.
//!
//! This function checks if a new temperature value is available. If the
//! measurement value has \b changed since last clear the function returns \c true.
//!
//! If the measurement has changed the function will automatically clear the
//! status bit.
//!
//! \note It is always possible to read out the current value of the
//! temperature using \ref AONBatMonTemperatureGet() or \ref AON_BatmonTempGetDegC()
//! but this function can be used to check if the measurement has changed.
//!
//! \return Returns \c true if the measurement value has changed and \c false
//! otherwise.
//!
//! \sa AONBatMonNewBatteryMeasureReady(), AONBatMonTemperatureGet(), AON_BatmonTempGetDegC()
//
//*****************************************************************************
__STATIC_INLINE bool
AONBatMonNewTempMeasureReady(void)
{
    bool bStatus;

    //
    // Check the status bit.
    //
    bStatus = HWREG(AON_BATMON_BASE + AON_BATMON_O_TEMPUPD) &
              AON_BATMON_TEMPUPD_STAT ? true : false;

    //
    // Clear status bit if set.
    //
    if(bStatus)
    {
        HWREG(AON_BATMON_BASE + AON_BATMON_O_TEMPUPD) = 1;
    }

    //
    // Return status.
    //
    return (bStatus);
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __AON_BATMON_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
