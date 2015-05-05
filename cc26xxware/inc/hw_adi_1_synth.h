/******************************************************************************
*  Filename:       hw_adi_1_synth.h
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

#ifndef __HW_ADI_1_SYNTH_H__
#define __HW_ADI_1_SYNTH_H__


//*****************************************************************************
//
// This section defines the register offsets of
// ADI_1_SYNTH component
//
//*****************************************************************************
// LDOVCO Control
#define ADI_1_SYNTH_O_VCOLDOCTL0                                    0x00000000

// Low DropOut Regulator for Voltage Controlled Oscillator Control 1
#define ADI_1_SYNTH_O_VCOLDOCTL1                                    0x00000001

// Low DropOut Regulator for Voltage Controlled Oscillator Configuration
#define ADI_1_SYNTH_O_VCOLDOCFG                                     0x00000002

// Synthesizer Low DropOut Regultaror Control 0
#define ADI_1_SYNTH_O_SLDOCTL0                                      0x00000003

// Synthesizer Low DropOut Regulator Control 1
#define ADI_1_SYNTH_O_SLDOCTL1                                      0x00000004

// Synthesizer Control of Initialisation
#define ADI_1_SYNTH_O_SYNTHCTLINIT                                  0x00000008

// Analog Test Control 0
#define ADI_1_SYNTH_O_ATESTCTL0                                     0x00000009

// Analog Test Control 1
#define ADI_1_SYNTH_O_ATESTCTL1                                     0x0000000A

// Status
#define ADI_1_SYNTH_O_STAT                                          0x0000000F

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_VCOLDOCTL0
//
//*****************************************************************************
// Field: [4]    ATEST_V_EN
//
// Enables regulated output voltage to ATEST.
//
// 0: Disabled
// 1: Enabled
//
//
#define ADI_1_SYNTH_VCOLDOCTL0_ATEST_V_EN                           0x00000010
#define ADI_1_SYNTH_VCOLDOCTL0_ATEST_V_EN_BITN                      4
#define ADI_1_SYNTH_VCOLDOCTL0_ATEST_V_EN_M                         0x00000010
#define ADI_1_SYNTH_VCOLDOCTL0_ATEST_V_EN_S                         4

// Field: [3]    BYPASS_REG_EN
//
// Bypass LDO and short VDDR and LDO output.
//
// 0: Disabled
// 1: Enabled
#define ADI_1_SYNTH_VCOLDOCTL0_BYPASS_REG_EN                        0x00000008
#define ADI_1_SYNTH_VCOLDOCTL0_BYPASS_REG_EN_BITN                   3
#define ADI_1_SYNTH_VCOLDOCTL0_BYPASS_REG_EN_M                      0x00000008
#define ADI_1_SYNTH_VCOLDOCTL0_BYPASS_REG_EN_S                      3

// Field: [2]    RDY_EN
//
// Enable LDO ready Signal generation circuit.
// EN must also be set. When circuit is enabled, it will set STAT.LDOVCO_RDY
// when VCOLDO is ready.
//
// 0: Disable
// 1: Enable
#define ADI_1_SYNTH_VCOLDOCTL0_RDY_EN                               0x00000004
#define ADI_1_SYNTH_VCOLDOCTL0_RDY_EN_BITN                          2
#define ADI_1_SYNTH_VCOLDOCTL0_RDY_EN_M                             0x00000004
#define ADI_1_SYNTH_VCOLDOCTL0_RDY_EN_S                             2

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_1_SYNTH_VCOLDOCTL0_BIAS_DIS                             0x00000002
#define ADI_1_SYNTH_VCOLDOCTL0_BIAS_DIS_BITN                        1
#define ADI_1_SYNTH_VCOLDOCTL0_BIAS_DIS_M                           0x00000002
#define ADI_1_SYNTH_VCOLDOCTL0_BIAS_DIS_S                           1

// Field: [0]    EN
//
// Enable regulator for supplying VCO, VCO Divider
//
// 0: Disabled
// 1: Enabled
#define ADI_1_SYNTH_VCOLDOCTL0_EN                                   0x00000001
#define ADI_1_SYNTH_VCOLDOCTL0_EN_BITN                              0
#define ADI_1_SYNTH_VCOLDOCTL0_EN_M                                 0x00000001
#define ADI_1_SYNTH_VCOLDOCTL0_EN_S                                 0

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_VCOLDOCTL1
//
//*****************************************************************************
// Field: [6]    ATEST_I_EN
//
// Enable test current (2.5% of pass device current) to ATEST.
//
// 0: Disabled
// 1: Enabled
//
//
#define ADI_1_SYNTH_VCOLDOCTL1_ATEST_I_EN                           0x00000040
#define ADI_1_SYNTH_VCOLDOCTL1_ATEST_I_EN_BITN                      6
#define ADI_1_SYNTH_VCOLDOCTL1_ATEST_I_EN_M                         0x00000040
#define ADI_1_SYNTH_VCOLDOCTL1_ATEST_I_EN_S                         6

// Field: [5:0] TRIM_OUT
//
// Trim LDO's output voltage linearly with a signed 2's complement number. Step
// size is approximately 5 mV.
//
// 0x20: -32 : 1.238V (Minimum Voltage)
// ...
// 0x2F: -1   : 1.389V
// 0x00: +0  : 1.400V (Default)
// 0x01: +1  : 1.404V
// ...
// 0x1F:+31 : 1.549V (Maximum Voltage)
//
// VCOLDOCTL0.ATEST_V_EN required to monitor voltage.
//
//
#define ADI_1_SYNTH_VCOLDOCTL1_TRIM_OUT_M                           0x0000003F
#define ADI_1_SYNTH_VCOLDOCTL1_TRIM_OUT_S                           0

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_VCOLDOCFG
//
//*****************************************************************************
// Field: [6]    DIV_BIAS_DIS
//
// Disable RF divider dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_1_SYNTH_VCOLDOCFG_DIV_BIAS_DIS                          0x00000040
#define ADI_1_SYNTH_VCOLDOCFG_DIV_BIAS_DIS_BITN                     6
#define ADI_1_SYNTH_VCOLDOCFG_DIV_BIAS_DIS_M                        0x00000040
#define ADI_1_SYNTH_VCOLDOCFG_DIV_BIAS_DIS_S                        6

// Field: [5:3] COMP_RES
//
// Trim compensation resistor in series with Miller cap in nonlinear steps. Default
// should be 0x3. Unsigned number.
//
// 0x0: 1066 ohms (Minimum Resistance)
// 0x1: 1230 ohms
// 0x2: 1454 ohms
// 0x3: 1777 ohms (Default)
// 0x4: 2285 ohms
// 0x5: 3200 ohms
// 0x6: 5333 ohms
// 0x7: 16000 ohms (Maximum Resistance)
#define ADI_1_SYNTH_VCOLDOCFG_COMP_RES_M                            0x00000038
#define ADI_1_SYNTH_VCOLDOCFG_COMP_RES_S                            3

// Field: [2:0] COMP_CAP
//
// Trim compensation Miller cap in linear steps of 0.5 pF. Default should be 0x4.
// Unsigned. Tradeoff stability for speed.
//
// 0x0: 3.5 pF (Maximum Capacitance)
// 0x1: 3.0 pF
// ...
// 0x4: 1.5pF (Default)
// ...
// 0x6: 0.5 pF
// 0x7: Open Circuit / No Capacitance / No Compensation (regardless of
// COMP_RES setting)
#define ADI_1_SYNTH_VCOLDOCFG_COMP_CAP_M                            0x00000007
#define ADI_1_SYNTH_VCOLDOCFG_COMP_CAP_S                            0

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_SLDOCTL0
//
//*****************************************************************************
// Field: [6]    COMP_CAP
//
// Enable compensation cap
//
// 0: Disable compensation (default)
// 1: Enable compensation (1pF Miller cap around pass device)
#define ADI_1_SYNTH_SLDOCTL0_COMP_CAP                               0x00000040
#define ADI_1_SYNTH_SLDOCTL0_COMP_CAP_BITN                          6
#define ADI_1_SYNTH_SLDOCTL0_COMP_CAP_M                             0x00000040
#define ADI_1_SYNTH_SLDOCTL0_COMP_CAP_S                             6

// Field: [5]    ATEST_I_EN
//
// Enable test current (2% of pass device current) to ATEST.
//
// 0: Disabled
// 1: Enabled
//
//
#define ADI_1_SYNTH_SLDOCTL0_ATEST_I_EN                             0x00000020
#define ADI_1_SYNTH_SLDOCTL0_ATEST_I_EN_BITN                        5
#define ADI_1_SYNTH_SLDOCTL0_ATEST_I_EN_M                           0x00000020
#define ADI_1_SYNTH_SLDOCTL0_ATEST_I_EN_S                           5

// Field: [4]    ATEST_V_EN
//
// Enables regulated output voltage to ATEST.
//
// 0: Disabled
// 1: Enabled
//
//
#define ADI_1_SYNTH_SLDOCTL0_ATEST_V_EN                             0x00000010
#define ADI_1_SYNTH_SLDOCTL0_ATEST_V_EN_BITN                        4
#define ADI_1_SYNTH_SLDOCTL0_ATEST_V_EN_M                           0x00000010
#define ADI_1_SYNTH_SLDOCTL0_ATEST_V_EN_S                           4

// Field: [2]    RDY_EN
//
// Enable LDO ready Signal generation circuit.
// EN must also be set. When circuit is enabled, it will set STAT.SLDO_RDY
// when SLDO is ready.
//
// 0: Disable
// 1: Enable
#define ADI_1_SYNTH_SLDOCTL0_RDY_EN                                 0x00000004
#define ADI_1_SYNTH_SLDOCTL0_RDY_EN_BITN                            2
#define ADI_1_SYNTH_SLDOCTL0_RDY_EN_M                               0x00000004
#define ADI_1_SYNTH_SLDOCTL0_RDY_EN_S                               2

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_1_SYNTH_SLDOCTL0_BIAS_DIS                               0x00000002
#define ADI_1_SYNTH_SLDOCTL0_BIAS_DIS_BITN                          1
#define ADI_1_SYNTH_SLDOCTL0_BIAS_DIS_M                             0x00000002
#define ADI_1_SYNTH_SLDOCTL0_BIAS_DIS_S                             1

// Field: [0]    EN
//
// Enable regulator for supplying RF synthesizer core, TDC and clock retimer
//
// 0: Disabled
// 1: Enabled
#define ADI_1_SYNTH_SLDOCTL0_EN                                     0x00000001
#define ADI_1_SYNTH_SLDOCTL0_EN_BITN                                0
#define ADI_1_SYNTH_SLDOCTL0_EN_M                                   0x00000001
#define ADI_1_SYNTH_SLDOCTL0_EN_S                                   0

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_SLDOCTL1
//
//*****************************************************************************
// Field: [5:0] TRIM_OUT
//
// Trim LDO's output voltage linearly with a signed 2's complement number. Step
// size is approximately 5 mV.
//
// 0x20: -32 : 1.11 (Minimum Voltage)
// ...
// 0x00: +0  : 1.26V (Default)
// ...
// 0x1F:+31 : 1.41V (Maximum Voltage)
//
// SLDOCTL0.ATEST_V_EN required to monitor voltage.
//
#define ADI_1_SYNTH_SLDOCTL1_TRIM_OUT_M                             0x0000003F
#define ADI_1_SYNTH_SLDOCTL1_TRIM_OUT_S                             0

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_SYNTHCTLINIT
//
//*****************************************************************************
// Field: [1]    CLK_EN
//
// Enable for clock from XOSC to synthesizer
#define ADI_1_SYNTH_SYNTHCTLINIT_CLK_EN                             0x00000002
#define ADI_1_SYNTH_SYNTHCTLINIT_CLK_EN_BITN                        1
#define ADI_1_SYNTH_SYNTHCTLINIT_CLK_EN_M                           0x00000002
#define ADI_1_SYNTH_SYNTHCTLINIT_CLK_EN_S                           1

// Field: [0]    DDI_RESET_N
//
// Reset digital core of synthesizer DDI
//
// 0: Reset DDI
// 1: No action
//
//
#define ADI_1_SYNTH_SYNTHCTLINIT_DDI_RESET_N                        0x00000001
#define ADI_1_SYNTH_SYNTHCTLINIT_DDI_RESET_N_BITN                   0
#define ADI_1_SYNTH_SYNTHCTLINIT_DDI_RESET_N_M                      0x00000001
#define ADI_1_SYNTH_SYNTHCTLINIT_DDI_RESET_N_S                      0

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_ATESTCTL0
//
//*****************************************************************************
// Field: [7:0] TESTSEL
//
// Control muxing of analog test signals from RF_TOP.
//
// Used in conjunction with ATESTCTL1.TESTSEL.
// ENUMs:
// NC                   No signal connected to ATEST0/1
// RXOUTIP_A0           RX_OUTIP to ATEST0
// RXOUTQP_A0           RX_OUTQP to ATEST0
// PEAKDETP_A0          PEADET_P to ATEST0
// IFADCP_A0            IFADC_P to ATEST0
// LDOV_A0              LDO_V to ATEST0
// RXOUTIN_A1           RX_OUTIN to ATEST1
// RXOUTQN_A1           RXOUTQN to ATEST1
// PEAKDETN_A1          PEAKDET_N to ATEST1
//
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_M                             0x000000FF
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_S                             0
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_NC                            0x00000000
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_RXOUTIP_A0                    0x00000001
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_RXOUTQP_A0                    0x00000002
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_PEAKDETP_A0                   0x00000004
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_IFADCP_A0                     0x00000008
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_LDOV_A0                       0x00000010
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_RXOUTIN_A1                    0x00000020
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_RXOUTQN_A1                    0x00000040
#define ADI_1_SYNTH_ATESTCTL0_TESTSEL_PEAKDETN_A1                   0x00000080

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_ATESTCTL1
//
//*****************************************************************************
// Field: [7:2] RESEREVED2
//
// Software should not rely on the value of a reserved. Writing any other value
// than the reset value may result in undefined behavior.
#define ADI_1_SYNTH_ATESTCTL1_RESEREVED2_M                          0x000000FC
#define ADI_1_SYNTH_ATESTCTL1_RESEREVED2_S                          2

// Field: [1:0] TESTSEL
//
// Control muxing of analog test signals from RF_TOP.
//
// Used in conjunction with ATESTCTL0.TESTSEL.
// ENUMs:
// NC                   No signal connected to ATEST0/1
// IFADCTESTN_A1        IFADC_TEST_N to ATEST1
// LDOITEST_A1          LDO_ITEST to ATEST1
//
#define ADI_1_SYNTH_ATESTCTL1_TESTSEL_M                             0x00000003
#define ADI_1_SYNTH_ATESTCTL1_TESTSEL_S                             0
#define ADI_1_SYNTH_ATESTCTL1_TESTSEL_NC                            0x00000000
#define ADI_1_SYNTH_ATESTCTL1_TESTSEL_IFADCTESTN_A1                 0x00000001
#define ADI_1_SYNTH_ATESTCTL1_TESTSEL_LDOITEST_A1                   0x00000002

//*****************************************************************************
//
// Register: ADI_1_SYNTH_O_STAT
//
//*****************************************************************************
// Field: [2]    SYNTH_TUNE_ACK
//
// Acknowledgement from digital part of frequency synthesizer that the current
// calibration step has completed.
//
// 0: The calibration phase set by DLO_DTX:SYNTHREG00.SYNTH_TUNE_PHASE has not
// finished
// 1: The calibration phase set by DLO_DTX:SYNTHREG00.SYNTH_TUNE_PHASE has
// finished
#define ADI_1_SYNTH_STAT_SYNTH_TUNE_ACK                             0x00000004
#define ADI_1_SYNTH_STAT_SYNTH_TUNE_ACK_BITN                        2
#define ADI_1_SYNTH_STAT_SYNTH_TUNE_ACK_M                           0x00000004
#define ADI_1_SYNTH_STAT_SYNTH_TUNE_ACK_S                           2

// Field: [1]    SLDO_RDY
//
// Status of SLDO
// Latched once high and can only be reset by toggling SLDOCTL0.RDY_EN.
//
// 0: Output is less than 90% of target value
// 1: Output is greater than 90% of target value.
#define ADI_1_SYNTH_STAT_SLDO_RDY                                   0x00000002
#define ADI_1_SYNTH_STAT_SLDO_RDY_BITN                              1
#define ADI_1_SYNTH_STAT_SLDO_RDY_M                                 0x00000002
#define ADI_1_SYNTH_STAT_SLDO_RDY_S                                 1

// Field: [0]    LDOVCO_RDY
//
// Status of LDOVCO.
// Latched once high and can only be reset by toggling VCOLDOCTL0.RDY_EN.
//
// 0: Output is less than 90% of target value
// 1: Output is greater than 90% of target value.
#define ADI_1_SYNTH_STAT_LDOVCO_RDY                                 0x00000001
#define ADI_1_SYNTH_STAT_LDOVCO_RDY_BITN                            0
#define ADI_1_SYNTH_STAT_LDOVCO_RDY_M                               0x00000001
#define ADI_1_SYNTH_STAT_LDOVCO_RDY_S                               0

#endif // __HW_ADI_1_SYNTH_H__
