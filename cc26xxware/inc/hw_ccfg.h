/******************************************************************************
*  Filename:       hw_ccfg.h
*  Revised:        $Date$
*  Revision:       $Revision$
*
* Copyright (c) 2015, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1) Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2) Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3) Neither the name of the ORGANIZATION nor the names of its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __HW_CCFG_H__
#define __HW_CCFG_H__


//*****************************************************************************
//
// This section defines the register offsets of
// CCFG component
//
//*****************************************************************************
// Mode Configuration 1
#define CCFG_O_MODE_CONF_1                                          0x00000FAC

// CCFG Size and Disable Flags
#define CCFG_O_SIZE_AND_DIS_FLAGS                                   0x00000FB0

// Mode Configuration 0
#define CCFG_O_MODE_CONF                                            0x00000FB4

// Voltage Load 0
#define CCFG_O_VOLT_LOAD_0                                          0x00000FB8

// Voltage Load 1
#define CCFG_O_VOLT_LOAD_1                                          0x00000FBC

// Real Time Clock Offset
#define CCFG_O_RTC_OFFSET                                           0x00000FC0

// Frequency Offset
#define CCFG_O_FREQ_OFFSET                                          0x00000FC4

// IEEE MAC Address 0
#define CCFG_O_IEEE_MAC_0                                           0x00000FC8

// IEEE MAC Address 1
#define CCFG_O_IEEE_MAC_1                                           0x00000FCC

// IEEE BLE Address 0
#define CCFG_O_IEEE_BLE_0                                           0x00000FD0

// IEEE BLE Address 1
#define CCFG_O_IEEE_BLE_1                                           0x00000FD4

// Bootloader Config
#define CCFG_O_BL_CONFIG                                            0x00000FD8

// Erase Configuration
#define CCFG_O_ERASE_CONF                                           0x00000FDC

// TI Options
#define CCFG_O_CCFG_TI_OPTIONS                                      0x00000FE0

// Test Access Points Enable 0
#define CCFG_O_CCFG_TAP_DAP_0                                       0x00000FE4

// Test Access Points Enable 1
#define CCFG_O_CCFG_TAP_DAP_1                                       0x00000FE8

// Image Valid
#define CCFG_O_IMAGE_VALID_CONF                                     0x00000FEC

// Protect Sectors 0-31
#define CCFG_O_CCFG_PROT_31_0                                       0x00000FF0

// Protect Sectors 32-63
#define CCFG_O_CCFG_PROT_63_32                                      0x00000FF4

// Protect Sectors 64-95
#define CCFG_O_CCFG_PROT_95_64                                      0x00000FF8

// Protect Sectors 96-127
#define CCFG_O_CCFG_PROT_127_96                                     0x00000FFC

//*****************************************************************************
//
// Register: CCFG_O_MODE_CONF_1
//
//*****************************************************************************
// Field: [23:20] ALT_DCDC_VMIN
//
// Minimum voltage to use DC/DC is (28 + ALT_DCDC_VMIN)/16.
// 0: 1.75V
// 1: 1.8125V
//  .
//  .
// 14: 2.625V
// 15: 2.6875V
#define CCFG_MODE_CONF_1_ALT_DCDC_VMIN_M                            0x00F00000
#define CCFG_MODE_CONF_1_ALT_DCDC_VMIN_S                            20

// Field: [19]    ALT_DCDC_DITHER_EN
//
// Enable DC/DC dithering
#define CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN                         0x00080000
#define CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN_BITN                    19
#define CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN_M                       0x00080000
#define CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN_S                       19

// Field: [18:16] ALT_DCDC_IPEAK
//
// Inductor peak current.
// 0: Min 31mA
// 4: Typical 47mA
// 7: Max 59mA
#define CCFG_MODE_CONF_1_ALT_DCDC_IPEAK_M                           0x00070000
#define CCFG_MODE_CONF_1_ALT_DCDC_IPEAK_S                           16

// Field: [15:12] DELTA_IBIAS_INIT
//
// Signed delta value for IBIAS_INIT
#define CCFG_MODE_CONF_1_DELTA_IBIAS_INIT_M                         0x0000F000
#define CCFG_MODE_CONF_1_DELTA_IBIAS_INIT_S                         12

// Field: [11:8] DELTA_IBIAS_OFFSET
//
// Signed delta value for IBIAS_OFFSET
#define CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET_M                       0x00000F00
#define CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET_S                       8

// Field: [7:0] XOSC_MAX_START
//
// Unsigned value of maximum XOSC startup time in units of 100us
#define CCFG_MODE_CONF_1_XOSC_MAX_START_M                           0x000000FF
#define CCFG_MODE_CONF_1_XOSC_MAX_START_S                           0

//*****************************************************************************
//
// Register: CCFG_O_SIZE_AND_DIS_FLAGS
//
//*****************************************************************************
// Field: [31:16] SIZE_OF_CCFG
//
// Total size of CCFG
#define CCFG_SIZE_AND_DIS_FLAGS_SIZE_OF_CCFG_M                      0xFFFF0000
#define CCFG_SIZE_AND_DIS_FLAGS_SIZE_OF_CCFG_S                      16

// Field: [15:2] DISABLE_FLAGS
//
// Reserved for functionality disable flags.
// Currently unused.
#define CCFG_SIZE_AND_DIS_FLAGS_DISABLE_FLAGS_M                     0x0000FFFC
#define CCFG_SIZE_AND_DIS_FLAGS_DISABLE_FLAGS_S                     2

// Field: [1]    DIS_ALT_DCDC_SETTING
//
// Disable flag for alternative DCDC setting
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING                0x00000002
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING_BITN           1
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING_M              0x00000002
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING_S              1

// Field: [0]    DIS_XOSC_OVR
//
// Disable flag for XOSC override functionality.
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR                        0x00000001
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_BITN                   0
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_M                      0x00000001
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_S                      0

//*****************************************************************************
//
// Register: CCFG_O_MODE_CONF
//
//*****************************************************************************
// Field: [27]    DCDC_RECHARGE
//
// 0: Use the DC/DC during recharge in powerdown
// 1:Do not use the DC/DC during recharge in powerdown (default)
#define CCFG_MODE_CONF_DCDC_RECHARGE                                0x08000000
#define CCFG_MODE_CONF_DCDC_RECHARGE_BITN                           27
#define CCFG_MODE_CONF_DCDC_RECHARGE_M                              0x08000000
#define CCFG_MODE_CONF_DCDC_RECHARGE_S                              27

// Field: [26]    DCDC_ACTIVE
//
// 0: Use the DC/DC during active mode
// 1: Do not use the DC/DC during active mode (default)
#define CCFG_MODE_CONF_DCDC_ACTIVE                                  0x04000000
#define CCFG_MODE_CONF_DCDC_ACTIVE_BITN                             26
#define CCFG_MODE_CONF_DCDC_ACTIVE_M                                0x04000000
#define CCFG_MODE_CONF_DCDC_ACTIVE_S                                26

// Field: [25]    VDDR_EXT_LOAD
//
// 0: VDDR may be loaded externally (may not be supported)
// 1: VDDR is not loaded externally (default)
#define CCFG_MODE_CONF_VDDR_EXT_LOAD                                0x02000000
#define CCFG_MODE_CONF_VDDR_EXT_LOAD_BITN                           25
#define CCFG_MODE_CONF_VDDR_EXT_LOAD_M                              0x02000000
#define CCFG_MODE_CONF_VDDR_EXT_LOAD_S                              25

// Field: [24]    VDDS_BOD_LEVEL
//
// 1: VDDS BOD level is 1.8V (or 1.65V for external regulator mode) (default).
// 0: VDDS BOD level is 2.0V (necessary for 125 degrees C or for external load
// mode, or for maximum PA output power on CC13xx).
//
// Note that production devices meant for 125 degrees C operation will have a
// VDDS_BOD level of 2.0V trimmed in e-fuse, which means that this field will have
// no function on these devices.
#define CCFG_MODE_CONF_VDDS_BOD_LEVEL                               0x01000000
#define CCFG_MODE_CONF_VDDS_BOD_LEVEL_BITN                          24
#define CCFG_MODE_CONF_VDDS_BOD_LEVEL_M                             0x01000000
#define CCFG_MODE_CONF_VDDS_BOD_LEVEL_S                             24

// Field: [23:22] SCLK_LF_OPTION
//
// 0: Low frequency clock derived from High Frequency XOSC
// 1: TBD: Digital input from AON (selects XOSC_LF as source and
// XOSC_LF_DIG_BYPASS=1)
// 2: Low frequency XOSC
// 3: Low frequency RCOSC (default)
#define CCFG_MODE_CONF_SCLK_LF_OPTION_M                             0x00C00000
#define CCFG_MODE_CONF_SCLK_LF_OPTION_S                             22

// Field: [20]    RTC_COMP
//
// 0: Apply RTC temperature compensation (may not be supported)
// 1: Don't apply temperature compensation to the RTC (default)
#define CCFG_MODE_CONF_RTC_COMP                                     0x00100000
#define CCFG_MODE_CONF_RTC_COMP_BITN                                20
#define CCFG_MODE_CONF_RTC_COMP_M                                   0x00100000
#define CCFG_MODE_CONF_RTC_COMP_S                                   20

// Field: [19:18] XOSC_FREQ
//
// 00: MEMS, reserved
// 01: BAW, reserved
// 10: 48MHz (may not be supported)
// 11: 24MHz (default)
#define CCFG_MODE_CONF_XOSC_FREQ_M                                  0x000C0000
#define CCFG_MODE_CONF_XOSC_FREQ_S                                  18

// Field: [17]    XOSC_CAP_MOD
//
// 0: Apply cap-array delta (may not be supported)
// 1: Don't apply cap-array delta (default)
//
#define CCFG_MODE_CONF_XOSC_CAP_MOD                                 0x00020000
#define CCFG_MODE_CONF_XOSC_CAP_MOD_BITN                            17
#define CCFG_MODE_CONF_XOSC_CAP_MOD_M                               0x00020000
#define CCFG_MODE_CONF_XOSC_CAP_MOD_S                               17

// Field: [16]    HF_COMP
//
// 0: Apply 48MHz compensation (may not be supported)
// 1: Don't apply 48MHz frequency compensation (default)
#define CCFG_MODE_CONF_HF_COMP                                      0x00010000
#define CCFG_MODE_CONF_HF_COMP_BITN                                 16
#define CCFG_MODE_CONF_HF_COMP_M                                    0x00010000
#define CCFG_MODE_CONF_HF_COMP_S                                    16

// Field: [15:8] XOSC_CAPARRAY_DELTA
//
// Signed 8-bit value, directly modifying trimmed XOSC cap-array value.
#define CCFG_MODE_CONF_XOSC_CAPARRAY_DELTA_M                        0x0000FF00
#define CCFG_MODE_CONF_XOSC_CAPARRAY_DELTA_S                        8

// Field: [7:0] VDDR_CAP
//
// Unsigned 8-bit integer, representing the minimum decoupling capacitance on VDDR,
// in units of 100nF. This should take into account capacitor tolerance and voltage
// dependent capacitance variation.
#define CCFG_MODE_CONF_VDDR_CAP_M                                   0x000000FF
#define CCFG_MODE_CONF_VDDR_CAP_S                                   0

//*****************************************************************************
//
// Register: CCFG_O_VOLT_LOAD_0
//
//*****************************************************************************
// Field: [31:24] VDDR_EXT_TP45
//
// Maximum external VDDR load in powerdown at +45C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TP45_M                            0xFF000000
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TP45_S                            24

// Field: [23:16] VDDR_EXT_TP25
//
// Maximum external VDDR load in powerdown at +25C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TP25_M                            0x00FF0000
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TP25_S                            16

// Field: [15:8] VDDR_EXT_TP5
//
// Maximum external VDDR load in powerdown at +5C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TP5_M                             0x0000FF00
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TP5_S                             8

// Field: [7:0] VDDR_EXT_TM15
//
// Maximum external VDDR load in powerdown at -15C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TM15_M                            0x000000FF
#define CCFG_VOLT_LOAD_0_VDDR_EXT_TM15_S                            0

//*****************************************************************************
//
// Register: CCFG_O_VOLT_LOAD_1
//
//*****************************************************************************
// Field: [31:24] VDDR_EXT_TP125
//
// Maximum external VDDR load in powerdown at +125C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP125_M                           0xFF000000
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP125_S                           24

// Field: [23:16] VDDR_EXT_TP105
//
// Maximum external VDDR load in powerdown at +105C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP105_M                           0x00FF0000
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP105_S                           16

// Field: [15:8] VDDR_EXT_TP85
//
// Maximum external VDDR load in powerdown at +85C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP85_M                            0x0000FF00
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP85_S                            8

// Field: [7:0] VDDR_EXT_TP65
//
// Maximum external VDDR load in powerdown at +65C, on a log2 scale with 4
// fractional bits.
// Current is defined as 10nA*2^N/16
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP65_M                            0x000000FF
#define CCFG_VOLT_LOAD_1_VDDR_EXT_TP65_S                            0

//*****************************************************************************
//
// Register: CCFG_O_RTC_OFFSET
//
//*****************************************************************************
// Field: [31:16] RTC_COMP_P0
//
// Signed 16-bit value, representing the RTC offset at 27 degrees C.
#define CCFG_RTC_OFFSET_RTC_COMP_P0_M                               0xFFFF0000
#define CCFG_RTC_OFFSET_RTC_COMP_P0_S                               16

// Field: [15:8] RTC_COMP_P1
//
// Signed 8-bit value, representing the first order RTC offset slope.
#define CCFG_RTC_OFFSET_RTC_COMP_P1_M                               0x0000FF00
#define CCFG_RTC_OFFSET_RTC_COMP_P1_S                               8

// Field: [7:0] RTC_COMP_P2
//
// Signed 8-bit value, representing the second order RTC offset slope. The actual
// RTC accumulation value would be defined as 1/32768*(1+d), where
// d = P0*2^(-20) + P1*(T-27)*2^(-21) + P2*(T-27)^2*2^(-27).
#define CCFG_RTC_OFFSET_RTC_COMP_P2_M                               0x000000FF
#define CCFG_RTC_OFFSET_RTC_COMP_P2_S                               0

//*****************************************************************************
//
// Register: CCFG_O_FREQ_OFFSET
//
//*****************************************************************************
// Field: [31:16] HF_COMP_P0
//
// Signed 16-bit value, representing the frequency offset at 27 degrees C.
#define CCFG_FREQ_OFFSET_HF_COMP_P0_M                               0xFFFF0000
#define CCFG_FREQ_OFFSET_HF_COMP_P0_S                               16

// Field: [15:8] HF_COMP_P1
//
// Signed 8-bit value, representing the first order frequency offset slope.
#define CCFG_FREQ_OFFSET_HF_COMP_P1_M                               0x0000FF00
#define CCFG_FREQ_OFFSET_HF_COMP_P1_S                               8

// Field: [7:0] HF_COMP_P2
//
// Signed 8-bit value, representing the second order frequency offset slope. The
// actual frequency of the clock would be defined as 48MHz*(1+d), where
// d = P0*2(-22) + P1*(T-27)*2^(-25) + P2*(T-27)^2*2^(-31).
#define CCFG_FREQ_OFFSET_HF_COMP_P2_M                               0x000000FF
#define CCFG_FREQ_OFFSET_HF_COMP_P2_S                               0

//*****************************************************************************
//
// Register: CCFG_O_IEEE_MAC_0
//
//*****************************************************************************
// Field: [31:0] ADDR
//
// Bits[31:0] of the 64-bits IEEE MAC address
#define CCFG_IEEE_MAC_0_ADDR_M                                      0xFFFFFFFF
#define CCFG_IEEE_MAC_0_ADDR_S                                      0

//*****************************************************************************
//
// Register: CCFG_O_IEEE_MAC_1
//
//*****************************************************************************
// Field: [31:0] ADDR
//
// Bits[63:32] of the 64-bits IEEE MAC address
#define CCFG_IEEE_MAC_1_ADDR_M                                      0xFFFFFFFF
#define CCFG_IEEE_MAC_1_ADDR_S                                      0

//*****************************************************************************
//
// Register: CCFG_O_IEEE_BLE_0
//
//*****************************************************************************
// Field: [31:0] ADDR
//
// Bits[31:0] of the 64-bits IEEE BLE address
#define CCFG_IEEE_BLE_0_ADDR_M                                      0xFFFFFFFF
#define CCFG_IEEE_BLE_0_ADDR_S                                      0

//*****************************************************************************
//
// Register: CCFG_O_IEEE_BLE_1
//
//*****************************************************************************
// Field: [31:0] ADDR
//
// Bits[63:32] of the 64-bits IEEE BLE address
#define CCFG_IEEE_BLE_1_ADDR_M                                      0xFFFFFFFF
#define CCFG_IEEE_BLE_1_ADDR_S                                      0

//*****************************************************************************
//
// Register: CCFG_O_BL_CONFIG
//
//*****************************************************************************
// Field: [31:24] BOOTLOADER_ENABLE
//
// 0xC5: Boot loader is enabled
// Any other value: Boot loader is disabled
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
//
#define CCFG_BL_CONFIG_BOOTLOADER_ENABLE_M                          0xFF000000
#define CCFG_BL_CONFIG_BOOTLOADER_ENABLE_S                          24

// Field: [16]    BL_LEVEL
//
// BL_LEVEL - sets the active level of the selected pin.
//
// 0: Active low.
// 1: Active high.
#define CCFG_BL_CONFIG_BL_LEVEL                                     0x00010000
#define CCFG_BL_CONFIG_BL_LEVEL_BITN                                16
#define CCFG_BL_CONFIG_BL_LEVEL_M                                   0x00010000
#define CCFG_BL_CONFIG_BL_LEVEL_S                                   16

// Field: [15:8] BL_PIN_NUMBER
//
// BL_PIN_NUMBER - is the number of the I/O-pin that is level checked if the boot
// loader failure analysis is enabled.
#define CCFG_BL_CONFIG_BL_PIN_NUMBER_M                              0x0000FF00
#define CCFG_BL_CONFIG_BL_PIN_NUMBER_S                              8

// Field: [7:0] BL_ENABLE
//
// BL_ENABLE - enables or disables the boot loader failure analysis..
//
// 0xC5: Failure analysis enabled.
// Any other value: Failure analysis disabled.
#define CCFG_BL_CONFIG_BL_ENABLE_M                                  0x000000FF
#define CCFG_BL_CONFIG_BL_ENABLE_S                                  0

//*****************************************************************************
//
// Register: CCFG_O_ERASE_CONF
//
//*****************************************************************************
// Field: [8]    CHIP_ERASE_DIS_N
//
// 0: Any chip erase request detected during boot will be ignored. The boot FW will
// clear AON_WUC:CTL1.CHIP_ERASE and proceed with normal boot sequence.
// 1: Any chip erase request detected during boot will be performed by the boot
// FW.
//
#define CCFG_ERASE_CONF_CHIP_ERASE_DIS_N                            0x00000100
#define CCFG_ERASE_CONF_CHIP_ERASE_DIS_N_BITN                       8
#define CCFG_ERASE_CONF_CHIP_ERASE_DIS_N_M                          0x00000100
#define CCFG_ERASE_CONF_CHIP_ERASE_DIS_N_S                          8

// Field: [0]    BANK_ERASE_DIS_N
//
// Setting this bit to 0 will disable the boot loader bank erase function.
// This bit will be tested by the ROM boot loader in order to verify if a
// received Bank Erase boot loader command can be executed or not.
// Bank erase is also referred to as mass erase.
//
#define CCFG_ERASE_CONF_BANK_ERASE_DIS_N                            0x00000001
#define CCFG_ERASE_CONF_BANK_ERASE_DIS_N_BITN                       0
#define CCFG_ERASE_CONF_BANK_ERASE_DIS_N_M                          0x00000001
#define CCFG_ERASE_CONF_BANK_ERASE_DIS_N_S                          0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_TI_OPTIONS
//
//*****************************************************************************
// Field: [7:0] TI_FA_ENABLE
//
// Must have a value of 0xC5 in order to enable the functionality of unlocking the
// TI FA (TI Failure Analysis) option with the unlock code.
// All other values will disable the functionality of unlocking the TI FA option
// with the unlock code.
// Unlock is configured by ROM startup code while in safezone dependent on
// TI_FA_ENABLE.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
//
#define CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE_M                         0x000000FF
#define CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE_S                         0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_TAP_DAP_0
//
//*****************************************************************************
// Field: [23:16] CPU_DAP_ENABLE
//
// 0xC5: AON_WUC:JTAGCFG.CPU_DAP will be set to 1 if
// FCFG1:FCFG1_TAP_DAP_0.CPU_DAP_ENABLE also have a value of 0xC5. Bit field is
// written by boot FW while in safezone.
//
// Any other value: AON_WUC:JTAGCFG.CPU_DAP will be set to 0 by boot FW while
// in safezone.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
#define CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE_M                        0x00FF0000
#define CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE_S                        16

// Field: [15:8] PRCM_TAP_ENABLE
//
// 0xC5: AON_WUC:JTAGCFG.PRCM_TAP will be set to 1 if
// FCFG1:FCFG1_TAP_DAP_0.PRCM_TAP_ENABLE also have a value of 0xC5. Bit field is
// written by boot FW while in safezone.
//
// Any other value: AON_WUC:JTAGCFG.PRCM_TAP will be set to 0 by boot FW while
// in safezone.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
#define CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE_M                       0x0000FF00
#define CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE_S                       8

// Field: [7:0] TEST_TAP_ENABLE
//
// 0xC5: AON_WUC:JTAGCFG.TEST_TAP will be set to 1 if
// FCFG1:FCFG1_TAP_DAP_0.TEST_TAP_ENABLE also have a value of 0xC5. Bit field is
// written by boot FW while in safezone.
//
// Any other value: AON_WUC:JTAGCFG.TEST_TAP will be set to 0 by boot FW while
// in safezone.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
#define CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE_M                       0x000000FF
#define CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE_S                       0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_TAP_DAP_1
//
//*****************************************************************************
// Field: [23:16] PBIST2_TAP_ENABLE
//
// 0xC5: AON_WUC:JTAGCFG.PBIST2_TAP will be set to 1 if
// FCFG1:FCFG1_TAP_DAP_1.PBIST2_TAP_ENABLE also have a value of 0xC5. Bit field is
// written by boot FW while in safezone.
//
// Any other value: AON_WUC:JTAGCFG.PBIST2_TAP will be set to 0 by boot FW
// while in safezone.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
#define CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE_M                     0x00FF0000
#define CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE_S                     16

// Field: [15:8] PBIST1_TAP_ENABLE
//
// 0xC5: AON_WUC:JTAGCFG.PBIST1_TAP will be set to 1 if
// FCFG1:FCFG1_TAP_DAP_1.PBIST1_TAP_ENABLE also have a value of 0xC5. Bit field is
// written by boot FW while in safezone.
//
// Any other value: AON_WUC:JTAGCFG.PBIST1_TAP will be set to 0 by boot FW
// while in safezone.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
#define CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE_M                     0x0000FF00
#define CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE_S                     8

// Field: [7:0] WUC_TAP_ENABLE
//
// 0xC5: AON_WUC:JTAGCFG.WUC_TAP will be set to 1 if
// FCFG1:FCFG1_TAP_DAP_1.WUC_TAP_ENABLE also have a value of 0xC5. Bit field is
// written by boot FW while in safezone.
//
// Any other value: AON_WUC:JTAGCFG.WUC_TAP will be set to 0 by boot FW while
// in safezone.
//
// NOTE! This parameter must be written to the value of 0xC5 by production test.
#define CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE_M                        0x000000FF
#define CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE_S                        0

//*****************************************************************************
//
// Register: CCFG_O_IMAGE_VALID_CONF
//
//*****************************************************************************
// Field: [31:0] IMAGE_VALID
//
// IMAGE_VALID:
// This field must have a value of 0x00000000 in order for enabling the boot
// sequence to transfer control to a flash image.
//
#define CCFG_IMAGE_VALID_CONF_IMAGE_VALID_M                         0xFFFFFFFF
#define CCFG_IMAGE_VALID_CONF_IMAGE_VALID_S                         0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_PROT_31_0
//
//*****************************************************************************
// Field: [31]    WRT_PROT_SEC_31
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_31                         0x80000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_31_BITN                    31
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_31_M                       0x80000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_31_S                       31

// Field: [30]    WRT_PROT_SEC_30
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_30                         0x40000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_30_BITN                    30
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_30_M                       0x40000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_30_S                       30

// Field: [29]    WRT_PROT_SEC_29
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_29                         0x20000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_29_BITN                    29
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_29_M                       0x20000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_29_S                       29

// Field: [28]    WRT_PROT_SEC_28
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_28                         0x10000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_28_BITN                    28
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_28_M                       0x10000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_28_S                       28

// Field: [27]    WRT_PROT_SEC_27
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_27                         0x08000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_27_BITN                    27
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_27_M                       0x08000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_27_S                       27

// Field: [26]    WRT_PROT_SEC_26
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_26                         0x04000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_26_BITN                    26
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_26_M                       0x04000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_26_S                       26

// Field: [25]    WRT_PROT_SEC_25
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_25                         0x02000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_25_BITN                    25
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_25_M                       0x02000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_25_S                       25

// Field: [24]    WRT_PROT_SEC_24
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_24                         0x01000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_24_BITN                    24
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_24_M                       0x01000000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_24_S                       24

// Field: [23]    WRT_PROT_SEC_23
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_23                         0x00800000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_23_BITN                    23
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_23_M                       0x00800000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_23_S                       23

// Field: [22]    WRT_PROT_SEC_22
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_22                         0x00400000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_22_BITN                    22
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_22_M                       0x00400000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_22_S                       22

// Field: [21]    WRT_PROT_SEC_21
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_21                         0x00200000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_21_BITN                    21
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_21_M                       0x00200000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_21_S                       21

// Field: [20]    WRT_PROT_SEC_20
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_20                         0x00100000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_20_BITN                    20
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_20_M                       0x00100000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_20_S                       20

// Field: [19]    WRT_PROT_SEC_19
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_19                         0x00080000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_19_BITN                    19
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_19_M                       0x00080000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_19_S                       19

// Field: [18]    WRT_PROT_SEC_18
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_18                         0x00040000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_18_BITN                    18
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_18_M                       0x00040000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_18_S                       18

// Field: [17]    WRT_PROT_SEC_17
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_17                         0x00020000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_17_BITN                    17
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_17_M                       0x00020000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_17_S                       17

// Field: [16]    WRT_PROT_SEC_16
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_16                         0x00010000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_16_BITN                    16
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_16_M                       0x00010000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_16_S                       16

// Field: [15]    WRT_PROT_SEC_15
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_15                         0x00008000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_15_BITN                    15
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_15_M                       0x00008000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_15_S                       15

// Field: [14]    WRT_PROT_SEC_14
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_14                         0x00004000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_14_BITN                    14
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_14_M                       0x00004000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_14_S                       14

// Field: [13]    WRT_PROT_SEC_13
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_13                         0x00002000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_13_BITN                    13
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_13_M                       0x00002000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_13_S                       13

// Field: [12]    WRT_PROT_SEC_12
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_12                         0x00001000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_12_BITN                    12
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_12_M                       0x00001000
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_12_S                       12

// Field: [11]    WRT_PROT_SEC_11
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_11                         0x00000800
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_11_BITN                    11
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_11_M                       0x00000800
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_11_S                       11

// Field: [10]    WRT_PROT_SEC_10
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_10                         0x00000400
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_10_BITN                    10
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_10_M                       0x00000400
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_10_S                       10

// Field: [9]    WRT_PROT_SEC_9
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_9                          0x00000200
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_9_BITN                     9
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_9_M                        0x00000200
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_9_S                        9

// Field: [8]    WRT_PROT_SEC_8
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_8                          0x00000100
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_8_BITN                     8
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_8_M                        0x00000100
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_8_S                        8

// Field: [7]    WRT_PROT_SEC_7
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_7                          0x00000080
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_7_BITN                     7
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_7_M                        0x00000080
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_7_S                        7

// Field: [6]    WRT_PROT_SEC_6
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_6                          0x00000040
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_6_BITN                     6
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_6_M                        0x00000040
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_6_S                        6

// Field: [5]    WRT_PROT_SEC_5
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_5                          0x00000020
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_5_BITN                     5
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_5_M                        0x00000020
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_5_S                        5

// Field: [4]    WRT_PROT_SEC_4
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_4                          0x00000010
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_4_BITN                     4
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_4_M                        0x00000010
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_4_S                        4

// Field: [3]    WRT_PROT_SEC_3
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_3                          0x00000008
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_3_BITN                     3
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_3_M                        0x00000008
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_3_S                        3

// Field: [2]    WRT_PROT_SEC_2
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_2                          0x00000004
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_2_BITN                     2
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_2_M                        0x00000004
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_2_S                        2

// Field: [1]    WRT_PROT_SEC_1
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_1                          0x00000002
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_1_BITN                     1
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_1_M                        0x00000002
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_1_S                        1

// Field: [0]    WRT_PROT_SEC_0
//
// 0: Sector protected
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_0                          0x00000001
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_0_BITN                     0
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_0_M                        0x00000001
#define CCFG_CCFG_PROT_31_0_WRT_PROT_SEC_0_S                        0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_PROT_63_32
//
//*****************************************************************************
// Field: [31]    WRT_PROT_SEC_63
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_63                        0x80000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_63_BITN                   31
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_63_M                      0x80000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_63_S                      31

// Field: [30]    WRT_PROT_SEC_62
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_62                        0x40000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_62_BITN                   30
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_62_M                      0x40000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_62_S                      30

// Field: [29]    WRT_PROT_SEC_61
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_61                        0x20000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_61_BITN                   29
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_61_M                      0x20000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_61_S                      29

// Field: [28]    WRT_PROT_SEC_60
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_60                        0x10000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_60_BITN                   28
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_60_M                      0x10000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_60_S                      28

// Field: [27]    WRT_PROT_SEC_59
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_59                        0x08000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_59_BITN                   27
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_59_M                      0x08000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_59_S                      27

// Field: [26]    WRT_PROT_SEC_58
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_58                        0x04000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_58_BITN                   26
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_58_M                      0x04000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_58_S                      26

// Field: [25]    WRT_PROT_SEC_57
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_57                        0x02000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_57_BITN                   25
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_57_M                      0x02000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_57_S                      25

// Field: [24]    WRT_PROT_SEC_56
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_56                        0x01000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_56_BITN                   24
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_56_M                      0x01000000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_56_S                      24

// Field: [23]    WRT_PROT_SEC_55
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_55                        0x00800000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_55_BITN                   23
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_55_M                      0x00800000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_55_S                      23

// Field: [22]    WRT_PROT_SEC_54
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_54                        0x00400000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_54_BITN                   22
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_54_M                      0x00400000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_54_S                      22

// Field: [21]    WRT_PROT_SEC_53
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_53                        0x00200000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_53_BITN                   21
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_53_M                      0x00200000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_53_S                      21

// Field: [20]    WRT_PROT_SEC_52
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_52                        0x00100000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_52_BITN                   20
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_52_M                      0x00100000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_52_S                      20

// Field: [19]    WRT_PROT_SEC_51
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_51                        0x00080000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_51_BITN                   19
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_51_M                      0x00080000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_51_S                      19

// Field: [18]    WRT_PROT_SEC_50
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_50                        0x00040000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_50_BITN                   18
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_50_M                      0x00040000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_50_S                      18

// Field: [17]    WRT_PROT_SEC_49
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_49                        0x00020000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_49_BITN                   17
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_49_M                      0x00020000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_49_S                      17

// Field: [16]    WRT_PROT_SEC_48
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_48                        0x00010000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_48_BITN                   16
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_48_M                      0x00010000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_48_S                      16

// Field: [15]    WRT_PROT_SEC_47
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_47                        0x00008000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_47_BITN                   15
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_47_M                      0x00008000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_47_S                      15

// Field: [14]    WRT_PROT_SEC_46
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_46                        0x00004000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_46_BITN                   14
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_46_M                      0x00004000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_46_S                      14

// Field: [13]    WRT_PROT_SEC_45
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_45                        0x00002000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_45_BITN                   13
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_45_M                      0x00002000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_45_S                      13

// Field: [12]    WRT_PROT_SEC_44
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_44                        0x00001000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_44_BITN                   12
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_44_M                      0x00001000
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_44_S                      12

// Field: [11]    WRT_PROT_SEC_43
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_43                        0x00000800
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_43_BITN                   11
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_43_M                      0x00000800
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_43_S                      11

// Field: [10]    WRT_PROT_SEC_42
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_42                        0x00000400
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_42_BITN                   10
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_42_M                      0x00000400
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_42_S                      10

// Field: [9]    WRT_PROT_SEC_41
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_41                        0x00000200
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_41_BITN                   9
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_41_M                      0x00000200
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_41_S                      9

// Field: [8]    WRT_PROT_SEC_40
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_40                        0x00000100
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_40_BITN                   8
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_40_M                      0x00000100
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_40_S                      8

// Field: [7]    WRT_PROT_SEC_39
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_39                        0x00000080
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_39_BITN                   7
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_39_M                      0x00000080
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_39_S                      7

// Field: [6]    WRT_PROT_SEC_38
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_38                        0x00000040
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_38_BITN                   6
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_38_M                      0x00000040
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_38_S                      6

// Field: [5]    WRT_PROT_SEC_37
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_37                        0x00000020
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_37_BITN                   5
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_37_M                      0x00000020
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_37_S                      5

// Field: [4]    WRT_PROT_SEC_36
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_36                        0x00000010
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_36_BITN                   4
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_36_M                      0x00000010
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_36_S                      4

// Field: [3]    WRT_PROT_SEC_35
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_35                        0x00000008
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_35_BITN                   3
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_35_M                      0x00000008
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_35_S                      3

// Field: [2]    WRT_PROT_SEC_34
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_34                        0x00000004
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_34_BITN                   2
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_34_M                      0x00000004
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_34_S                      2

// Field: [1]    WRT_PROT_SEC_33
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_33                        0x00000002
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_33_BITN                   1
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_33_M                      0x00000002
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_33_S                      1

// Field: [0]    WRT_PROT_SEC_32
//
// 0: Sector protected
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_32                        0x00000001
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_32_BITN                   0
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_32_M                      0x00000001
#define CCFG_CCFG_PROT_63_32_WRT_PROT_SEC_32_S                      0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_PROT_95_64
//
//*****************************************************************************
// Field: [31]    WRT_PROT_SEC_95
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_95                        0x80000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_95_BITN                   31
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_95_M                      0x80000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_95_S                      31

// Field: [30]    WRT_PROT_SEC_94
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_94                        0x40000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_94_BITN                   30
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_94_M                      0x40000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_94_S                      30

// Field: [29]    WRT_PROT_SEC_93
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_93                        0x20000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_93_BITN                   29
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_93_M                      0x20000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_93_S                      29

// Field: [28]    WRT_PROT_SEC_92
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_92                        0x10000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_92_BITN                   28
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_92_M                      0x10000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_92_S                      28

// Field: [27]    WRT_PROT_SEC_91
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_91                        0x08000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_91_BITN                   27
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_91_M                      0x08000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_91_S                      27

// Field: [26]    WRT_PROT_SEC_90
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_90                        0x04000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_90_BITN                   26
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_90_M                      0x04000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_90_S                      26

// Field: [25]    WRT_PROT_SEC_89
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_89                        0x02000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_89_BITN                   25
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_89_M                      0x02000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_89_S                      25

// Field: [24]    WRT_PROT_SEC_88
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_88                        0x01000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_88_BITN                   24
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_88_M                      0x01000000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_88_S                      24

// Field: [23]    WRT_PROT_SEC_87
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_87                        0x00800000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_87_BITN                   23
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_87_M                      0x00800000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_87_S                      23

// Field: [22]    WRT_PROT_SEC_86
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_86                        0x00400000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_86_BITN                   22
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_86_M                      0x00400000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_86_S                      22

// Field: [21]    WRT_PROT_SEC_85
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_85                        0x00200000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_85_BITN                   21
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_85_M                      0x00200000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_85_S                      21

// Field: [20]    WRT_PROT_SEC_84
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_84                        0x00100000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_84_BITN                   20
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_84_M                      0x00100000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_84_S                      20

// Field: [19]    WRT_PROT_SEC_83
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_83                        0x00080000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_83_BITN                   19
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_83_M                      0x00080000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_83_S                      19

// Field: [18]    WRT_PROT_SEC_82
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_82                        0x00040000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_82_BITN                   18
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_82_M                      0x00040000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_82_S                      18

// Field: [17]    WRT_PROT_SEC_81
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_81                        0x00020000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_81_BITN                   17
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_81_M                      0x00020000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_81_S                      17

// Field: [16]    WRT_PROT_SEC_80
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_80                        0x00010000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_80_BITN                   16
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_80_M                      0x00010000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_80_S                      16

// Field: [15]    WRT_PROT_SEC_79
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_79                        0x00008000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_79_BITN                   15
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_79_M                      0x00008000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_79_S                      15

// Field: [14]    WRT_PROT_SEC_78
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_78                        0x00004000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_78_BITN                   14
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_78_M                      0x00004000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_78_S                      14

// Field: [13]    WRT_PROT_SEC_77
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_77                        0x00002000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_77_BITN                   13
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_77_M                      0x00002000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_77_S                      13

// Field: [12]    WRT_PROT_SEC_76
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_76                        0x00001000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_76_BITN                   12
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_76_M                      0x00001000
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_76_S                      12

// Field: [11]    WRT_PROT_SEC_75
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_75                        0x00000800
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_75_BITN                   11
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_75_M                      0x00000800
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_75_S                      11

// Field: [10]    WRT_PROT_SEC_74
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_74                        0x00000400
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_74_BITN                   10
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_74_M                      0x00000400
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_74_S                      10

// Field: [9]    WRT_PROT_SEC_73
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_73                        0x00000200
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_73_BITN                   9
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_73_M                      0x00000200
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_73_S                      9

// Field: [8]    WRT_PROT_SEC_72
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_72                        0x00000100
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_72_BITN                   8
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_72_M                      0x00000100
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_72_S                      8

// Field: [7]    WRT_PROT_SEC_71
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_71                        0x00000080
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_71_BITN                   7
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_71_M                      0x00000080
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_71_S                      7

// Field: [6]    WRT_PROT_SEC_70
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_70                        0x00000040
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_70_BITN                   6
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_70_M                      0x00000040
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_70_S                      6

// Field: [5]    WRT_PROT_SEC_69
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_69                        0x00000020
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_69_BITN                   5
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_69_M                      0x00000020
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_69_S                      5

// Field: [4]    WRT_PROT_SEC_68
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_68                        0x00000010
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_68_BITN                   4
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_68_M                      0x00000010
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_68_S                      4

// Field: [3]    WRT_PROT_SEC_67
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_67                        0x00000008
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_67_BITN                   3
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_67_M                      0x00000008
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_67_S                      3

// Field: [2]    WRT_PROT_SEC_66
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_66                        0x00000004
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_66_BITN                   2
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_66_M                      0x00000004
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_66_S                      2

// Field: [1]    WRT_PROT_SEC_65
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_65                        0x00000002
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_65_BITN                   1
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_65_M                      0x00000002
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_65_S                      1

// Field: [0]    WRT_PROT_SEC_64
//
// 0: Sector protected
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_64                        0x00000001
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_64_BITN                   0
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_64_M                      0x00000001
#define CCFG_CCFG_PROT_95_64_WRT_PROT_SEC_64_S                      0

//*****************************************************************************
//
// Register: CCFG_O_CCFG_PROT_127_96
//
//*****************************************************************************
// Field: [31]    WRT_PROT_SEC_127
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_127                      0x80000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_127_BITN                 31
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_127_M                    0x80000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_127_S                    31

// Field: [30]    WRT_PROT_SEC_126
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_126                      0x40000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_126_BITN                 30
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_126_M                    0x40000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_126_S                    30

// Field: [29]    WRT_PROT_SEC_125
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_125                      0x20000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_125_BITN                 29
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_125_M                    0x20000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_125_S                    29

// Field: [28]    WRT_PROT_SEC_124
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_124                      0x10000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_124_BITN                 28
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_124_M                    0x10000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_124_S                    28

// Field: [27]    WRT_PROT_SEC_123
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_123                      0x08000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_123_BITN                 27
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_123_M                    0x08000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_123_S                    27

// Field: [26]    WRT_PROT_SEC_122
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_122                      0x04000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_122_BITN                 26
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_122_M                    0x04000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_122_S                    26

// Field: [25]    WRT_PROT_SEC_121
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_121                      0x02000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_121_BITN                 25
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_121_M                    0x02000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_121_S                    25

// Field: [24]    WRT_PROT_SEC_120
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_120                      0x01000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_120_BITN                 24
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_120_M                    0x01000000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_120_S                    24

// Field: [23]    WRT_PROT_SEC_119
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_119                      0x00800000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_119_BITN                 23
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_119_M                    0x00800000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_119_S                    23

// Field: [22]    WRT_PROT_SEC_118
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_118                      0x00400000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_118_BITN                 22
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_118_M                    0x00400000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_118_S                    22

// Field: [21]    WRT_PROT_SEC_117
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_117                      0x00200000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_117_BITN                 21
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_117_M                    0x00200000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_117_S                    21

// Field: [20]    WRT_PROT_SEC_116
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_116                      0x00100000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_116_BITN                 20
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_116_M                    0x00100000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_116_S                    20

// Field: [19]    WRT_PROT_SEC_115
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_115                      0x00080000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_115_BITN                 19
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_115_M                    0x00080000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_115_S                    19

// Field: [18]    WRT_PROT_SEC_114
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_114                      0x00040000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_114_BITN                 18
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_114_M                    0x00040000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_114_S                    18

// Field: [17]    WRT_PROT_SEC_113
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_113                      0x00020000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_113_BITN                 17
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_113_M                    0x00020000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_113_S                    17

// Field: [16]    WRT_PROT_SEC_112
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_112                      0x00010000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_112_BITN                 16
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_112_M                    0x00010000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_112_S                    16

// Field: [15]    WRT_PROT_SEC_111
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_111                      0x00008000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_111_BITN                 15
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_111_M                    0x00008000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_111_S                    15

// Field: [14]    WRT_PROT_SEC_110
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_110                      0x00004000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_110_BITN                 14
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_110_M                    0x00004000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_110_S                    14

// Field: [13]    WRT_PROT_SEC_109
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_109                      0x00002000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_109_BITN                 13
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_109_M                    0x00002000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_109_S                    13

// Field: [12]    WRT_PROT_SEC_108
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_108                      0x00001000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_108_BITN                 12
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_108_M                    0x00001000
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_108_S                    12

// Field: [11]    WRT_PROT_SEC_107
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_107                      0x00000800
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_107_BITN                 11
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_107_M                    0x00000800
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_107_S                    11

// Field: [10]    WRT_PROT_SEC_106
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_106                      0x00000400
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_106_BITN                 10
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_106_M                    0x00000400
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_106_S                    10

// Field: [9]    WRT_PROT_SEC_105
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_105                      0x00000200
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_105_BITN                 9
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_105_M                    0x00000200
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_105_S                    9

// Field: [8]    WRT_PROT_SEC_104
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_104                      0x00000100
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_104_BITN                 8
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_104_M                    0x00000100
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_104_S                    8

// Field: [7]    WRT_PROT_SEC_103
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_103                      0x00000080
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_103_BITN                 7
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_103_M                    0x00000080
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_103_S                    7

// Field: [6]    WRT_PROT_SEC_102
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_102                      0x00000040
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_102_BITN                 6
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_102_M                    0x00000040
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_102_S                    6

// Field: [5]    WRT_PROT_SEC_101
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_101                      0x00000020
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_101_BITN                 5
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_101_M                    0x00000020
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_101_S                    5

// Field: [4]    WRT_PROT_SEC_100
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_100                      0x00000010
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_100_BITN                 4
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_100_M                    0x00000010
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_100_S                    4

// Field: [3]    WRT_PROT_SEC_99
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_99                       0x00000008
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_99_BITN                  3
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_99_M                     0x00000008
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_99_S                     3

// Field: [2]    WRT_PROT_SEC_98
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_98                       0x00000004
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_98_BITN                  2
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_98_M                     0x00000004
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_98_S                     2

// Field: [1]    WRT_PROT_SEC_97
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_97                       0x00000002
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_97_BITN                  1
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_97_M                     0x00000002
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_97_S                     1

// Field: [0]    WRT_PROT_SEC_96
//
// 0: Sector protected
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_96                       0x00000001
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_96_BITN                  0
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_96_M                     0x00000001
#define CCFG_CCFG_PROT_127_96_WRT_PROT_SEC_96_S                     0

#endif // __HW_CCFG_H__
