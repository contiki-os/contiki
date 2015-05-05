/******************************************************************************
*  Filename:       ddi.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the DDI master interface.
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
//! \addtogroup ddi_api
//! @{
//
//*****************************************************************************

#ifndef __DDI_H__
#define __DDI_H__

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
#include <inc/hw_ddi.h>
#include <inc/hw_aux_smph.h>
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
    #define DDI16BitWrite                   NOROM_DDI16BitWrite
    #define DDI16BitfieldWrite              NOROM_DDI16BitfieldWrite
    #define DDI16BitRead                    NOROM_DDI16BitRead
    #define DDI16BitfieldRead               NOROM_DDI16BitfieldRead
#endif

//*****************************************************************************
//
// Number of register in the DDI slave
//
//*****************************************************************************
#define DDI_SLAVE_REGS          64

//*****************************************************************************
//
// Defines that can be passed to the DDIConfigSet()
//
//*****************************************************************************
#define DDI_NO_WAIT         0x00000000
#define DDI_WAIT_FOR_ACK    0x00000004
#define DDI_SPEED_2         0x00000000
#define DDI_SPEED_4         0x00000001
#define DDI_SPEED_8         0x00000002
#define DDI_SPEED_16        0x00000003
#define DDI_CONFIG_MASK     0x00000007

//*****************************************************************************
//
// Defines that is used to control the ADI slave and master
//
//*****************************************************************************
#define DDI_PROTECT         0x00000080
#define DDI_ACK             0x00000001
#define DDI_SYNC            0x00000000

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************


//*****************************************************************************
//
// Helper functions
//
//*****************************************************************************

//*****************************************************************************
//
//! \internal
//!
//! \brief Safely write to AUX ADI/DDI interfaces using a semaphore.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param nAddr is the register address.
//! \param nData is the data to write to the register.
//! \param nSize is the register access size in bytes.
//! \return Returns \c true if the base address is valid and \c false
//! otherwise.
//!
//! \endinternal
//
//*****************************************************************************
__STATIC_INLINE void
AuxAdiDdiSafeWrite(uint32_t nAddr, uint32_t nData, uint32_t nSize)
{
    // Disable interrupts and remember whether to re-enable
    bool bIrqEnabled = !CPUcpsid();
    // Acquire semaphore for accessing ADI/DDI in AUX, perform access, release semaphore
    while (!HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH0));
    switch (nSize) {
    case 1:             HWREGB(nAddr) = (uint8_t)nData; break;
    case 2:             HWREGH(nAddr) = (uint16_t)nData; break;
    case 4: default:    HWREG(nAddr)  = nData; break;
    }
    HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH0) = 1;
    // Restore interrupt enable
    if (bIrqEnabled) {
        CPUcpsie();
    }
}

//*****************************************************************************
//
//! \internal
//!
//! \brief Safely read from AUX ADI/DDI interfaces using a semaphore.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param nAddr is the register address.
//! \param nSize is the register access size in bytes.
//! \return Returns the data read.
//!
//! \endinternal
//
//*****************************************************************************
__STATIC_INLINE uint32_t
AuxAdiDdiSafeRead(uint32_t nAddr, uint32_t nSize)
{
    uint32_t nRet;
    // Disable interrupts and remember whether to re-enable
    bool bIrqEnabled = !CPUcpsid();
    // Acquire semaphore for accessing ADI/DDI in AUX, perform access, release semaphore
    while (!HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH0));
    switch (nSize) {
    case 1:             nRet = HWREGB(nAddr); break;
    case 2:             nRet = HWREGH(nAddr); break;
    case 4: default:    nRet = HWREG(nAddr); break;
    }
    HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH0) = 1;
    // Restore interrupt enable
    if (bIrqEnabled) {
        CPUcpsie();
    }
    return nRet;
}

#ifdef DRIVERLIB_DEBUG
//*****************************************************************************
//
//! \internal
//!
//! \brief Check a DDI base address.
//!
//! This function determines if a DDI port base address is valid.
//!
//! \param ui32Base is the base address of the DDI port.
//!
//! \return Returns \c true if the base address is valid and \c false
//! otherwise.
//!
//! \endinternal
//
//*****************************************************************************
static bool
DDIBaseValid(uint32_t ui32Base)
{
    return(ui32Base == AUX_DDI0_OSC_BASE);
}
#endif

//*****************************************************************************
//
//! \brief Get the status of the DDI.
//!
//! This function will get the value of the status register. Value that can
//! be passed to this function are DDI.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//!
//! \return Returns the current value of the status register.
//
//*****************************************************************************
__STATIC_INLINE uint32_t
DDIStatusGet(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));

    //
    // Return the status value for the correct DDI Slave.
    //
    return AuxAdiDdiSafeRead(ui32Base + DDI_O_SLAVESTAT, 4);
}

//*****************************************************************************
//
//! \brief Configure the DDI Slave.
//!
//! Use this function to configure the interface between the DDI master and the
//! DDI slave. The configuration values for the DDI slave are used to set the
//! speed of the DDI interface and define if the master should wait for
//! acknowledge from the slave.
//!
//! The speed is set using:
//! - \ref DDI_SPEED_2
//! - \ref DDI_SPEED_4
//! - \ref DDI_SPEED_8
//! - \ref DDI_SPEED_16
//!
//! The acknowledge is set using:
//! - \ref DDI_NO_WAIT
//! - \ref DDI_WAIT_FOR_ACK
//!
//! The configuration value must be a bitwised OR'ed combination of these two features.
//!
//! If the \c bProtect parameter is set the configuration register in the ADI slave is
//! locked for write.
//!
//! \note Once the lock bit has been set, it is no longer
//! possible to write the configuration register.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//! \param ui32Config is the configuration of the DDI slave.
//! Must be an OR'ed combination of 'speed' and 'acknowledge config'.
//! \param bProtect decides if the register access should be protected.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDIConfigSet(uint32_t ui32Base, uint32_t ui32Config, bool bProtect)
{
    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(((ui32Config & 0x4) == DDI_NO_WAIT) ||
           ((ui32Config & 0x4) == DDI_WAIT_FOR_ACK));
    ASSERT(((ui32Config & 0x3) == DDI_SPEED_2) ||
           ((ui32Config & 0x3) == DDI_SPEED_4) ||
           ((ui32Config & 0x3) == DDI_SPEED_8) ||
           ((ui32Config & 0x3) == DDI_SPEED_16));

    //
    // Configure the DDI slave.
    //
    AuxAdiDdiSafeWrite(
        ui32Base + DDI_O_SLAVECONF,
        (ui32Config & 0x7) | (bProtect ? DDI_PROTECT : 0),
         4
    );
}

//*****************************************************************************
//
//! \brief Synchronize a DDI slave.
//!
//! This function will perform a sync on the DDI slave by issuing a NOP
//! DDI/DDI command to the master with REQ=0. In other words, the master
//! performs a dummy write request to ensure the master and slave are
//! synchronized.
//!
//! \note It is recommended to sync with all the DDI slaves before a power down
//! of a DDI master.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDISync(uint32_t ui32Base)
{
    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));

    //
    // Synchronize the DDI slave to guarantee future write operations.
    //
    AuxAdiDdiSafeWrite(ui32Base + DDI_O_SLAVESTAT, DDI_SYNC, 1);
}

//*****************************************************************************
//
//! \brief Protect a DDI slave by locking the register access.
//!
//! This function will lock the register interface to the DDI.
//!
//! \note Once locked it is no longer possible to change the configuration
//! register in the DDI slave.
//!
//! \note This function uses read-modify-write to guarantee the integrity of
//! the configuration. This might take exceedingly long time, so if the
//! configuration is known it is advised to use the \ref DDIConfigSet() for
//! protecting the DDI slave configuration.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDIProtect(uint32_t ui32Base)
{
    uint32_t ui32Val;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));

    //
    // Lock the register interface on the DDI slave.
    //
    ui32Val = AuxAdiDdiSafeRead(ui32Base + DDI_O_SLAVECONF, 4);
    ui32Val |= DDI_PROTECT;
    AuxAdiDdiSafeWrite(ui32Base + DDI_O_SLAVECONF, ui32Val, 4);
}

//*****************************************************************************
//
//! \brief Write a 32 bit value to a register in the DDI slave.
//!
//! This function will write a value to a register in the analog
//! domain.
//!
//! \note This operation is write only for the specified register. No
//! conservation of the previous value of the register will be kept (i.e. this
//! is NOT read-modify-write on the register).
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//! \param ui32Reg is the register to write.
//! \param ui32Val is the 32 bit value to write to the register.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDI32RegWrite(uint32_t ui32Base, uint32_t ui32Reg,
              uint32_t ui32Val)
{
    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);

    //
    // Write the value to the register.
    //
    AuxAdiDdiSafeWrite(ui32Base + ui32Reg, ui32Val, 4);
}

//*****************************************************************************
//
//! \brief Read the value in a 32 bit register.
//!
//! This function will read a register in the analog domain and return
//! the value as an \c uint32_t.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//! \param ui32Reg is the 32 bit register to read.
//!
//! \return Returns the 32 bit value of the analog register.
//
//*****************************************************************************
__STATIC_INLINE uint32_t
DDI32RegRead(uint32_t ui32Base, uint32_t ui32Reg)
{
    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);

    //
    // Read the register and return the value.
    //
    return AuxAdiDdiSafeRead(ui32Base + ui32Reg, 4);
}

//*****************************************************************************
//
//! \brief Set specific bits in a DDI slave register.
//!
//! This function will set bits in a register in the analog domain.
//!
//! \note This operation is write only for the specified register.
//! This function is used to set bits in specific register in the
//! DDI slave. Only bits in the selected register are affected by the
//! operation.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//! \param ui32Reg is the base register to assert the bits in.
//! \param ui32Val is the 32 bit one-hot encoded value specifying which
//! bits to set in the register.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDI32BitsSet(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Val)
{
    uint32_t ui32RegOffset;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);

    //
    // Get the correct address of the first register used for setting bits
    // in the DDI slave.
    //
    ui32RegOffset = DDI_O_SET;

    //
    // Set the selected bits.
    //
    AuxAdiDdiSafeWrite(ui32Base + ui32RegOffset + ui32Reg, ui32Val, 4);
}

//*****************************************************************************
//
//! \brief Clear specific bits in a 32 bit DDI register.
//!
//! This function will clear bits in a register in the analog domain.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is DDI base address.
//! \param ui32Reg is the base registers to clear the bits in.
//! \param ui32Val is the 32 bit one-hot encoded value specifying which
//! bits to clear in the register.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDI32BitsClear(uint32_t ui32Base, uint32_t ui32Reg,
               uint32_t ui32Val)
{
    uint32_t ui32RegOffset;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);

    //
    // Get the correct address of the first register used for setting bits
    // in the DDI slave.
    //
    ui32RegOffset = DDI_O_CLR;

    //
    // Clear the selected bits.
    //
    AuxAdiDdiSafeWrite(ui32Base + ui32RegOffset + ui32Reg, ui32Val, 4);
}

//*****************************************************************************
//
//! \brief Set a value on any 8 bits inside a 32 bit register in the DDI slave.
//!
//! This function allows byte (8 bit access) to the DDI slave registers.
//!
//! Use this function to write any value in the range 0-7 bits aligned on a
//! byte boundary. Fx. for writing the value 0b101 to bits 1-3 set
//! <tt>ui16Val = 0x0A</tt> and <tt>ui16Mask = 0x0E</tt>. Bits 0 and 5-7 will
//! not be affected by the operation, as long as the corresponding bits are
//! not set in the \c ui16Mask.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is the base address of the DDI port.
//! \param ui32Reg is the Least Significant Register in the DDI slave that
//! will be affected by the write operation.
//! \param ui32Byte is the byte number to access within the 32 bit register.
//! \param ui16Mask is the mask defining which of the 8 bits that should be
//! overwritten. The mask must be defined in the lower half of the 16 bits.
//! \param ui16Val is the value to write. The value must be defined in the lower
//! half of the 16 bits.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDI8SetValBit(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Byte,
              uint16_t ui16Mask, uint16_t ui16Val)
{
    uint32_t ui32RegOffset;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);
    ASSERT(!(ui16Val & 0xFF00));
    ASSERT(!(ui16Mask & 0xFF00));

    //
    // Get the correct address of the first register used for setting bits
    // in the DDI slave.
    //
    ui32RegOffset = DDI_O_MASK8B + (ui32Reg << 1) + (ui32Byte << 1);

    //
    // Set the selected bits.
    //
    AuxAdiDdiSafeWrite(ui32Base + ui32RegOffset, (ui16Mask << 8) | ui16Val, 2);
}

//*****************************************************************************
//
//! \brief Set a value on any 16 bits inside a 32 bit register aligned on a
//! half-word boundary in the DDI slave.
//!
//! This function allows 16 bit masked access to the DDI slave registers.
//!
//! Use this function to write any value in the range 0-15 bits aligned on a
//! half-word boundary. Fx. for writing the value 0b101 to bits 1-3 set
//! <tt>ui32Val = 0x000A</tt> and <tt>ui32Mask = 0x000E</tt>. Bits 0 and 5-15 will not be
//! affected by the operation, as long as the corresponding bits are not set
//! in the \c ui32Mask.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is the base address of the DDI port.
//! \param ui32Reg is register to access.
//! \param bWriteHigh defines which part of the register to write in.
//! \param ui32Mask is the mask defining which of the 16 bit that should be
//! overwritten. The mask must be defined in the lower half of the 32 bits.
//! \param ui32Val is the value to write. The value must be defined in the lower
//! half of the 32 bits.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
DDI16SetValBit(uint32_t ui32Base, uint32_t ui32Reg, bool bWriteHigh,
               uint32_t ui32Mask, uint32_t ui32Val)
{
    uint32_t ui32RegOffset;

    //
    // Check the arguments.
    //
    ASSERT(DDIBaseValid(ui32Base));
    ASSERT(ui32Reg < DDI_SLAVE_REGS);
    ASSERT(!(ui32Val & 0xFFFF0000));
    ASSERT(!(ui32Mask & 0xFFFF0000));

    //
    // Get the correct address of the first register used for setting bits
    // in the DDI slave.
    //
    ui32RegOffset = DDI_O_MASK16B + (ui32Reg << 1) + (bWriteHigh ? 4 : 0);

    //
    // Set the selected bits.
    //
    AuxAdiDdiSafeWrite(ui32Base + ui32RegOffset, (ui32Mask << 16) | ui32Val, 4);
}

//*****************************************************************************
//
//! \brief Write a single bit using a 16-bit maskable write.
//!
//! A '1' is written to the bit if \c ui32WrData is non-zero, else a '0' is written.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is the base address of the DDI port.
//! \param ui32Reg is register to access.
//! \param ui32Mask is the mask defining which of the 16 bit that should be
//! overwritten. The mask must be defined in the lower half of the 32 bits.
//! \param ui32WrData is the value to write. The value must be defined in the lower
//! half of the 32 bits.
//!
//! \return None
//
//*****************************************************************************
extern void DDI16BitWrite(uint32_t ui32Base, uint32_t ui32Reg,
                          uint32_t ui32Mask, uint32_t ui32WrData);


//*****************************************************************************
//
//! \brief Write a bit field via the DDI using 16-bit maskable write.
//!
//! Requires that bitfields not space the low/high word boundary.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is the base address of the DDI port.
//! \param ui32Reg is register to access.
//! \param ui32Mask is the mask defining which of the 16 bits that should be
//! overwritten. The mask must be defined in the lower half of the 32 bits.
//! \param ui32Shift
//! \param ui32Data
//!
//! \return None
//
//*****************************************************************************
extern void DDI16BitfieldWrite(uint32_t ui32Base, uint32_t ui32Reg,
                               uint32_t ui32Mask, uint32_t ui32Shift,
                               uint16_t ui32Data);

//*****************************************************************************
//
//! \brief Read a bit via the DDI using 16-bit READ.
//!
//! Return a zero if bit selected by mask is '0'. Else returns the mask.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is the base address of the DDI module.
//! \param ui32Reg is the register to read.
//! \param ui32Mask defines the bit which should be read.
//!
//! \return None
//
//*****************************************************************************
extern uint16_t DDI16BitRead(uint32_t ui32Base, uint32_t ui32Reg,
                             uint32_t ui32Mask);

//*****************************************************************************
//
//! \brief Read a bitfield via the DDI using 16-bit READ.
//!
//! Requires that bit fields not space the low/high word boundary.
//! Return data aligned to bit 0.
//!
//! \note Both the AUX module and the clock for the AUX SMPH module must be
//! enabled before calling this function.
//!
//! \param ui32Base is the base address of the DDI port.
//! \param ui32Reg is register to access.
//! \param ui32Mask is the mask defining which of the 16 bits that should be
//! overwritten. The mask must be defined in the lower half of the 32 bits.
//! \param ui32Shift defines the required shift of the data to align with bit 0.
//!
//! \return None
//
//*****************************************************************************
extern uint16_t DDI16BitfieldRead(uint32_t ui32Base, uint32_t ui32Reg,
                                  uint32_t ui32Mask, uint32_t ui32Shift);

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_DDI16BitWrite
        #undef  DDI16BitWrite
        #define DDI16BitWrite                   ROM_DDI16BitWrite
    #endif
    #ifdef ROM_DDI16BitfieldWrite
        #undef  DDI16BitfieldWrite
        #define DDI16BitfieldWrite              ROM_DDI16BitfieldWrite
    #endif
    #ifdef ROM_DDI16BitRead
        #undef  DDI16BitRead
        #define DDI16BitRead                    ROM_DDI16BitRead
    #endif
    #ifdef ROM_DDI16BitfieldRead
        #undef  DDI16BitfieldRead
        #define DDI16BitfieldRead               ROM_DDI16BitfieldRead
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

#endif // __DDI_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
