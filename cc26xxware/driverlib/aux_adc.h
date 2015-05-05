/******************************************************************************
*  Filename:       aux_adc.h
*  Revised:        2015-01-13 16:59:55 +0100 (ti, 13 jan 2015)
*  Revision:       42365
*
*  Description:    Defines and prototypes for the AUX Analog-to-Digital
*                  Converter
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
//! \addtogroup auxadc_api
//! @{
//
//*****************************************************************************

#ifndef __AUX_ADC_H__
#define __AUX_ADC_H__

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
#include <inc/hw_event.h>
#include <inc/hw_adi.h>
#include <inc/hw_adi_4_aux.h>
#include <inc/hw_aux_anaif.h>
#include <driverlib/rom.h>

//*****************************************************************************
//
// Defines for ADC reference sources.
//
//*****************************************************************************
#define AUXADC_REF_FIXED                (0 << ADI_4_AUX_ADCREF0_SRC_S)
#define AUXADC_REF_VDDA_REL             (1 << ADI_4_AUX_ADCREF0_SRC_S)

//*****************************************************************************
//
// Defines for the ADC FIFO status bits.
//
//*****************************************************************************
#define AUXADC_FIFO_EMPTY_M             AUX_ANAIF_ADCFIFOSTAT_EMPTY_M
#define AUXADC_FIFO_ALMOST_FULL_M       AUX_ANAIF_ADCFIFOSTAT_ALMOST_FULL_M
#define AUXADC_FIFO_FULL_M              AUX_ANAIF_ADCFIFOSTAT_FULL_M
#define AUXADC_FIFO_UNDERFLOW_M         AUX_ANAIF_ADCFIFOSTAT_UNDERFLOW_M
#define AUXADC_FIFO_OVERFLOW_M          AUX_ANAIF_ADCFIFOSTAT_OVERFLOW_M

//*****************************************************************************
//
// Defines for supported ADC triggers.
//
//*****************************************************************************
#define AUXADC_TRIGGER_MANUAL           EVENT_AUXSEL0_EV_NONE
#define AUXADC_TRIGGER_GPT0A            EVENT_AUXSEL0_EV_GPT0A
#define AUXADC_TRIGGER_GPT0B            EVENT_AUXSEL0_EV_GPT0B
#define AUXADC_TRIGGER_GPT1A            EVENT_AUXSEL0_EV_GPT1A
#define AUXADC_TRIGGER_GPT1B            EVENT_AUXSEL0_EV_GPT1B
#define AUXADC_TRIGGER_GPT2A            EVENT_AUXSEL0_EV_GPT2A
#define AUXADC_TRIGGER_GPT2B            EVENT_AUXSEL0_EV_GPT2B
#define AUXADC_TRIGGER_GPT3A            EVENT_AUXSEL0_EV_GPT3A
#define AUXADC_TRIGGER_GPT3B            EVENT_AUXSEL0_EV_GPT3B

//*****************************************************************************
//
// Defines for ADC sampling type for synchronous operation.
//
//*****************************************************************************
#define AUXADC_SAMPLE_TIME_2P7_US       (3 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_5P3_US       (4 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_10P6_US      (5 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_21P3_US      (6 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_42P6_US      (7 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_85P3_US      (8 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_170_US       (9 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_341_US       (10 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_682_US       (11 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_1P37_MS      (12 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_2P73_MS      (13 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_5P46_MS      (14 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)
#define AUXADC_SAMPLE_TIME_10P9_MS      (15 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S)


//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************


//*****************************************************************************
//
//! \brief Disables the ADC
//
//*****************************************************************************
extern void
AUXADCDisable(void);

//*****************************************************************************
//
//! \brief Enables the ADC for asynchronous operation
//!
//! In asynchronous operation, the ADC samples continuously between
//! conversions.
//!
//! The ADC trigger starts the conversion. Note that the first conversion may
//! be invalid if the sampling period is too short.
//!
//! \param refSource
//!     ADC reference source:
//!     - \ref AUXADC_REF_FIXED (nominally 4.3 V)
//!     - \ref AUXADC_REF_VDDA_REL (nominally VDDS)
//! \param trigger
//!     ADC conversion trigger:
//!     - \ref AUXADC_TRIGGER_MANUAL
//!     - \ref AUXADC_TRIGGER_GPT0A
//!     - \ref AUXADC_TRIGGER_GPT0B
//!     - \ref AUXADC_TRIGGER_GPT1A
//!     - \ref AUXADC_TRIGGER_GPT1B
//!     - \ref AUXADC_TRIGGER_GPT2A
//!     - \ref AUXADC_TRIGGER_GPT2B
//!     - \ref AUXADC_TRIGGER_GPT3A
//!     - \ref AUXADC_TRIGGER_GPT3B
//
//*****************************************************************************
extern void
AUXADCEnableAsync(uint32_t refSource, uint32_t trigger);

//*****************************************************************************
//
//! \brief Enables the ADC for synchronous operation
//!
//! In synchronous operation, the ADC is idle between a conversion and
//! subsequent samplings.
//!
//! The ADC trigger starts sampling with specified duration, followed by the
//! conversion. Note that the first conversion may be invalid if the sampling
//! period is too short.
//!
//! \param refSource
//!     ADC reference source:
//!     - \ref AUXADC_REF_FIXED (nominally 4.3 V)
//!     - \ref AUXADC_REF_VDDA_REL (nominally VDDS)
//! \param sampleTime
//!     ADC sampling time:
//!     - \ref AUXADC_SAMPLE_TIME_2P7_US
//!     - \ref AUXADC_SAMPLE_TIME_5P3_US
//!     - \ref AUXADC_SAMPLE_TIME_10P6_US
//!     - \ref AUXADC_SAMPLE_TIME_21P3_US
//!     - \ref AUXADC_SAMPLE_TIME_42P6_US
//!     - \ref AUXADC_SAMPLE_TIME_85P3_US
//!     - \ref AUXADC_SAMPLE_TIME_170_US
//!     - \ref AUXADC_SAMPLE_TIME_341_US
//!     - \ref AUXADC_SAMPLE_TIME_682_US
//!     - \ref AUXADC_SAMPLE_TIME_1P37_MS
//!     - \ref AUXADC_SAMPLE_TIME_2P73_MS
//!     - \ref AUXADC_SAMPLE_TIME_5P46_MS
//!     - \ref AUXADC_SAMPLE_TIME_10P9_MS
//! \param trigger
//!     ADC conversion trigger:
//!     - \ref AUXADC_TRIGGER_MANUAL
//!     - \ref AUXADC_TRIGGER_GPT0A
//!     - \ref AUXADC_TRIGGER_GPT0B
//!     - \ref AUXADC_TRIGGER_GPT1A
//!     - \ref AUXADC_TRIGGER_GPT1B
//!     - \ref AUXADC_TRIGGER_GPT2A
//!     - \ref AUXADC_TRIGGER_GPT2B
//!     - \ref AUXADC_TRIGGER_GPT3A
//!     - \ref AUXADC_TRIGGER_GPT3B
//
//*****************************************************************************
extern void
AUXADCEnableSync(uint32_t refSource, uint32_t sampleTime, uint32_t trigger);

//*****************************************************************************
//
//! \brief Flushes the ADC FIFO
//!
//! This empties the FIFO and clears the underflow/overflow flags.
//!
//! Note: This function must only be called while the ADC is enabled.
//
//*****************************************************************************
extern void
AUXADCFlushFifo(void);

//*****************************************************************************
//
//! \brief Generates a single manual ADC trigger
//!
//! For synchronous mode, the trigger starts sampling followed by conversion.
//! For asynchronous mode, the trigger starts conversion.
//
//*****************************************************************************
__STATIC_INLINE void
AUXADCGenManualTrigger(void)
{
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCTRIG) = 0;
}

//*****************************************************************************
//
//! \brief Returns flags indicating the status of the ADC FIFO.
//!
//! The flags indicate FIFO empty, full and almost full, and whether
//! overflow/underflow has occurred.
//!
//! \return
//|     A combination (bitwise OR) of the following flags:
//!     - \ref AUXADC_FIFO_EMPTY_M
//!     - \ref AUXADC_FIFO_ALMOST_FULL_M
//!     - \ref AUXADC_FIFO_FULL_M
//!     - \ref AUXADC_FIFO_UNDERFLOW_M
//!     - \ref AUXADC_FIFO_OVERFLOW_M
//
//*****************************************************************************
__STATIC_INLINE uint32_t
AUXADCGetFifoStatus(void)
{
    return HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCFIFOSTAT);
}

//*****************************************************************************
//
//! \brief Waits for and returns the first sample in the ADC FIFO
//!
//! This function waits until there is at least one sample in the ADC FIFO. It
//! then pops and returns the first sample from the FIFO.
//!
//! Note that this procedure will deadlock if called without setting up ADC
//! trigger generation in advance. The trigger can either be manual or
//! periodical (using a GPT).
//!
//! \return The first (12-bit) sample from the ADC FIFO
//
//*****************************************************************************
extern uint32_t
AUXADCReadFifo(void);

//*****************************************************************************
//
//! \brief Selects internal or external input for the ADC
//!
//! Note that calling this function also selects the same input for AUX_COMPB.
//!
//! \param input
//!     Internal/external input selection:
//!     - \ref ADC_COMPB_IN_VDD1P2V
//!     - \ref ADC_COMPB_IN_VSSA
//!     - \ref ADC_COMPB_IN_VDDA3P3V
//!     - \ref ADC_COMPB_IN_AUXIO7
//!     - \ref ADC_COMPB_IN_AUXIO6
//!     - \ref ADC_COMPB_IN_AUXIO5
//!     - \ref ADC_COMPB_IN_AUXIO4
//!     - \ref ADC_COMPB_IN_AUXIO3
//!     - \ref ADC_COMPB_IN_AUXIO2
//!     - \ref ADC_COMPB_IN_AUXIO1
//!     - \ref ADC_COMPB_IN_AUXIO0
//
//*****************************************************************************
__STATIC_INLINE void
AUXADCSelectInput(uint32_t input)
{
    HapiSelectADCCompBInput(input);
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __AUX_ADC_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
