/******************************************************************************
*  Filename:       spis.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Prototypes and macros for the SPI Slave controller
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
//! \addtogroup spis_api
//! @{
//
//*****************************************************************************

#ifndef __SPIS_H__
#define __SPIS_H__

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
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <inc/hw_spis.h>
#include <driverlib/debug.h>
#include <driverlib/interrupt.h>

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
    #define SPISDataPut                     NOROM_SPISDataPut
    #define SPISTxGetValue                  NOROM_SPISTxGetValue
    #define SPISDataGet                     NOROM_SPISDataGet
    #define SPISRxGetValue                  NOROM_SPISRxGetValue
    #define SPISIntStatus                   NOROM_SPISIntStatus
#endif

//*****************************************************************************
//
// FIFO size
//
//*****************************************************************************
#define TX_FIFO_SIZE            16          // Size of the TX FIFO
#define RX_FIFO_SIZE            16          // Size of the RX FIFO

//*****************************************************************************
//
// SPIS configuration parameters
//
//*****************************************************************************
#define SPIS_POS_CLK_POL        0x00000001  // Data captured on rising edge
#define SPIS_NEG_CLK_POL        0x00000000  // Data captured on falling edge
#define SPIS_TX_BIG_ENDIAN      0x00000002  // TX FIFO is big endian
#define SPIS_TX_LITTLE_ENDIAN   0x00000000  // TX FIFO is little endian
#define SPIS_RX_BIG_ENDIAN      0x00000004  // RX FIFO is big endian
#define SPIS_RX_LITTLE_ENDIAN   0x00000000  // RX FIFO is little endian
#define SPIS_TX_DMA_SINGLE      0x00000008  // TX FIFO single DMA request
#define SPIS_TX_DMA_BURST       0x00000000  // TX FIFO burst DMA request
#define SPIS_RX_DMA_SINGLE      0x00000010  // RX FIFO single DMA request
#define SPIS_RX_DMA_BURST       0x00000000  // RX FIFO burst DMA request

//*****************************************************************************
//
// Event source that can be mask to generate interrupts
//
//*****************************************************************************
#define SPIS_TX_FULL            0x00000001  // TX FIFO is full
#define SPIS_TX_EMPTY           0x00000002  // TX FIFO is empty
#define SPIS_TX_GE_WATERMARK    0x00000004  // TX FIFO is above/equal watermark
#define SPIS_TX_LE_WATERMARK    0x00000008  // TX FIFO is below/equal watermark
#define SPIS_TX_HASDATA         0x00000010  // TX FIFO has data
#define SPIS_TX_UNDER_FLOW      0x00000020  // TX FIFO is out of data
#define SPIS_TX_OVER_FLOW       0x00000040  // TX FIFO write attempt failed
#define SPIS_TX_MASK            0x0000007F  // TX FIFO event mask
#define SPIS_RX_FULL            0x00000100  // RX FIFO is full
#define SPIS_RX_EMPTY           0x00000200  // RX FIFO is empty
#define SPIS_RX_GE_WATERMARK    0x00000400  // RX FIFO is above/equal watermark
#define SPIS_RX_LE_WATERMARK    0x00000800  // RX FIFO is below/equal watermark
#define SPIS_RX_HASDATA         0x00001000  // RX FIFO has data
#define SPIS_RX_UNDER_FLOW      0x00002000  // RX FIFO read attempt failed
#define SPIS_RX_OVER_FLOW       0x00004000  // RX FIFO is full + 1
#define SPIS_RX_MASK            0x00007F00  // RX FIFO event mask
#define SPIS_CHIP_SELECT        0x00010000  // Chip select asserted
#define SPIS_INCOMPLETE         0x00020000  // Incomplete transfer
#define SPIS_PRX_OVERFLOW       0x00040000  // RX overflow
#define SPIS_BYTE_DONE          0x00080000  // Byte transfer complete
#define SPIS_DMA_DONE_TX        0x00100000  // DMA done for TX FIFO Channel
#define SPIS_DMA_DONE_RX        0x00200000  // DMA done for RX FIFO Channel
#define SPIS_GP_MASK            0x003F0000

//*****************************************************************************
//
// Values used to setup the DMA controller to listen on events from the RX/TX
// FIFO status registers
//
//*****************************************************************************
#define SPIS_TX_DMA_FULL      ( SPIS_TXFEVSRC_SEL_FULL       ) // TX FIFO is full
#define SPIS_TX_DMA_EMPTY     ( SPIS_TXFEVSRC_SEL_EMPTY      ) // TX FIFO is empty
#define SPIS_TX_DMA_GE_WMARK  ( SPIS_TXFEVSRC_SEL_GE_THR     ) // TX FIFO is above/equal watermark
#define SPIS_TX_DMA_LE_WMARK  ( SPIS_TXFEVSRC_SEL_LE_THR     ) // TX FIFO is below/equal watermark
#define SPIS_TX_DMA_HASDATA   ( SPIS_TXFEVSRC_SEL_NOT_EMPTY  ) // TX FIFO has data
#define SPIS_TX_DMA_ALWAYS    ( SPIS_TXFEVSRC_SEL_ONE        ) // Always '1'
#define SPIS_TX_DMA_NONE      ( SPIS_TXFEVSRC_SEL_ZERO       ) // Always '0'

#define SPIS_RX_DMA_FULL      (( SPIS_RXFEVSRC_SEL_FULL      ) << 8 ) // RX FIFO is full
#define SPIS_RX_DMA_EMPTY     (( SPIS_RXFEVSRC_SEL_EMPTY     ) << 8 ) // RX FIFO is empty
#define SPIS_RX_DMA_GE_WMARK  (( SPIS_RXFEVSRC_SEL_GE_THR    ) << 8 ) // RX FIFO is above/equal watermark
#define SPIS_RX_DMA_LE_WMARK  (( SPIS_RXFEVSRC_SEL_LE_THR    ) << 8 ) // RX FIFO is below/equal watermark
#define SPIS_RX_DMA_HASDATA   (( SPIS_RXFEVSRC_SEL_NOT_EMPTY ) << 8 ) // RX FIFO has data
#define SPIS_RX_DMA_ALWAYS    (( SPIS_RXFEVSRC_SEL_ONE       ) << 8 ) // Always '1'
#define SPIS_RX_DMA_NONE      (( SPIS_RXFEVSRC_SEL_ZERO      ) << 8 ) // Always '0'

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Configures the SPIS module.
//!
//! This function configures the clock polarity, data endianness, DMA
//! single/burst request mode and the watermark of the RX/TX FIFO. If the
//! watermark in each FIFO is not used to generate events, the value should be
//! zero (0x0).
//!
//! The SPIS module has no timing constraints. It always runs 1:1 with the
//! system clock (maximum of 48 MHz). Note however, that if the system clock
//! is slower than 48 MHz the SPI clock input must be lowered accordingly in
//! so, that it never exceeds the 1:1 relation with the system clock.
//!
//! \param ui32Config is the configuration parameter.
//! The parameter should be a bitwise OR of the following values.
//! - Clock polarity:
//!   - \ref SPIS_POS_CLK_POL
//!   - \ref SPIS_NEG_CLK_POL
//! - TX FIFO endianess:
//!   - \ref SPIS_TX_BIG_ENDIAN
//!   - \ref SPIS_TX_LITTLE_ENDIAN
//! - RX FIFO endianess:
//!   - \ref SPIS_RX_BIG_ENDIAN
//!   - \ref SPIS_RX_LITTLE_ENDIAN
//! - TX FIFO DMA request type:
//!   - \ref SPIS_TX_DMA_SINGLE
//!   - \b SPIS_TX_DMA_BURST
//! - RX FIFO DMA request type:
//!   - \ref SPIS_RX_DMA_SINGLE
//!   - \ref SPIS_RX_DMA_BURST
//! \param ui32RxWatermark defines the RX buffer watermark.
//! \param ui32TxWatermark defines the TX buffer watermark.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISConfig(uint32_t ui32Config, uint32_t ui32RxWatermark,
           uint32_t ui32TxWatermark)
{
    //
    // Set the master enable bit in the config register.
    //
    HWREG(SPIS_BASE + SPIS_O_RXFTHR) = ui32RxWatermark & SPIS_RXFTHR_CNT_M;

    //
    // Set the master enable bit in the config register.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFTHR) = ui32TxWatermark & SPIS_TXFTHR_CNT_M;

    //
    // Configure the format and DMA operation of the SPI Slave.
    //
    HWREG(SPIS_BASE + SPIS_O_CFG) = ui32Config & 0x1F;
}

//*****************************************************************************
//
//! \brief Puts a data element into the SPIS transmit FIFO.
//!
//! This function places the supplied data into the transmit FIFO of the
//! specified SPIS module.
//!
//! \note The upper 24 bits of the \c ui32Data are discarded by the hardware.
//!
//! \param ui32Data is the data to be transmitted over the SPIS interface.
//!
//! \return None
//
//*****************************************************************************
extern void SPISDataPut(uint32_t ui32Data);

//*****************************************************************************
//
//! \brief Puts a data element into the SPIS transmit FIFO.
//!
//! This function places the supplied data into the transmit FIFO of the
//! specified SPIS module. If there is no space in the FIFO, then this
//! function returns a zero.
//!
//! \note The upper 24 bits of the \c ui32Data are discarded by the hardware.
//!
//! \param ui32Data is the data to be transmitted over the SPIS interface.
//!
//! \return Returns the number of elements written to the SPIS transmit FIFO
//
//*****************************************************************************
__STATIC_INLINE int32_t
SPISDataPutNonBlocking(uint32_t ui32Data)
{
    //
    // Check for space to write.
    //
    if(!(HWREG(SPIS_BASE + SPIS_O_TXSTAT) & SPIS_TXSTAT_FULL))
    {
        HWREG(SPIS_BASE + SPIS_O_TXFPUSH) = ui32Data;
        return(1);
    }
    else
    {
        return(0);
    }
}

//*****************************************************************************
//
//! \brief Flush the Tx FIFO.
//!
//! Use this function to flush the Tx FIFO. The status register for the Tx FIFO
//! is updated accordingly.
//!
//! \note No flags are cleared by flushing the FIFO, but some flags might be
//! set due to the change in the FIFO content.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISTxFlush(void)
{
    //
    // Flush the transmit FIFO.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFFLUSH) = 0x1;

    //
    // Clear The Hasdata Flag.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFFLAGSCLRN) = ~SPIS_TXFFLAGSCLRN_NOT_EMPTY;
}

//*****************************************************************************
//
//! \brief Get the current number of data elements in the TX FIFO.
//!
//! Use this function to get the current number of bytes stored in the Tx FIFO.
//!
//! \return Returns the number of bytes stored in the TX FIFO.
//
//*****************************************************************************
__STATIC_INLINE uint32_t
SPISTxGetNumBytes(void)
{
    //
    // Return the current number of data elements in the TX FIFO.
    //
    return HWREG(SPIS_BASE + SPIS_O_TXFCNT);
}

//*****************************************************************************
//
//! \brief Get a specific value in the Tx FIFO.
//!
//! This function can be used to peek into the Tx FIFO and retrieve a value
//! at a specific location in the buffer.
//!
//! \param ui32Index specifies the index of the value in the Tx FIFO to fetch.
//!
//! \return Returns the value in the Tx FIFO specified by the \c ui32Index.
//
//*****************************************************************************
extern uint32_t SPISTxGetValue(uint32_t ui32Index);

//*****************************************************************************
//
//! \brief Gets a data element from the SPIS Rx FIFO.
//!
//! This function gets received data from the receive FIFO of the SPIS module
//! and places that data into the location specified by the
//! \c pui32Data parameter.
//!
//! \note Only the lower 8 bits of the value written to \c pui32Data contain
//! valid data.
//!
//! \param pui32Data is a pointer to a storage location for data that was
//! received over the SPIS interface.
//!
//! \return None
//
//*****************************************************************************
extern void SPISDataGet(uint32_t *pui32Data);

//*****************************************************************************
//
//! \brief Gets a data element in a non-blocking fashion from the SPIS receive FIFO.
//!
//! This function gets received data from the receive FIFO of the specified
//! SPIS module and places that data into the location specified by the
//! \c pui32Data parameter. If there is no data in the FIFO, then this function
//! returns a zero.
//!
//! \note Only the lower 8 bits of the value written to \c pui32Data contain
//! valid data.
//!
//! \param pui32Data is a pointer to a storage location for data that was
//! received over the SPIS interface.
//!
//! \return Returns the number of elements read from the SPIS receive FIFO
//
//*****************************************************************************
__STATIC_INLINE int32_t
SPISDataGetNonBlocking(uint32_t *pui32Data)
{
    //
    // Check for data to read.
    //
    if(HWREG(SPIS_BASE + SPIS_O_RXFSTAT) & SPIS_RXFSTAT_NOT_EMPTY)
    {
        *pui32Data = HWREG(SPIS_BASE + SPIS_O_RXFPOP);
        return(1);
    }
    else
    {
        return(0);
    }
}

//*****************************************************************************
//
//! \brief Flush the Rx FIFO.
//!
//! Use this function to flush the Rx FIFO. The status register for the Rx FIFO
//! is updated accordingly.
//!
//! \note While the \ref SPIS_RX_HASDATA flag is explicitly cleared when flushing
//! the RX FIFO to avoid that both \ref SPIS_RX_HASDATA and \ref SPIS_RX_EMPTY,
//! are set at the same time, other flags might retain there values even though
//! the conditions responsible for setting the flag have changed.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISRxFlush(void)
{
    //
    // Flush the receive FIFO.
    //
    HWREG(SPIS_BASE + SPIS_O_RXFFLUSH) = 0x1;

    //
    // Clear the HASDATA flag.
    //
    HWREG(SPIS_BASE + SPIS_O_RXFFLAGSCLRN) = ~SPIS_RXFFLAGSCLRN_NOT_EMPTY;
}

//*****************************************************************************
//
//! \brief Get the current number of bytes in the RX FIFO.
//!
//! Use this function to get the current number of bytes stored in the Rx FIFO.
//!
//! \return Returns the number of bytes stored in the RX FIFO.
//
//*****************************************************************************
__STATIC_INLINE uint32_t
SPISRxGetNumBytes(void)
{
    //
    // Return the current number of bytes in the RX FIFO.
    //
    return HWREG(SPIS_BASE + SPIS_O_RXCNT);
}

//*****************************************************************************
//
//! \brief Get a specific value in the Rx FIFO.
//!
//! This function can be used to peek into the Rx FIFO and retrieve a value
//! at a specific location in the buffer.
//!
//! \param ui32Index specifies the index of the value in the Rx FIFO to fetch.
//!
//! \return Returns the value in the Tx FIFO specified by the \c ui32Index
//
//*****************************************************************************
extern uint32_t SPISRxGetValue(uint32_t ui32Index);

//*****************************************************************************
//
//! \brief Determines whether the SPIS transmitter is busy or not.
//!
//! Allows the caller to determine whether all transmitted bytes have cleared
//! the transmitter hardware. If \c false is returned, then the transmit FIFO
//! is empty and all bits of the last transmitted word have left the hardware
//! shift register.
//!
//! \return Returns status of SPIS transmitter.
//! - \c true  : The SPIS has data in the TX buffer.
//! - \c false : The buffer is empty.
//
//*****************************************************************************
__STATIC_INLINE bool
SPISBusy(void)
{
    //
    // Determine if the SPIS is busy.
    //
    return((HWREG(SPIS_BASE + SPIS_O_TXSTAT) & SPIS_TXSTAT_FULL) ? true : false);
}

//*****************************************************************************
//
//! \brief Enables individual SPIS interrupt sources.
//!
//! Enables the indicated SPIS interrupt sources. Only the sources that are
//! enabled can be reflected to the processor interrupt; disabled sources have
//! no effect on the processor.
//!
//! The \c ui32IntFlags parameter contains the
//! maskable interrupt source for both the RX/TX FIFO and the General purpose
//! flags.
//!
//! \param ui32IntFlags is a bit mask of the interrupt sources to be enabled.
//! - The TX event mask must be a bitwise OR of the values:
//!   - \ref SPIS_TX_FULL
//!   - \ref SPIS_TX_EMPTY
//!   - \ref SPIS_TX_GE_WATERMARK
//!   - \ref SPIS_TX_LE_WATERMARK
//!   - \ref SPIS_TX_HASDATA
//!   - \ref SPIS_TX_UNDER_FLOW
//!   - \ref SPIS_TX_OVER_FLOW
//! - The RX event mask must be an bitwise OR of the values:
//!   - \ref SPIS_RX_FULL
//!   - \ref SPIS_RX_EMPTY
//!   - \ref SPIS_RX_GE_WATERMARK
//!   - \ref SPIS_RX_LE_WATERMARK
//!   - \ref SPIS_RX_HASDATA
//!   - \ref SPIS_RX_UNDER_FLOW
//!   - \ref SPIS_RX_OVER_FLOW
//! - The General Purpose event mask must be an bitwise OR of the values:
//!   - \ref SPIS_CHIP_SELECT
//!   - \ref SPIS_INCOMPLETE
//!   - \ref SPIS_PRX_OVERFLOW
//!   - \ref SPIS_BYTE_DONE
//!   - \ref SPIS_DMA_DONE_RX
//!   - \ref SPIS_DMA_DONE_TX
//!   - \ref SPIS_GP_MASK
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISIntEnable(uint32_t ui32IntFlags)
{
    //
    // Set the maskable TX interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFFLAGSMASK) = ui32IntFlags & SPIS_TX_MASK;

    //
    // Set the maskable RX interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_RXFFLAGSMASK) = (ui32IntFlags & SPIS_RX_MASK) >> 8;

    //
    // Set the maskable general interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_GPFLAGSMASK) = (ui32IntFlags & SPIS_GP_MASK) >> 16;
}

//*****************************************************************************
//
//! \brief Disables individual SPIS interrupt sources.
//!
//! Disables the indicated SPIS interrupt sources. Only the sources that are
//! enabled can be reflected to the processor interrupt; disabled sources have
//! no effect on the processor.
//!
//! The \c ui32IntFlags parameter contains the
//! maskable interrupt source for both the RX/TX FIFO and the General purpose
//! flags
//!
//! \param ui32IntFlags is a bit mask of the interrupt sources to be disabled.
//! - The TX event mask must be a bitwise OR of the values:
//!   - \ref SPIS_TX_FULL
//!   - \ref SPIS_TX_EMPTY
//!   - \ref SPIS_TX_GE_WATERMARK
//!   - \ref SPIS_TX_LE_WATERMARK
//!   - \ref SPIS_TX_HASDATA
//!   - \ref SPIS_TX_UNDER_FLOW
//!   - \ref SPIS_TX_OVER_FLOW
//! - The RX event mask must be an bitwise OR of the values:
//!   - \ref SPIS_RX_FULL
//!   - \ref SPIS_RX_EMPTY
//!   - \ref SPIS_RX_GE_WATERMARK
//!   - \ref SPIS_RX_LE_WATERMARK
//!   - \ref SPIS_RX_HASDATA
//!   - \ref SPIS_RX_UNDER_FLOW
//!   - \ref SPIS_RX_OVER_FLOW
//! - The General Purpose event mask must be an bitwise OR of the values:
//!   - \ref SPIS_CHIP_SELECT
//!   - \ref SPIS_INCOMPLETE
//!   - \ref SPIS_PRX_OVERFLOW
//!   - \ref SPIS_BYTE_DONE
//!   - \ref SPIS_DMA_DONE_RX
//!   - \ref SPIS_DMA_DONE_TX
//!   - \ref SPIS_GP_MASK
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISIntDisable(uint32_t ui32IntFlags)
{
    //
    // Disable the maskable TX interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFFLAGSMASK) &= ~(ui32IntFlags & SPIS_TX_MASK);

    //
    // Disable the maskable RX interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_RXFFLAGSMASK) &= ~((ui32IntFlags & SPIS_RX_MASK) >> 8);

    //
    // Disable the maskable general interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_GPFLAGSMASK) &= ~((ui32IntFlags & SPIS_GP_MASK) >> 16);
}

//*****************************************************************************
//
//! \brief Clears SPIS interrupt sources.
//!
//! The specified SPIS interrupt sources are cleared so that they no longer
//! assert. This function must be called in the interrupt handler to keep the
//! interrupts from being recognized again immediately upon exit.
//!
//! \note Because there is a write buffer in the Cortex-M3 processor, it may
//! take several clock cycles before the interrupt source is actually cleared.
//! Therefore, it is recommended that the interrupt source be cleared early in
//! the interrupt handler (as opposed to the very last action) to avoid
//! returning from the interrupt handler before the interrupt source is
//! actually cleared. Failure to do so may result in the interrupt handler
//! being immediately reentered (because the interrupt controller still sees
//! the interrupt source asserted).
//!
//! TBD - How do we clear the DMA interrupts
//!
//! \param ui32IntFlags is a bit mask of the interrupt sources to be cleared.
//! - The TX event mask must be a bitwise OR of the values:
//!   - \ref SPIS_TX_FULL
//!   - \ref SPIS_TX_EMPTY
//!   - \ref SPIS_TX_GE_WATERMARK
//!   - \ref SPIS_TX_LE_WATERMARK
//!   - \ref SPIS_TX_HASDATA
//!   - \ref SPIS_TX_UNDER_FLOW
//!   - \ref SPIS_TX_OVER_FLOW
//! - The RX event mask must be an bitwise OR of the values:
//!   - \ref SPIS_RX_FULL
//!   - \ref SPIS_RX_EMPTY
//!   - \ref SPIS_RX_GE_WATERMARK
//!   - \ref SPIS_RX_LE_WATERMARK
//!   - \ref SPIS_RX_HASDATA
//!   - \ref SPIS_RX_UNDER_FLOW
//!   - \ref SPIS_RX_OVER_FLOW
//! - The General Purpose event mask must be an bitwise OR of the values:
//!   - \ref SPIS_CHIP_SELECT
//!   - \ref SPIS_INCOMPLETE
//!   - \ref SPIS_PRX_OVERFLOW
//!   - \ref SPIS_BYTE_DONE
//!   - \ref SPIS_DMA_DONE_RX
//!   - \ref SPIS_DMA_DONE_TX
//!   - \ref SPIS_GP_MASK
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISIntClear(uint32_t ui32IntFlags)
{
    //
    // Clear the TX interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_TXFFLAGSCLRN) = ~(ui32IntFlags & SPIS_TX_MASK);

    //
    // Clear the RX interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_RXFFLAGSCLRN) = ~((ui32IntFlags & SPIS_RX_MASK) >> 8);

    //
    // Clear the general interrupt source.
    //
    HWREG(SPIS_BASE + SPIS_O_GPFLAGS) = ~((ui32IntFlags & SPIS_GP_MASK) >> 16);
}

//*****************************************************************************
//
//! \brief Gets the current interrupt status.
//!
//! This function returns the interrupt status for the SPIS module. Either the
//! raw interrupt status or the status of interrupts that are allowed to
//! reflect to the processor can be returned.
//!
//! \param bMasked requests either raw or masked interrupt status.
//! - \c false : Raw interrupt status.
//! - \c true  : Masked interrupt status.
//!
//! \return Returns the current interrupt status consisting of a bitwise OR value
//! of the available interrupts sources.
//! - TX events:
//!   - \ref SPIS_TX_FULL
//!   - \ref SPIS_TX_EMPTY
//!   - \ref SPIS_TX_GE_WATERMARK
//!   - \ref SPIS_TX_LE_WATERMARK
//!   - \ref SPIS_TX_HASDATA
//!   - \ref SPIS_TX_UNDER_FLOW
//!   - \ref SPIS_TX_OVER_FLOW
//! - RX events:
//!   - \ref SPIS_RX_FULL
//!   - \ref SPIS_RX_EMPTY
//!   - \ref SPIS_RX_GE_WATERMARK
//!   - \ref SPIS_RX_LE_WATERMARK
//!   - \ref SPIS_RX_HASDATA
//!   - \ref SPIS_RX_UNDER_FLOW
//!   - \ref SPIS_RX_OVER_FLOW
//! - General Purpose events:
//!   - \ref SPIS_CHIP_SELECT
//!   - \ref SPIS_INCOMPLETE
//!   - \ref SPIS_PRX_OVERFLOW
//!   - \ref SPIS_BYTE_DONE
//!   - \ref SPIS_DMA_DONE_RX
//!   - \ref SPIS_DMA_DONE_TX
//!   - \ref SPIS_GP_MASK
//
//*****************************************************************************
extern uint32_t SPISIntStatus(bool bMasked);

//*****************************************************************************
//
//! \brief Registers an interrupt handler for the Serial Peripheral Interface Slave.
//!
//! This sets the handler to be called when an SPIS interrupt
//! occurs. This will enable the global interrupt in the interrupt controller;
//! specific SPIS interrupts must be enabled via \ref SPISIntEnable(). If necessary,
//! it is the interrupt handler's responsibility to clear the interrupt source
//! via \ref SPISIntClear().
//!
//! \param pfnHandler is a pointer to the function to be called when the
//! Serial Peripheral Interface Slave interrupt occurs.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
__STATIC_INLINE void
SPISIntRegister(void (*pfnHandler)(void))
{
    //
    // Register the interrupt handler, returning an error if an error occurs.
    //
    IntRegister(INT_SPIS, pfnHandler);

    //
    // Enable the synchronous serial interface interrupt.
    //
    IntEnable(INT_SPIS);
}

//*****************************************************************************
//
//! \brief Unregisters an interrupt handler for the Serial Peripheral Interface Slave.
//!
//! This function will clear the handler to be called when a SPIS
//! interrupt occurs. This will also mask off the interrupt in the interrupt
//! controller so that the interrupt handler is no longer called.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
__STATIC_INLINE void
SPISIntUnregister(void)
{
    //
    // Disable the interrupt.
    //
    IntDisable(INT_SPIS);

    //
    // Unregister the interrupt handler.
    //
    IntUnregister(INT_SPIS);
}

//*****************************************************************************
//
//! \brief Enable SPIS DMA operation by setting event source.
//!
//! This function selects \b both TX and RX FIFO event source for SPI Slave.
//! The argument \c ui32DMASetting must be an OR'ed value of \b one TX source
//! and \b one RX source.
//!
//! For event sources requiring a threshold to be set this is done by
//! \ref SPISConfig().
//!
//! \note The uDMA controller must also be set up before DMA can be used with
//! the SPIS.
//!
//! \param ui32DMASetting holds the OR'ed value of DMA event source for
//! \b both TX and RX. Valid parameters are:
//! - \ref SPIS_TX_DMA_FULL / \ref SPIS_RX_DMA_FULL : FIFO is full.
//! - \ref SPIS_TX_DMA_EMPTY / \ref SPIS_RX_DMA_EMPTY : FIFO is empty.
//! - \ref SPIS_TX_DMA_GE_WMARK / \ref SPIS_RX_DMA_GE_WMARK : FIFO greater than or equal to TX FIFO threshold count set by \ref SPISConfig().
//! - \ref SPIS_TX_DMA_LE_WMARK / \ref SPIS_RX_DMA_LE_WMARK : FIFO less than or equal to TX FIFO threshold count set by \ref SPISConfig().
//! - \ref SPIS_TX_DMA_HASDATA / \ref SPIS_RX_DMA_HASDATA : FIFO has one or more bytes.
//! - \ref SPIS_TX_DMA_ALWAYS / \ref SPIS_RX_DMA_ALWAYS : Force event flag high.
//! - \ref SPIS_TX_DMA_NONE / \ref SPIS_RX_DMA_NONE : Disable events.
//!
//! \return None
//!
//! \sa \ref SPISConfig()
//
//*****************************************************************************
__STATIC_INLINE void
SPISDmaEnable( uint32_t ui32DMASetting )
{
    //
    // Set the requested bits in the SPIS TX DMA control register.
    //
    HWREG( SPIS_BASE + SPIS_O_TXFEVSRC ) = ui32DMASetting & SPIS_TXFEVSRC_SEL_M;

    //
    // Set the requested bits in the SPIS RX DMA control register.
    //
    HWREG( SPIS_BASE + SPIS_O_RXFEVSRC ) = ( ui32DMASetting >> 8 ) & SPIS_RXFEVSRC_SEL_M;
}

//*****************************************************************************
//
//! \brief Disable SPIS DMA operation.
//!
//! This function is used to disable DMA operations based on specific events
//! in the SPIS module.
//!
//! \note The uDMA controller must also be set up before DMA can be used
//! with the SPIS. This function will disable the DMA events for both RX and
//! TX FIFO.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPISDmaDisable( void )
{
    //
    // Clear the requested bits in the SPIS TX DMA control register.
    //
    HWREG( SPIS_BASE + SPIS_O_TXFEVSRC ) = SPIS_TXFEVSRC_SEL_ZERO;

    //
    // Clear the requested bits in the SPIS RX DMA control register.
    //
    HWREG( SPIS_BASE + SPIS_O_RXFEVSRC ) = SPIS_RXFEVSRC_SEL_ZERO;
}

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_SPISDataPut
        #undef  SPISDataPut
        #define SPISDataPut                     ROM_SPISDataPut
    #endif
    #ifdef ROM_SPISTxGetValue
        #undef  SPISTxGetValue
        #define SPISTxGetValue                  ROM_SPISTxGetValue
    #endif
    #ifdef ROM_SPISDataGet
        #undef  SPISDataGet
        #define SPISDataGet                     ROM_SPISDataGet
    #endif
    #ifdef ROM_SPISRxGetValue
        #undef  SPISRxGetValue
        #define SPISRxGetValue                  ROM_SPISRxGetValue
    #endif
    #ifdef ROM_SPISIntStatus
        #undef  SPISIntStatus
        #define SPISIntStatus                   ROM_SPISIntStatus
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

#endif //  __SPIS_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
