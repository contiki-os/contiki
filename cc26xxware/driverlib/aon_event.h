/******************************************************************************
*  Filename:       aon_event.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the AON Event fabric.
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
//! \addtogroup aonevent_api
//! @{
//
//*****************************************************************************

#ifndef __AON_EVENT_H__
#define __AON_EVENT_H__

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
#include <inc/hw_device.h>
#include <inc/hw_aon_event.h>
#include <driverlib/debug.h>

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
    #define AONEventMcuWakeUpSet            NOROM_AONEventMcuWakeUpSet
    #define AONEventMcuWakeUpGet            NOROM_AONEventMcuWakeUpGet
    #define AONEventAuxWakeUpSet            NOROM_AONEventAuxWakeUpSet
    #define AONEventAuxWakeUpGet            NOROM_AONEventAuxWakeUpGet
    #define AONEventMcuSet                  NOROM_AONEventMcuSet
    #define AONEventMcuGet                  NOROM_AONEventMcuGet
#endif

//*****************************************************************************
//
// Event sources for the event AON fabric.
// Note: Events are level-triggered active high
//
//*****************************************************************************
#define AON_EVENT_IO0          AON_EVENT_DIO_0 // Edge detect on DIO0
#define AON_EVENT_IO1          AON_EVENT_DIO_1 // Edge detect on DIO1
#define AON_EVENT_IO2          AON_EVENT_DIO_2 // Edge detect on DIO2
#define AON_EVENT_IO3          AON_EVENT_DIO_3 // Edge detect on DIO3
#define AON_EVENT_IO4          AON_EVENT_DIO_4 // Edge detect on DIO4
#define AON_EVENT_IO5          AON_EVENT_DIO_5 // Edge detect on DIO5
#define AON_EVENT_IO6          AON_EVENT_DIO_6 // Edge detect on DIO6
#define AON_EVENT_IO7          AON_EVENT_DIO_7 // Edge detect on DIO7
#define AON_EVENT_IO8          AON_EVENT_DIO_8 // Edge detect on DIO8
#define AON_EVENT_IO9          AON_EVENT_DIO_9 // Edge detect on DIO9
#define AON_EVENT_IO10         AON_EVENT_DIO_10 // Edge detect on DIO10
#define AON_EVENT_IO11         AON_EVENT_DIO_11 // Edge detect on DIO11
#define AON_EVENT_IO12         AON_EVENT_DIO_12 // Edge detect on DIO12
#define AON_EVENT_IO13         AON_EVENT_DIO_13 // Edge detect on DIO13
#define AON_EVENT_IO14         AON_EVENT_DIO_14 // Edge detect on DIO14
#define AON_EVENT_IO15         AON_EVENT_DIO_15 // Edge detect on DIO15
#define AON_EVENT_IO16         AON_EVENT_DIO_16 // Edge detect on DIO16
#define AON_EVENT_IO17         AON_EVENT_DIO_17 // Edge detect on DIO17
#define AON_EVENT_IO18         AON_EVENT_DIO_18 // Edge detect on DIO18
#define AON_EVENT_IO19         AON_EVENT_DIO_19 // Edge detect on DIO19
#define AON_EVENT_IO20         AON_EVENT_DIO_20 // Edge detect on DIO20
#define AON_EVENT_IO21         AON_EVENT_DIO_21 // Edge detect on DIO21
#define AON_EVENT_IO22         AON_EVENT_DIO_22 // Edge detect on DIO22
#define AON_EVENT_IO23         AON_EVENT_DIO_23 // Edge detect on DIO23
#define AON_EVENT_IO24         AON_EVENT_DIO_24 // Edge detect on DIO24
#define AON_EVENT_IO25         AON_EVENT_DIO_25 // Edge detect on DIO25
#define AON_EVENT_IO26         AON_EVENT_DIO_26 // Edge detect on DIO26
#define AON_EVENT_IO27         AON_EVENT_DIO_27 // Edge detect on DIO27
#define AON_EVENT_IO28         AON_EVENT_DIO_28 // Edge detect on DIO28
#define AON_EVENT_IO29         AON_EVENT_DIO_29 // Edge detect on DIO29
#define AON_EVENT_IO30         AON_EVENT_DIO_30 // Edge detect on DIO30
#define AON_EVENT_IO31         AON_EVENT_DIO_31 // Edge detect on DIO31
#define AON_EVENT_IO           32 // Edge detect on any DIO
#define AON_EVENT_SPISRTX      33 // SPIS byte transmitted
#define AON_EVENT_SPISCS       34 // SPIS CS_N asserted
#define AON_EVENT_RTC0         35 // RTC channel 0
#define AON_EVENT_RTC1         36 // RTC channel 1
#define AON_EVENT_RTC2         37 // RTC channel 2
#define AON_EVENT_RTC0DLY      38 // RTC channel 0 - delayed event
#define AON_EVENT_RTC1DLY      39 // RTC channel 1 - delayed event
#define AON_EVENT_RTC2DLY      40 // RTC channel 2 - delayed event
#define AON_EVENT_RTCDLYC      41 // RTC combined delayed event
                                  // Event ID 42 is reserved for future use
#define AON_EVENT_JTAG         43 // JTAG generated event
#define AON_EVENT_AUX0         44 // SW programmable
                                  // default: Statemachine done
#define AON_EVENT_AUX1         45 // SW programmable
                                  // default: Error event
#define AON_EVENT_AUX2         46 // SW programmable
                                  // default: RTC Channel 2 clear
#define AON_EVENT_AUX_CMPA     47 // Comparator A triggered
#define AON_EVENT_AUX_CMPB     48 // Comparator B triggered
#define AON_EVENT_AUX_ADC_DONE 49 // ADC conversion completed
#define AON_EVENT_AUX_TDC_DONE 50 // TDC completed or timed out
#define AON_EVENT_AUX_TIMER0   51 // Timer 0 event
#define AON_EVENT_AUX_TIMER1   52 // Timer 1 event

#define AON_EVENT_BATMON_TEMP  53 //
#define AON_EVENT_BATMON_VOLT  54 //
								  // Event ID 55-62 is reserved for future use
#define AON_EVENT_NULL         63 // No event

//*****************************************************************************
//
// Values that can be passed to AONEventSetMCUWakeUp() and returned
// by AONEventGetMCUWakeUp().
//
//*****************************************************************************
#define AON_EVENT_MCU_WU0 0     // Programmable MCU wakeup event 0
#define AON_EVENT_MCU_WU1 1     // Programmable MCU wakeup event 1
#define AON_EVENT_MCU_WU2 2     // Programmable MCU wakeup event 2
#define AON_EVENT_MCU_WU3 3     // Programmable MCU wakeup event 3

//*****************************************************************************
//
// Values that can be passed to TBD
//
//*****************************************************************************
#define AON_EVENT_AUX_WU0 0     // Programmable AUX wakeup event 0
#define AON_EVENT_AUX_WU1 1     // Programmable AUX wakeup event 1
#define AON_EVENT_AUX_WU2 2     // Programmable AUX wakeup event 2

//*****************************************************************************
//
// Values that can be passed to TBD
//
//*****************************************************************************
#define AON_EVENT_MCU_EVENT0 0  // Programmable event source fed to
                                // MCU event fabric (first of 3)
#define AON_EVENT_MCU_EVENT1 1  // Programmable event source fed to
                                // MCU event fabric (second of 3)
#define AON_EVENT_MCU_EVENT2 2  // Programmable event source fed to
                                // MCU event fabric (third of 3)

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Select event source for the specified MCU wakeup programmable event.
//!
//! The AON event fabric has a total of four programmable events that can
//! wake-up the MCU. The events are forwarded to the wakeup controller (WUC).
//!
//! \note The four programmable event sources are effectively OR'ed together
//!  to form a single wake-up event forwarded to the WUC.
//!
//! \param ui32MCUWUEvent is one of four programmable MCU wakeup event sources.
//! - \ref AON_EVENT_MCU_WU0
//! - \ref AON_EVENT_MCU_WU1
//! - \ref AON_EVENT_MCU_WU2
//! - \ref AON_EVENT_MCU_WU3
//! \param ui32EventSrc is an event source for the event AON fabric.
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \return None
//!
//! \sa AONEventMcuWakeUpGet()
//
//*****************************************************************************
extern void AONEventMcuWakeUpSet(uint32_t ui32MCUWUEvent,
                                 uint32_t ui32EventSrc);

//*****************************************************************************
//
//! \brief Get event source for the specified MCU wakeup programmable event.
//!
//! The AON event fabric has a total of four programmable events that can
//! wake-up the MCU. The events are forwarded to the wakeup controller (WUC).
//!
//! \param ui32MCUWUEvent is one of four programmable MCU wakeup event sources.
//! - \ref AON_EVENT_MCU_WU0
//! - \ref AON_EVENT_MCU_WU1
//! - \ref AON_EVENT_MCU_WU2
//! - \ref AON_EVENT_MCU_WU3
//!
//! \return Returns an event source for the event AON fabric, i.e. one of the
//! following:
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \sa AONEventMcuWakeUpSet()
//
//*****************************************************************************
extern uint32_t AONEventMcuWakeUpGet(uint32_t ui32MCUWUEvent);

//*****************************************************************************
//
//! \brief Select event source for the specified AUX wakeup programmable event.
//!
//! The AON event fabric has a total of three programmable events that can
//! wake-up the AUX domain. The events are forwarded to the wakeup
//! controller (WUC).
//!
//! \note The three programmable event sources are effectively OR'ed together
//!  to form a single wake-up event forwarded to the WUC.
//!
//! \param ui32AUXWUEvent is one of three programmable AUX wakeup event sources.
//! - \ref AON_EVENT_AUX_WU0
//! - \ref AON_EVENT_AUX_WU1
//! - \ref AON_EVENT_AUX_WU2
//! \param ui32EventSrc is an event sources for the event AON fabric.
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \return None
//!
//! \sa AONEventAuxWakeUpGet()
//
//*****************************************************************************
extern void AONEventAuxWakeUpSet(uint32_t ui32AUXWUEvent,
                                 uint32_t ui32EventSrc);

//*****************************************************************************
//
//! \brief Get event source for the specified AUX wakeup programmable event.
//!
//! The AON event fabric has a total of three programmable events that can
//! wake-up the AUX domain. The events are forwarded to the wakeup
//! controller (WUC).
//!
//! \param ui32AUXWUEvent is one of three programmable AUX wakeup event sources.
//! - \ref AON_EVENT_AUX_WU0
//! - \ref AON_EVENT_AUX_WU1
//! - \ref AON_EVENT_AUX_WU2
//!
//! \return Returns an event source for the event AON fabric, i.e. one of the following:
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \sa AONEventAuxWakeUpSet()
//
//*****************************************************************************
extern uint32_t AONEventAuxWakeUpGet(uint32_t ui32AUXWUEvent);

//*****************************************************************************
//
//! \brief Select event source for the specified programmable event forwarded to the
//! MCU event fabric.
//!
//! The AON event fabric has a total of three programmable events that can
//! be forwarded to the MCU event fabric.
//!
//! \note The three programmable event sources are forwarded to the following
//! MCU event fabric events:
//! - \ref EVENT_AON_PROG_0
//! - \ref EVENT_AON_PROG_1
//! - \ref EVENT_AON_PROG_2
//!
//! \param ui32MCUEvent is one of three programmable events forwarded to the
//! MCU event fabric.
//! - \ref AON_EVENT_MCU_EVENT0
//! - \ref AON_EVENT_MCU_EVENT1
//! - \ref AON_EVENT_MCU_EVENT2
//! \param ui32EventSrc is event sources for the event AON fabric.
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \return None
//!
//! \sa AONEventMcuGet()
//
//*****************************************************************************
extern void AONEventMcuSet(uint32_t ui32MCUEvent, uint32_t ui32EventSrc);

//*****************************************************************************
//
//! \brief Get source for the specified programmable event forwarded to the MCU event
//! fabric.
//!
//! The AON event fabric has a total of three programmable events that can
//! be forwarded to the MCU event fabric.
//!
//! \param ui32MCUEvent is one of three programmable events forwarded to the
//! MCU event fabric.
//! - \ref AON_EVENT_MCU_EVENT0
//! - \ref AON_EVENT_MCU_EVENT1
//! - \ref AON_EVENT_MCU_EVENT2
//!
//! \return Returns an event source for the event AON fabric, i.e. one of the following:
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \sa AONEventMcuSet()
//
//*****************************************************************************
extern uint32_t AONEventMcuGet(uint32_t ui32MCUEvent);

//*****************************************************************************
//
//! \brief Select event source forwarded to AON Real Time Clock (RTC).
//!
//! A programmable event can be forwarded to the AON real time clock
//! for triggering a capture event on RTC channel 1.
//!
//! \param ui32EventSrc is the event sources for the event AON fabric.
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \return None
//!
//! \sa AONEventRtcGet()
//
//*****************************************************************************
__STATIC_INLINE void
AONEventRtcSet(uint32_t ui32EventSrc)
{
    uint32_t ui32Ctrl;

    //
    // Check the arguments.
    //
    ASSERT(ui32EventSrc <= AON_EVENT_NULL);

    ui32Ctrl = HWREG(AON_EVENT_BASE + AON_EVENT_O_RTCSEL);
    ui32Ctrl &= ~(AON_EVENT_RTCSEL_RTC_CH1_CAPT_EV_M);
    ui32Ctrl |= (ui32EventSrc & 0x3f) << AON_EVENT_RTCSEL_RTC_CH1_CAPT_EV_S;

    HWREG(AON_EVENT_BASE + AON_EVENT_O_RTCSEL) = ui32Ctrl;
}

//*****************************************************************************
//
//! \brief Get event source forwarded to AON Real Time Clock (RTC).
//!
//! A programmable event can be forwarded to the AON real time clock
//! for triggering a capture event on RTC channel 1.
//!
//! \return Returns an event source to the event AON fabric, i.e. one of the following:
//! - AON_EVENT_DIO0
//! - AON_EVENT_DIO1
//! - ...
//! - \ref AON_EVENT_NULL
//!
//! \sa AONEventRtcSet()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
AONEventRtcGet(void)
{
    uint32_t ui32EventSrc;

    //
    // Return the active event.
    //
    ui32EventSrc = HWREG(AON_EVENT_BASE + AON_EVENT_O_RTCSEL);

    return ((ui32EventSrc & AON_EVENT_RTCSEL_RTC_CH1_CAPT_EV_M) >>
            AON_EVENT_RTCSEL_RTC_CH1_CAPT_EV_S);
}

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_AONEventMcuWakeUpSet
        #undef  AONEventMcuWakeUpSet
        #define AONEventMcuWakeUpSet            ROM_AONEventMcuWakeUpSet
    #endif
    #ifdef ROM_AONEventMcuWakeUpGet
        #undef  AONEventMcuWakeUpGet
        #define AONEventMcuWakeUpGet            ROM_AONEventMcuWakeUpGet
    #endif
    #ifdef ROM_AONEventAuxWakeUpSet
        #undef  AONEventAuxWakeUpSet
        #define AONEventAuxWakeUpSet            ROM_AONEventAuxWakeUpSet
    #endif
    #ifdef ROM_AONEventAuxWakeUpGet
        #undef  AONEventAuxWakeUpGet
        #define AONEventAuxWakeUpGet            ROM_AONEventAuxWakeUpGet
    #endif
    #ifdef ROM_AONEventMcuSet
        #undef  AONEventMcuSet
        #define AONEventMcuSet                  ROM_AONEventMcuSet
    #endif
    #ifdef ROM_AONEventMcuGet
        #undef  AONEventMcuGet
        #define AONEventMcuGet                  ROM_AONEventMcuGet
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

#endif //  __AON_EVENT_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
