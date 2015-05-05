/******************************************************************************
*  Filename:       hw_adi_4_aux.h
*  Revised:        2015-01-15 18:41:47 +0100 (to, 15 jan 2015)
*  Revision:       42400
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

#ifndef __HW_ADI_4_AUX_H__
#define __HW_ADI_4_AUX_H__


//*****************************************************************************
//
// This section defines the register offsets of
// ADI_4_AUX component
//
//*****************************************************************************
// Current Source
#define ADI_4_AUX_O_ISRC                                            0x00000004

// Comparator
#define ADI_4_AUX_O_COMP                                            0x00000005

// Charge Pump Test
#define ADI_4_AUX_O_CHPTEST                                         0x00000006

// ADC Control
#define ADI_4_AUX_O_ADC0                                            0x00000008

// Control ADC
#define ADI_4_AUX_O_ADC1                                            0x00000009

// Analog To Digital Reference 0
#define ADI_4_AUX_O_ADCREF0                                         0x0000000A

// Analog To Digital Reference 1
#define ADI_4_AUX_O_ADCREF1                                         0x0000000B

//*****************************************************************************
//
// Register: ADI_4_AUX_O_ISRC
//
//*****************************************************************************
// Field: [7:2] TRIM
//
// Adjust current from current source.
//
// Output currents may be combined to get desired total current.
// ENUMs:
// NC                   No current connected
// 0P25U                0.25 uA
// 0P5U                 0.5 uA
// 1P0U                 1.0 uA
// 2P0U                 2.0 uA
// 4P5U                 4.5 uA
// 11P75U               11.75 uA
//
#define ADI_4_AUX_ISRC_TRIM_M                                       0x000000FC
#define ADI_4_AUX_ISRC_TRIM_S                                       2
#define ADI_4_AUX_ISRC_TRIM_NC                                      0x00000000
#define ADI_4_AUX_ISRC_TRIM_0P25U                                   0x00000004
#define ADI_4_AUX_ISRC_TRIM_0P5U                                    0x00000008
#define ADI_4_AUX_ISRC_TRIM_1P0U                                    0x00000010
#define ADI_4_AUX_ISRC_TRIM_2P0U                                    0x00000020
#define ADI_4_AUX_ISRC_TRIM_4P5U                                    0x00000040
#define ADI_4_AUX_ISRC_TRIM_11P75U                                  0x00000080

// Field: [0]    EN
//
// Current source enable
#define ADI_4_AUX_ISRC_EN                                           0x00000001
#define ADI_4_AUX_ISRC_EN_BITN                                      0
#define ADI_4_AUX_ISRC_EN_M                                         0x00000001
#define ADI_4_AUX_ISRC_EN_S                                         0

//*****************************************************************************
//
// Register: ADI_4_AUX_O_COMP
//
//*****************************************************************************
// Field: [7]    COMPA_REF_RES_EN
//
// Enables 400kohm resistance from COMPA reference node to ground. Used with
// COMPA_REF_CURR_EN to generate voltage reference for cap-sense.
#define ADI_4_AUX_COMP_COMPA_REF_RES_EN                             0x00000080
#define ADI_4_AUX_COMP_COMPA_REF_RES_EN_BITN                        7
#define ADI_4_AUX_COMP_COMPA_REF_RES_EN_M                           0x00000080
#define ADI_4_AUX_COMP_COMPA_REF_RES_EN_S                           7

// Field: [6]    COMPA_REF_CURR_EN
//
// Enables 2uA IPTAT current from ISRC to COMPA reference node. Requires ISRC.EN
// = 1. Used with COMPA_REF_RES_EN to generate voltage reference for cap-sense.
#define ADI_4_AUX_COMP_COMPA_REF_CURR_EN                            0x00000040
#define ADI_4_AUX_COMP_COMPA_REF_CURR_EN_BITN                       6
#define ADI_4_AUX_COMP_COMPA_REF_CURR_EN_M                          0x00000040
#define ADI_4_AUX_COMP_COMPA_REF_CURR_EN_S                          6

// Field: [5:3] COMPB_TRIM
//
// COMPB voltage reference trim temperature coded:
// ENUMs:
// DIV1                 No reference division
// DIV2                 Divide reference by 2
// DIV3                 Divide reference by 3
// DIV4                 Divide reference by 4
//
#define ADI_4_AUX_COMP_COMPB_TRIM_M                                 0x00000038
#define ADI_4_AUX_COMP_COMPB_TRIM_S                                 3
#define ADI_4_AUX_COMP_COMPB_TRIM_DIV1                              0x00000000
#define ADI_4_AUX_COMP_COMPB_TRIM_DIV2                              0x00000008
#define ADI_4_AUX_COMP_COMPB_TRIM_DIV3                              0x00000018
#define ADI_4_AUX_COMP_COMPB_TRIM_DIV4                              0x00000038

// Field: [2]    COMPB_EN
//
// Comparator B enable
#define ADI_4_AUX_COMP_COMPB_EN                                     0x00000004
#define ADI_4_AUX_COMP_COMPB_EN_BITN                                2
#define ADI_4_AUX_COMP_COMPB_EN_M                                   0x00000004
#define ADI_4_AUX_COMP_COMPB_EN_S                                   2

// Field: [0]    COMPA_EN
//
// COMPA enable
#define ADI_4_AUX_COMP_COMPA_EN                                     0x00000001
#define ADI_4_AUX_COMP_COMPA_EN_BITN                                0
#define ADI_4_AUX_COMP_COMPA_EN_M                                   0x00000001
#define ADI_4_AUX_COMP_COMPA_EN_S                                   0

//*****************************************************************************
//
// Register: ADI_4_AUX_O_CHPTEST
//
//*****************************************************************************
// Field: [7]    CHP_EN
//
// Enable AUX Charge pump
#define ADI_4_AUX_CHPTEST_CHP_EN                                    0x00000080
#define ADI_4_AUX_CHPTEST_CHP_EN_BITN                               7
#define ADI_4_AUX_CHPTEST_CHP_EN_M                                  0x00000080
#define ADI_4_AUX_CHPTEST_CHP_EN_S                                  7

//*****************************************************************************
//
// Register: ADI_4_AUX_O_ADC0
//
//*****************************************************************************
// Field: [7]    SMPL_MODE
//
// ADC Sampling mode:
//
// 0: Synchronous mode
// 1: Asynchronous mode
//
// The ADC does a sample-and-hold before conversion. In synchronous mode the
// sampling starts when the ADC clock detects a rising edge on the trigger signal.
// Jitter/uncertainty will be inferred in the detection if the trigger signal
// originates from a domain that is asynchronous to the ADC clock. SMPL_CYCLE_EXP
// determines the the duration of sampling as 2^(SMPL_CYCLE_EXP+1) * 1/6MHz.
// Conversion starts immediately after sampling ends.
//
// In asynchronous mode the sampling is continuous when enabled. Sampling ends
// and conversion starts immediately with the rising edge of the trigger signal.
// Sampling restarts when the conversion has finished.
// Asynchronous mode is useful when it is important to avoid jitter in the
// sampling instant of an externally driven signal
#define ADI_4_AUX_ADC0_SMPL_MODE                                    0x00000080
#define ADI_4_AUX_ADC0_SMPL_MODE_BITN                               7
#define ADI_4_AUX_ADC0_SMPL_MODE_M                                  0x00000080
#define ADI_4_AUX_ADC0_SMPL_MODE_S                                  7

// Field: [6:3] SMPL_CYCLE_EXP
//
// Controls number of 6 MHz clock cycles in sampling/tracking mode before
// conversion. The sampling time will be 2^(SMPL_CYCLE_EXP + 1) 6 MHz clock
// periods. Giving a range of 2 to 65536 clock periods sampling time (up to 10.9ms).
// This setting only has effect in synchronous mode (SMPL_MODE =0). The default
// minimum value should be 3 (2.7us sampling time). Less than that should be
// considered experimental.
#define ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_M                             0x00000078
#define ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S                             3

// Field: [1]    RESET_N
//
// Reset ADC digital subchip, active low. ADC must be reset every time it is
// reconfigured.
//
// 0: Reset
// 1: Normal operation
//
#define ADI_4_AUX_ADC0_RESET_N                                      0x00000002
#define ADI_4_AUX_ADC0_RESET_N_BITN                                 1
#define ADI_4_AUX_ADC0_RESET_N_M                                    0x00000002
#define ADI_4_AUX_ADC0_RESET_N_S                                    1

// Field: [0]    EN
//
// ADC Enable
//
// 0: Disable
// 1: Enable
//
#define ADI_4_AUX_ADC0_EN                                           0x00000001
#define ADI_4_AUX_ADC0_EN_BITN                                      0
#define ADI_4_AUX_ADC0_EN_M                                         0x00000001
#define ADI_4_AUX_ADC0_EN_S                                         0

//*****************************************************************************
//
// Register: ADI_4_AUX_O_ADC1
//
//*****************************************************************************
//*****************************************************************************
//
// Register: ADI_4_AUX_O_ADCREF0
//
//*****************************************************************************
// Field: [6]    REF_ON_IDLE
//
// Keep ADCREF powered up in IDLE state when ADC0.SMPL_MODE = 0.
//
// Set to 1 if ADC0.SMPL_CYCLE_EXP is less than 6 (21.3us sampling time)
#define ADI_4_AUX_ADCREF0_REF_ON_IDLE                               0x00000040
#define ADI_4_AUX_ADCREF0_REF_ON_IDLE_BITN                          6
#define ADI_4_AUX_ADCREF0_REF_ON_IDLE_M                             0x00000040
#define ADI_4_AUX_ADCREF0_REF_ON_IDLE_S                             6

// Field: [3]    SRC
//
// ADC reference source:
//
// 0: Fixed reference = 4.3V
// 1: Relative reference = VDDA
//
//
#define ADI_4_AUX_ADCREF0_SRC                                       0x00000008
#define ADI_4_AUX_ADCREF0_SRC_BITN                                  3
#define ADI_4_AUX_ADCREF0_SRC_M                                     0x00000008
#define ADI_4_AUX_ADCREF0_SRC_S                                     3

// Field: [0]    EN
//
// ADC reference module enable:
//
// 0: ADC reference module powered down
// 1: ADC reference module enabled
#define ADI_4_AUX_ADCREF0_EN                                        0x00000001
#define ADI_4_AUX_ADCREF0_EN_BITN                                   0
#define ADI_4_AUX_ADCREF0_EN_M                                      0x00000001
#define ADI_4_AUX_ADCREF0_EN_S                                      0

//*****************************************************************************
//
// Register: ADI_4_AUX_O_ADCREF1
//
//*****************************************************************************
// Field: [5:0] VTRIM
//
// Trim output voltage of ADC fixed reference (64 steps, 2's complement). Applies
// only for ADCREF0.SRC = 0.
//
// Examples:
// 0x00 - nominal voltage 1.43V
// 0x01 - nominal + 0.4% 1.435V
// 0x3F - nominal - 0.4% 1.425V
// 0x1F - maximum voltage 1.6V
// 0x20 - minimum voltage 1.3V
#define ADI_4_AUX_ADCREF1_VTRIM_M                                   0x0000003F
#define ADI_4_AUX_ADCREF1_VTRIM_S                                   0

#endif // __HW_ADI_4_AUX_H__
