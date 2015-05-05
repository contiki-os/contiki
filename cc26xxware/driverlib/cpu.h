/******************************************************************************
*  Filename:       cpu.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the CPU instruction wrapper
*                  functions.
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
//! \addtogroup cpu_api
//! @{
//
//*****************************************************************************

#ifndef __CPU_H__
#define __CPU_H__

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
    #define CPUcpsid                        NOROM_CPUcpsid
    #define CPUprimask                      NOROM_CPUprimask
    #define CPUcpsie                        NOROM_CPUcpsie
    #define CPUbasepriGet                   NOROM_CPUbasepriGet
    #define CPUdelay                        NOROM_CPUdelay
#endif

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Disable all external interrupts.
//!
//! Use this function to disable all system interrupts. This function is
//! implemented as a wrapper function for the CPSID instruction.
//!
//! \return Returns the state of \b PRIMASK on entry
//
//*****************************************************************************
extern uint32_t CPUcpsid(void);

//*****************************************************************************
//
//! \brief Get the current interrupt state.
//!
//! Use this function to retrieve the current state of the interrupts. This
//! function is implemented as a wrapper function returning the state of
//! PRIMASK.
//!
//! \return Returns the state of the \b PRIMASK (indicating whether interrupts
//! are enabled or disabled).
//
//*****************************************************************************
extern uint32_t CPUprimask(void);

//*****************************************************************************
//
//! \brief Enable all external interrupts.
//!
//! Use this function to enable all system interrupts. This function is
//! implemented as a wrapper function for the CPSIE instruction.
//!
//! \return Returns the state of \b PRIMASK on entry.
//
//*****************************************************************************
extern uint32_t CPUcpsie(void);

//*****************************************************************************
//
//! \brief Wait for interrupt.
//!
//! Use this function to let the CM3 wait for the next interrupt. This
//! function is implemented as a wrapper function for the WFI instruction.
//!
//! \return None
//
//*****************************************************************************
#if defined(codered) || defined(gcc) || defined(sourcerygxx)
__STATIC_INLINE void __attribute__((always_inline))
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
	__asm("    wfi\n");
}
#endif
#if defined(__IAR_SYSTEMS_ICC__) || defined(DOXYGEN)
__STATIC_INLINE void
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
    __asm("    wfi\n");
}
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm __STATIC_INLINE void
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
    wfi;
    bx      lr
}
#endif
#if defined(__TI_COMPILER_VERSION__)
__STATIC_INLINE void
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
    __asm("    wfi\n");
}
#endif

//*****************************************************************************
//
//! \brief Wait for event.
//!
//! Use this function to let the CM3 wait for the next event. This
//! function is implemented as a wrapper function for the WFE instruction.
//!
//! \return None
//
//*****************************************************************************
#if defined(codered) || defined(gcc) || defined(sourcerygxx)
__STATIC_INLINE void __attribute__((always_inline))
CPUwfe(void)
{
    //
    // Wait for the next event.
    //
    __asm("    wfe\n");
}
#endif
#if defined(__IAR_SYSTEMS_ICC__) || defined(DOXYGEN)
__STATIC_INLINE void
CPUwfe(void)
{
    //
    // Wait for the next event.
    //
    __asm("    wfe\n");
}
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm __STATIC_INLINE void
CPUwfe(void)
{
    //
    // Wait for the next event.
    //
    wfe;
    bx      lr
}
#endif
#if defined(__TI_COMPILER_VERSION__)
__STATIC_INLINE void
CPUwfe(void)
{
    //
    // Wait for the next event.
    //
    __asm("    wfe\n");
}
#endif

//*****************************************************************************
//
//! \brief Send event.
//!
//! Use this function to let the CM3 send an event. This function is
//! implemented as a wrapper function for the SEV instruction.
//!
//! \return None
//
//*****************************************************************************
#if defined(codered) || defined(gcc) || defined(sourcerygxx)
__STATIC_INLINE void __attribute__((always_inline))
CPUsev(void)
{
    //
    // Send event.
    //
    __asm("    sev\n");
}
#endif
#if defined(__IAR_SYSTEMS_ICC__) || defined(DOXYGEN)
__STATIC_INLINE void
CPUsev(void)
{
    //
    // Send event.
    //
    __asm("    sev\n");
}
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm __STATIC_INLINE void
CPUsev(void)
{
    //
    // Send event.
    //
    sev;
    bx      lr
}
#endif
#if defined(__TI_COMPILER_VERSION__)
__STATIC_INLINE void
CPUsev(void)
{
    //
    // Send event.
    //
    __asm("    sev\n");
}
#endif


//*****************************************************************************
//
//! \brief Update the interrupt priority disable level.
//!
//! Use this function to change the level of priority that will disable
//! interrupts with a lower priority level.
//!
//! \param ui32NewBasepri is the new basis priority level to set.
//!
//! \return None
//
//*****************************************************************************
#if defined(gcc)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
__STATIC_INLINE void __attribute__ ((naked))
CPUbasepriSet(uint32_t ui32NewBasepri)
{
   //
   // Set the BASEPRI register.
   //
   __asm("    msr     BASEPRI, r0\n"
         "    bx      lr\n");
}
#pragma GCC diagnostic pop
#endif
#if defined(__IAR_SYSTEMS_ICC__) || defined(DOXYGEN)
__STATIC_INLINE void
CPUbasepriSet(uint32_t ui32NewBasepri)
{
    //
    // Set the BASEPRI register.
    //
    __asm("    msr     BASEPRI, r0\n");
}
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm __STATIC_INLINE void
CPUbasepriSet(uint32_t ui32NewBasepri)
{
    //
    // Set the BASEPRI register.
    //
    msr     BASEPRI, r0;
    bx      lr
}
#endif
#if defined(__TI_COMPILER_VERSION__)
__STATIC_INLINE void
CPUbasepriSet(uint32_t ui32NewBasepri)
{
    //
    // Set the BASEPRI register.
    //
    __asm("    msr     BASEPRI, r0\n");
}
#endif

//*****************************************************************************
//
//! \brief Get the interrupt priority disable level.
//!
//! Use this function to get the the level of priority that will disable
//! interrupts with a lower priority level.
//!
//! \return Returns the value of the \b BASEPRI register.
//
//*****************************************************************************
extern uint32_t CPUbasepriGet(void);

//*****************************************************************************
//
//! \brief Provide a small delay.
//!
//! This function provides means for generating a constant length delay. It
//! is written in assembly to keep the delay consistent across tool chains,
//! avoiding the need to tune the delay based on the tool chain in use.
//!
//! The loop takes 3 cycles/loop.
//!
//! \param ui32Count is the number of delay loop iterations to perform.
//!
//! \return None
//
//*****************************************************************************
extern void CPUdelay(uint32_t ui32Count);

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_CPUcpsid
        #undef  CPUcpsid
        #define CPUcpsid                        ROM_CPUcpsid
    #endif
    #ifdef ROM_CPUprimask
        #undef  CPUprimask
        #define CPUprimask                      ROM_CPUprimask
    #endif
    #ifdef ROM_CPUcpsie
        #undef  CPUcpsie
        #define CPUcpsie                        ROM_CPUcpsie
    #endif
    #ifdef ROM_CPUbasepriGet
        #undef  CPUbasepriGet
        #define CPUbasepriGet                   ROM_CPUbasepriGet
    #endif
    #ifdef ROM_CPUdelay
        #undef  CPUdelay
        #define CPUdelay                        ROM_CPUdelay
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

#endif // __CPU_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
