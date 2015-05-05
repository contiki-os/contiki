/******************************************************************************
*  Filename:       spis.c
*  Revised:        2015-01-13 16:59:55 +0100 (ti, 13 jan 2015)
*  Revision:       42365
*
*  Description:    Driver for the SPI Slave.
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

#include <driverlib/spis.h>

//*****************************************************************************
//
// Handle support for DriverLib in ROM:
// This section will undo prototype renaming made in the header file
//
//*****************************************************************************
#ifndef DRIVERLIB_GENERATE_ROM
    #undef  SPISDataPut
    #define SPISDataPut                     NOROM_SPISDataPut
    #undef  SPISTxGetValue
    #define SPISTxGetValue                  NOROM_SPISTxGetValue
    #undef  SPISDataGet
    #define SPISDataGet                     NOROM_SPISDataGet
    #undef  SPISRxGetValue
    #define SPISRxGetValue                  NOROM_SPISRxGetValue
    #undef  SPISIntStatus
    #define SPISIntStatus                   NOROM_SPISIntStatus
#endif

//*****************************************************************************
//
// This is the mapping between an TX Fifo index and the corresponding
// register.
//
//*****************************************************************************
static const uint32_t g_pui32SPISTxFifo[] =
{
    SPIS_O_TXFMEM0, SPIS_O_TXFMEM1, SPIS_O_TXFMEM2, SPIS_O_TXFMEM3, SPIS_O_TXFMEM4,
    SPIS_O_TXFMEM5, SPIS_O_TXFMEM6, SPIS_O_TXFMEM7, SPIS_O_TXFMEM8, SPIS_O_TXFMEM9,
    SPIS_O_TXFMEM10, SPIS_O_TXFMEM11, SPIS_O_TXFMEM12, SPIS_O_TXFMEM13,
    SPIS_O_TXFMEM14, SPIS_O_TXFMEM15
};

//*****************************************************************************
//
// This is the mapping between an RX Fifo index and the corresponding
// register.
//
//*****************************************************************************
static const uint32_t g_pui32SPISRxFifo[] =
{
    SPIS_O_RXFMEM0, SPIS_O_RXFMEM1, SPIS_O_RXFMEM2, SPIS_O_RXFMEM3, SPIS_O_RXFMEM4,
    SPIS_O_RXFMEM5, SPIS_O_RXFMEM6, SPIS_O_RXFMEM7, SPIS_O_RXFMEM8, SPIS_O_RXFMEM9,
    SPIS_O_RXFMEM10, SPIS_O_RXFMEM11, SPIS_O_RXFMEM12, SPIS_O_RXFMEM13,
    SPIS_O_RXFMEM14, SPIS_O_RXFMEM15
};

//*****************************************************************************
//
//! Puts a data element into the SPIS transmit FIFO
//
//*****************************************************************************
void
SPISDataPut(uint32_t ui32Data)
{
    //
    // Wait until there is space.
    //
    while(HWREG(SPIS_BASE + SPIS_O_TXSTAT) & SPIS_TXSTAT_FULL)
    {
    }

    //
    // Write the data to the SPIS Tx Fifo.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFPUSH) = ui32Data;
}

//*****************************************************************************
//
//! Get a specific value in the Tx Fifo
//
//*****************************************************************************
uint32_t
SPISTxGetValue(uint32_t ui32Index)
{
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT(ui32Index < TX_FIFO_SIZE);

    //
    // Find the correct register.
    //
    ui32Reg = g_pui32SPISTxFifo[ui32Index];

    //
    // Return the value of the TX Fifo at the specified index.
    //
    return HWREG(SPIS_BASE + ui32Reg);
}

//*****************************************************************************
//
//! Gets a data element from the SPIS Rx FIFO
//
//*****************************************************************************
void
SPISDataGet(uint32_t *pui32Data)
{
    //
    // Wait until there is data to be read.
    //
    while(!(HWREG(SPIS_BASE + SPIS_O_RXFSTAT) & SPIS_RXFSTAT_NOT_EMPTY))
    {
    }

    //
    // Read data from SPIS Rx Fifo.
    //
    *pui32Data = HWREG(SPIS_BASE + SPIS_O_RXFPOP);
}

//*****************************************************************************
//
//! Get a specific value in the Rx Fifo
//
//*****************************************************************************
uint32_t
SPISRxGetValue(uint32_t ui32Index)
{
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT(ui32Index < RX_FIFO_SIZE);

    //
    // Find the correct register.
    //
    ui32Reg = g_pui32SPISRxFifo[ui32Index];

    //
    // Return the value of the RX Fifo at the specified index.
    //
    return HWREG(SPIS_BASE + ui32Reg);
}

//*****************************************************************************
//
//! Gets the current interrupt status
//!
//! \param bMasked is \b false if the raw interrupt status is required or
//! \b true if the masked interrupt status is required.
//!
//! This function returns the interrupt status for the SPIS module. Either the
//! raw interrupt status or the status of interrupts that are allowed to
//! reflect to the processor can be returned.
//!
//! \return The current interrupt status consisting of a bitwise OR value
//! of the available interrupts sources as described in \b SPISIntEnable().
//
//*****************************************************************************
uint32_t
SPISIntStatus(bool bMasked)
{
    uint32_t ui32IntStatus, ui32Tmp;

    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    if(bMasked)
    {
        ui32Tmp = HWREG(SPIS_BASE + SPIS_O_TXFFLAGSCLRN);
        ui32IntStatus = ui32Tmp & HWREG(SPIS_BASE + SPIS_O_TXFFLAGSMASK);
        ui32Tmp = HWREG(SPIS_BASE + SPIS_O_RXFFLAGSCLRN);
        ui32IntStatus |= (ui32Tmp & HWREG(SPIS_BASE + SPIS_O_RXFFLAGSMASK)) << 8;
        ui32Tmp = HWREG(SPIS_BASE + SPIS_O_GPFLAGS);
        ui32IntStatus |= (ui32Tmp & HWREG(SPIS_BASE + SPIS_O_GPFLAGSMASK)) << 16;
    }
    else
    {
        ui32IntStatus = HWREG(SPIS_BASE + SPIS_O_TXFFLAGSCLRN) & SPIS_TX_MASK;
        ui32IntStatus |= (HWREG(SPIS_BASE + SPIS_O_RXFFLAGSCLRN) << 8) & SPIS_RX_MASK;
        ui32IntStatus |= (HWREG(SPIS_BASE + SPIS_O_GPFLAGS) << 16) & SPIS_GP_MASK;
    }
    return ui32IntStatus;
}
