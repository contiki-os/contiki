/******************************************************************************
*  Filename:       ioc.h
*  Revised:        2015-01-14 12:12:44 +0100 (on, 14 jan 2015)
*  Revision:       42373
*
*  Description:    Defines and prototypes for the IO Controller.
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
//! \addtogroup ioc_api
//! @{
//
//*****************************************************************************

#ifndef __IOC_H__
#define __IOC_H__

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
#include <inc/hw_ioc.h>
#include <inc/hw_ints.h>
#include <driverlib/interrupt.h>
#include <driverlib/debug.h>
#include <driverlib/gpio.h>

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
    #define IOCPortConfigureSet             NOROM_IOCPortConfigureSet
    #define IOCPortConfigureGet             NOROM_IOCPortConfigureGet
    #define IOCIOShutdownSet                NOROM_IOCIOShutdownSet
    #define IOCIOJTagSet                    NOROM_IOCIOJTagSet
    #define IOCIOModeSet                    NOROM_IOCIOModeSet
    #define IOCIOIntSet                     NOROM_IOCIOIntSet
    #define IOCIOPortPullSet                NOROM_IOCIOPortPullSet
    #define IOCIOHystSet                    NOROM_IOCIOHystSet
    #define IOCIOInputSet                   NOROM_IOCIOInputSet
    #define IOCIOSlewCtrlSet                NOROM_IOCIOSlewCtrlSet
    #define IOCIODrvStrengthSet             NOROM_IOCIODrvStrengthSet
    #define IOCIOPortIdSet                  NOROM_IOCIOPortIdSet
    #define IOCIntEnable                    NOROM_IOCIntEnable
    #define IOCIntDisable                   NOROM_IOCIntDisable
    #define IOCPinTypeGpioInput             NOROM_IOCPinTypeGpioInput
    #define IOCPinTypeGpioOutput            NOROM_IOCPinTypeGpioOutput
    #define IOCPinTypeUart                  NOROM_IOCPinTypeUart
    #define IOCPinTypeSsiMaster             NOROM_IOCPinTypeSsiMaster
    #define IOCPinTypeSsiSlave              NOROM_IOCPinTypeSsiSlave
    #define IOCPinTypeI2c                   NOROM_IOCPinTypeI2c
    #define IOCPinTypeSpis                  NOROM_IOCPinTypeSpis
    #define IOCPinTypeAux                   NOROM_IOCPinTypeAux
#endif

//*****************************************************************************
//
// Number of IOs (max. total of 32)
//
//*****************************************************************************
#define NUM_IO_MAX 32

//*****************************************************************************
//
// The following fields are IO Id for the IOC module
//
//*****************************************************************************
#define IOID_0                  0x00000000  // IO Id 0
#define IOID_1                  0x00000001  // IO Id 1
#define IOID_2                  0x00000002  // IO Id 2
#define IOID_3                  0x00000003  // IO Id 3
#define IOID_4                  0x00000004  // IO Id 4
#define IOID_5                  0x00000005  // IO Id 5
#define IOID_6                  0x00000006  // IO Id 6
#define IOID_7                  0x00000007  // IO Id 7
#define IOID_8                  0x00000008  // IO Id 8
#define IOID_9                  0x00000009  // IO Id 9
#define IOID_10                 0x0000000A  // IO Id 10
#define IOID_11                 0x0000000B  // IO Id 11
#define IOID_12                 0x0000000C  // IO Id 12
#define IOID_13                 0x0000000D  // IO Id 13
#define IOID_14                 0x0000000E  // IO Id 14
#define IOID_15                 0x0000000F  // IO Id 15
#define IOID_16                 0x00000010  // IO Id 16
#define IOID_17                 0x00000011  // IO Id 17
#define IOID_18                 0x00000012  // IO Id 18
#define IOID_19                 0x00000013  // IO Id 19
#define IOID_20                 0x00000014  // IO Id 20
#define IOID_21                 0x00000015  // IO Id 21
#define IOID_22                 0x00000016  // IO Id 22
#define IOID_23                 0x00000017  // IO Id 23
#define IOID_24                 0x00000018  // IO Id 24
#define IOID_25                 0x00000019  // IO Id 25
#define IOID_26                 0x0000001A  // IO Id 26
#define IOID_27                 0x0000001B  // IO Id 27
#define IOID_28                 0x0000001C  // IO Id 28
#define IOID_29                 0x0000001D  // IO Id 29
#define IOID_30                 0x0000001E  // IO Id 30
#define IOID_31                 0x0000001F  // IO Id 31
#define IOID_UNUSED             0xFFFFFFFF  // Unsued IO Id

#define IOC_IOID_MASK           0x000000FF  // IOC IO Id bit mask

//*****************************************************************************
//
// Number of IO ports
//
//*****************************************************************************
#define NUM_IO_PORTS 56

//*****************************************************************************
//
// IOC Peripheral Port Mapping
//
//*****************************************************************************
#define IOC_PORT_GPIO             0x00000000  // Default general purpose IO usage
#define IOC_PORT_AON_SCS          0x00000001  // AON SPI-S SCS Pin
#define IOC_PORT_AON_SCK          0x00000002  // AON SPI-S SCK Pin
#define IOC_PORT_AON_SDI          0x00000003  // AON SPI-S SDI Pin
#define IOC_PORT_AON_SDO          0x00000004  // AON SPI-S SDO Pin
#define IOC_PORT_AON_CLK32K       0x00000007  // AON External 32kHz clock
#define IOC_PORT_AUX_IO           0x00000008  // AUX IO Pin
#define IOC_PORT_MCU_SSI0_RX      0x00000009  // MCU SSI0 Receive Pin
#define IOC_PORT_MCU_SSI0_TX      0x0000000A  // MCU SSI0 Transmit Pin
#define IOC_PORT_MCU_SSI0_FSS     0x0000000B  // MCU SSI0 FSS Pin
#define IOC_PORT_MCU_SSI0_CLK     0x0000000C  // MCU SSI0 Clock Pin
#define IOC_PORT_MCU_I2C_MSSDA    0x0000000D  // MCU I2C Data Pin
#define IOC_PORT_MCU_I2C_MSSCL    0x0000000E  // MCU I2C Clock Pin
#define IOC_PORT_MCU_UART0_RX     0x0000000F  // MCU UART0 Receive Pin
#define IOC_PORT_MCU_UART0_TX     0x00000010  // MCU UART0 Transmit Pin
#define IOC_PORT_MCU_UART0_CTS    0x00000011  // MCU UART0 Clear To Send Pin
#define IOC_PORT_MCU_UART0_RTS    0x00000012  // MCU UART0 Request To Send Pin
#define IOC_PORT_MCU_PORT_EVENT0  0x00000017  // MCU PORT EVENT 0
#define IOC_PORT_MCU_PORT_EVENT1  0x00000018  // MCU PORT EVENT 1
#define IOC_PORT_MCU_PORT_EVENT2  0x00000019  // MCU PORT EVENT 2
#define IOC_PORT_MCU_PORT_EVENT3  0x0000001A  // MCU PORT EVENT 3
#define IOC_PORT_MCU_PORT_EVENT4  0x0000001B  // MCU PORT EVENT 4
#define IOC_PORT_MCU_PORT_EVENT5  0x0000001C  // MCU PORT EVENT 5
#define IOC_PORT_MCU_PORT_EVENT6  0x0000001D  // MCU PORT EVENT 6
#define IOC_PORT_MCU_PORT_EVENT7  0x0000001E  // MCU PORT EVENT 7
#define IOC_PORT_MCU_SWV          0x00000020  // Serial Wire Viewer
#define IOC_PORT_MCU_SSI1_RX      0x00000021  // MCU SSI1 Receive Pin
#define IOC_PORT_MCU_SSI1_TX      0x00000022  // MCU SSI1 Transmit Pin
#define IOC_PORT_MCU_SSI1_FSS     0x00000023  // MCU SSI1 FSS Pin
#define IOC_PORT_MCU_SSI1_CLK     0x00000024  // MCU SSI1 Clock Pin
#define IOC_PORT_MCU_I2S_AD0      0x00000025  // MCU I2S Data Pin 0
#define IOC_PORT_MCU_I2S_AD1      0x00000026  // MCU I2S Data Pin 1
#define IOC_PORT_MCU_I2S_WCLK     0x00000027  // MCU I2S Frame/Word Clock
#define IOC_PORT_MCU_I2S_BCLK     0x00000028  // MCU I2S Bit Clock
#define IOC_PORT_MCU_I2S_MCLK     0x00000029  // MCU I2S Master clock 2
#define IOC_PORT_RFC_TRC          0x0000002E  // RF Core Tracer
#define IOC_PORT_RFC_GPO0         0x0000002F  // RC Core Data Out Pin 0
#define IOC_PORT_RFC_GPO1         0x00000030  // RC Core Data Out Pin 1
#define IOC_PORT_RFC_GPO2         0x00000031  // RC Core Data Out Pin 2
#define IOC_PORT_RFC_GPO3         0x00000032  // RC Core Data Out Pin 3
#define IOC_PORT_RFC_GPI0         0x00000033  // RC Core Data In Pin 0
#define IOC_PORT_RFC_GPI1         0x00000034  // RC Core Data In Pin 1

//*****************************************************************************
//
// Defines for enabling/disabling an IO
//
//*****************************************************************************
#define IOC_SLEW_ENABLE         0x00001000
#define IOC_SLEW_DISABLE        0x00000000
#define IOC_INPUT_ENABLE        0x20000000
#define IOC_INPUT_DISABLE       0x00000000
#define IOC_HYST_ENABLE         0x40000000
#define IOC_HYST_DISABLE        0x00000000
#define IOC_JTAG_TDO_ENABLE     0x00080000
#define IOC_JTAG_TDI_ENABLE     0x00100000
#define IOC_JTAG_DISABLE        0x00000000

//*****************************************************************************
//
// Values that can be used to set the shutdown mode of an IO
//
//*****************************************************************************
#define IOC_NO_WAKE_UP          0x00000000
#define IOC_WAKE_ON_LOW         0x10000000
#define IOC_WAKE_ON_HIGH        0x18000000

//*****************************************************************************
//
// Values that can be used to set the IO Mode of an IO
//
//*****************************************************************************
#define IOC_IOMODE_NORMAL       0x00000000  // Normal Input/Output
#define IOC_IOMODE_INV          0x01000000  // Inverted Input/Output
#define IOC_IOMODE_OPEN_DRAIN_NORMAL \
                                0x04000000  // Open Drain, Normal Input/Output
#define IOC_IOMODE_OPEN_DRAIN_INV \
                                0x05000000  // Open Drain, Inverted
                                            // Input/Output
#define IOC_IOMODE_OPEN_SRC_NORMAL \
                                0x06000000  // Open Source, Normal Input/Output
#define IOC_IOMODE_OPEN_SRC_INV \
                                0x07000000  // Open Source, Inverted
                                            // Input/Output

//*****************************************************************************
//
// Values that can be used to set the edge detection on an IO
//
//*****************************************************************************
#define IOC_NO_EDGE             0x00000000  // No edge detection
#define IOC_FALLING_EDGE        0x00010000  // Edge detection on falling edge
#define IOC_RISING_EDGE         0x00020000  // Edge detection on rising edge
#define IOC_BOTH_EDGES          0x00030000  // Edge detection on both edges
#define IOC_INT_ENABLE          0x00040000  // Enable interrupt on edge detect
#define IOC_INT_DISABLE         0x00000000  // Disable interrupt on edge detect
#define IOC_INT_M               0x00070000  // Int config mask

//*****************************************************************************
//
// Values that be used to set pull on an IO
//
//*****************************************************************************
#define IOC_NO_IOPULL           0x00006000  // No IO pull
#define IOC_IOPULL_UP           0x00004000  // Pull up
#define IOC_IOPULL_DOWN         0x00002000  // Pull down
#define IOC_IOPULL_M            0x00006000  // Pull config mask
#define IOC_IOPULL_M            0x00006000

//*****************************************************************************
//
// Values that can be used to select the drive strength of an IO
//
//*****************************************************************************
#define IOC_CURRENT_2MA         0x00000000  // 2mA drive strength
#define IOC_CURRENT_4MA         0x00000400  // 4mA drive strength
#define IOC_CURRENT_8MA         0x00000800  // 4 or 8mA drive strength
#define IOC_CURRENT_16MA        0x00000C00  // Up to 16mA drive strength

#define IOC_STRENGTH_AUTO       0x00000000  // Automatic Drive Strength
                                            // (2/4/8 mA @ VVDS)
#define IOC_STRENGTH_MAX        0x00000300  // Maximum Drive Strength
                                            // (2/4/8 mA @ 1.8V)
#define IOC_STRENGTH_MED        0x00000200  // Medium Drive Strength
                                            // (2/4/8 mA @ 2.5V)
#define IOC_STRENGTH_MIN        0x00000100  // Miniumm Drive Strength
                                            // (2/4/8 mA @ 3.3V)
//*****************************************************************************
//
// Defines for standard IO setup
//
//*****************************************************************************
#define IOC_STD_INPUT           (IOC_CURRENT_2MA | IOC_STRENGTH_AUTO |      \
                                 IOC_NO_IOPULL | IOC_SLEW_DISABLE |         \
                                 IOC_HYST_DISABLE | IOC_NO_EDGE |           \
                                 IOC_INT_DISABLE | IOC_IOMODE_NORMAL | \
                                 IOC_NO_WAKE_UP | IOC_INPUT_ENABLE |        \
                                 IOC_JTAG_DISABLE)
#define IOC_STD_OUTPUT          (IOC_CURRENT_2MA | IOC_STRENGTH_AUTO |      \
                                 IOC_NO_IOPULL | IOC_SLEW_DISABLE |         \
                                 IOC_HYST_DISABLE | IOC_NO_EDGE |           \
                                 IOC_INT_DISABLE | IOC_IOMODE_NORMAL | \
                                 IOC_NO_WAKE_UP | IOC_INPUT_DISABLE |       \
                                 IOC_JTAG_DISABLE)

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Set the configuration of an IO port.
//!
//! This function is used to configure the functionality of an IO.
//!
//! The \c ui32IOId parameter specifies which IO to configure.
//!
//! The \c ui32PortId parameter specifies which functional peripheral to hook
//! up to this IO.
//!
//! The \c ui32IOConfig parameter consists of a bitwise OR'ed value of all
//! the available configuration modes
//!
//! \note All IO Ports are tied to a specific functionality in a sub module
//! except for the \ref IOC_PORT_AUX_IO. Each of the IOs in the AUX domain are
//! hardcoded to a specific IO. When enabling one or more pins for the AUX
//! domain, they should all be configured to using \ref IOC_PORT_AUX_IO.
//!
//! \param ui32IOId defines the IO to configure and must be one of the following:
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32PortId selects the functional IO port to connect.
//! The available IO ports are:
//! - \ref IOC_PORT_GPIO
//! - \ref IOC_PORT_AON_SCS
//! - \ref IOC_PORT_AON_SCK
//! - \ref IOC_PORT_AON_SDI
//! - \ref IOC_PORT_AON_SDO
//! - \ref IOC_PORT_AON_CLK32K
//! - \ref IOC_PORT_AUX_IO
//! - \ref IOC_PORT_MCU_SSI0_RX
//! - \ref IOC_PORT_MCU_SSI0_TX
//! - \ref IOC_PORT_MCU_SSI0_FSS
//! - \ref IOC_PORT_MCU_SSI0_CLK
//! - \ref IOC_PORT_MCU_I2C_MSSDA
//! - \ref IOC_PORT_MCU_I2C_MSSCL
//! - \ref IOC_PORT_MCU_UART0_RX
//! - \ref IOC_PORT_MCU_UART0_TX
//! - \ref IOC_PORT_MCU_UART0_CTS
//! - \ref IOC_PORT_MCU_UART0_RTS
//! - \ref IOC_PORT_MCU_PORT_EVENT0
//! - \ref IOC_PORT_MCU_PORT_EVENT1
//! - \ref IOC_PORT_MCU_PORT_EVENT2
//! - \ref IOC_PORT_MCU_PORT_EVENT3
//! - \ref IOC_PORT_MCU_PORT_EVENT4
//! - \ref IOC_PORT_MCU_PORT_EVENT5
//! - \ref IOC_PORT_MCU_PORT_EVENT6
//! - \ref IOC_PORT_MCU_PORT_EVENT7
//! - \ref IOC_PORT_MCU_SWV
//! - \ref IOC_PORT_MCU_SSI1_RX
//! - \ref IOC_PORT_MCU_SSI1_TX
//! - \ref IOC_PORT_MCU_SSI1_FSS
//! - \ref IOC_PORT_MCU_SSI1_CLK
//! - \ref IOC_PORT_MCU_I2S_AD0
//! - \ref IOC_PORT_MCU_I2S_AD1
//! - \ref IOC_PORT_MCU_I2S_WCLK
//! - \ref IOC_PORT_MCU_I2S_BCLK
//! - \ref IOC_PORT_MCU_I2S_MCLK
//! - \ref IOC_PORT_RFC_TRC
//! - \ref IOC_PORT_RFC_GPO0
//! - \ref IOC_PORT_RFC_GPO1
//! - \ref IOC_PORT_RFC_GPO2
//! - \ref IOC_PORT_RFC_GPO3
//! - \ref IOC_PORT_RFC_GPI0
//! - \ref IOC_PORT_RFC_GPI1
//! \param ui32IOConfig is the IO configuration consisting of
//! the bitwise OR of all configuration modes:
//! - Input/output mode:
//!   - \ref IOC_IOMODE_NORMAL
//!   - \ref IOC_IOMODE_INV
//!   - \ref IOC_IOMODE_OPEN_DRAIN_NORMAL
//!   - \ref IOC_IOMODE_OPEN_DRAIN_INV
//!   - \ref IOC_IOMODE_OPEN_SRC_NORMAL
//!   - \ref IOC_IOMODE_OPEN_SRC_INV
//! - Wake up mode:
//!   - \ref IOC_NO_WAKE_UP
//!   - \ref IOC_WAKE_ON_LOW
//!   - \ref IOC_WAKE_ON_HIGH
//! - JTAG configuration modes:
//!   - \ref IOC_JTAG_TDO_ENABLE
//!   - \ref IOC_JTAG_TDI_ENABLE
//!   - \ref IOC_JTAG_DISABLE
//! - Edge detection mode:
//!   - \ref IOC_NO_EDGE
//!   - \ref IOC_FALLING_EDGE
//!   - \ref IOC_RISING_EDGE
//!   - \ref IOC_BOTH_EDGES
//! - Interrupt mode on edge detection:
//!   - \ref IOC_INT_ENABLE
//!   - \ref IOC_INT_DISABLE
//! - Pull mode:
//!   - \ref IOC_NO_IOPULL
//!   - \ref IOC_IOPULL_UP
//!   - \ref IOC_IOPULL_DOWN
//! - Input mode:
//!   - \ref IOC_INPUT_ENABLE
//!   - \ref IOC_INPUT_DISABLE
//! - Hysteresis mode:
//!   - \ref IOC_HYST_ENABLE
//!   - \ref IOC_HYST_DISABLE
//! - Slew control mode:
//!   - \ref IOC_SLEW_ENABLE
//!   - \ref IOC_SLEW_DISABLE
//! - Maximum current mode:
//!   - \ref IOC_CURRENT_2MA
//!   - \ref IOC_CURRENT_4MA
//!   - \ref IOC_CURRENT_8MA
//!   - \ref IOC_CURRENT_16MA
//! - Drive strength mode:
//!   - \ref IOC_STRENGTH_AUTO
//!   - \ref IOC_STRENGTH_MAX
//!   - \ref IOC_STRENGTH_MED
//!   - \ref IOC_STRENGTH_MIN
//!
//! \return None
//
//*****************************************************************************
extern void IOCPortConfigureSet(uint32_t ui32IOId, uint32_t ui32PortId,
                                uint32_t ui32IOConfig);

//*****************************************************************************
//
//! \brief Get the configuration of an IO port.
//!
//! This function is used for getting the configuration of an IO.
//!
//! Each IO port has a dedicated register for setting up the IO. This function
//! returns the current configuration for the given IO.
//!
//! \param ui32IOId selects the IO to return the configuration for.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//!
//! \return Returns the IO Port configuration.
//! See \ref IOCPortConfigureSet() for configuration options.
//
//*****************************************************************************
extern uint32_t IOCPortConfigureGet(uint32_t ui32IOId);

//*****************************************************************************
//
//! \brief Set wake-up on an IO port.
//!
//! This function is used to set the wake-up mode of an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32IOShutdown enables wake-up on LOW/HIGH by this IO port.
//! - \ref IOC_NO_WAKE_UP
//! - \ref IOC_WAKE_ON_LOW
//! - \ref IOC_WAKE_ON_HIGH
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOShutdownSet(uint32_t ui32IOId, uint32_t ui32IOShutdown);

//*****************************************************************************
//
//! \brief Set wake-up on an IO port.
//!
//! This function is used to set the wake-up mode of an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32IOJTag enables the IO for JTAG operation.
//! - \ref IOC_JTAG_TDO_ENABLE
//! - \ref IOC_JTAG_TDI_ENABLE
//! - \ref IOC_JTAG_DISABLE
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOJTagSet(uint32_t ui32IOId, uint32_t ui32IOJTag);

//*****************************************************************************
//
//! \brief Set the IO Mode of an IO Port.
//!
//! This function is used to set the input/output mode of an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32IOMode sets the port IO Mode.
//! - \ref IOC_IOMODE_NORMAL
//! - \ref IOC_IOMODE_INV
//! - \ref IOC_IOMODE_OPEN_DRAIN_NORMAL
//! - \ref IOC_IOMODE_OPEN_DRAIN_INV
//! - \ref IOC_IOMODE_OPEN_SRC_NORMAL
//! - \ref IOC_IOMODE_OPEN_SRC_INV
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOModeSet(uint32_t ui32IOId, uint32_t ui32IOMode);

//*****************************************************************************
//
//! \brief Setup interrupt detection on an IO Port.
//!
//! This function is used to setup the interrupt detection on an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32Int enables/disables interrupt generation on this IO port.
//! - \ref IOC_INT_ENABLE
//! - \ref IOC_INT_DISABLE
//! \param ui32EdgeDet enables/disables edge detection events on this IO port.
//! - \ref IOC_NO_EDGE
//! - \ref IOC_FALLING_EDGE
//! - \ref IOC_RISING_EDGE
//! - \ref IOC_BOTH_EDGES
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOIntSet(uint32_t ui32IOId, uint32_t ui32Int,
                        uint32_t ui32EdgeDet);

//*****************************************************************************
//
//! \brief Set the pull on an IO port.
//!
//! This function is used to configure the pull on an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32Pull enables/disables pull on this IO port.
//! - \ref IOC_NO_IOPULL
//! - \ref IOC_IOPULL_UP
//! - \ref IOC_IOPULL_DOWN
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOPortPullSet(uint32_t ui32IOId, uint32_t ui32Pull);

//*****************************************************************************
//
//! \brief Configure hysteresis on and IO port.
//!
//! This function is used to enable/disable hysteresis on an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32Hysteresis enable/disable input hysteresis on IO.
//! - \ref IOC_HYST_ENABLE
//! - \ref IOC_HYST_DISABLE
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOHystSet(uint32_t ui32IOId, uint32_t ui32Hysteresis);

//*****************************************************************************
//
//! \brief Enable/disable IO port as input.
//!
//! This function is used to enable/disable input on an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32Input enable/disable input on IO.
//! - \ref IOC_INPUT_ENABLE
//! - \ref IOC_INPUT_DISABLE
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOInputSet(uint32_t ui32IOId, uint32_t ui32Input);

//*****************************************************************************
//
//! \brief Enable/disable the slew control on an IO port.
//!
//! This function is used to enable/disable slew control on an IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32SlewEnable enables/disables the slew control on an output.
//! - \ref IOC_SLEW_ENABLE
//! - \ref IOC_SLEW_DISABLE
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOSlewCtrlSet(uint32_t ui32IOId, uint32_t ui32SlewEnable);

//*****************************************************************************
//
//! \brief Configure the drive strength and maxium current of an IO port.
//!
//! This function sets the drive strength of an IO port.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32IOCurrent sets the IO current on the IO port.
//! - \ref IOC_CURRENT_2MA
//! - \ref IOC_CURRENT_4MA
//! - \ref IOC_CURRENT_8MA
//! - \ref IOC_CURRENT_16MA
//! \param ui32DrvStrength sets the drive strength for the IO port.
//! - \ref IOC_STRENGTH_AUTO
//! - \ref IOC_STRENGTH_MAX
//! - \ref IOC_STRENGTH_MED
//! - \ref IOC_STRENGTH_MIN
//!
//! \return None
//
//*****************************************************************************
extern void IOCIODrvStrengthSet(uint32_t ui32IOId, uint32_t ui32IOCurrent,
                                uint32_t ui32DrvStrength);

//*****************************************************************************
//
//! \brief Setup the Port ID for this IO.
//!
//! The \c ui32PortId specifies which functional peripheral to hook up to this
//! IO.
//!
//! \param ui32IOId defines the IO to configure.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! \param ui32PortId selects the port to map to the IO.
//! - \ref IOC_PORT_GPIO
//! - \ref IOC_PORT_AON_SCS
//! - \ref IOC_PORT_AON_SCK
//! - \ref IOC_PORT_AON_SDI
//! - \ref IOC_PORT_AON_SDO
//! - \ref IOC_PORT_AON_CLK32K
//! - \ref IOC_PORT_AUX_IO
//! - \ref IOC_PORT_MCU_SSI0_RX
//! - \ref IOC_PORT_MCU_SSI0_TX
//! - \ref IOC_PORT_MCU_SSI0_FSS
//! - \ref IOC_PORT_MCU_SSI0_CLK
//! - \ref IOC_PORT_MCU_I2C_MSSDA
//! - \ref IOC_PORT_MCU_I2C_MSSCL
//! - \ref IOC_PORT_MCU_UART0_RX
//! - \ref IOC_PORT_MCU_UART0_TX
//! - \ref IOC_PORT_MCU_UART0_CTS
//! - \ref IOC_PORT_MCU_UART0_RTS
//! - \ref IOC_PORT_MCU_PORT_EVENT0
//! - \ref IOC_PORT_MCU_PORT_EVENT1
//! - \ref IOC_PORT_MCU_PORT_EVENT2
//! - \ref IOC_PORT_MCU_PORT_EVENT3
//! - \ref IOC_PORT_MCU_PORT_EVENT4
//! - \ref IOC_PORT_MCU_PORT_EVENT5
//! - \ref IOC_PORT_MCU_PORT_EVENT6
//! - \ref IOC_PORT_MCU_PORT_EVENT7
//! - \ref IOC_PORT_MCU_SWV
//! - \ref IOC_PORT_MCU_SSI1_RX
//! - \ref IOC_PORT_MCU_SSI1_TX
//! - \ref IOC_PORT_MCU_SSI1_FSS
//! - \ref IOC_PORT_MCU_SSI1_CLK
//! - \ref IOC_PORT_MCU_I2S_AD0
//! - \ref IOC_PORT_MCU_I2S_AD1
//! - \ref IOC_PORT_MCU_I2S_WCLK
//! - \ref IOC_PORT_MCU_I2S_BCLK
//! - \ref IOC_PORT_MCU_I2S_MCLK
//! - \ref IOC_PORT_RFC_TRC
//! - \ref IOC_PORT_RFC_GPO0
//! - \ref IOC_PORT_RFC_GPO1
//! - \ref IOC_PORT_RFC_GPO2
//! - \ref IOC_PORT_RFC_GPO3
//! - \ref IOC_PORT_RFC_GPI0
//! - \ref IOC_PORT_RFC_GPI1
//!
//! \return None
//
//*****************************************************************************
extern void IOCIOPortIdSet(uint32_t ui32IOId, uint32_t ui32PortId);

//*****************************************************************************
//
//! \brief Register an interrupt handler for an IO edge interrupt.
//!
//! This function does the actual registering of the interrupt handler. This
//! function enables the global interrupt in the interrupt controller; specific
//! IO interrupts must be enabled via \ref IOCIntEnable(). It is the interrupt
//! handler's responsibility to clear the interrupt source.
//!
//! \param pfnHandler is a pointer to the function to be called when the
//! IOC interrupt occurs.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
__STATIC_INLINE void
IOCIntRegister(void (*pfnHandler)(void))
{
    //
    // Register the interrupt handler.
    //
    IntRegister(INT_EDGE_DETECT, pfnHandler);

    //
    // Enable the IO edge interrupt.
    //
    IntEnable(INT_EDGE_DETECT);
}

//*****************************************************************************
//
//! \brief Unregisters an interrupt handler for a IO edge interrupt.
//!
//! This function does the actual unregistering of the interrupt handler.  It
//! clears the handler to be called when an IO edge interrupt occurs.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
__STATIC_INLINE void
IOCIntUnregister(void)
{
    //
    // Disable the interrupts.
    //
    IntDisable(INT_EDGE_DETECT);

    //
    // Unregister the interrupt handler.
    //
    IntUnregister(INT_EDGE_DETECT);
}

//*****************************************************************************
//
//! \brief Enables individual IO edge detect interrupt.
//!
//! This function enables the indicated IO edge interrupt sources. Only the
//! sources that are enabled can be reflected to the processor interrupt;
//! disabled sources have no effect on the processor.
//!
//! \param ui32IOId is the IO to enable edge detect interrupt for.
//!
//! \return None
//
//*****************************************************************************
extern void IOCIntEnable(uint32_t ui32IOId);

//*****************************************************************************
//
//! \brief Disables individual IO edge interrupt sources.
//!
//! This function disables the indicated IO edge interrupt source. Only the
//! sources that are enabled can be reflected to the processor interrupt;
//! disabled sources have no effect on the processor.
//!
//! \param ui32IOId is the IO edge interrupt source to be disabled.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//!
//! \return None
//
//*****************************************************************************
extern void IOCIntDisable(uint32_t ui32IOId);

//*****************************************************************************
//
//! \brief Clears the IO edge interrupt source.
//!
//! The specified IO edge interrupt source is cleared, so that it no longer
//! asserts. This function must be called in the interrupt handler to keep the
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
//! \param ui32IOId is the IO causing the interrupt.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
IOCIntClear(uint32_t ui32IOId)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32IOId <= IOID_31);

    //
    // Clear the requested interrupt source by clearing the event.
    //
    GPIOEventClear(1 << ui32IOId);
}

//*****************************************************************************
//
//! \brief Returns the status of the IO interrupts.
//!
//! TBD : Please check with designer if IOC configuration bit
//! (IOC_IOCFG0_EDGE_IRQ_EN) works as a masking bit for the interrupt generation.
//! In other words will GPIO events always appear in the EVENT register in the GPIO module.
//!
//! \param ui32IOId is the IO to get the status for.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE uint32_t
IOCIntStatus(uint32_t ui32IOId)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32IOId <= IOID_31);

    //
    // Clear the requested interrupt source by clearing the event.
    //
    return (GPIOEventGet(1 << ui32IOId));
}


//*****************************************************************************
//
//! \brief Setup an IO for standard GPIO input.
//!
//! Setup an IO for standard GPIO input with the following configuration:
//! - Port ID:
//!   - \ref IOC_PORT_GPIO
//! - Configuration:
//!   - \ref IOC_CURRENT_2MA
//!   - \ref IOC_STRENGTH_AUTO
//!   - \ref IOC_NO_IOPULL
//!   - \ref IOC_SLEW_DISABLE
//!   - \ref IOC_HYST_DISABLE
//!   - \ref IOC_NO_EDGE
//!   - \ref IOC_INT_DISABLE
//!   - \ref IOC_IOMODE_NORMAL
//!   - \ref IOC_NO_WAKE_UP
//!   - \ref IOC_INPUT_ENABLE
//!   - \ref IOC_JTAG_DISABLE
//!
//! \param ui32IOId is the IO to setup for GPIO input
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeGpioInput(uint32_t ui32IOId);

//*****************************************************************************
//
//! \brief Setup an IO for standard GPIO output.
//!
//! Setup an IO for standard GPIO output with the following configuration:
//! - Port ID:
//!   - \ref IOC_PORT_GPIO
//! - Configuration:
//!   - \ref IOC_CURRENT_2MA
//!   - \ref IOC_STRENGTH_AUTO
//!   - \ref IOC_NO_IOPULL
//!   - \ref IOC_SLEW_DISABLE
//!   - \ref IOC_HYST_DISABLE
//!   - \ref IOC_NO_EDGE
//!   - \ref IOC_INT_DISABLE
//!   - \ref IOC_IOMODE_NORMAL
//!   - \ref IOC_NO_WAKE_UP
//!   - \ref IOC_INPUT_DISABLE
//!   - \ref IOC_JTAG_DISABLE
//!
//! \param ui32IOId is the IO to setup for GPIO output
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeGpioOutput(uint32_t ui32IOId);

//*****************************************************************************
//
//! \brief Configure a set of IOs for standard UART peripheral control.
//!
//! The UART pins must be properly configured for the UART peripheral to
//! function correctly. This function provides a typical configuration for
//! those pin(s). Other configurations may work as well depending upon the
//! board setup (for example, using the on-chip pull-ups).
//!
//! \note If a UART pin is not intended to be used, then the parameter in the
//! function should be \ref IOID_UNUSED.
//!
//! \param ui32Base is the base address of the UART module.
//! \param ui32Rx is the IO Id of the IO to use as UART Receive.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Tx is the IO Id of the IO to use as UART Transmit.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Cts is the IO Id of the IO to use for UART Clear to send.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Rts is the IO Id of the IO to use for UART Request to send.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeUart(uint32_t ui32Base, uint32_t ui32Rx,
                           uint32_t ui32Tx, uint32_t ui32Cts,
                           uint32_t ui32Rts);

//*****************************************************************************
//
//! \brief Configure a set of IOs for standard SSI peripheral master control.
//!
//! \param ui32Base is the mase address of the SSI module to connect to the IOs
//! \param ui32Rx is the IO to connect to the SSI MISO line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Tx is the IO to connect to the SSI MOSI line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Fss is the IO to connect to the SSI FSS line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Clk is the IO to connect to the SSI Clock output line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeSsiMaster(uint32_t ui32Base, uint32_t ui32Rx,
                                uint32_t ui32Tx, uint32_t ui32Fss,
                                uint32_t ui32Clk);

//*****************************************************************************
//
//! \brief Configure a set of IOs for standard SSI peripheral slave control.
//!
//! \param ui32Base is the base address of the SSI module to connect to the IOs
//! \param ui32Rx is the IO to connect to the SSI MOSI line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Tx is the IO to connect to the SSI MISO line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Fss is the IO to connect to the SSI FSS line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Clk is the IO to connect to the SSI Clock input line.
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeSsiSlave(uint32_t ui32Base, uint32_t ui32Rx,
                               uint32_t ui32Tx, uint32_t ui32Fss,
                               uint32_t ui32Clk);

//*****************************************************************************
//
//! \brief Configure a set of IOs for standard I2C peripheral control.
//!
//! \param ui32Base is the base address of the I2C module to connect to the IOs
//! \param ui32Data is the I2C data line
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Clk is the I2C input clock
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeI2c(uint32_t ui32Base, uint32_t ui32Data,
                          uint32_t ui32Clk);

//*****************************************************************************
//
//! \brief Configure a set of IOs for standard SPIS peripheral control.
//!
//! \param ui32Rx is the IO to connect to the SPIs MOSI line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Tx is the IO to connect to the SPIS MISO line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Fss is the IO to connect to the SPIS FSS line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//! \param ui32Clk is the IO to connect to the SPIS Clock input line.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeSpis(uint32_t ui32Rx, uint32_t ui32Tx, uint32_t ui32Fss,
                           uint32_t ui32Clk);

//*****************************************************************************
//
//! \brief Configure an IO for AUX control.
//!
//! Use this function to enable AUX to control a specific IO. Please note, that
//! when using AUX to control the IO, the input/output control in the IOC is
//! bypassed and completely controlled by AUX, so enabling or disabling input
//! in the IOC has no effect.
//!
//! \note The IOs available for AUX control can vary from device to device.
//!
//! \param ui32IOId is the IO to setup for AUX usage.
//! - \ref IOID_0
//! - ...
//! - \ref IOID_31
//! - \ref IOID_UNUSED
//!
//! \return None
//
//*****************************************************************************
extern void IOCPinTypeAux(uint32_t ui32IOId);

//*****************************************************************************
//
// Support for DriverLib in ROM:
// Redirect to implementation in ROM when available.
//
//*****************************************************************************
#ifndef DRIVERLIB_NOROM
    #include <driverlib/rom.h>
    #ifdef ROM_IOCPortConfigureSet
        #undef  IOCPortConfigureSet
        #define IOCPortConfigureSet             ROM_IOCPortConfigureSet
    #endif
    #ifdef ROM_IOCPortConfigureGet
        #undef  IOCPortConfigureGet
        #define IOCPortConfigureGet             ROM_IOCPortConfigureGet
    #endif
    #ifdef ROM_IOCIOShutdownSet
        #undef  IOCIOShutdownSet
        #define IOCIOShutdownSet                ROM_IOCIOShutdownSet
    #endif
    #ifdef ROM_IOCIOJTagSet
        #undef  IOCIOJTagSet
        #define IOCIOJTagSet                    ROM_IOCIOJTagSet
    #endif
    #ifdef ROM_IOCIOModeSet
        #undef  IOCIOModeSet
        #define IOCIOModeSet                    ROM_IOCIOModeSet
    #endif
    #ifdef ROM_IOCIOIntSet
        #undef  IOCIOIntSet
        #define IOCIOIntSet                     ROM_IOCIOIntSet
    #endif
    #ifdef ROM_IOCIOPortPullSet
        #undef  IOCIOPortPullSet
        #define IOCIOPortPullSet                ROM_IOCIOPortPullSet
    #endif
    #ifdef ROM_IOCIOHystSet
        #undef  IOCIOHystSet
        #define IOCIOHystSet                    ROM_IOCIOHystSet
    #endif
    #ifdef ROM_IOCIOInputSet
        #undef  IOCIOInputSet
        #define IOCIOInputSet                   ROM_IOCIOInputSet
    #endif
    #ifdef ROM_IOCIOSlewCtrlSet
        #undef  IOCIOSlewCtrlSet
        #define IOCIOSlewCtrlSet                ROM_IOCIOSlewCtrlSet
    #endif
    #ifdef ROM_IOCIODrvStrengthSet
        #undef  IOCIODrvStrengthSet
        #define IOCIODrvStrengthSet             ROM_IOCIODrvStrengthSet
    #endif
    #ifdef ROM_IOCIOPortIdSet
        #undef  IOCIOPortIdSet
        #define IOCIOPortIdSet                  ROM_IOCIOPortIdSet
    #endif
    #ifdef ROM_IOCIntEnable
        #undef  IOCIntEnable
        #define IOCIntEnable                    ROM_IOCIntEnable
    #endif
    #ifdef ROM_IOCIntDisable
        #undef  IOCIntDisable
        #define IOCIntDisable                   ROM_IOCIntDisable
    #endif
    #ifdef ROM_IOCPinTypeGpioInput
        #undef  IOCPinTypeGpioInput
        #define IOCPinTypeGpioInput             ROM_IOCPinTypeGpioInput
    #endif
    #ifdef ROM_IOCPinTypeGpioOutput
        #undef  IOCPinTypeGpioOutput
        #define IOCPinTypeGpioOutput            ROM_IOCPinTypeGpioOutput
    #endif
    #ifdef ROM_IOCPinTypeUart
        #undef  IOCPinTypeUart
        #define IOCPinTypeUart                  ROM_IOCPinTypeUart
    #endif
    #ifdef ROM_IOCPinTypeSsiMaster
        #undef  IOCPinTypeSsiMaster
        #define IOCPinTypeSsiMaster             ROM_IOCPinTypeSsiMaster
    #endif
    #ifdef ROM_IOCPinTypeSsiSlave
        #undef  IOCPinTypeSsiSlave
        #define IOCPinTypeSsiSlave              ROM_IOCPinTypeSsiSlave
    #endif
    #ifdef ROM_IOCPinTypeI2c
        #undef  IOCPinTypeI2c
        #define IOCPinTypeI2c                   ROM_IOCPinTypeI2c
    #endif
    #ifdef ROM_IOCPinTypeSpis
        #undef  IOCPinTypeSpis
        #define IOCPinTypeSpis                  ROM_IOCPinTypeSpis
    #endif
    #ifdef ROM_IOCPinTypeAux
        #undef  IOCPinTypeAux
        #define IOCPinTypeAux                   ROM_IOCPinTypeAux
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

#endif // __IOC_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************
