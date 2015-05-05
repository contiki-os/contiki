/******************************************************************************
*  Filename:       hw_adi_0_rf.h
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

#ifndef __HW_ADI_0_RF_H__
#define __HW_ADI_0_RF_H__


//*****************************************************************************
//
// This section defines the register offsets of
// ADI_0_RF component
//
//*****************************************************************************
// LNA and Antenna Diversity Control
#define ADI_0_RF_O_LNACTL0                                          0x00000000

// LNA Gain and Input Device Control
#define ADI_0_RF_O_LNACTL1                                          0x00000001

// LNA Bias and RXTX Pin Control
#define ADI_0_RF_O_LNACTL2                                          0x00000002

// IFAMP Control
#define ADI_0_RF_O_IFAMPCTL0                                        0x00000003

// IFAMP Gain Control
#define ADI_0_RF_O_IFAMPCTL1                                        0x00000004

// IFAMP Output Attenuation Control
#define ADI_0_RF_O_IFAMPCTL2                                        0x00000005

// PA Control
#define ADI_0_RF_O_PACTL0                                           0x00000006

// PA Gain and Power Control
#define ADI_0_RF_O_PACTL1                                           0x00000007

// PA Antenna Diversity Control
#define ADI_0_RF_O_PACTL2                                           0x00000008

// RFLDO Control
#define ADI_0_RF_O_RFLDO0                                           0x00000009

// RFLDO Output Trim
#define ADI_0_RF_O_RFLDO1                                           0x0000000A

// RFLDO Compensation Trim
#define ADI_0_RF_O_RFLDO2                                           0x0000000B

// Intermediate Frequency ADC Trim and Configuration
#define ADI_0_RF_O_IFADCCTL0                                        0x0000000C

// Intermediate Frequency ADC Trim and Configuration
#define ADI_0_RF_O_IFADCLFCFG0                                      0x0000000D

// Intermediate Frequency ADC Trim and Configuration
#define ADI_0_RF_O_IFADCLFCFG1                                      0x0000000E

// Intermediate Frequency ADC Ttrim and Configuration
#define ADI_0_RF_O_IFADCDAC                                         0x0000000F

// IFADC Quantizer Trim and Control
#define ADI_0_RF_O_IFADCQUANT0                                      0x00000010

// Intermediate Frequency ADC Trim and Configuration
#define ADI_0_RF_O_IFADCCTL1                                        0x00000012

// Intermediate Frequency ADC Configuration
#define ADI_0_RF_O_IFADCCTL2                                        0x00000013

// IFADC Analog Supply LDO
#define ADI_0_RF_O_IFALDO1                                          0x00000018

// IFADC Analog Supply LDO
#define ADI_0_RF_O_IFALDO2                                          0x00000019

// IFADC Analog Supply LDO
#define ADI_0_RF_O_IFALDO3                                          0x0000001A

// IFADC Digital Supply LDO
#define ADI_0_RF_O_IFDLDO1                                          0x0000001B

// IFADC Digital Supply LDO
#define ADI_0_RF_O_IFDLDO2                                          0x0000001C

// IFADC Digital Supply LDO
#define ADI_0_RF_O_IFDLDO3                                          0x0000001D

// IFAMP Bias Current Control
#define ADI_0_RF_O_IFAMPCTL3                                        0x0000001E

// Status
#define ADI_0_RF_O_STAT                                             0x0000001F

//*****************************************************************************
//
// Register: ADI_0_RF_O_LNACTL0
//
//*****************************************************************************
// Field: [7:4] MIX_AD
//
// Antenna diversity control in mixers.
// Other bit combinations than the enumerated ones are illegal.
// ENUMs:
// MIX_DIFF_MODE        Differential mode.
// MIX_SE_RFP           I-channel and Q-channel mixers are single balanced and
// connected to RFP LNA.
// MIX_SE_RFN           I-channel and Q-channel mixers are single balanced and
// connected to RFN LNA.
//
#define ADI_0_RF_LNACTL0_MIX_AD_M                                   0x000000F0
#define ADI_0_RF_LNACTL0_MIX_AD_S                                   4
#define ADI_0_RF_LNACTL0_MIX_AD_MIX_DIFF_MODE                       0x00000000
#define ADI_0_RF_LNACTL0_MIX_AD_MIX_SE_RFP                          0x00000050
#define ADI_0_RF_LNACTL0_MIX_AD_MIX_SE_RFN                          0x000000A0

// Field: [3:2] LNA_AD
//
// Antenna diversity control.
// Other bit combinations than the enumerated ones are not allowed.
// ENUMs:
// LNA_DIFF_MODE        LNA is in differential mode and utilizes RFP and RFN.
// LNA_SE_RFP           LNA is single ended and connected to RFP.
// LNA_SE_RFN           LNA is single ended and connected to RFN.
//
#define ADI_0_RF_LNACTL0_LNA_AD_M                                   0x0000000C
#define ADI_0_RF_LNACTL0_LNA_AD_S                                   2
#define ADI_0_RF_LNACTL0_LNA_AD_LNA_DIFF_MODE                       0x00000000
#define ADI_0_RF_LNACTL0_LNA_AD_LNA_SE_RFP                          0x00000004
#define ADI_0_RF_LNACTL0_LNA_AD_LNA_SE_RFN                          0x00000008

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_0_RF_LNACTL0_BIAS_DIS                                   0x00000002
#define ADI_0_RF_LNACTL0_BIAS_DIS_BITN                              1
#define ADI_0_RF_LNACTL0_BIAS_DIS_M                                 0x00000002
#define ADI_0_RF_LNACTL0_BIAS_DIS_S                                 1

// Field: [0]    EN
//
// LNA enable signal.
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_LNACTL0_EN                                         0x00000001
#define ADI_0_RF_LNACTL0_EN_BITN                                    0
#define ADI_0_RF_LNACTL0_EN_M                                       0x00000001
#define ADI_0_RF_LNACTL0_EN_S                                       0
#define ADI_0_RF_LNACTL0_EN_DIS                                     0x00000000
#define ADI_0_RF_LNACTL0_EN_EN                                      0x00000001

//*****************************************************************************
//
// Register: ADI_0_RF_O_LNACTL1
//
//*****************************************************************************
// Field: [5:4] DEV_CTL
//
// LNA input device control.
//
// 00: Half of input devices on with capacitor cross coupling. Default setting
// in differential mode. See LNACTL0.LNA_AD.
// 01: All devices on with capacitor cross coupling to half the devices.
// 10: All devices on, no cross coupling. Default setting in single ended mode.
// 11:  1/4 of input devices on, no cross coupling, gain is reduced by 3dB
// compared to '00' setting. Part of AGC when using differential mode.
#define ADI_0_RF_LNACTL1_DEV_CTL_M                                  0x00000030
#define ADI_0_RF_LNACTL1_DEV_CTL_S                                  4

// Field: [3:0] GAIN
//
// LNA gain control. Thermometer encoded.
//
// All other values are not supported.
// ENUMs:
// MAX_MINUS_12         LNA gain set to -12 dB
// MAX_MINUS_9          LNA gain set to -9 dB
// MAX_MINUS_6          LNA gain set to -6 dB
// MAX_MINUS_3          LNA gain set to -3 dB
// MAX                  LNA gain set to 0 dB
//
#define ADI_0_RF_LNACTL1_GAIN_M                                     0x0000000F
#define ADI_0_RF_LNACTL1_GAIN_S                                     0
#define ADI_0_RF_LNACTL1_GAIN_MAX_MINUS_12                          0x00000000
#define ADI_0_RF_LNACTL1_GAIN_MAX_MINUS_9                           0x00000001
#define ADI_0_RF_LNACTL1_GAIN_MAX_MINUS_6                           0x00000003
#define ADI_0_RF_LNACTL1_GAIN_MAX_MINUS_3                           0x00000007
#define ADI_0_RF_LNACTL1_GAIN_MAX                                   0x0000000F

//*****************************************************************************
//
// Register: ADI_0_RF_O_LNACTL2
//
//*****************************************************************************
// Field: [7:6] RXTX_PIN
//
// Control of  RXTX pin. TheRXTXpin is used when LNA uses external bias. See
// LNACTL2.EXT_BIAS.
//
// 00: RXTXpin = 0 (RX)
// 01: RXTXpin = high impedance (TX)
// 10: RXTXpin = 0
// 11: RXTXpin = 1
#define ADI_0_RF_LNACTL2_RXTX_PIN_M                                 0x000000C0
#define ADI_0_RF_LNACTL2_RXTX_PIN_S                                 6

// Field: [4]    EXT_BIAS
//
// LNA bias method
//
// 0: LNA is biased with internal resistors.
// 1: LNA is biased externally through balun.
#define ADI_0_RF_LNACTL2_EXT_BIAS                                   0x00000010
#define ADI_0_RF_LNACTL2_EXT_BIAS_BITN                              4
#define ADI_0_RF_LNACTL2_EXT_BIAS_M                                 0x00000010
#define ADI_0_RF_LNACTL2_EXT_BIAS_S                                 4

// Field: [3:0] IB
//
// LNA bias current control. Linear steps. Will be trimmed in production test.
//
// 0x0: Minimum
// 0xF: Maximum
#define ADI_0_RF_LNACTL2_IB_M                                       0x0000000F
#define ADI_0_RF_LNACTL2_IB_S                                       0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFAMPCTL0
//
//*****************************************************************************
// Field: [7:3] TRIM
//
// Trim bits for IFAMP gain. The trim circuit consist of a binary weighted resistor
// ladder which gives a non-linear gain versus bit value curve.
//
// 0x00: Max gain (default setting)
// 0x1F: Min gain
#define ADI_0_RF_IFAMPCTL0_TRIM_M                                   0x000000F8
#define ADI_0_RF_IFAMPCTL0_TRIM_S                                   3

// Field: [2]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_0_RF_IFAMPCTL0_BIAS_DIS                                 0x00000004
#define ADI_0_RF_IFAMPCTL0_BIAS_DIS_BITN                            2
#define ADI_0_RF_IFAMPCTL0_BIAS_DIS_M                               0x00000004
#define ADI_0_RF_IFAMPCTL0_BIAS_DIS_S                               2

// Field: [1]    EN_Q
//
// Q-channel IFAMP enable signal
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFAMPCTL0_EN_Q                                     0x00000002
#define ADI_0_RF_IFAMPCTL0_EN_Q_BITN                                1
#define ADI_0_RF_IFAMPCTL0_EN_Q_M                                   0x00000002
#define ADI_0_RF_IFAMPCTL0_EN_Q_S                                   1
#define ADI_0_RF_IFAMPCTL0_EN_Q_DIS                                 0x00000000
#define ADI_0_RF_IFAMPCTL0_EN_Q_EN                                  0x00000002

// Field: [0]    EN_I
//
// I-channel IFAMP enable signal
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFAMPCTL0_EN_I                                     0x00000001
#define ADI_0_RF_IFAMPCTL0_EN_I_BITN                                0
#define ADI_0_RF_IFAMPCTL0_EN_I_M                                   0x00000001
#define ADI_0_RF_IFAMPCTL0_EN_I_S                                   0
#define ADI_0_RF_IFAMPCTL0_EN_I_DIS                                 0x00000000
#define ADI_0_RF_IFAMPCTL0_EN_I_EN                                  0x00000001

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFAMPCTL1
//
//*****************************************************************************
// Field: [5:0] GAIN
//
// IFAMP gain control. Thermometer encoded.
//
// 0x00: Min gain (Max - 18.0dB)
// 0x01: Max gain - 15.0dB
// 0x03: Max gain - 12.0dB
// 0x07: Max gain - 9.0dB
// 0x0F: Max gain - 6.0dB
// 0x1F: Max gain - 3.0dB
// 0x3F: Max gain
//
// All other values are not supported.
// ENUMs:
// MAX_MINUS_18         IFAMP gain set to maximum -18 dB
// MAX_MINUS_15         IFAMP gain set to maximum -15 dB
// MAX_MINUS_12         IFAMP gain set to maximum -12 dB
// MAX_MINUS_9          IFAMP gain set to maximum -9 dB
// MAX_MINUS_6          IFAMP gain set to maximum -6 dB
// MAX_MINUS_3          IFAMP gain set to maximum -3 dB
// MAX                  IFAMP gain set to maximum
//
#define ADI_0_RF_IFAMPCTL1_GAIN_M                                   0x0000003F
#define ADI_0_RF_IFAMPCTL1_GAIN_S                                   0
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX_MINUS_18                        0x00000000
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX_MINUS_15                        0x00000001
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX_MINUS_12                        0x00000003
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX_MINUS_9                         0x00000007
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX_MINUS_6                         0x0000000F
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX_MINUS_3                         0x0000001F
#define ADI_0_RF_IFAMPCTL1_GAIN_MAX                                 0x0000003F

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFAMPCTL2
//
//*****************************************************************************
// Field: [2:0] ATTN
//
// IFAMP output attenuation control in 3 dB steps
// ENUMs:
// MIN                  0 dB
// MINUS_3              -3 dB
// MINUS_6              -6 dB
// MINUS_9              -9 dB
// MINUS_12             -12 dB
// MINUS_15             -15 dB
// MINUS_18             -18 dB
// MINUS_21             -21 dB
//
#define ADI_0_RF_IFAMPCTL2_ATTN_M                                   0x00000007
#define ADI_0_RF_IFAMPCTL2_ATTN_S                                   0
#define ADI_0_RF_IFAMPCTL2_ATTN_MIN                                 0x00000000
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_3                             0x00000001
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_6                             0x00000002
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_9                             0x00000003
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_12                            0x00000004
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_15                            0x00000005
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_18                            0x00000006
#define ADI_0_RF_IFAMPCTL2_ATTN_MINUS_21                            0x00000007

//*****************************************************************************
//
// Register: ADI_0_RF_O_PACTL0
//
//*****************************************************************************
// Field: [7:3] TRIM
//
// Trim of bias current to get constant output power over process and temperature.
// Will be trimmed in production test.
//
// 0x00: Minimum
// 0x1F: Maximum
#define ADI_0_RF_PACTL0_TRIM_M                                      0x000000F8
#define ADI_0_RF_PACTL0_TRIM_S                                      3

// Field: [2]    PEAKDET_EN
//
// PA peakdetect circuit enable signal.
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_PACTL0_PEAKDET_EN                                  0x00000004
#define ADI_0_RF_PACTL0_PEAKDET_EN_BITN                             2
#define ADI_0_RF_PACTL0_PEAKDET_EN_M                                0x00000004
#define ADI_0_RF_PACTL0_PEAKDET_EN_S                                2
#define ADI_0_RF_PACTL0_PEAKDET_EN_DIS                              0x00000000
#define ADI_0_RF_PACTL0_PEAKDET_EN_EN                               0x00000004

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define ADI_0_RF_PACTL0_BIAS_DIS                                    0x00000002
#define ADI_0_RF_PACTL0_BIAS_DIS_BITN                               1
#define ADI_0_RF_PACTL0_BIAS_DIS_M                                  0x00000002
#define ADI_0_RF_PACTL0_BIAS_DIS_S                                  1

// Field: [0]    EN
//
// PA enable signal.
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_PACTL0_EN                                          0x00000001
#define ADI_0_RF_PACTL0_EN_BITN                                     0
#define ADI_0_RF_PACTL0_EN_M                                        0x00000001
#define ADI_0_RF_PACTL0_EN_S                                        0
#define ADI_0_RF_PACTL0_EN_DIS                                      0x00000000
#define ADI_0_RF_PACTL0_EN_EN                                       0x00000001

//*****************************************************************************
//
// Register: ADI_0_RF_O_PACTL1
//
//*****************************************************************************
// Field: [7:6] GAIN
//
// Gain control in PA 1st stage.
//
// x0: Max gain
// 01: Lower gain
// 11: Min gain
#define ADI_0_RF_PACTL1_GAIN_M                                      0x000000C0
#define ADI_0_RF_PACTL1_GAIN_S                                      6

// Field: [5:0] IB
//
// PA output power control
//
// 0x00: Min output power
// 0x3F: Max output power
//
//
#define ADI_0_RF_PACTL1_IB_M                                        0x0000003F
#define ADI_0_RF_PACTL1_IB_S                                        0

//*****************************************************************************
//
// Register: ADI_0_RF_O_PACTL2
//
//*****************************************************************************
// Field: [7:5] RXTX
//
// Control of RFP and RFN when used to control external range extender device. (ie.
// If RFP is used as a single ended RF input, RFN is available to be used as a
// control output)
//
// 0x0: Default
// 0x2: RFP is low
// 0x3: RFP is high (1.4V)
// 0x6: RFN is low
// 0x7: RFN is high (1.4V)
//
// Other bit combinations are not valid.
#define ADI_0_RF_PACTL2_RXTX_M                                      0x000000E0
#define ADI_0_RF_PACTL2_RXTX_S                                      5

// Field: [4:3] CM
//
// Debug / experimental registers. Do not use!
//
//
#define ADI_0_RF_PACTL2_CM_M                                        0x00000018
#define ADI_0_RF_PACTL2_CM_S                                        3

// Field: [2:0] AD
//
// When PACTL0.EN = 1:
// 000, 100: PA is differential
// 001, 101: PA is single ended and connected to RFP.
// 010, 110: PA is single ended and connected to RFN.
//
// When PACTL0.EN = 0:
// 101: RFN PA NMOS is turned on giving bias to RFP LNA when configured in
// external bias mode. Same functionality as RXTXpin.
// 110: RFP PA NMOS is turned on giving bias to RFN LNA when configured in
// external bias mode. Same functionality as RXTXpin.
//
// Other bit combinations are not valid.
#define ADI_0_RF_PACTL2_AD_M                                        0x00000007
#define ADI_0_RF_PACTL2_AD_S                                        0

//*****************************************************************************
//
// Register: ADI_0_RF_O_RFLDO0
//
//*****************************************************************************
// Field: [5]    ATEST_I_EN
//
// Enable test current (2% of pass device current) to ATEST.
// 0: Disabled
// 1: Enabled
//
//
#define ADI_0_RF_RFLDO0_ATEST_I_EN                                  0x00000020
#define ADI_0_RF_RFLDO0_ATEST_I_EN_BITN                             5
#define ADI_0_RF_RFLDO0_ATEST_I_EN_M                                0x00000020
#define ADI_0_RF_RFLDO0_ATEST_I_EN_S                                5

// Field: [4]    ATEST_V_EN
//
// Enables regulated output voltage to ATEST.
// 0: Disabled
// 1: Enabled
//
//
#define ADI_0_RF_RFLDO0_ATEST_V_EN                                  0x00000010
#define ADI_0_RF_RFLDO0_ATEST_V_EN_BITN                             4
#define ADI_0_RF_RFLDO0_ATEST_V_EN_M                                0x00000010
#define ADI_0_RF_RFLDO0_ATEST_V_EN_S                                4

// Field: [3]    BYPASS_REG_EN
//
// Bypass LDO and short VDDR to PA, LNA and Mixer. RFLDO0.EN must be 0b1 to use
// this mode.
//
// 0: Disabled
// 1: Enabled
#define ADI_0_RF_RFLDO0_BYPASS_REG_EN                               0x00000008
#define ADI_0_RF_RFLDO0_BYPASS_REG_EN_BITN                          3
#define ADI_0_RF_RFLDO0_BYPASS_REG_EN_M                             0x00000008
#define ADI_0_RF_RFLDO0_BYPASS_REG_EN_S                             3

// Field: [2]    RDY_EN
//
// Enables generation of the LDO ready signal. Read result from STAT.RF_LDO.
// RFLDO0.EN must be 0b1 in order to use this feature.
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_RFLDO0_RDY_EN                                      0x00000004
#define ADI_0_RF_RFLDO0_RDY_EN_BITN                                 2
#define ADI_0_RF_RFLDO0_RDY_EN_M                                    0x00000004
#define ADI_0_RF_RFLDO0_RDY_EN_S                                    2
#define ADI_0_RF_RFLDO0_RDY_EN_DIS                                  0x00000000
#define ADI_0_RF_RFLDO0_RDY_EN_EN                                   0x00000004

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
// ENUMs:
// EN                   Enable
// DIS                  Disable
//
#define ADI_0_RF_RFLDO0_BIAS_DIS                                    0x00000002
#define ADI_0_RF_RFLDO0_BIAS_DIS_BITN                               1
#define ADI_0_RF_RFLDO0_BIAS_DIS_M                                  0x00000002
#define ADI_0_RF_RFLDO0_BIAS_DIS_S                                  1
#define ADI_0_RF_RFLDO0_BIAS_DIS_EN                                 0x00000000
#define ADI_0_RF_RFLDO0_BIAS_DIS_DIS                                0x00000002

// Field: [0]    EN
//
// Enable signal for RFLDO. Powers PA, LNA and Mixer
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_RFLDO0_EN                                          0x00000001
#define ADI_0_RF_RFLDO0_EN_BITN                                     0
#define ADI_0_RF_RFLDO0_EN_M                                        0x00000001
#define ADI_0_RF_RFLDO0_EN_S                                        0
#define ADI_0_RF_RFLDO0_EN_DIS                                      0x00000000
#define ADI_0_RF_RFLDO0_EN_EN                                       0x00000001

//*****************************************************************************
//
// Register: ADI_0_RF_O_RFLDO1
//
//*****************************************************************************
// Field: [6:0] TRIM_OUT
//
// Trims output voltage in steps of approximately 5mV linear steps.
// The trim is unsigned and uncentered.
//
// 0x00: ~893 mV (Minimum Output Voltage)
// 0x01: ~897 mV
// ...
// 0x68: ~1.40 V (Target Value)
// 0x3F: ~1.52 V (Maximum Value)
//
// RFLDO0.ATEST_V_EN is needed for trimming.
//
//
#define ADI_0_RF_RFLDO1_TRIM_OUT_M                                  0x0000007F
#define ADI_0_RF_RFLDO1_TRIM_OUT_S                                  0

//*****************************************************************************
//
// Register: ADI_0_RF_O_RFLDO2
//
//*****************************************************************************
// Field: [5:3] COMP_RES
//
// Trim compensation resistor in series with Miller cap in nonlinear steps. Default
// should be 0x1. Unsigned number.
//
// 0x0: 1066 ohms (Minimum Resistance)
// 0x1: 1230 ohms (Default)
// 0x2: 1454 ohms
// 0x3: 1777 ohms
// 0x4: 2285 ohms
// 0x5: 3200 ohms
// 0x6: 5333 ohms
// 0x7: 16000 ohms (Maximum Resistance)
#define ADI_0_RF_RFLDO2_COMP_RES_M                                  0x00000038
#define ADI_0_RF_RFLDO2_COMP_RES_S                                  3

// Field: [2:0] COMP_CAP
//
// Trim compensation Miller cap in linear steps of 0.5 pF. Default should be 0x6.
// Unsigned number.
//
// 0x0: 3.5 pF (Maximum Capacitance)
// 0x1: 3.0 pF
// ...
// 0x6: 0.5 pF (Default)
// 0x7: Open Circuit / No Capacitance / No Compensation (regardless of
// COMP_RES setting)
//
#define ADI_0_RF_RFLDO2_COMP_CAP_M                                  0x00000007
#define ADI_0_RF_RFLDO2_COMP_CAP_S                                  0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCCTL0
//
//*****************************************************************************
// Field: [7:4] INT2ADJ
//
// Adjust integrator 2 gain, given by gm/C. The list below indicates the resulting
// gm while C is fixed (~235fF).  Note MSB signifies high power (1) vs. Low power
// (0) mode.
//
// 0x0: 27.8 uS
// 0x1: 26.0 uS
// 0x2: 24.0 uS
// 0x3: 22.7 uS
// 0x4: 21.2 uS
// 0x5: 20.1 uS
// 0x6: 18.9 uS
// 0x7: 18.1 uS
// 0x8: 32.0 uS
// 0x9: 29.6 uS
// 0xA: 27.1 uS
// 0xB: 25.4 uS
// 0xC: 23.6 uS
// 0xD: 22.3 uS (Default)
// 0xE: 20.8 uS
// 0xF: 19.8 uS
#define ADI_0_RF_IFADCCTL0_INT2ADJ_M                                0x000000F0
#define ADI_0_RF_IFADCCTL0_INT2ADJ_S                                4

// Field: [3:2] AAFCAP
//
// Adjust AAF damping
// Nominal fc (Mag = -3dB)
//
// 0: 6.49 MHz
// 1: 5.31 MHz
// 2: 4.49 MHz
// 3: 3.59 MHz (Default)
#define ADI_0_RF_IFADCCTL0_AAFCAP_M                                 0x0000000C
#define ADI_0_RF_IFADCCTL0_AAFCAP_S                                 2

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
// ENUMs:
// EN                   Enable (default)
// DIS                  Disable
//
#define ADI_0_RF_IFADCCTL0_BIAS_DIS                                 0x00000002
#define ADI_0_RF_IFADCCTL0_BIAS_DIS_BITN                            1
#define ADI_0_RF_IFADCCTL0_BIAS_DIS_M                               0x00000002
#define ADI_0_RF_IFADCCTL0_BIAS_DIS_S                               1
#define ADI_0_RF_IFADCCTL0_BIAS_DIS_EN                              0x00000000
#define ADI_0_RF_IFADCCTL0_BIAS_DIS_DIS                             0x00000002

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCLFCFG0
//
//*****************************************************************************
// Field: [7:4] FF1ADJ
//
// Adjust FF1 gain (transconductance/gm from first integrator into quantizer), note
// MSB signifies high power (1) vs. Low power (0) mode.
//
// 0x0: 23.8 uS - Default
// 0x1: 22.3 uS
// 0x2: 20.6 uS
// 0x3: 19.5 us
// 0x4: 18.1 uS
// 0x5: 17.2 uS
// 0x6: 16.2 uS
// 0x7: 15.5 uS
// 0x8: 27.2 uS
// 0x9: 25.2 uS
// 0xA: 23.1 uS
// 0xB: 21.6 uS
// 0xC: 20.0 uS
// 0xD: 18.9 uS
// 0xE: 17.7 uS
// 0xF: 16.8 uS
#define ADI_0_RF_IFADCLFCFG0_FF1ADJ_M                               0x000000F0
#define ADI_0_RF_IFADCLFCFG0_FF1ADJ_S                               4

// Field: [3:0] INT3ADJ
//
// Adjust integrator 3 gain, see
// IFADCCTL0.INT2ADJ for valid trim values
// Default value, 0x6
#define ADI_0_RF_IFADCLFCFG0_INT3ADJ_M                              0x0000000F
#define ADI_0_RF_IFADCLFCFG0_INT3ADJ_S                              0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCLFCFG1
//
//*****************************************************************************
// Field: [7:4] FF3ADJ
//
// Adjust FF3 gain (transconductance/gm from third integrator into quantizer). See
// IFADCLFCFG0.FF1ADJ for values. Default value is 0x4
#define ADI_0_RF_IFADCLFCFG1_FF3ADJ_M                               0x000000F0
#define ADI_0_RF_IFADCLFCFG1_FF3ADJ_S                               4

// Field: [3:0] FF2ADJ
//
// Adjust FF2 gain (transconductance/gm from second integrator into quantizer). See
// IFADCLFCFG0.FF1ADJ for values. Default value is 0x3
#define ADI_0_RF_IFADCLFCFG1_FF2ADJ_M                               0x0000000F
#define ADI_0_RF_IFADCLFCFG1_FF2ADJ_S                               0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCDAC
//
//*****************************************************************************
// Field: [7]    MODE
//
// Selects DAC return to zero mode
// ENUMs:
// NRTZ                 NRTZ enabled. Default
// RTZ                  RTZ enabled
//
#define ADI_0_RF_IFADCDAC_MODE                                      0x00000080
#define ADI_0_RF_IFADCDAC_MODE_BITN                                 7
#define ADI_0_RF_IFADCDAC_MODE_M                                    0x00000080
#define ADI_0_RF_IFADCDAC_MODE_S                                    7
#define ADI_0_RF_IFADCDAC_MODE_NRTZ                                 0x00000000
#define ADI_0_RF_IFADCDAC_MODE_RTZ                                  0x00000080

// Field: [6:1] TRIM
//
// Trim feedback-DAC current in uA. Increasing this current will give a more
// aggressive noise shaping but it also reduce the gain into the IFADC.
//
// 0x00: 0.00 uA
// 0x01: 0.83 uA
// 0x02: 1.66 uA
// 0x03: 2.50 uA
// 0x04: 3.33 uA
// 0x05: 4.17 uA
// 0x06: 5.00 uA
// 0x07: 5.83 uA
// 0x08: 6.67 uA
// 0x09: 7.50 uA
// 0x0A: 8.33 uA
// 0x0B: 9.17 uA
// 0x0C: 10.00 uA
// 0x0D: 10.83 uA - Default
// 0x0E: 11.67 uA
// 0x0F: 12.50 uA
// 0x10: 13.33 uA
// 0x11: 14.17 uA
// 0x12:  15.00 uA
// 0x13:  15.83 uA
// 0x14:  16.67 uA
// 0x15:  17.50 uA
// 0x16:  18.33 uA
// 0x17:  19.17 uA
// 0x18:  20.00 uA
// 0x19:  20.83 uA
// 0x1A:  21.67 uA
// 0x1B:  22.50 uA
// 0x1C:  23.33 uA
// 0x1D:  24.17 uA
// 0x1E:  25.00 uA
// 0x1F:  25.83 uA
//
// Note that bit 6 is not used
#define ADI_0_RF_IFADCDAC_TRIM_M                                    0x0000007E
#define ADI_0_RF_IFADCDAC_TRIM_S                                    1

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCQUANT0
//
//*****************************************************************************
// Field: [3]    AUTOCAL_EN
//
// Quantizer auto calibrate enable.
//
// Before this sequence is initiated the IFADCs to be used must be enabled with
// IFADCCTL1.ADCIEN and/or  IFADCCTL1.ADCQEN. Then reset digital sub-blocks with
//  IFADCCTL2.RESETN before the auto calibrate is enabled.
#define ADI_0_RF_IFADCQUANT0_AUTOCAL_EN                             0x00000008
#define ADI_0_RF_IFADCQUANT0_AUTOCAL_EN_BITN                        3
#define ADI_0_RF_IFADCQUANT0_AUTOCAL_EN_M                           0x00000008
#define ADI_0_RF_IFADCQUANT0_AUTOCAL_EN_S                           3

// Field: [2:0] TH
//
// Threshold adjust for quantizer. NOM is ~25mV.
//
// 0: Single threshold
// 1: NOM * 1/4
// 2: NOM * 2/4
// 3: NOM * 3/4
// 4: NOM
// 5: NOM * 5/4 - Default
// 6: NOM * 6/4
// 7: NOM * 7/4
#define ADI_0_RF_IFADCQUANT0_TH_M                                   0x00000007
#define ADI_0_RF_IFADCQUANT0_TH_S                                   0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCCTL1
//
//*****************************************************************************
// Field: [7:5] DITHERTRIM
//
// Adjust dither output current
//
// 0: 250.0 nA (Default)
// 1: 500.0 nA
// 2: 749.8 nA
// 3: 999.7 nA
// 4: 1249 nA
// 5: 1499 nA
// 6: 1749 nA
// 7: 1999 nA
#define ADI_0_RF_IFADCCTL1_DITHERTRIM_M                             0x000000E0
#define ADI_0_RF_IFADCCTL1_DITHERTRIM_S                             5

// Field: [4]    ADCIEN
//
// Enable I-channel IFADC
//
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFADCCTL1_ADCIEN                                   0x00000010
#define ADI_0_RF_IFADCCTL1_ADCIEN_BITN                              4
#define ADI_0_RF_IFADCCTL1_ADCIEN_M                                 0x00000010
#define ADI_0_RF_IFADCCTL1_ADCIEN_S                                 4
#define ADI_0_RF_IFADCCTL1_ADCIEN_DIS                               0x00000000
#define ADI_0_RF_IFADCCTL1_ADCIEN_EN                                0x00000010

// Field: [3]    ADCQEN
//
// Enable Q-channel IFADC
//
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFADCCTL1_ADCQEN                                   0x00000008
#define ADI_0_RF_IFADCCTL1_ADCQEN_BITN                              3
#define ADI_0_RF_IFADCCTL1_ADCQEN_M                                 0x00000008
#define ADI_0_RF_IFADCCTL1_ADCQEN_S                                 3
#define ADI_0_RF_IFADCCTL1_ADCQEN_DIS                               0x00000000
#define ADI_0_RF_IFADCCTL1_ADCQEN_EN                                0x00000008

// Field: [2:1] DITHEREN
//
// Dither algorithm select.
//
// This register is also used for integrity check of IFADC output test modes
// (IFADCCTL2.ADCLFSROUTEN=1). IFADC output bits in test mode is combined as
// following:
//
// IFADC output Q[1:0] = 10 (static) and IFADC output  I[1:0] is configured
// according to the list below (the right side is the resulting IFADC output):
//
// 0: 00
// 1: PRBS2d 0
// 2: 0 PRBS2
// 3: PRBS2d PRBS2
//
//
// ENUMs:
// DIS                  Disable dither (Default)
// ENS                  Single pseudo random sequence, white dither
// ENSD                 White noise dither, double power (same sequence on both
// outputs)
// ENG                  Gaussian dither, two different pseudo random sequences
// combined
//
#define ADI_0_RF_IFADCCTL1_DITHEREN_M                               0x00000006
#define ADI_0_RF_IFADCCTL1_DITHEREN_S                               1
#define ADI_0_RF_IFADCCTL1_DITHEREN_DIS                             0x00000000
#define ADI_0_RF_IFADCCTL1_DITHEREN_ENS                             0x00000002
#define ADI_0_RF_IFADCCTL1_DITHEREN_ENSD                            0x00000004
#define ADI_0_RF_IFADCCTL1_DITHEREN_ENG                             0x00000006

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFADCCTL2
//
//*****************************************************************************
// Field: [7]    RESETN
//
// Reset all digital blocks (active low).
// Must be done after the IFADC has been enabled to reset digital sub-IP. First
// enable the IFADC withIFADCCTL1.ADCIEN and/or  IFADCCTL1.ADCQEN. Then reset
// the device by pulling the signal low for at least one clock cycle, then back
// high.
// ENUMs:
// EN                   Disable
// DIS                  Enable (default)
//
#define ADI_0_RF_IFADCCTL2_RESETN                                   0x00000080
#define ADI_0_RF_IFADCCTL2_RESETN_BITN                              7
#define ADI_0_RF_IFADCCTL2_RESETN_M                                 0x00000080
#define ADI_0_RF_IFADCCTL2_RESETN_S                                 7
#define ADI_0_RF_IFADCCTL2_RESETN_EN                                0x00000000
#define ADI_0_RF_IFADCCTL2_RESETN_DIS                               0x00000080

// Field: [6]    CLKGENEN
//
// Enable clock generator module
// ENUMs:
// DIS                  Disable
// EN                   Enable (default)
//
#define ADI_0_RF_IFADCCTL2_CLKGENEN                                 0x00000040
#define ADI_0_RF_IFADCCTL2_CLKGENEN_BITN                            6
#define ADI_0_RF_IFADCCTL2_CLKGENEN_M                               0x00000040
#define ADI_0_RF_IFADCCTL2_CLKGENEN_S                               6
#define ADI_0_RF_IFADCCTL2_CLKGENEN_DIS                             0x00000000
#define ADI_0_RF_IFADCCTL2_CLKGENEN_EN                              0x00000040

// Field: [5]    ADCDIGCLKEN
//
// Enable clock output from IFADC to decimator
//
// ENUMs:
// DIS                  Disable
// EN                   Enable (default)
//
#define ADI_0_RF_IFADCCTL2_ADCDIGCLKEN                              0x00000020
#define ADI_0_RF_IFADCCTL2_ADCDIGCLKEN_BITN                         5
#define ADI_0_RF_IFADCCTL2_ADCDIGCLKEN_M                            0x00000020
#define ADI_0_RF_IFADCCTL2_ADCDIGCLKEN_S                            5
#define ADI_0_RF_IFADCCTL2_ADCDIGCLKEN_DIS                          0x00000000
#define ADI_0_RF_IFADCCTL2_ADCDIGCLKEN_EN                           0x00000020

// Field: [1]    INVCLKOUT
//
// Control phase inversion of IFADC clock output
// ENUMs:
// DIS                  Keep default IFADC output clock phase
// EN                   Invert IFADC output clock phase (default)
//
#define ADI_0_RF_IFADCCTL2_INVCLKOUT                                0x00000002
#define ADI_0_RF_IFADCCTL2_INVCLKOUT_BITN                           1
#define ADI_0_RF_IFADCCTL2_INVCLKOUT_M                              0x00000002
#define ADI_0_RF_IFADCCTL2_INVCLKOUT_S                              1
#define ADI_0_RF_IFADCCTL2_INVCLKOUT_DIS                            0x00000000
#define ADI_0_RF_IFADCCTL2_INVCLKOUT_EN                             0x00000002

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFALDO1
//
//*****************************************************************************
// Field: [4]    ERR_AMP_ZERO_EN
//
// Enables a zero in the LDO error amplifier's output to improve stability at the
// cost of bandwidth.
//
// 0: Disabled (Default)
// 1: Enabled
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFALDO1_ERR_AMP_ZERO_EN                            0x00000010
#define ADI_0_RF_IFALDO1_ERR_AMP_ZERO_EN_BITN                       4
#define ADI_0_RF_IFALDO1_ERR_AMP_ZERO_EN_M                          0x00000010
#define ADI_0_RF_IFALDO1_ERR_AMP_ZERO_EN_S                          4
#define ADI_0_RF_IFALDO1_ERR_AMP_ZERO_EN_DIS                        0x00000000
#define ADI_0_RF_IFALDO1_ERR_AMP_ZERO_EN_EN                         0x00000010

// Field: [3]    BYPASS_REG_EN
//
// Bypass LDO and short VDDR on LDO output to IFADC.
//
// 0: Disabled (Default)
// 1: Enabled (IFALDO1.EN must be enabled)
#define ADI_0_RF_IFALDO1_BYPASS_REG_EN                              0x00000008
#define ADI_0_RF_IFALDO1_BYPASS_REG_EN_BITN                         3
#define ADI_0_RF_IFALDO1_BYPASS_REG_EN_M                            0x00000008
#define ADI_0_RF_IFALDO1_BYPASS_REG_EN_S                            3

// Field: [2]    RDY_EN
//
// Enable LDO Ready Signal. Read result in STAT.IFLDOS_RDY.
//
// 0: Disable ready signal generation circuit
// 1: Enable ready signal generation circuit (IFALDO1.EN must be enabled)
// (Default)
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFALDO1_RDY_EN                                     0x00000004
#define ADI_0_RF_IFALDO1_RDY_EN_BITN                                2
#define ADI_0_RF_IFALDO1_RDY_EN_M                                   0x00000004
#define ADI_0_RF_IFALDO1_RDY_EN_S                                   2
#define ADI_0_RF_IFALDO1_RDY_EN_DIS                                 0x00000000
#define ADI_0_RF_IFALDO1_RDY_EN_EN                                  0x00000004

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
// ENUMs:
// EN                   Enable
// DIS                  Disable
//
#define ADI_0_RF_IFALDO1_BIAS_DIS                                   0x00000002
#define ADI_0_RF_IFALDO1_BIAS_DIS_BITN                              1
#define ADI_0_RF_IFALDO1_BIAS_DIS_M                                 0x00000002
#define ADI_0_RF_IFALDO1_BIAS_DIS_S                                 1
#define ADI_0_RF_IFALDO1_BIAS_DIS_EN                                0x00000000
#define ADI_0_RF_IFALDO1_BIAS_DIS_DIS                               0x00000002

// Field: [0]    EN
//
// Enable IFADC's regulator for analog blocks.
// ENUMs:
// DIS                  Disable
// EN                   Enable (Default)
//
#define ADI_0_RF_IFALDO1_EN                                         0x00000001
#define ADI_0_RF_IFALDO1_EN_BITN                                    0
#define ADI_0_RF_IFALDO1_EN_M                                       0x00000001
#define ADI_0_RF_IFALDO1_EN_S                                       0
#define ADI_0_RF_IFALDO1_EN_DIS                                     0x00000000
#define ADI_0_RF_IFALDO1_EN_EN                                      0x00000001

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFALDO2
//
//*****************************************************************************
// Field: [7:5] COMP_CAP
//
// Trim compensation Miller cap in linear steps of 0.5 pF. Default should be 0x6.
// Unsigned. Tradeoff stability for speed.
//
// 0x0: 3.5 pF (Maximum Capacitance)
// 0x1: 3.0 pF
// ...
// 0x6: 0.5 pF (Default)
// 0x7: Open Circuit / No Capacitance / No Compensation (regardless of
// IFALDO3.COMP_RES setting)
#define ADI_0_RF_IFALDO2_COMP_CAP_M                                 0x000000E0
#define ADI_0_RF_IFALDO2_COMP_CAP_S                                 5

// Field: [4:0] TRIM_OUT
//
// Trim LDO's output voltage linearly with a signed 2's complement number. Step
// size is approximately 8 mV.
//
// 0x10: -16 : 1.324V (Minimum Voltage)
// ...
// 0x1F: -1   : 1.395V
// 0x00: +0  : 1.403V (Default)
// 0x01: +1  : 1.407V
// ...
// 0x0F:+15 : 1.474V (Maximum Voltage)
//
//
#define ADI_0_RF_IFALDO2_TRIM_OUT_M                                 0x0000001F
#define ADI_0_RF_IFALDO2_TRIM_OUT_S                                 0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFALDO3
//
//*****************************************************************************
// Field: [4]    ATEST_V_EN
//
// Enables regulated output voltage to ATEST. Used to trim the LDO.
//
// 0: Disabled
// 1: Enabled
//
//
#define ADI_0_RF_IFALDO3_ATEST_V_EN                                 0x00000010
#define ADI_0_RF_IFALDO3_ATEST_V_EN_BITN                            4
#define ADI_0_RF_IFALDO3_ATEST_V_EN_M                               0x00000010
#define ADI_0_RF_IFALDO3_ATEST_V_EN_S                               4

// Field: [3]    ATEST_I_EN
//
// Enable test current (8.33% or 1/12 of LDO load current) to ATEST.
//
// 0: Disabled
// 1: Enabled
//
//
#define ADI_0_RF_IFALDO3_ATEST_I_EN                                 0x00000008
#define ADI_0_RF_IFALDO3_ATEST_I_EN_BITN                            3
#define ADI_0_RF_IFALDO3_ATEST_I_EN_M                               0x00000008
#define ADI_0_RF_IFALDO3_ATEST_I_EN_S                               3

// Field: [2:0] COMP_RES
//
// Trim compensation resistor in series with Miller cap in nonlinear steps. Default
// should be 0x0. Unsigned number.
//
// 0x0: 1066 ohms (Minimum Resistance, Default)
// 0x1: 1230 ohms
// 0x2: 1454 ohms
// 0x3: 1777 ohms
// 0x4: 2285 ohms
// 0x5: 3200 ohms
// 0x6: 5333 ohms
// 0x7: 16000 ohms (Maximum Resistance)
#define ADI_0_RF_IFALDO3_COMP_RES_M                                 0x00000007
#define ADI_0_RF_IFALDO3_COMP_RES_S                                 0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFDLDO1
//
//*****************************************************************************
// Field: [3]    BYPASS_REG_EN
//
// Bypass LDO and short VDDR to LDO's output to ADC.
//
// 0: Disabled (Default)
// 1: Enabled
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFDLDO1_BYPASS_REG_EN                              0x00000008
#define ADI_0_RF_IFDLDO1_BYPASS_REG_EN_BITN                         3
#define ADI_0_RF_IFDLDO1_BYPASS_REG_EN_M                            0x00000008
#define ADI_0_RF_IFDLDO1_BYPASS_REG_EN_S                            3
#define ADI_0_RF_IFDLDO1_BYPASS_REG_EN_DIS                          0x00000000
#define ADI_0_RF_IFDLDO1_BYPASS_REG_EN_EN                           0x00000008

// Field: [2]    RDY_EN
//
// Enable LDO Ready Signal. Read result in STAT.IFLDOS_RDY.
//
// 0: Disable ready signal generation circuit
// 1: Enable ready signal generation circuit (IFDIGLDO_EN must be enabled)
// (Default)
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFDLDO1_RDY_EN                                     0x00000004
#define ADI_0_RF_IFDLDO1_RDY_EN_BITN                                2
#define ADI_0_RF_IFDLDO1_RDY_EN_M                                   0x00000004
#define ADI_0_RF_IFDLDO1_RDY_EN_S                                   2
#define ADI_0_RF_IFDLDO1_RDY_EN_DIS                                 0x00000000
#define ADI_0_RF_IFDLDO1_RDY_EN_EN                                  0x00000004

// Field: [1]    BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
// ENUMs:
// EN                   Enable
// DIS                  Disable
//
#define ADI_0_RF_IFDLDO1_BIAS_DIS                                   0x00000002
#define ADI_0_RF_IFDLDO1_BIAS_DIS_BITN                              1
#define ADI_0_RF_IFDLDO1_BIAS_DIS_M                                 0x00000002
#define ADI_0_RF_IFDLDO1_BIAS_DIS_S                                 1
#define ADI_0_RF_IFDLDO1_BIAS_DIS_EN                                0x00000000
#define ADI_0_RF_IFDLDO1_BIAS_DIS_DIS                               0x00000002

// Field: [0]    EN
//
// Enable IFADC LDO for digital blocks.
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFDLDO1_EN                                         0x00000001
#define ADI_0_RF_IFDLDO1_EN_BITN                                    0
#define ADI_0_RF_IFDLDO1_EN_M                                       0x00000001
#define ADI_0_RF_IFDLDO1_EN_S                                       0
#define ADI_0_RF_IFDLDO1_EN_DIS                                     0x00000000
#define ADI_0_RF_IFDLDO1_EN_EN                                      0x00000001

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFDLDO2
//
//*****************************************************************************
// Field: [7:5] COMP_CAP
//
// Trim compensation Miller cap in linear steps of 0.5 pF. Default should be 0x6.
// Unsigned.
//
// 0x0: 3.5 pF (Maximum Capacitance)
// 0x1: 3.0 pF
// ...
// 0x6: 0.5 pF (Default)
// 0x7: Open Circuit / No Capacitance / No Compensation (regardless of
// IFDLDO3.COMP_RES setting)
#define ADI_0_RF_IFDLDO2_COMP_CAP_M                                 0x000000E0
#define ADI_0_RF_IFDLDO2_COMP_CAP_S                                 5

// Field: [4:0] TRIM_OUT
//
// Trim LDO's output voltage linearly with a signed 2's complement number. Step
// size is approximately 8 mV.
//
// 0x10: -16 : 1.128V (Minimum Voltage)
// ...
// 0x1F: -1   : 1.198V
// 0x00: +0  : 1.206V (Default)
// 0x01: +1  : 1.210V
// ...
// 0x0F:+15 : 1.277V (Maximum Voltage)
//
//
#define ADI_0_RF_IFDLDO2_TRIM_OUT_M                                 0x0000001F
#define ADI_0_RF_IFDLDO2_TRIM_OUT_S                                 0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFDLDO3
//
//*****************************************************************************
// Field: [4]    ATEST_V_EN
//
// Enables regulated output voltage to ATEST. Used to trim the LDO.
//
// 0: Disabled
// 1: Enabled
//
//
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFDLDO3_ATEST_V_EN                                 0x00000010
#define ADI_0_RF_IFDLDO3_ATEST_V_EN_BITN                            4
#define ADI_0_RF_IFDLDO3_ATEST_V_EN_M                               0x00000010
#define ADI_0_RF_IFDLDO3_ATEST_V_EN_S                               4
#define ADI_0_RF_IFDLDO3_ATEST_V_EN_DIS                             0x00000000
#define ADI_0_RF_IFDLDO3_ATEST_V_EN_EN                              0x00000010

// Field: [3]    ATEST_I_EN
//
// Enable test current (10% of LDO load current) to ATEST.
//
// 0: Disabled
// 1: Enabled
//
//
// ENUMs:
// DIS                  Disable
// EN                   Enable
//
#define ADI_0_RF_IFDLDO3_ATEST_I_EN                                 0x00000008
#define ADI_0_RF_IFDLDO3_ATEST_I_EN_BITN                            3
#define ADI_0_RF_IFDLDO3_ATEST_I_EN_M                               0x00000008
#define ADI_0_RF_IFDLDO3_ATEST_I_EN_S                               3
#define ADI_0_RF_IFDLDO3_ATEST_I_EN_DIS                             0x00000000
#define ADI_0_RF_IFDLDO3_ATEST_I_EN_EN                              0x00000008

// Field: [2:0] COMP_RES
//
// Trim compensation resistor in series with Miller cap in nonlinear steps. Default
// should be 0x0. Unsigned number.
//
// 0x0: 1066 ohms (Minimum Resistance, Default)
// 0x1: 1230 ohms
// 0x2: 1454 ohms
// 0x3: 1777 ohms
// 0x4: 2285 ohms
// 0x5: 3200 ohms
// 0x6: 5333 ohms
// 0x7: 16000 ohms (Maximum Resistance)
#define ADI_0_RF_IFDLDO3_COMP_RES_M                                 0x00000007
#define ADI_0_RF_IFDLDO3_COMP_RES_S                                 0

//*****************************************************************************
//
// Register: ADI_0_RF_O_IFAMPCTL3
//
//*****************************************************************************
// Field: [6:4] AAF_CAP_EN
//
// AAF capacitor control. Linear cap steps (ie. Non-linear BW steps)
//
// 0x0: Smallest cap, largest BW
// 0x7: Largest cap, narrowest BW
#define ADI_0_RF_IFAMPCTL3_AAF_CAP_EN_M                             0x00000070
#define ADI_0_RF_IFAMPCTL3_AAF_CAP_EN_S                             4

// Field: [3:0] IB
//
// IFAMP bias current control. Linear steps.
//
// 0x0: Min bias current
// 0x7: Default bias current
// 0xF: Max bias current
#define ADI_0_RF_IFAMPCTL3_IB_M                                     0x0000000F
#define ADI_0_RF_IFAMPCTL3_IB_S                                     0

//*****************************************************************************
//
// Register: ADI_0_RF_O_STAT
//
//*****************************************************************************
// Field: [7:3] IFADC_CALVAL_OUT
//
// Quant Cal values from either I or Q quantizer
//
// Use:
// IFADCTEST.QCALDBCSEL to select comparator,
// IFADCTEST.QCALDBIQSEL to select I/Q, and
// IFADCQUANT1.DBG_CAL_LEGSEL to select leg
#define ADI_0_RF_STAT_IFADC_CALVAL_OUT_M                            0x000000F8
#define ADI_0_RF_STAT_IFADC_CALVAL_OUT_S                            3

// Field: [2]    IFADC_CALDONE
//
// IFADC quantizer calibration done
#define ADI_0_RF_STAT_IFADC_CALDONE                                 0x00000004
#define ADI_0_RF_STAT_IFADC_CALDONE_BITN                            2
#define ADI_0_RF_STAT_IFADC_CALDONE_M                               0x00000004
#define ADI_0_RF_STAT_IFADC_CALDONE_S                               2

// Field: [1]    IFLDOS_RDY
//
// IFADC LDOs Ready (if only one is enabled the bit indicate the status of the
// enabled LDO)
//
//
#define ADI_0_RF_STAT_IFLDOS_RDY                                    0x00000002
#define ADI_0_RF_STAT_IFLDOS_RDY_BITN                               1
#define ADI_0_RF_STAT_IFLDOS_RDY_M                                  0x00000002
#define ADI_0_RF_STAT_IFLDOS_RDY_S                                  1

// Field: [0]    RF_LDO
//
// RF LDO Ready
#define ADI_0_RF_STAT_RF_LDO                                        0x00000001
#define ADI_0_RF_STAT_RF_LDO_BITN                                   0
#define ADI_0_RF_STAT_RF_LDO_M                                      0x00000001
#define ADI_0_RF_STAT_RF_LDO_S                                      0

#endif // __HW_ADI_0_RF_H__
