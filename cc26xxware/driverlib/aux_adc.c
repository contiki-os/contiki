/******************************************************************************
*  Filename:       aux_adc.c
*  Revised:        2015-01-13 16:59:55 +0100 (ti, 13 jan 2015)
*  Revision:       42365
*
*  Description:    Driver for the AUX Time to Digital Converter interface.
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

#include <driverlib/aux_adc.h>
#include <inc/hw_memmap.h>
#include <inc/hw_aux_wuc.h>
#include <driverlib/adi.h>
#include <driverlib/event.h>

//*****************************************************************************
//
//! \brief Disables the ADC
//
//*****************************************************************************
void
AUXADCDisable(void)
{
    // Disable the ADC reference
    ADI8BitsClear(AUX_ADI4_BASE, ADI_4_AUX_O_ADCREF0, ADI_4_AUX_ADCREF0_EN_M | ADI_4_AUX_ADCREF0_REF_ON_IDLE_M | ADI_4_AUX_ADCREF0_SRC_M);

    // Assert reset and disable the ADC
    ADI8BitsClear(AUX_ADI4_BASE, ADI_4_AUX_O_ADC0, ADI_4_AUX_ADC0_EN_M | ADI_4_AUX_ADC0_RESET_N_M | ADI_4_AUX_ADC0_SMPL_MODE_M | ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_M);

    // Disable the ADC clock (no need to wait since IOB_WUC_ADCCLKCTL_ACK goes low immediately)
    HWREG(AUX_WUC_BASE + AUX_WUC_O_ADCCLKCTL) = 0;

    // Disable the ADC data interface
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) = 0;
}

//*****************************************************************************
//
// Enables the ADC for asynchronous operation
//
//*****************************************************************************
void
AUXADCEnableAsync(uint32_t refSource, uint32_t trigger)
{
    // Enable the ADC reference, with the following options:
    // - SRC: Set when using relative reference
    // - REF_ON_IDLE: Always cleared since there is no idle state in asynchronous operation
    ADI8BitsSet(AUX_ADI4_BASE, ADI_4_AUX_O_ADCREF0, refSource | ADI_4_AUX_ADCREF0_EN_M);

    // Enable the ADC clock
    HWREG(AUX_WUC_BASE + AUX_WUC_O_ADCCLKCTL) = AUX_WUC_ADCCLKCTL_REQ_M;
    while (!(HWREG(AUX_WUC_BASE + AUX_WUC_O_ADCCLKCTL) & AUX_WUC_ADCCLKCTL_ACK_M));

    // Enable the ADC data interface
    if (trigger == AUXADC_TRIGGER_MANUAL) {
        // Manual trigger: No need to configure event routing from GPT
        HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) = AUX_ANAIF_ADCCTL_START_SRC_NO_EVENT0 | AUX_ANAIF_ADCCTL_CMD_EN;
    } else {
        // GPT trigger: Configure event routing via MCU_EV to the AUX domain
        HWREG(EVENT_BASE + EVENT_O_AUXSEL0) = trigger;
        HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) = AUX_ANAIF_ADCCTL_START_SRC_MCU_EV | AUX_ANAIF_ADCCTL_CMD_EN;
    }

    // Release reset and enable the ADC
    ADI8BitsSet(AUX_ADI4_BASE, ADI_4_AUX_O_ADC0, ADI_4_AUX_ADC0_EN_M | ADI_4_AUX_ADC0_RESET_N_M | ADI_4_AUX_ADC0_SMPL_MODE_M);
}

//*****************************************************************************
//
// Enables the ADC for synchronous operation
//
//*****************************************************************************
void
AUXADCEnableSync(uint32_t refSource, uint32_t sampleTime, uint32_t trigger)
{
    // Enable the ADC reference, with the following options:
    // - SRC: Set when using relative reference
    // - REF_ON_IDLE: Set when using fixed reference and sample time < 6
    uint8_t adcref0 = refSource | ADI_4_AUX_ADCREF0_EN_M;
    if (!refSource && (sampleTime < 6)) {
        adcref0 |= ADI_4_AUX_ADCREF0_REF_ON_IDLE_M;
    }
    ADI8BitsSet(AUX_ADI4_BASE, ADI_4_AUX_O_ADCREF0, adcref0);

    // Enable the ADC clock
    HWREG(AUX_WUC_BASE + AUX_WUC_O_ADCCLKCTL) = AUX_WUC_ADCCLKCTL_REQ_M;
    while (!(HWREG(AUX_WUC_BASE + AUX_WUC_O_ADCCLKCTL) & AUX_WUC_ADCCLKCTL_ACK_M));

    // Enable the ADC data interface
    if (trigger == AUXADC_TRIGGER_MANUAL) {
        // Manual trigger: No need to configure event routing from GPT
        HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) = AUX_ANAIF_ADCCTL_START_SRC_NO_EVENT0 | AUX_ANAIF_ADCCTL_CMD_EN;
    } else {
        // GPT trigger: Configure event routing via MCU_EV to the AUX domain
        HWREG(EVENT_BASE + EVENT_O_AUXSEL0) = trigger;
        HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) = AUX_ANAIF_ADCCTL_START_SRC_MCU_EV | AUX_ANAIF_ADCCTL_CMD_EN;
    }

    // Release reset and enable the ADC
    ADI8BitsSet(AUX_ADI4_BASE, ADI_4_AUX_O_ADC0, ADI_4_AUX_ADC0_EN_M | ADI_4_AUX_ADC0_RESET_N_M | sampleTime);
}

//*****************************************************************************
//
// Flushes the ADC FIFO
//
//*****************************************************************************
void
AUXADCFlushFifo(void)
{
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) |= 0x00000002;
    HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCCTL) &= 0x00000002;
}

//*****************************************************************************
//
// Waits for and returns the first sample in the ADC FIFO
//
//*****************************************************************************
uint32_t
AUXADCReadFifo(void) {

    // Wait until there is at least one sample in the FIFO
    while (HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCFIFOSTAT) & AUX_ANAIF_ADCFIFOSTAT_EMPTY_M);

    // Return the first sample from the FIFO
    return HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCFIFO);
}
