/******************************************************************************
*  Filename:       uart.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the UART.
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
//! \addtogroup uart_api
//! @{
//
//*****************************************************************************

#ifndef __UART_H__
#define __UART_H__

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
#include <inc/hw_uart.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/interrupt.h>
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
    #define UARTFIFOLevelGet                NOROM_UARTFIFOLevelGet
    #define UARTConfigSetExpClk             NOROM_UARTConfigSetExpClk
    #define UARTConfigGetExpClk             NOROM_UARTConfigGetExpClk
    #define UARTDisable                     NOROM_UARTDisable
    #define UARTCharGetNonBlocking          NOROM_UARTCharGetNonBlocking
    #define UARTCharGet                     NOROM_UARTCharGet
    #define UARTCharPutNonBlocking          NOROM_UARTCharPutNonBlocking
    #define UARTCharPut                     NOROM_UARTCharPut
    #define UARTIntRegister                 NOROM_UARTIntRegister
    #define UARTIntUnregister               NOROM_UARTIntUnregister
#endif

//*****************************************************************************
//
// Values that can be passed to UARTIntEnable, UARTIntDisable, and UARTIntClear
// as the ui32IntFlags parameter, and returned from UARTIntStatus.
//
//*****************************************************************************
#define UART_INT_OE             0x400       // Overrun Error Interrupt Mask
#define UART_INT_BE             0x200       // Break Error Interrupt Mask
#define UART_INT_PE             0x100       // Parity Error Interrupt Mask
#define UART_INT_FE             0x080       // Framing Error Interrupt Mask
#define UART_INT_RT             0x040       // Receive Timeout Interrupt Mask
#define UART_INT_TX             0x020       // Transmit Interrupt Mask
#define UART_INT_RX             0x010       // Receive Interrupt Mask
#define UART_INT_CTS            0x002       // CTS Modem Interrupt Mask

//*****************************************************************************
//
// Values that can be passed to UARTConfigSetExpClk as the ui32Config parameter
// and returned by UARTConfigGetExpClk in the pui32Config parameter.
// Additionally, the UART_CONFIG_PAR_* subset can be passed to
// UARTParityModeSet as the ui32Parity parameter, and are returned by
// UARTParityModeGet.
//
//*****************************************************************************
#define UART_CONFIG_WLEN_MASK   0x00000060  // Mask for extracting word length
#define UART_CONFIG_WLEN_8      0x00000060  // 8 bit data
#define UART_CONFIG_WLEN_7      0x00000040  // 7 bit data
#define UART_CONFIG_WLEN_6      0x00000020  // 6 bit data
#define UART_CONFIG_WLEN_5      0x00000000  // 5 bit data
#define UART_CONFIG_STOP_MASK   0x00000008  // Mask for extracting stop bits
#define UART_CONFIG_STOP_ONE    0x00000000  // One stop bit
#define UART_CONFIG_STOP_TWO    0x00000008  // Two stop bits
#define UART_CONFIG_PAR_MASK    0x00000086  // Mask for extracting parity
#define UART_CONFIG_PAR_NONE    0x00000000  // No parity
#define UART_CONFIG_PAR_EVEN    0x00000006  // Even parity
#define UART_CONFIG_PAR_ODD     0x00000002  // Odd parity
#define UART_CONFIG_PAR_ONE     0x00000082  // Parity bit is one
#define UART_CONFIG_PAR_ZERO    0x00000086  // Parity bit is zero

//*****************************************************************************
//
// Values that can be passed to UARTFIFOLevelSet as the ui32TxLevel parameter
// and returned by UARTFIFOLevelGet in the pui32TxLevel.
//
//*****************************************************************************
#define UART_FIFO_TX1_8         0x00000000  // Transmit interrupt at 1/8 Full
#define UART_FIFO_TX2_8         0x00000001  // Transmit interrupt at 1/4 Full
#define UART_FIFO_TX4_8         0x00000002  // Transmit interrupt at 1/2 Full
#define UART_FIFO_TX6_8         0x00000003  // Transmit interrupt at 3/4 Full
#define UART_FIFO_TX7_8         0x00000004  // Transmit interrupt at 7/8 Full

//*****************************************************************************
//
// Values that can be passed to UARTFIFOLevelSet as the ui32RxLevel parameter
// and returned by UARTFIFOLevelGet in the pui32RxLevel.
//
//*****************************************************************************
#define UART_FIFO_RX1_8         0x00000000  // Receive interrupt at 1/8 Full
#define UART_FIFO_RX2_8         0x00000008  // Receive interrupt at 1/4 Full
#define UART_FIFO_RX4_8         0x00000010  // Receive interrupt at 1/2 Full
#define UART_FIFO_RX6_8         0x00000018  // Receive interrupt at 3/4 Full
#define UART_FIFO_RX7_8         0x00000020  // Receive interrupt at 7/8 Full

//*****************************************************************************
//
// Values that can be passed to UARTDMAEnable() and UARTDMADisable().
//
//*****************************************************************************
#define UART_DMA_ERR_RXSTOP     0x00000004  // Stop DMA receive if UART error
#define UART_DMA_TX             0x00000002  // Enable DMA for transmit
#define UART_DMA_RX             0x00000001  // Enable DMA for receive

//*****************************************************************************
//
// Values returned from UARTRxErrorGet().
//
//*****************************************************************************
#define UART_RXERROR_OVERRUN    0x00000008
#define UART_RXERROR_BREAK      0x00000004
#define UART_RXERROR_PARITY     0x00000002
#define UART_RXERROR_FRAMING    0x00000001

//*****************************************************************************
//
// Values that can be passed to UARTTxIntModeSet() or returned from
// UARTTxIntModeGet().
//
//*****************************************************************************
#define UART_TXINT_MODE_FIFO    0x00000000
#define UART_TXINT_MODE_EOT     0x00000010

//*****************************************************************************
//
// Values returned from the UARTBusy().
//
//*****************************************************************************
#define UART_BUSY               0x00000001
#define UART_IDLE               0x00000000

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

#ifdef DRIVERLIB_DEBUG
//*****************************************************************************
//
//! \internal
//!
//! \brief Checks a UART base address.
//!
//! This function determines if a UART port base address is valid.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns \c true if the base address is valid and \c false
//! otherwise.
//
//*****************************************************************************
static bool
UARTBaseValid(uint32_t ui32Base)
{
    return(ui32Base == UART0_BASE);
}
#endif

//*****************************************************************************
//
//! \brief Sets the type of parity.
//!
//! This function sets the type of parity to use for transmitting and expect
//! when receiving.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32Parity specifies the type of parity to use. The last two allow
//! direct control of the parity bit; it is always either one or zero based on
//! the mode.
//! - \ref UART_CONFIG_PAR_NONE
//! - \ref UART_CONFIG_PAR_EVEN
//! - \ref UART_CONFIG_PAR_ODD
//! - \ref UART_CONFIG_PAR_ONE
//! - \ref UART_CONFIG_PAR_ZERO
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTParityModeSet(uint32_t ui32Base, uint32_t ui32Parity)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));
    ASSERT((ui32Parity == UART_CONFIG_PAR_NONE) ||
           (ui32Parity == UART_CONFIG_PAR_EVEN) ||
           (ui32Parity == UART_CONFIG_PAR_ODD) ||
           (ui32Parity == UART_CONFIG_PAR_ONE) ||
           (ui32Parity == UART_CONFIG_PAR_ZERO));

    //
    // Set the parity mode.
    //
    HWREG(ui32Base + UART_O_LCRH) = ((HWREG(ui32Base + UART_O_LCRH) &
                                      ~(UART_LCRH_SPS | UART_LCRH_EPS |
                                        UART_LCRH_PEN)) | ui32Parity);
}

//*****************************************************************************
//
//! \brief Gets the type of parity currently being used.
//!
//! This function gets the type of parity used for transmitting data and
//! expected when receiving data.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns the current parity settings, specified as one of:
//! - \ref UART_CONFIG_PAR_NONE
//! - \ref UART_CONFIG_PAR_EVEN
//! - \ref UART_CONFIG_PAR_ODD
//! - \ref UART_CONFIG_PAR_ONE
//! - \ref UART_CONFIG_PAR_ZERO
//
//*****************************************************************************
__STATIC_INLINE uint32_t
UARTParityModeGet(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Return the current parity setting
    //
    return(HWREG(ui32Base + UART_O_LCRH) &
           (UART_LCRH_SPS | UART_LCRH_EPS | UART_LCRH_PEN));
}

//*****************************************************************************
//
//! \brief Sets the FIFO level at which interrupts are generated.
//!
//! This function sets the FIFO level at which transmit and receive interrupts
//! are generated.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32TxLevel is the transmit FIFO interrupt level, specified as one of:
//! - \ref UART_FIFO_TX1_8
//! - \ref UART_FIFO_TX2_8
//! - \ref UART_FIFO_TX4_8
//! - \ref UART_FIFO_TX6_8
//! - \ref UART_FIFO_TX7_8
//! \param ui32RxLevel is the receive FIFO interrupt level, specified as one of:
//! - \ref UART_FIFO_RX1_8
//! - \ref UART_FIFO_RX2_8
//! - \ref UART_FIFO_RX4_8
//! - \ref UART_FIFO_RX6_8
//! - \ref UART_FIFO_RX7_8
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTFIFOLevelSet(uint32_t ui32Base, uint32_t ui32TxLevel,
                 uint32_t ui32RxLevel)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));
    ASSERT((ui32TxLevel == UART_FIFO_TX1_8) ||
           (ui32TxLevel == UART_FIFO_TX2_8) ||
           (ui32TxLevel == UART_FIFO_TX4_8) ||
           (ui32TxLevel == UART_FIFO_TX6_8) ||
           (ui32TxLevel == UART_FIFO_TX7_8));
    ASSERT((ui32RxLevel == UART_FIFO_RX1_8) ||
           (ui32RxLevel == UART_FIFO_RX2_8) ||
           (ui32RxLevel == UART_FIFO_RX4_8) ||
           (ui32RxLevel == UART_FIFO_RX6_8) ||
           (ui32RxLevel == UART_FIFO_RX7_8));

    //
    // Set the FIFO interrupt levels.
    //
    HWREG(ui32Base + UART_O_IFLS) = ui32TxLevel | ui32RxLevel;
}

//*****************************************************************************
//
//! \brief Gets the FIFO level at which interrupts are generated.
//!
//! This function gets the FIFO level at which transmit and receive interrupts
//! are generated.
//!
//! \param ui32Base is the base address of the UART port.
//! \param pui32TxLevel is a pointer to storage for the transmit FIFO level,
//! returned as one of:
//! - \ref UART_FIFO_TX1_8
//! - \ref UART_FIFO_TX2_8
//! - \ref UART_FIFO_TX4_8
//! - \ref UART_FIFO_TX6_8
//! - \ref UART_FIFO_TX7_8
//! \param pui32RxLevel is a pointer to storage for the receive FIFO level,
//! returned as one of:
//! - \ref UART_FIFO_RX1_8
//! - \ref UART_FIFO_RX2_8
//! - \ref UART_FIFO_RX4_8
//! - \ref UART_FIFO_RX6_8
//! - \ref UART_FIFO_RX7_8
//!
//! \return None
//
//*****************************************************************************
extern void UARTFIFOLevelGet(uint32_t ui32Base, uint32_t *pui32TxLevel,
                             uint32_t *pui32RxLevel);

//*****************************************************************************
//
//! \brief Sets the configuration of a UART.
//!
//! This function configures the UART for operation in the specified data
//! format.
//!
//! \note The peripheral clock is not necessarily the same as the processor
//! clock. The frequency of the peripheral clock is set by the system control.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32UARTClk is the rate of the clock supplied to the UART module.
//! \param ui32Baud is the desired baud rate.
//! \param ui32Config is the data format for the port.
//! The parameter is the bitwise OR of three values:
//! - Number of data bits
//!   - \ref UART_CONFIG_WLEN_8 : 8 data bits per byte.
//!   - \ref UART_CONFIG_WLEN_7 : 7 data bits per byte.
//!   - \ref UART_CONFIG_WLEN_6 : 6 data bits per byte.
//!   - \ref UART_CONFIG_WLEN_5 : 5 data bits per byte.
//! - Number of stop bits
//!   - \ref UART_CONFIG_STOP_ONE : One stop bit.
//!   - \ref UART_CONFIG_STOP_TWO : Two stop bits.
//! - Parity
//!   - \ref UART_CONFIG_PAR_NONE
//!   - \ref UART_CONFIG_PAR_EVEN
//!   - \ref UART_CONFIG_PAR_ODD
//!   - \ref UART_CONFIG_PAR_ONE
//!   - \ref UART_CONFIG_PAR_ZERO
//!
//! \return None
//
//*****************************************************************************
extern void UARTConfigSetExpClk(uint32_t ui32Base, uint32_t ui32UARTClk,
                                uint32_t ui32Baud, uint32_t ui32Config);

//*****************************************************************************
//
//! \brief Gets the current configuration of a UART.
//!
//! The baud rate and data format for the UART is determined, given an
//! explicitly provided peripheral clock (hence the ExpClk suffix). The
//! returned baud rate is the actual baud rate; it may not be the exact baud
//! rate requested or an "official" baud rate. The data format returned in
//! \c pui32Config is enumerated the same as the \c ui32Config parameter of
//! \ref UARTConfigSetExpClk().
//!
//! \note The peripheral clock is not necessarily the same as the processor
//! clock. The frequency of the peripheral clock is set by the system control.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32UARTClk is the rate of the clock supplied to the UART module.
//! \param pui32Baud is a pointer to storage for the baud rate.
//! \param pui32Config is a pointer to storage for the data format.
//!
//! \return None
//
//*****************************************************************************
extern void UARTConfigGetExpClk(uint32_t ui32Base, uint32_t ui32UARTClk,
                                uint32_t *pui32Baud, uint32_t *pui32Config);

//*****************************************************************************
//
//! \brief Enables transmitting and receiving.
//!
//! This function sets the UARTEN, TXE, and RXE bits, and enables the transmit
//! and receive FIFOs.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTEnable(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Enable the FIFO.
    //
    HWREG(ui32Base + UART_O_LCRH) |= UART_LCRH_FEN;

    //
    // Enable RX, TX, and the UART.
    //
    HWREG(ui32Base + UART_O_CTL) |= (UART_CTL_UARTEN | UART_CTL_TXE |
                                     UART_CTL_RXE);
}

//*****************************************************************************
//
//! \brief Disables transmitting and receiving.
//!
//! This function clears the UARTEN, TXE, and RXE bits, waits for the end of
//! transmission of the current character, and flushes the transmit FIFO.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return None
//
//*****************************************************************************
extern void UARTDisable(uint32_t ui32Base);

//*****************************************************************************
//
//! \brief Enables the transmit and receive FIFOs.
//!
//! This functions enables the transmit and receive FIFOs in the UART.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTFIFOEnable(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Enable the FIFO.
    //
    HWREG(ui32Base + UART_O_LCRH) |= UART_LCRH_FEN;
}

//*****************************************************************************
//
//! \brief Disables the transmit and receive FIFOs.
//!
//! This functions disables the transmit and receive FIFOs in the UART.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTFIFODisable(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Disable the FIFO.
    //
    HWREG(ui32Base + UART_O_LCRH) &= ~(UART_LCRH_FEN);
}

//*****************************************************************************
//
//! \brief Sets the operating mode for the UART transmit interrupt.
//!
//! This function allows the mode of the UART transmit interrupt to be set. By
//! default, the transmit interrupt is asserted when the FIFO level falls past
//! a threshold set via a call to \ref UARTFIFOLevelSet(). Alternatively, if this
//! function is called with \c ui32Mode set to \ref UART_TXINT_MODE_EOT, the
//! transmit interrupt is asserted once the transmitter is completely idle -
//! the transmit FIFO is empty and all bits, including any stop bits, have
//! cleared the transmitter.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32Mode is the operating mode for the transmit interrupt.
//! - \ref UART_TXINT_MODE_EOT  : Trigger interrupts when the transmitter is idle.
//! - \ref UART_TXINT_MODE_FIFO : Trigger based on the current transmit FIFO level.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTTxIntModeSet(uint32_t ui32Base, uint32_t ui32Mode)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));
    ASSERT((ui32Mode == UART_TXINT_MODE_EOT) ||
           (ui32Mode == UART_TXINT_MODE_FIFO));

    //
    // Set or clear the EOT bit of the UART control register as appropriate.
    //
    HWREG(ui32Base + UART_O_CTL) = ((HWREG(ui32Base + UART_O_CTL) &
                                     ~(UART_TXINT_MODE_EOT |
                                       UART_TXINT_MODE_FIFO)) | ui32Mode);
}

//*****************************************************************************
//
//! \brief Returns the current operating mode for the UART transmit interrupt.
//!
//! This function returns the current operating mode for the UART transmit
//! interrupt.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns operating mode of UART transmit interrupt.
//! - \ref UART_TXINT_MODE_FIFO : Interrupt is set to be asserted based upon
//! the level of the transmit FIFO.
//! - \ref UART_TXINT_MODE_EOT  : transmit interrupt is set to be
//! asserted once the transmitter is completely idle - the transmit FIFO is
//! empty and all bits, including any stop bits, have cleared the transmitter.
//
//*****************************************************************************
__STATIC_INLINE uint32_t
UARTTxIntModeGet(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Return the current transmit interrupt mode.
    //
    return(HWREG(ui32Base + UART_O_CTL) & (UART_TXINT_MODE_EOT |
                                           UART_TXINT_MODE_FIFO));
}

//*****************************************************************************
//
//! \brief Determines if there are any characters in the receive FIFO.
//!
//! This function returns a flag indicating whether or not there is data
//! available in the receive FIFO.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns status of the receive FIFO.
//! - \c true  : There is data in the receive FIFO.
//! - \c false : There is no data in the receive FIFO.
//
//*****************************************************************************
__STATIC_INLINE bool
UARTCharsAvail(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Return the availability of characters.
    //
    return((HWREG(ui32Base + UART_O_FR) & UART_FR_RXFE) ? false : true);
}

//*****************************************************************************
//
//! \brief Determines if there is any space in the transmit FIFO.
//!
//! This function returns a flag indicating whether or not there is space
//! available in the transmit FIFO.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns status of the transmit FIFO.
//! - \c true  : There is space available in the transmit FIFO.
//! - \c false : There is no space available in the transmit FIFO.
//
//*****************************************************************************
__STATIC_INLINE bool
UARTSpaceAvail(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Return the availability of space.
    //
    return((HWREG(ui32Base + UART_O_FR) & UART_FR_TXFF) ? false : true);
}

//*****************************************************************************
//
//! \brief Receives a character from the specified port.
//!
//! This function gets a character from the receive FIFO for the specified
//! port.
//!
//! \note The \ref UARTCharsAvail() function should be called before
//! attempting to call this function.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns the character read from the specified port, cast as an
//! \c int32_t. A \c -1 is returned if there are no characters present in the
//! receive FIFO.
//!
//! \sa \ref UARTCharsAvail()
//
//*****************************************************************************
extern int32_t UARTCharGetNonBlocking(uint32_t ui32Base);

//*****************************************************************************
//
//! \brief Waits for a character from the specified port.
//!
//! This function gets a character from the receive FIFO for the specified
//! port. If there are no characters available, this function waits until a
//! character is received before returning.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns the character read from the specified port, cast as an
//! \c int32_t.
//
//*****************************************************************************
extern int32_t UARTCharGet(uint32_t ui32Base);

//*****************************************************************************
//
//! \brief Sends a character to the specified port.
//!
//! This function writes the character \c ui8Data to the transmit FIFO for the
//! specified port. This function does not block, so if there is no space
//! available, then a \c false is returned, and the application must retry the
//! function later.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui8Data is the character to be transmitted.
//!
//! \return Returns status of the character transmit.
//! - \c true  : The character was successfully placed in the transmit FIFO.
//! - \c false : There was no space available in the transmit FIFO. Try again later.
//
//*****************************************************************************
extern bool UARTCharPutNonBlocking(uint32_t ui32Base, uint8_t ui8Data);

//*****************************************************************************
//
//! \brief Waits to send a character from the specified port.
//!
//! This function sends the character \c ui8Data to the transmit FIFO for the
//! specified port. If there is no space available in the transmit FIFO, this
//! function waits until there is space available before returning.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui8Data is the character to be transmitted.
//!
//! \return None
//
//*****************************************************************************
extern void UARTCharPut(uint32_t ui32Base, uint8_t ui8Data);

//*****************************************************************************
//
//! \brief Determines whether the UART transmitter is busy or not.
//!
//! Allows the caller to determine whether all transmitted bytes have cleared
//! the transmitter hardware. If \c false is returned, the transmit FIFO is
//! empty and all bits of the last transmitted character, including all stop
//! bits, have left the hardware shift register.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns status of UART transmitter.
//! - \c true  : UART is transmitting.
//! - \c false : All transmissions are complete.
//
//*****************************************************************************
__STATIC_INLINE bool
UARTBusy(uint32_t ui32Base)
{
    //
    // Check the argument.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Determine if the UART is busy.
    //
    return((HWREG(ui32Base + UART_O_FR) & UART_FR_BUSY) ?
           UART_BUSY : UART_IDLE);
}

//*****************************************************************************
//
//! \brief Causes a BREAK to be sent.
//!
//! \note For proper transmission of a break
//! command, the break must be asserted for at least two complete frames.
//!
//! \param ui32Base is the base address of the UART port.
//! \param bBreakState controls the output level.
//! - \c true  : Asserts a break condition on the UART.
//! - \c false : Removes the break condition.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTBreakCtl(uint32_t ui32Base, bool bBreakState)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Set the break condition as requested.
    //
    HWREG(ui32Base + UART_O_LCRH) =
         (bBreakState ?
         (HWREG(ui32Base + UART_O_LCRH) | UART_LCRH_BRK) :
         (HWREG(ui32Base + UART_O_LCRH) & ~(UART_LCRH_BRK)));
}

//*****************************************************************************
//
//! \brief Registers an interrupt handler for a UART interrupt.
//!
//! This function does the actual registering of the interrupt handler.  This
//! function enables the global interrupt in the interrupt controller; specific
//! UART interrupts must be enabled via \ref UARTIntEnable(). It is the interrupt
//! handler's responsibility to clear the interrupt source.
//!
//! \param ui32Base is the base address of the UART port.
//! \param pfnHandler is a pointer to the function to be called when the
//! UART interrupt occurs.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
extern void UARTIntRegister(uint32_t ui32Base, void (*pfnHandler)(void));

//*****************************************************************************
//
//! \brief Unregisters an interrupt handler for a UART interrupt.
//!
//! This function does the actual unregistering of the interrupt handler. It
//! clears the handler to be called when a UART interrupt occurs.  This
//! function also masks off the interrupt in the interrupt controller so that
//! the interrupt handler no longer is called.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
extern void UARTIntUnregister(uint32_t ui32Base);

//*****************************************************************************
//
//! \brief Enables individual UART interrupt sources.
//!
//! This function enables the indicated UART interrupt sources. Only the
//! sources that are enabled can be reflected to the processor interrupt;
//! disabled sources have no effect on the processor.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32IntFlags is the bit mask of the interrupt sources to be enabled.
//! The parameter is the bitwise OR of any of the following:
//! - \ref UART_INT_OE  : Overrun Error interrupt.
//! - \ref UART_INT_BE  : Break Error interrupt.
//! - \ref UART_INT_PE  : Parity Error interrupt.
//! - \ref UART_INT_FE  : Framing Error interrupt.
//! - \ref UART_INT_RT  : Receive Timeout interrupt.
//! - \ref UART_INT_TX  : Transmit interrupt.
//! - \ref UART_INT_RX  : Receive interrupt.
//! - \ref UART_INT_CTS : CTS interrupt.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTIntEnable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Enable the specified interrupts.
    //
    HWREG(ui32Base + UART_O_IMSC) |= ui32IntFlags;
}

//*****************************************************************************
//
//! \brief Disables individual UART interrupt sources.
//!
//! This function disables the indicated UART interrupt sources. Only the
//! sources that are enabled can be reflected to the processor interrupt;
//! disabled sources have no effect on the processor.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32IntFlags is the bit mask of the interrupt sources to be disabled.
//! - \ref UART_INT_OE  : Overrun Error interrupt.
//! - \ref UART_INT_BE  : Break Error interrupt.
//! - \ref UART_INT_PE  : Parity Error interrupt.
//! - \ref UART_INT_FE  : Framing Error interrupt.
//! - \ref UART_INT_RT  : Receive Timeout interrupt.
//! - \ref UART_INT_TX  : Transmit interrupt.
//! - \ref UART_INT_RX  : Receive interrupt.
//! - \ref UART_INT_CTS : CTS interrupt.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTIntDisable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Disable the specified interrupts.
    //
    HWREG(ui32Base + UART_O_IMSC) &= ~(ui32IntFlags);
}

//*****************************************************************************
//
//! \brief Gets the current interrupt status.
//!
//! This function returns the interrupt status for the specified UART. Either
//! the raw interrupt status or the status of interrupts that are allowed to
//! reflect to the processor can be returned.
//!
//! \param ui32Base is the base address of the UART port.
//! \param bMasked selects either raw or masked interrupt.
//! - \c true  : Masked interrupt status is required.
//! - \c false : Raw interrupt status is required.
//!
//! \return Returns the current interrupt status, enumerated as a bit field of:
//! - \ref UART_INT_OE  : Overrun Error interrupt.
//! - \ref UART_INT_BE  : Break Error interrupt.
//! - \ref UART_INT_PE  : Parity Error interrupt.
//! - \ref UART_INT_FE  : Framing Error interrupt.
//! - \ref UART_INT_RT  : Receive Timeout interrupt.
//! - \ref UART_INT_TX  : Transmit interrupt.
//! - \ref UART_INT_RX  : Receive interrupt.
//! - \ref UART_INT_CTS : CTS interrupt.
//
//*****************************************************************************
__STATIC_INLINE uint32_t
UARTIntStatus(uint32_t ui32Base, bool bMasked)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    if(bMasked)
    {
        return(HWREG(ui32Base + UART_O_MIS));
    }
    else
    {
        return(HWREG(ui32Base + UART_O_RIS));
    }
}

//*****************************************************************************
//
//! \brief Clears UART interrupt sources.
//!
//! The specified UART interrupt sources are cleared, so that they no longer
//! assert. This function must be called in the interrupt handler to keep the
//! interrupt from being recognized again immediately upon exit.
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
//! \param ui32Base is the base address of the UART port.
//! \param ui32IntFlags is a bit mask of the interrupt sources to be cleared.
//! - \ref UART_INT_OE  : Overrun Error interrupt.
//! - \ref UART_INT_BE  : Break Error interrupt.
//! - \ref UART_INT_PE  : Parity Error interrupt.
//! - \ref UART_INT_FE  : Framing Error interrupt.
//! - \ref UART_INT_RT  : Receive Timeout interrupt.
//! - \ref UART_INT_TX  : Transmit interrupt.
//! - \ref UART_INT_RX  : Receive interrupt.
//! - \ref UART_INT_CTS : CTS interrupt.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTIntClear(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    //
    // Check the arguments
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Clear the requested interrupt sources
    //
    HWREG(ui32Base + UART_O_ICR) = ui32IntFlags;
}

//*****************************************************************************
//
//! \brief Enable UART DMA operation.
//!
//! The specified UART DMA features are enabled. The UART can be
//! configured to use DMA for transmit or receive, and to disable
//! receive if an error occurs.
//!
//! \note The uDMA controller must also be set up before DMA can be used
//! with the UART.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32DMAFlags is a bit mask of the DMA features to enable.
//! The parameter is the bitwise OR of any of the following values:
//! - UART_DMA_RX         : Enable DMA for receive.
//! - UART_DMA_TX         : Enable DMA for transmit.
//! - UART_DMA_ERR_RXSTOP : Disable DMA receive on UART error.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTDMAEnable(uint32_t ui32Base, uint32_t ui32DMAFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Set the requested bits in the UART DMA control register.
    //
    HWREG(ui32Base + UART_O_DMACTL) |= ui32DMAFlags;
}

//*****************************************************************************
//
//! \brief Disable UART DMA operation.
//!
//! This function is used to disable UART DMA features that were enabled
//! by \ref UARTDMAEnable(). The specified UART DMA features are disabled.
//!
//! \param ui32Base is the base address of the UART port.
//! \param ui32DMAFlags is a bit mask of the DMA features to disable.
//! The parameter is the bitwise OR of any of the following values:
//! - UART_DMA_RX         : Enable DMA for receive.
//! - UART_DMA_TX         : Enable DMA for transmit.
//! - UART_DMA_ERR_RXSTOP : Disable DMA receive on UART error.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTDMADisable(uint32_t ui32Base, uint32_t ui32DMAFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Clear the requested bits in the UART DMA control register.
    //
    HWREG(ui32Base + UART_O_DMACTL) &= ~ui32DMAFlags;
}

//*****************************************************************************
//
//! \brief Gets current receiver errors.
//!
//! This function returns the current state of each of the 4 receiver error
//! sources. The returned errors are equivalent to the four error bits
//! returned via the previous call to \ref UARTCharGet() or \ref UARTCharGetNonBlocking()
//! with the exception that the overrun error is set immediately the overrun
//! occurs rather than when a character is next read.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return Returns a bitwise OR combination of the receiver error flags:
//! - \ref UART_RXERROR_FRAMING
//! - \ref UART_RXERROR_PARITY
//! - \ref UART_RXERROR_BREAK
//! - \ref UART_RXERROR_OVERRUN
//
//*****************************************************************************
__STATIC_INLINE uint32_t
UARTRxErrorGet(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Return the current value of the receive status register.
    //
    return(HWREG(ui32Base + UART_O_RSR) & 0x0000000F);
}

//*****************************************************************************
//
//! \brief Clears all reported receiver errors.
//!
//! This function is used to clear all receiver error conditions reported via
//! \ref UARTRxErrorGet(). If using the overrun, framing error, parity error or
//! break interrupts, this function must be called after clearing the interrupt
//! to ensure that later errors of the same type trigger another interrupt.
//!
//! \param ui32Base is the base address of the UART port.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
UARTRxErrorClear(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(UARTBaseValid(ui32Base));

    //
    // Any write to the Error Clear Register will clear all bits which are
    // currently set.
    //
    HWREG(ui32Base + UART_O_ECR) = 0;
}

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_UARTFIFOLevelGet
        #undef  UARTFIFOLevelGet
        #define UARTFIFOLevelGet                ROM_UARTFIFOLevelGet
    #endif
    #ifdef ROM_UARTConfigSetExpClk
        #undef  UARTConfigSetExpClk
        #define UARTConfigSetExpClk             ROM_UARTConfigSetExpClk
    #endif
    #ifdef ROM_UARTConfigGetExpClk
        #undef  UARTConfigGetExpClk
        #define UARTConfigGetExpClk             ROM_UARTConfigGetExpClk
    #endif
    #ifdef ROM_UARTDisable
        #undef  UARTDisable
        #define UARTDisable                     ROM_UARTDisable
    #endif
    #ifdef ROM_UARTCharGetNonBlocking
        #undef  UARTCharGetNonBlocking
        #define UARTCharGetNonBlocking          ROM_UARTCharGetNonBlocking
    #endif
    #ifdef ROM_UARTCharGet
        #undef  UARTCharGet
        #define UARTCharGet                     ROM_UARTCharGet
    #endif
    #ifdef ROM_UARTCharPutNonBlocking
        #undef  UARTCharPutNonBlocking
        #define UARTCharPutNonBlocking          ROM_UARTCharPutNonBlocking
    #endif
    #ifdef ROM_UARTCharPut
        #undef  UARTCharPut
        #define UARTCharPut                     ROM_UARTCharPut
    #endif
    #ifdef ROM_UARTIntRegister
        #undef  UARTIntRegister
        #define UARTIntRegister                 ROM_UARTIntRegister
    #endif
    #ifdef ROM_UARTIntUnregister
        #undef  UARTIntUnregister
        #define UARTIntUnregister               ROM_UARTIntUnregister
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

#endif //  __UART_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
