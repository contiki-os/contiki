/******************************************************************************
*  Filename:       hw_adi_3_refsys.h
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

#ifndef __HW_ADI_3_REFSYS_H__
#define __HW_ADI_3_REFSYS_H__


//*****************************************************************************
//
// This section defines the register offsets of
// ADI_3_REFSYS component
//
//*****************************************************************************
// Analog Test Control 0
#define ADI_3_REFSYS_O_ATESTCTL0                                    0x00000000

// Analog Test Control
#define ADI_3_REFSYS_O_SPARE0                                       0x00000001

// Internal register
#define ADI_3_REFSYS_O_REFSYSCTL0                                   0x00000002

// Internal register
#define ADI_3_REFSYS_O_REFSYSCTL1                                   0x00000003

// Internal register
#define ADI_3_REFSYS_O_REFSYSCTL2                                   0x00000004

// Internal register
#define ADI_3_REFSYS_O_REFSYSCTL3                                   0x00000005

// DCDC Control Register 0
#define ADI_3_REFSYS_O_DCDCCTL0                                     0x00000006

// DCDC Control Register 1
#define ADI_3_REFSYS_O_DCDCCTL1                                     0x00000007

// DCDC Control Register 2
#define ADI_3_REFSYS_O_DCDCCTL2                                     0x00000008

// DCDC Control Register 3
#define ADI_3_REFSYS_O_DCDCCTL3                                     0x00000009

// DCDC Control Register 5
#define ADI_3_REFSYS_O_DCDCCTL4                                     0x0000000A

// DCDC Control Register 6
#define ADI_3_REFSYS_O_DCDCCTL5                                     0x0000000B

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_ATESTCTL0
//
//*****************************************************************************
// Field: [7:4] SPARE4
//
// RSVD, not used at this time.
#define ADI_3_REFSYS_ATESTCTL0_SPARE4_M                             0x000000F0
#define ADI_3_REFSYS_ATESTCTL0_SPARE4_S                             4

// Field: [3:0] TEST_CTL
//
// ATEST muxing:
// ENUMs:
// NC                   No signal connected to ATEST outputs
// DIGLDOV_A1           DIGLDO_V on ATEST1
// REFSYSP_A1           REFSYSP on ATEST1
// DIGLDOI_A0           DIGLDO_I on ATEST0
// DCDC_A0              DCDC on ATEST0
//
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_M                           0x0000000F
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_S                           0
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_NC                          0x00000000
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_DIGLDOV_A1                  0x00000001
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_REFSYSP_A1                  0x00000002
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_DIGLDOI_A0                  0x00000004
#define ADI_3_REFSYS_ATESTCTL0_TEST_CTL_DCDC_A0                     0x00000008

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_SPARE0
//
//*****************************************************************************
// Field: [7:0] SPARE0
//
// Software should not rely on the value of a reserved. Writing any other value
// than the reset value may result in undefined behavior.
#define ADI_3_REFSYS_SPARE0_SPARE0_M                                0x000000FF
#define ADI_3_REFSYS_SPARE0_SPARE0_S                                0

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_REFSYSCTL0
//
//*****************************************************************************
// Field: [7:0] TESTCTL
//
// Internal Register. Customers can control this through TI provided API
// ENUMs:
// NC                   No signal connected to ATEST1 output
// IPTAT2U              IPTAT_2U routed to ATEST1
// IVREF4U              IVREF_4U routed to ATEST1
// IREF4U               IREF_4U routed to ATEST1
// VBG                  VBG routed to ATEST1
// VBGUNBUFF            VBG_UNBUFF routed to ATEST1
// VREF0P8V             VREF_0P8V routed to ATEST1
// VTEMP                VTEMP routed to ATEST1
// BMCOMPOUT            BM_COMP_OUT routed to ATEST1
//
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_M                           0x000000FF
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_S                           0
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_NC                          0x00000000
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_IPTAT2U                     0x00000001
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_IVREF4U                     0x00000002
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_IREF4U                      0x00000004
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_VBG                         0x00000008
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_VBGUNBUFF                   0x00000010
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_VREF0P8V                    0x00000020
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_VTEMP                       0x00000040
#define ADI_3_REFSYS_REFSYSCTL0_TESTCTL_BMCOMPOUT                   0x00000080

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_REFSYSCTL1
//
//*****************************************************************************
// Field: [7:3] TRIM_VDDS_BOD
//
// Internal Register. Customers can control this through TI provided API
// ENUMs:
// POS_4                Default VDDS_BOD threashold value
// POS_5                +1 step
// POS_6                +2 steps
// POS_7                +3 steps
// POS_0                -4 steps
// POS_1                -3 steps
// POS_2                -2 steps
// POS_3                -1 step
// POS_12               +8 steps
// POS_13               +9 steps
// POS_14               +10 steps
// POS_15               +11 steps
// POS_8                +4 steps
// POS_9                +5 steps
// POS_10               +6 steps
// POS_11               +7 steps
// POS_20               +16 steps
// POS_21               +17 steps
// POS_22               +18 steps
// POS_23               +19 steps
// POS_16               +12 steps
// POS_17               +13 steps
// POS_18               +14 steps
// POS_19               +15 steps
// POS_28               +24 steps
// POS_29               +25 steps
// POS_30               +26 steps
// POS_31               +27 steps
// POS_24               +20 steps
// POS_25               +21 steps
// POS_26               +22 steps
// POS_27               +23 steps
//
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_M                     0x000000F8
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_S                     3
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_4                 0x00000000
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_5                 0x00000008
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_6                 0x00000010
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_7                 0x00000018
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_0                 0x00000020
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_1                 0x00000028
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_2                 0x00000030
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_3                 0x00000038
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_12                0x00000040
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_13                0x00000048
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_14                0x00000050
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_15                0x00000058
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_8                 0x00000060
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_9                 0x00000068
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_10                0x00000070
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_11                0x00000078
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_20                0x00000080
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_21                0x00000088
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_22                0x00000090
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_23                0x00000098
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_16                0x000000A0
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_17                0x000000A8
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_18                0x000000B0
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_19                0x000000B8
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_28                0x000000C0
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_29                0x000000C8
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_30                0x000000D0
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_31                0x000000D8
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_24                0x000000E0
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_25                0x000000E8
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_26                0x000000F0
#define ADI_3_REFSYS_REFSYSCTL1_TRIM_VDDS_BOD_POS_27                0x000000F8

// Field: [2]    BATMON_COMP_TEST_EN
//
// Internal Register. Customers can control this through TI provided API
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_3_REFSYS_REFSYSCTL1_BATMON_COMP_TEST_EN                 0x00000004
#define ADI_3_REFSYS_REFSYSCTL1_BATMON_COMP_TEST_EN_BITN            2
#define ADI_3_REFSYS_REFSYSCTL1_BATMON_COMP_TEST_EN_M               0x00000004
#define ADI_3_REFSYS_REFSYSCTL1_BATMON_COMP_TEST_EN_S               2
#define ADI_3_REFSYS_REFSYSCTL1_BATMON_COMP_TEST_EN_DIS             0x00000000
#define ADI_3_REFSYS_REFSYSCTL1_BATMON_COMP_TEST_EN_EN              0x00000004

// Field: [1:0] TESTCTL
//
// Internal Register. Customers can control this through TI provided API
// ENUMs:
// NC                   No signal connected to ATEST1 output.
// BMCOMPIN             BM_COMP_IN from BATMON routed to ATEST1
// IPTAT1U              IPTAT_1U from BATMON routed to ATEST1
//
#define ADI_3_REFSYS_REFSYSCTL1_TESTCTL_M                           0x00000003
#define ADI_3_REFSYS_REFSYSCTL1_TESTCTL_S                           0
#define ADI_3_REFSYS_REFSYSCTL1_TESTCTL_NC                          0x00000000
#define ADI_3_REFSYS_REFSYSCTL1_TESTCTL_BMCOMPIN                    0x00000001
#define ADI_3_REFSYS_REFSYSCTL1_TESTCTL_IPTAT1U                     0x00000002

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_REFSYSCTL2
//
//*****************************************************************************
// Field: [7:4] TRIM_VREF
//
// Internal Register. Customers can control this through TI provided API
#define ADI_3_REFSYS_REFSYSCTL2_TRIM_VREF_M                         0x000000F0
#define ADI_3_REFSYS_REFSYSCTL2_TRIM_VREF_S                         4

// Field: [1:0] TRIM_TSENSE
//
// Internal Register. Customers can control this through TI provided API
#define ADI_3_REFSYS_REFSYSCTL2_TRIM_TSENSE_M                       0x00000003
#define ADI_3_REFSYS_REFSYSCTL2_TRIM_TSENSE_S                       0

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_REFSYSCTL3
//
//*****************************************************************************
// Field: [7]    BOD_BG_TRIM_EN
//
// Internal Register. Customers can control this through TI provided API
#define ADI_3_REFSYS_REFSYSCTL3_BOD_BG_TRIM_EN                      0x00000080
#define ADI_3_REFSYS_REFSYSCTL3_BOD_BG_TRIM_EN_BITN                 7
#define ADI_3_REFSYS_REFSYSCTL3_BOD_BG_TRIM_EN_M                    0x00000080
#define ADI_3_REFSYS_REFSYSCTL3_BOD_BG_TRIM_EN_S                    7

// Field: [6]    VTEMP_EN
//
// Internal Register. Customers can control this through TI provided API
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_3_REFSYS_REFSYSCTL3_VTEMP_EN                            0x00000040
#define ADI_3_REFSYS_REFSYSCTL3_VTEMP_EN_BITN                       6
#define ADI_3_REFSYS_REFSYSCTL3_VTEMP_EN_M                          0x00000040
#define ADI_3_REFSYS_REFSYSCTL3_VTEMP_EN_S                          6
#define ADI_3_REFSYS_REFSYSCTL3_VTEMP_EN_DIS                        0x00000000
#define ADI_3_REFSYS_REFSYSCTL3_VTEMP_EN_EN                         0x00000040

// Field: [5:0] TRIM_VBG
//
// Internal Register. Customers can control this through TI provided API
#define ADI_3_REFSYS_REFSYSCTL3_TRIM_VBG_M                          0x0000003F
#define ADI_3_REFSYS_REFSYSCTL3_TRIM_VBG_S                          0

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_DCDCCTL0
//
//*****************************************************************************
// Field: [7:5] GLDO_ISRC
//
// Set charge and re-charge current level.
// 2's complement encoding.
//
// 0x0: Default 11mA.
// 0x3: Max 15mA.
// 0x4: Max 5mA
//
#define ADI_3_REFSYS_DCDCCTL0_GLDO_ISRC_M                           0x000000E0
#define ADI_3_REFSYS_DCDCCTL0_GLDO_ISRC_S                           5

// Field: [4:0] VDDR_TRIM
//
// Set the VDDR voltage.
// Proprietary encoding.
//
// Increase voltage to max: 0x00, 0x01, 0x02 ... 0x15.
// Decrease voltage to min: 0x00, 0x1F, 0x1E, 0x1D ... 0x16.
// Step size = 16mV
//
// 0x00: Default, about 1.63V.
// 0x05: Typical voltage after trim voltage 1.71V.
// 0x15: Max voltage 1.96V.
// 0x16: Min voltage 1.47V.
#define ADI_3_REFSYS_DCDCCTL0_VDDR_TRIM_M                           0x0000001F
#define ADI_3_REFSYS_DCDCCTL0_VDDR_TRIM_S                           0

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_DCDCCTL1
//
//*****************************************************************************
// Field: [7:6] IPTAT_TRIM
//
// Trim GLDO bias current.
// Proprietary encoding.
//
// 0x0: Default
// 0x1: Increase GLDO bias by 1.3x.
// 0x2: Increase GLDO bias by 1.6x.
// 0x3: Decrease GLDO bias by 0.7x.
#define ADI_3_REFSYS_DCDCCTL1_IPTAT_TRIM_M                          0x000000C0
#define ADI_3_REFSYS_DCDCCTL1_IPTAT_TRIM_S                          6

// Field: [5]    VDDR_OK_HYST
//
// Increase the hysteresis for when VDDR is considered ok.
//
// 0: Hysteresis = 60mV
// 1: Hysteresis = 70mV
#define ADI_3_REFSYS_DCDCCTL1_VDDR_OK_HYST                          0x00000020
#define ADI_3_REFSYS_DCDCCTL1_VDDR_OK_HYST_BITN                     5
#define ADI_3_REFSYS_DCDCCTL1_VDDR_OK_HYST_M                        0x00000020
#define ADI_3_REFSYS_DCDCCTL1_VDDR_OK_HYST_S                        5

// Field: [4:0] VDDR_TRIM_SLEEP
//
// Set the min VDDR voltage threshold during sleep mode.
// Proprietary encoding.
//
// Increase voltage to max: 0x00, 0x01, 0x02 ... 0x15.
// Decrease voltage to min: 0x00, 0x1F, 0x1E, 0x1D ... 0x16.
// Step size = 16mV
//
// 0x00: Default, about 1.63V.
// 0x19: Typical voltage after trim voltage 1.52V.
// 0x15: Max voltage 1.96V.
// 0x16: Min voltage 1.47V.
#define ADI_3_REFSYS_DCDCCTL1_VDDR_TRIM_SLEEP_M                     0x0000001F
#define ADI_3_REFSYS_DCDCCTL1_VDDR_TRIM_SLEEP_S                     0

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_DCDCCTL2
//
//*****************************************************************************
// Field: [6]    TURNON_EA_SW
//
// Turn on erroramp switch
//
// 0: Erroramp Off (Default)
// 1: Erroramp On. Turns on GLDO error amp switch.
//
//
#define ADI_3_REFSYS_DCDCCTL2_TURNON_EA_SW                          0x00000040
#define ADI_3_REFSYS_DCDCCTL2_TURNON_EA_SW_BITN                     6
#define ADI_3_REFSYS_DCDCCTL2_TURNON_EA_SW_M                        0x00000040
#define ADI_3_REFSYS_DCDCCTL2_TURNON_EA_SW_S                        6

// Field: [5]    TEST_VDDR
//
// Connect VDDR to ATEST bus
//
// 0: Not connected.
// 1: Connected
//
// Set TESTSEL = 0x0 first before setting this bit.
//
#define ADI_3_REFSYS_DCDCCTL2_TEST_VDDR                             0x00000020
#define ADI_3_REFSYS_DCDCCTL2_TEST_VDDR_BITN                        5
#define ADI_3_REFSYS_DCDCCTL2_TEST_VDDR_M                           0x00000020
#define ADI_3_REFSYS_DCDCCTL2_TEST_VDDR_S                           5

// Field: [4]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_3_REFSYS_DCDCCTL2_BIAS_DIS                              0x00000010
#define ADI_3_REFSYS_DCDCCTL2_BIAS_DIS_BITN                         4
#define ADI_3_REFSYS_DCDCCTL2_BIAS_DIS_M                            0x00000010
#define ADI_3_REFSYS_DCDCCTL2_BIAS_DIS_S                            4

// Field: [3:0] TESTSEL
//
// Select signal for test bus, one hot.
//
//
// ENUMs:
// NC                   No signal connected to test bus.
// ERRAMP_OUT           Error amp output voltage connected to test bus.
// PASSGATE             Pass transistor gate voltage connected to test bus.
// IB1U                 1uA bias current connected to test bus.
// VDDROK               VDDR_OK connected to test bus.
//
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_M                             0x0000000F
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_S                             0
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_NC                            0x00000000
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_ERRAMP_OUT                    0x00000001
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_PASSGATE                      0x00000002
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_IB1U                          0x00000004
#define ADI_3_REFSYS_DCDCCTL2_TESTSEL_VDDROK                        0x00000008

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_DCDCCTL3
//
//*****************************************************************************
//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_DCDCCTL4
//
//*****************************************************************************
// Field: [7:6] DEADTIME_TRIM
//
// Adjust the supply voltage threshold below which the non overlap delay of the
// switch driver block decreases.
// Unsigned encoding.
//
// 0x0: Min 2.24v. Default.
// 0x3: Max 2.78v.
#define ADI_3_REFSYS_DCDCCTL4_DEADTIME_TRIM_M                       0x000000C0
#define ADI_3_REFSYS_DCDCCTL4_DEADTIME_TRIM_S                       6

// Field: [5:3] LOW_EN_SEL
//
// Control NMOS switch strength in linear steps.
// Unsigned encoding.
//
// 0x0: Switches are off. Dcdc will not operate.
// 0x1: Minimum switch strength.
// 0x7: Maximum switch strength. Typical setting.
#define ADI_3_REFSYS_DCDCCTL4_LOW_EN_SEL_M                          0x00000038
#define ADI_3_REFSYS_DCDCCTL4_LOW_EN_SEL_S                          3

// Field: [2:0] HIGH_EN_SEL
//
// Control PMOS switch strength in linear steps.
// Unsigned encoding.
//
// 0x0: Switches are off. Dcdc will not operate.
// 0x1: Minimum switch strength.
// 0x7: Maximum switch strength. Typical setting.
#define ADI_3_REFSYS_DCDCCTL4_HIGH_EN_SEL_M                         0x00000007
#define ADI_3_REFSYS_DCDCCTL4_HIGH_EN_SEL_S                         0

//*****************************************************************************
//
// Register: ADI_3_REFSYS_O_DCDCCTL5
//
//*****************************************************************************
// Field: [5]    TESTN
//
// Buck converter NMOS switch is turned on.
//
// 0: Default
//
// When TESTP = TESTN = 0 then the inductor shunt switch is turned on and
// PMOS and NMOS switches are off.
#define ADI_3_REFSYS_DCDCCTL5_TESTN                                 0x00000020
#define ADI_3_REFSYS_DCDCCTL5_TESTN_BITN                            5
#define ADI_3_REFSYS_DCDCCTL5_TESTN_M                               0x00000020
#define ADI_3_REFSYS_DCDCCTL5_TESTN_S                               5

// Field: [4]    TESTP
//
// Buck converter PMOS switch is turned on.
//
// 0: Default
//
// When TESTP = TESTN = 0 then the inductor shunt switch is turned on and
// PMOS and NMOS switches are off.
#define ADI_3_REFSYS_DCDCCTL5_TESTP                                 0x00000010
#define ADI_3_REFSYS_DCDCCTL5_TESTP_BITN                            4
#define ADI_3_REFSYS_DCDCCTL5_TESTP_M                               0x00000010
#define ADI_3_REFSYS_DCDCCTL5_TESTP_S                               4

// Field: [3]    DITHER_EN
//
// Enable switching frequency randomizer
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_3_REFSYS_DCDCCTL5_DITHER_EN                             0x00000008
#define ADI_3_REFSYS_DCDCCTL5_DITHER_EN_BITN                        3
#define ADI_3_REFSYS_DCDCCTL5_DITHER_EN_M                           0x00000008
#define ADI_3_REFSYS_DCDCCTL5_DITHER_EN_S                           3
#define ADI_3_REFSYS_DCDCCTL5_DITHER_EN_DIS                         0x00000000
#define ADI_3_REFSYS_DCDCCTL5_DITHER_EN_EN                          0x00000008

// Field: [2:0] IPEAK
//
// Set inductor peak current.
// Unsigned encoding.
//
// 0x0: Min 31mA
// 0x4: Typical 47mA
// 0x7: Max 59mA
//
#define ADI_3_REFSYS_DCDCCTL5_IPEAK_M                               0x00000007
#define ADI_3_REFSYS_DCDCCTL5_IPEAK_S                               0

#endif // __HW_ADI_3_REFSYS_H__
