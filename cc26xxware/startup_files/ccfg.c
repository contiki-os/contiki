/******************************************************************************
*  Filename:       ccfg.c
*  Revised:        $Date: 2015-01-15 15:45:13 +0100 (to, 15 jan 2015) $
*  Revision:       $Revision: 14826 $
*
*  Description:    Customer Configuration CC26xx PG2 device family.
*
*  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#include <stdint.h>
#include <inc/hw_types.h>
#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>

//*****************************************************************************
//
// Set the values of the individual bit fields.
//
//*****************************************************************************

//**************************************************
// Minimum voltage to use for alternative DCDC setting
//
// (28 + ALT_DCDC_VMIN)/16:
// 0: 1.75V 
// 1: 1.8125V 
// ...
// 14: 2.625V 
// 15: 2.6875V 
//**************************************************
#define SET_CCFG_MODE_CONF_1_ALT_DCDC_VMIN              0x8

//**************************************************
// Enable DC/DC dithering for alternative DCDC setting
//**************************************************
#define SET_CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN         0x0        // Disable
//#define SET_CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN       0x1        // Enable

//**************************************************
// Inductor peak current for alternative DCDC setting
// 0: Min 31mA 
// 4: Typical 47mA 
// 7: Max 59mA 
//**************************************************
#define SET_CCFG_MODE_CONF_1_ALT_DCDC_IPEAK             0x2

//**************************************************
// Signed delta value for IBIAS_INIT
//**************************************************
#define SET_CCFG_MODE_CONF_1_DELTA_IBIAS_INIT           0x0

//**************************************************
// Signed delta value for IBIAS_OFFSET
//**************************************************
#define SET_CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET         0x0

//**************************************************
// Unsigned value of maximum XOSC startup time in units of 100us
//**************************************************
#define SET_CCFG_MODE_CONF_1_XOSC_MAX_START             0x10

//**************************************************
// Disable flag for alternative DCDC setting
//**************************************************
#define SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING 0x0       // Alternate DCDC setting enabled
//#define SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING 0x1     // Alternate DCDC setting disabled

//**************************************************
// Disable flag for XOSC override functionality
//**************************************************
//#define SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR      0x0        // Enable override
#define SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR        0x1        // Disable override

//**************************************************
// Select DC/DC during recharge
//**************************************************
#define SET_CCFG_MODE_CONF_DCDC_RECHARGE                0x0        // Use the DC/DC during recharge in powerdown
// #define SET_CCFG_MODE_CONF_DCDC_RECHARGE             0x1        // Do not use the DC/DC during recharge in powerdown

//**************************************************
// Select DC/DC during active mode
//**************************************************
#define SET_CCFG_MODE_CONF_DCDC_ACTIVE                  0x0        // Use the DC/DC during active mode
// #define SET_CCFG_MODE_CONF_DCDC_ACTIVE               0x1        // Do not use the DC/DC during active mode

//**************************************************
// VDDR external load
//**************************************************
// #define SET_CCFG_MODE_CONF_VDDR_EXT_LOAD             0x0        // VDDR may be loaded externally (may not be supported)
#define SET_CCFG_MODE_CONF_VDDR_EXT_LOAD                0x1        // VDDR is not loaded externally

//**************************************************
// VDDS BOD level
// 
// Note that production devices meant for 
// 125 degrees C operation will have a VDDS_BOD 
// level of 2.0V trimmed in e-fuse, which means 
// that VDDS_BOD_LEVEL in CCFG will have no 
// function on these devices.
//**************************************************
// #define SET_CCFG_MODE_CONF_VDDS_BOD_LEVEL            0x0        // VDDS BOD level is 2.0V (necessary for 125 deg C, ext load mode, or for maximum PA output power on CC13xx)
#define SET_CCFG_MODE_CONF_VDDS_BOD_LEVEL               0x1        // VDDS BOD level is 1.8V (or 1.65V for external regulator mode)

//**************************************************
// SCLK LF option
//**************************************************
// #define SET_CCFG_MODE_CONF_SCLK_LF_OPTION            0x0        // Low frequency clock derived from High Frequency XOSC
// #define SET_CCFG_MODE_CONF_SCLK_LF_OPTION            0x1        // TBD: Digital input from AON (selects XOSC_LF as source and XOSC_LF_DIG_BYPASS=1)
#define SET_CCFG_MODE_CONF_SCLK_LF_OPTION               0x2        // Low frequency XOSC
// #define SET_CCFG_MODE_CONF_SCLK_LF_OPTION            0x3        // Low frequency RCOSC

//**************************************************
// RTC compensation
//**************************************************
// #define SET_CCFG_MODE_CONF_RTC_COMP                  0x0        // Apply RTC temperature compensation (may not be supported)
#define SET_CCFG_MODE_CONF_RTC_COMP                     0x1        // Don't apply temperature compensation to the RTC

//**************************************************
// XOSC frequency
//**************************************************
// #define SET_CCFG_MODE_CONF_XOSC_FREQ                 0x0        // MEMS, reserved
// #define SET_CCFG_MODE_CONF_XOSC_FREQ                 0x1        // BAW, reserved
// #define SET_CCFG_MODE_CONF_XOSC_FREQ                 0x2        // 48MHz (may not be supported)
#define SET_CCFG_MODE_CONF_XOSC_FREQ                    0x3        // 24MHz

//**************************************************
// Enable XOSC cap-array delta
//**************************************************
// #define SET_CCFG_MODE_CONF_XOSC_CAP_MOD              0x0        // Apply cap-array delta (may not be supported) 
#define SET_CCFG_MODE_CONF_XOSC_CAP_MOD                 0x1        // Don't apply cap-array delta 

//**************************************************
// HF compensation
//**************************************************
// #define SET_CCFG_MODE_CONF_HF_COMP                   0x0        // Apply 48MHz compensation (may not be supported)
#define SET_CCFG_MODE_CONF_HF_COMP                      0x1        // Don't apply 48MHz frequency compensation 

//**************************************************
// Value of XOSC cap-array delta
//**************************************************
#define SET_CCFG_MODE_CONF_XOSC_CAPARRAY_DELTA          0xFF       // Signed 8-bit value, directly modifying trimmed XOSC cap-array value

//**************************************************
// Value of VDDR capacitance
//
// This should take into account capacitor tolerance 
// and voltage dependent capacitance variation.
//**************************************************
#define SET_CCFG_MODE_CONF_VDDR_CAP                     0x3A       // Unsigned 8-bit integer representing the min. decoupling capacitance on VDDR in units of 100nF

//**************************************************
// TI FA (Failure Analysis)
//**************************************************
// #define SET_CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE        0x00       // Disable unlocking of TI FA option.
#define SET_CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE           0xC5       // Enable unlocking of TI FA option with the unlock code

//**************************************************
// CPU DAP access control
//**************************************************
// #define SET_CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE       0x00       // Access disabled
#define SET_CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE          0xC5       // Access enabled if also enabled in FCFG

//**************************************************
// PRCM TAP access control
//**************************************************
// #define SET_CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE      0x00       // Access disabled
#define SET_CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE         0xC5       // Access enabled if also enabled in FCFG

//**************************************************
// TEST TAP access control
//**************************************************
// #define SET_CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE      0x00       // Access disabled
#define SET_CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE         0xC5       // Access enabled if also enabled in FCFG

//**************************************************
// PBIST2 TAP access control
//**************************************************
// #define SET_CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE    0x00       // Access disabled
#define SET_CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE       0xC5       // Access enabled if also enabled in FCFG

//**************************************************
// PBIST1 TAP access control
//**************************************************
// #define SET_CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE    0x00       // Access disabled
#define SET_CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE       0xC5       // Access enabled if also enabled in FCFG

//**************************************************
// WUC TAP access control
//**************************************************
// #define SET_CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE       0x00       // Access disabled
#define SET_CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE          0xC5       // Access enabled if also enabled in FCFG

//**************************************************
// Bootloader enable
//**************************************************
// #define SET_CCFG_BL_CONFIG_BOOTLOADER_ENABLE         0x00       // Disable
#define SET_CCFG_BL_CONFIG_BOOTLOADER_ENABLE            0xC5       // Enable

//**************************************************
// Bootloader pin level configuration
//**************************************************
// #define SET_CCFG_BL_CONFIG_BL_LEVEL                  0x0        // Active low
#define SET_CCFG_BL_CONFIG_BL_LEVEL                     0x1        // Active high

//**************************************************
// Bootloader failure analysis pin number
//**************************************************
#define SET_CCFG_BL_CONFIG_BL_PIN_NUMBER                0xFF

//**************************************************
// Bootloader failure analysis
//**************************************************
// #define SET_CCFG_BL_CONFIG_BL_ENABLE                 0xC5       // Enabled
#define SET_CCFG_BL_CONFIG_BL_ENABLE                    0xFF       // Disabled

//**************************************************
// Chip erase
//**************************************************
// #define SET_CCFG_ERASE_CONF_CHIP_ERASE_DIS_N         0x0        // Any chip erase request detected during boot will be ignored
#define SET_CCFG_ERASE_CONF_CHIP_ERASE_DIS_N            0x1        // Any chip erase request detected during boot will be performed by the boot FW

//**************************************************
// Bank erase (aka Mass erase)
//**************************************************
// #define SET_CCFG_ERASE_CONF_BANK_ERASE_DIS_N         0x0        // Disable the boot loader bank erase function
#define SET_CCFG_ERASE_CONF_BANK_ERASE_DIS_N            0x1        // Enable the boot loader bank erase function

//**************************************************
// Alternative IEEE 802.15.4 MAC address
// 
// If different from 0xFFFFFFFF it overrides the 
// address in FCFG.
//**************************************************
#define SET_CCFG_IEEE_MAC_0                             0xFFFFFFFF // Bits [31:0]
#define SET_CCFG_IEEE_MAC_1                             0xFFFFFFFF // Bits [63:32]

//**************************************************
// Alternative BLE address
// 
// If different from 0xFFFFFFFF it overrides the 
// address in FCFG.
//**************************************************
#define SET_CCFG_IEEE_BLE_0                             0xFFFFFFFF // Bits [31:0]
#define SET_CCFG_IEEE_BLE_1                             0xFFFFFFFF // Bits [63:32]

//**************************************************
// Flash sector write protection
//
// Each bit write protects (program and erase) one 
// sector (4kB). Bit must be set to 0 in order to enable 
// sector write protect. The sector write protection 
// is enabled by setting corresponding bit in the 
// FSM_BSLE0- and FSM_BSLP0-registers in the flash 
// controller.
//**************************************************
#define SET_CCFG_CCFG_PROT_31_0                         0xFFFFFFFF
#define SET_CCFG_CCFG_PROT_63_32                        0xFFFFFFFF
#define SET_CCFG_CCFG_PROT_95_64                        0xFFFFFFFF
#define SET_CCFG_CCFG_PROT_127_96                       0xFFFFFFFF


//**************************************************
// CCFG values that should not be modified.
//**************************************************
#define SET_CCFG_SIZE_AND_DIS_FLAGS_SIZE_OF_CCFG         0x0054
#define SET_CCFG_SIZE_AND_DIS_FLAGS_DISABLE_FLAGS        0x3FFF

#define SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TP45               0xFF
#define SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TP25               0xFF
#define SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TP5                0xFF
#define SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TM15               0xFF

#define SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP125              0xFF
#define SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP105              0xFF
#define SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP85               0xFF
#define SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP65               0xFF

#define SET_CCFG_RTC_OFFSET_RTC_COMP_P0                  0xFFFF
#define SET_CCFG_RTC_OFFSET_RTC_COMP_P1                  0xFF
#define SET_CCFG_RTC_OFFSET_RTC_COMP_P2                  0xFF

#define SET_CCFG_FREQ_OFFSET_HF_COMP_P0                  0xFFFF
#define SET_CCFG_FREQ_OFFSET_HF_COMP_P1                  0xFF
#define SET_CCFG_FREQ_OFFSET_HF_COMP_P2                  0xFF

#define SET_CCFG_IMAGE_VALID_CONF_IMAGE_VALID            0x00000000

//*****************************************************************************
//
// Concatenate bit fields to registers.
// DO NOT EDIT!
//
//*****************************************************************************
#define DEFAULT_CCFG_MODE_CONF_1         ( \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_1_ALT_DCDC_VMIN      << CCFG_MODE_CONF_1_ALT_DCDC_VMIN_S      )) | ~CCFG_MODE_CONF_1_ALT_DCDC_VMIN_M      ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN << CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN_S )) | ~CCFG_MODE_CONF_1_ALT_DCDC_DITHER_EN_M ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_1_ALT_DCDC_IPEAK     << CCFG_MODE_CONF_1_ALT_DCDC_IPEAK_S     )) | ~CCFG_MODE_CONF_1_ALT_DCDC_IPEAK_M     ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_1_DELTA_IBIAS_INIT   << CCFG_MODE_CONF_1_DELTA_IBIAS_INIT_S   )) | ~CCFG_MODE_CONF_1_DELTA_IBIAS_INIT_M   ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET << CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET_S )) | ~CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET_M ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_1_XOSC_MAX_START     << CCFG_MODE_CONF_1_XOSC_MAX_START_S     )) | ~CCFG_MODE_CONF_1_XOSC_MAX_START_M     ) )

#define DEFAULT_CCFG_SIZE_AND_DIS_FLAGS  ( \
	 ( ((uint32_t)( SET_CCFG_SIZE_AND_DIS_FLAGS_SIZE_OF_CCFG         << CCFG_SIZE_AND_DIS_FLAGS_SIZE_OF_CCFG_S         )) | ~CCFG_SIZE_AND_DIS_FLAGS_SIZE_OF_CCFG_M         ) & \
	 ( ((uint32_t)( SET_CCFG_SIZE_AND_DIS_FLAGS_DISABLE_FLAGS        << CCFG_SIZE_AND_DIS_FLAGS_DISABLE_FLAGS_S        )) | ~CCFG_SIZE_AND_DIS_FLAGS_DISABLE_FLAGS_M        ) & \
	 ( ((uint32_t)( SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING << CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING_S )) | ~CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING_M ) & \
	 ( ((uint32_t)( SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR         << CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_S         )) | ~CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_M         ) )

#define DEFAULT_CCFG_MODE_CONF           ( \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_DCDC_RECHARGE       << CCFG_MODE_CONF_DCDC_RECHARGE_S       )) | ~CCFG_MODE_CONF_DCDC_RECHARGE_M       ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_DCDC_ACTIVE         << CCFG_MODE_CONF_DCDC_ACTIVE_S         )) | ~CCFG_MODE_CONF_DCDC_ACTIVE_M         ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_VDDR_EXT_LOAD       << CCFG_MODE_CONF_VDDR_EXT_LOAD_S       )) | ~CCFG_MODE_CONF_VDDR_EXT_LOAD_M       ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_VDDS_BOD_LEVEL      << CCFG_MODE_CONF_VDDS_BOD_LEVEL_S      )) | ~CCFG_MODE_CONF_VDDS_BOD_LEVEL_M      ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_SCLK_LF_OPTION      << CCFG_MODE_CONF_SCLK_LF_OPTION_S      )) | ~CCFG_MODE_CONF_SCLK_LF_OPTION_M      ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_RTC_COMP            << CCFG_MODE_CONF_RTC_COMP_S            )) | ~CCFG_MODE_CONF_RTC_COMP_M            ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_XOSC_FREQ           << CCFG_MODE_CONF_XOSC_FREQ_S           )) | ~CCFG_MODE_CONF_XOSC_FREQ_M           ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_XOSC_CAP_MOD        << CCFG_MODE_CONF_XOSC_CAP_MOD_S        )) | ~CCFG_MODE_CONF_XOSC_CAP_MOD_M        ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_HF_COMP             << CCFG_MODE_CONF_HF_COMP_S             )) | ~CCFG_MODE_CONF_HF_COMP_M             ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_XOSC_CAPARRAY_DELTA << CCFG_MODE_CONF_XOSC_CAPARRAY_DELTA_S )) | ~CCFG_MODE_CONF_XOSC_CAPARRAY_DELTA_M ) & \
	 ( ((uint32_t)( SET_CCFG_MODE_CONF_VDDR_CAP            << CCFG_MODE_CONF_VDDR_CAP_S            )) | ~CCFG_MODE_CONF_VDDR_CAP_M            ) )

#define DEFAULT_CCFG_VOLT_LOAD_0         ( \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TP45 << CCFG_VOLT_LOAD_0_VDDR_EXT_TP45_S )) | ~CCFG_VOLT_LOAD_0_VDDR_EXT_TP45_M ) & \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TP25 << CCFG_VOLT_LOAD_0_VDDR_EXT_TP25_S )) | ~CCFG_VOLT_LOAD_0_VDDR_EXT_TP25_M ) & \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TP5  << CCFG_VOLT_LOAD_0_VDDR_EXT_TP5_S  )) | ~CCFG_VOLT_LOAD_0_VDDR_EXT_TP5_M  ) & \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_0_VDDR_EXT_TM15 << CCFG_VOLT_LOAD_0_VDDR_EXT_TM15_S )) | ~CCFG_VOLT_LOAD_0_VDDR_EXT_TM15_M ) )

#define DEFAULT_CCFG_VOLT_LOAD_1         ( \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP125 << CCFG_VOLT_LOAD_1_VDDR_EXT_TP125_S )) | ~CCFG_VOLT_LOAD_1_VDDR_EXT_TP125_M ) & \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP105 << CCFG_VOLT_LOAD_1_VDDR_EXT_TP105_S )) | ~CCFG_VOLT_LOAD_1_VDDR_EXT_TP105_M ) & \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP85  << CCFG_VOLT_LOAD_1_VDDR_EXT_TP85_S  )) | ~CCFG_VOLT_LOAD_1_VDDR_EXT_TP85_M  ) & \
	 ( ((uint32_t)( SET_CCFG_VOLT_LOAD_1_VDDR_EXT_TP65  << CCFG_VOLT_LOAD_1_VDDR_EXT_TP65_S  )) | ~CCFG_VOLT_LOAD_1_VDDR_EXT_TP65_M  ) )

#define DEFAULT_CCFG_RTC_OFFSET          ( \
	 ( ((uint32_t)( SET_CCFG_RTC_OFFSET_RTC_COMP_P0 << CCFG_RTC_OFFSET_RTC_COMP_P0_S )) | ~CCFG_RTC_OFFSET_RTC_COMP_P0_M ) & \
	 ( ((uint32_t)( SET_CCFG_RTC_OFFSET_RTC_COMP_P1 << CCFG_RTC_OFFSET_RTC_COMP_P1_S )) | ~CCFG_RTC_OFFSET_RTC_COMP_P1_M ) & \
	 ( ((uint32_t)( SET_CCFG_RTC_OFFSET_RTC_COMP_P2 << CCFG_RTC_OFFSET_RTC_COMP_P2_S )) | ~CCFG_RTC_OFFSET_RTC_COMP_P2_M ) )

#define DEFAULT_CCFG_FREQ_OFFSET         ( \
	 ( ((uint32_t)( SET_CCFG_FREQ_OFFSET_HF_COMP_P0 << CCFG_FREQ_OFFSET_HF_COMP_P0_S )) | ~CCFG_FREQ_OFFSET_HF_COMP_P0_M ) & \
	 ( ((uint32_t)( SET_CCFG_FREQ_OFFSET_HF_COMP_P1 << CCFG_FREQ_OFFSET_HF_COMP_P1_S )) | ~CCFG_FREQ_OFFSET_HF_COMP_P1_M ) & \
	 ( ((uint32_t)( SET_CCFG_FREQ_OFFSET_HF_COMP_P2 << CCFG_FREQ_OFFSET_HF_COMP_P2_S )) | ~CCFG_FREQ_OFFSET_HF_COMP_P2_M ) )

#define DEFAULT_CCFG_IEEE_MAC_0          SET_CCFG_IEEE_MAC_0
#define DEFAULT_CCFG_IEEE_MAC_1          SET_CCFG_IEEE_MAC_1
#define DEFAULT_CCFG_IEEE_BLE_0          SET_CCFG_IEEE_BLE_0
#define DEFAULT_CCFG_IEEE_BLE_1          SET_CCFG_IEEE_BLE_1

#define DEFAULT_CCFG_BL_CONFIG           ( \
	 ( ((uint32_t)( SET_CCFG_BL_CONFIG_BOOTLOADER_ENABLE << CCFG_BL_CONFIG_BOOTLOADER_ENABLE_S )) | ~CCFG_BL_CONFIG_BOOTLOADER_ENABLE_M ) & \
	 ( ((uint32_t)( SET_CCFG_BL_CONFIG_BL_LEVEL          << CCFG_BL_CONFIG_BL_LEVEL_S          )) | ~CCFG_BL_CONFIG_BL_LEVEL_M          ) & \
	 ( ((uint32_t)( SET_CCFG_BL_CONFIG_BL_PIN_NUMBER     << CCFG_BL_CONFIG_BL_PIN_NUMBER_S     )) | ~CCFG_BL_CONFIG_BL_PIN_NUMBER_M     ) & \
	 ( ((uint32_t)( SET_CCFG_BL_CONFIG_BL_ENABLE         << CCFG_BL_CONFIG_BL_ENABLE_S         )) | ~CCFG_BL_CONFIG_BL_ENABLE_M         ) )

#define DEFAULT_CCFG_ERASE_CONF          ( \
	 ( ((uint32_t)( SET_CCFG_ERASE_CONF_CHIP_ERASE_DIS_N << CCFG_ERASE_CONF_CHIP_ERASE_DIS_N_S )) | ~CCFG_ERASE_CONF_CHIP_ERASE_DIS_N_M ) & \
	 ( ((uint32_t)( SET_CCFG_ERASE_CONF_BANK_ERASE_DIS_N << CCFG_ERASE_CONF_BANK_ERASE_DIS_N_S )) | ~CCFG_ERASE_CONF_BANK_ERASE_DIS_N_M ) )

#define DEFAULT_CCFG_CCFG_TI_OPTIONS     ( \
	 ( ((uint32_t)( SET_CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE << CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE_S )) | ~CCFG_CCFG_TI_OPTIONS_TI_FA_ENABLE_M ) )

#define DEFAULT_CCFG_CCFG_TAP_DAP_0      ( \
	 ( ((uint32_t)( SET_CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE  << CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE_S  )) | ~CCFG_CCFG_TAP_DAP_0_CPU_DAP_ENABLE_M  ) & \
	 ( ((uint32_t)( SET_CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE << CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE_S )) | ~CCFG_CCFG_TAP_DAP_0_PRCM_TAP_ENABLE_M ) & \
	 ( ((uint32_t)( SET_CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE << CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE_S )) | ~CCFG_CCFG_TAP_DAP_0_TEST_TAP_ENABLE_M ) )

#define DEFAULT_CCFG_CCFG_TAP_DAP_1      ( \
	 ( ((uint32_t)( SET_CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE << CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE_S )) | ~CCFG_CCFG_TAP_DAP_1_PBIST2_TAP_ENABLE_M ) & \
	 ( ((uint32_t)( SET_CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE << CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE_S )) | ~CCFG_CCFG_TAP_DAP_1_PBIST1_TAP_ENABLE_M ) & \
	 ( ((uint32_t)( SET_CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE    << CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE_S    )) | ~CCFG_CCFG_TAP_DAP_1_WUC_TAP_ENABLE_M    ) )

#define DEFAULT_CCFG_IMAGE_VALID_CONF    ( \
	 ( ((uint32_t)( SET_CCFG_IMAGE_VALID_CONF_IMAGE_VALID << CCFG_IMAGE_VALID_CONF_IMAGE_VALID_S )) | ~CCFG_IMAGE_VALID_CONF_IMAGE_VALID_M ) )

#define DEFAULT_CCFG_CCFG_PROT_31_0      SET_CCFG_CCFG_PROT_31_0  
#define DEFAULT_CCFG_CCFG_PROT_63_32     SET_CCFG_CCFG_PROT_63_32 
#define DEFAULT_CCFG_CCFG_PROT_95_64     SET_CCFG_CCFG_PROT_95_64 
#define DEFAULT_CCFG_CCFG_PROT_127_96    SET_CCFG_CCFG_PROT_127_96

//*****************************************************************************
//
// Customer Configuration Area in Lock Page
//
//*****************************************************************************
#if defined(__IAR_SYSTEMS_ICC__)
// Section ".ccfg" is available in IAR CC26xx device patch v0.6.1 or higher
__root const ccfg_t __ccfg @ ".ccfg" =
#elif defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(__ccfg, ".ccfg")
#pragma RETAIN(__ccfg)
const ccfg_t __ccfg =
#else
const ccfg_t __ccfg __attribute__((section(".ccfg")))=
#endif
{                                     // Mapped to address
    DEFAULT_CCFG_MODE_CONF_1        , // 0x50003FAC (0x50003xxx maps to last
    DEFAULT_CCFG_SIZE_AND_DIS_FLAGS , // 0x50003FB0  sector in FLASH.
    DEFAULT_CCFG_MODE_CONF          , // 0x50003FB4  Independent of FLASH size)
    DEFAULT_CCFG_VOLT_LOAD_0        , // 0x50003FB8 
    DEFAULT_CCFG_VOLT_LOAD_1        , // 0x50003FBC
    DEFAULT_CCFG_RTC_OFFSET         , // 0x50003FC0
    DEFAULT_CCFG_FREQ_OFFSET        , // 0x50003FC4
    DEFAULT_CCFG_IEEE_MAC_0         , // 0x50003FC8
    DEFAULT_CCFG_IEEE_MAC_1         , // 0x50003FCC
    DEFAULT_CCFG_IEEE_BLE_0         , // 0x50003FD0
    DEFAULT_CCFG_IEEE_BLE_1         , // 0x50003FD4
    DEFAULT_CCFG_BL_CONFIG          , // 0x50003FD8
    DEFAULT_CCFG_ERASE_CONF         , // 0x50003FDC
    DEFAULT_CCFG_CCFG_TI_OPTIONS    , // 0x50003FE0
    DEFAULT_CCFG_CCFG_TAP_DAP_0     , // 0x50003FE4
    DEFAULT_CCFG_CCFG_TAP_DAP_1     , // 0x50003FE8
    DEFAULT_CCFG_IMAGE_VALID_CONF   , // 0x50003FEC
    DEFAULT_CCFG_CCFG_PROT_31_0     , // 0x50003FF0
    DEFAULT_CCFG_CCFG_PROT_63_32    , // 0x50003FF4
    DEFAULT_CCFG_CCFG_PROT_95_64    , // 0x50003FF8
    DEFAULT_CCFG_CCFG_PROT_127_96   , // 0x50003FFC
};
