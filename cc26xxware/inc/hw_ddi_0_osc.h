/******************************************************************************
*  Filename:       hw_ddi_0_osc.h
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

#ifndef __HW_DDI_0_OSC_H__
#define __HW_DDI_0_OSC_H__


//*****************************************************************************
//
// This section defines the register offsets of
// DDI_0_OSC component
//
//*****************************************************************************
// Control 0
#define DDI_0_OSC_O_CTL0                                            0x00000000

// Comtrol 1
#define DDI_0_OSC_O_CTL1                                            0x00000004

// RADC External Configuration
#define DDI_0_OSC_O_RADCEXTCFG                                      0x00000008

// Amplitude Compensation Control
#define DDI_0_OSC_O_AMPCOMPCTL                                      0x0000000C

// Amplitude Compensation Threashold 1
#define DDI_0_OSC_O_AMPCOMPTH1                                      0x00000010

// Amplitude Compensation Threashold 2
#define DDI_0_OSC_O_AMPCOMPTH2                                      0x00000014

// Analog Bypass Values 1
#define DDI_0_OSC_O_ANABYPASSVAL1                                   0x00000018

// Internal register
#define DDI_0_OSC_O_ANABYPASSVAL2                                   0x0000001C

// Analog Test Control
#define DDI_0_OSC_O_ATESTCTL                                        0x00000020

// ADC Doubler Nanoamp Control
#define DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL                            0x00000024

// XOSCHF Control
#define DDI_0_OSC_O_XOSCHFCTL                                       0x00000028

// Low Frequency Oscillator Control
#define DDI_0_OSC_O_LFOSCCTL                                        0x0000002C

// RCOSCHF Control
#define DDI_0_OSC_O_RCOSCHFCTL                                      0x00000030

// Status 0
#define DDI_0_OSC_O_STAT0                                           0x00000034

// Status 1
#define DDI_0_OSC_O_STAT1                                           0x00000038

// Status 2
#define DDI_0_OSC_O_STAT2                                           0x0000003C

//*****************************************************************************
//
// Register: DDI_0_OSC_O_CTL0
//
//*****************************************************************************
// Field: [31]    XTAL_IS_24M
//
// Set based on the accurate high frequency XTAL or BAW.
//
// 0: 48MHz XTAL or 48MHz BAW
// 1: 24MHz XTAL
// ENUMs:
// 48M                  48MHz
// 24M                  24MHz
//
#define DDI_0_OSC_CTL0_XTAL_IS_24M                                  0x80000000
#define DDI_0_OSC_CTL0_XTAL_IS_24M_BITN                             31
#define DDI_0_OSC_CTL0_XTAL_IS_24M_M                                0x80000000
#define DDI_0_OSC_CTL0_XTAL_IS_24M_S                                31
#define DDI_0_OSC_CTL0_XTAL_IS_24M_48M                              0x00000000
#define DDI_0_OSC_CTL0_XTAL_IS_24M_24M                              0x80000000

// Field: [30]    DOUBLER_BYPASS_CTL
//
// When this bit is set, the XOSC_HF doubler is bypassed - i.e. the XOSC_HF clock
// is not double but is instead routed to the output of the doubler.
//
// 0: Hardware controls doubler bypass.
// 1: Doubler is bypassed
//
// The hardware will bypass the doubler when a 48MHz XTAL is connected to
// XOSC_HF.
//
#define DDI_0_OSC_CTL0_DOUBLER_BYPASS_CTL                           0x40000000
#define DDI_0_OSC_CTL0_DOUBLER_BYPASS_CTL_BITN                      30
#define DDI_0_OSC_CTL0_DOUBLER_BYPASS_CTL_M                         0x40000000
#define DDI_0_OSC_CTL0_DOUBLER_BYPASS_CTL_S                         30

// Field: [29]    BYPASS_XOSC_LF_CLK_QUAL
//
// Bypass XOSC_LF clock gating. Extremely useful not to get glitch on sclk_lf.
// Should be '1' once sclk_lf source is switched to xosc_lf.
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL                      0x20000000
#define DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_BITN                 29
#define DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_M                    0x20000000
#define DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_S                    29

// Field: [28]    BYPASS_RCOSC_LF_CLK_QUAL
//
// Override enable of clock gate that gates RCOSC_LF clock being fed to GF MUX.
// Extremely useful not to get glitch on sclk_lf. Should be '1' once sclk_lf source
// is switched to rcosc_lf
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL                     0x10000000
#define DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_BITN                28
#define DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_M                   0x10000000
#define DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_S                   28

// Field: [27:26] DOUBLER_START_DURATION
//
// Controls the Doubler startup duration. This is the time that  determines when
// the doubler output is good from the start of the doubler enable sequence. The
// time the doubler has to lock is determined by this bitfield and also
// DOUBLER_RESET_DURATION. DOUBLER_RESET_DURATION determines when reset ends -
// i.e. locking can start. This field determines when locking must complete. The
// allowable lock time is this setting minus the DOUBLER_RESET_DURATION setting.
//
// 0: 10 us minumum duration
// 1: 12 us minimum duration
// 2: 14 us minimum duration
// 3: 16 us minimum duration
//
//
#define DDI_0_OSC_CTL0_DOUBLER_START_DURATION_M                     0x0C000000
#define DDI_0_OSC_CTL0_DOUBLER_START_DURATION_S                     26

// Field: [25]    DOUBLER_RESET_DURATION
//
// Controls the doubler reset duration - the time that DOULBER_RESET and DOUBLER_EN
// are both active in the beginning of the doubler startup sequence.
//
// 0: 2 us minimum duration
// 1: 3 us minimum duration
//
//
#define DDI_0_OSC_CTL0_DOUBLER_RESET_DURATION                       0x02000000
#define DDI_0_OSC_CTL0_DOUBLER_RESET_DURATION_BITN                  25
#define DDI_0_OSC_CTL0_DOUBLER_RESET_DURATION_M                     0x02000000
#define DDI_0_OSC_CTL0_DOUBLER_RESET_DURATION_S                     25

// Field: [22]    FORCE_KICKSTART_EN
//
// Kickstart the rcosc_hf oscillator if the high-frequency system clock (sclk_hf)
// is enabled while the OSC_DIG  is in the HOLD state (no high frequency oscillator
// is enabled), and the source of sclk_lf is the xosc_hf.
//
// 0: Kickstart disabled
// 1: Kickstart enable.
//
// The sclk_hf source is selected by SCLK_HF_SRC_SEL. It is needed to ensure
// there are no system hangs on very rapid wakeup request following a powerdown
// request.
//
//
#define DDI_0_OSC_CTL0_FORCE_KICKSTART_EN                           0x00400000
#define DDI_0_OSC_CTL0_FORCE_KICKSTART_EN_BITN                      22
#define DDI_0_OSC_CTL0_FORCE_KICKSTART_EN_M                         0x00400000
#define DDI_0_OSC_CTL0_FORCE_KICKSTART_EN_S                         22

// Field: [16]    ALLOW_SCLK_HF_SWITCHING
//
// 0: Default - Switching of HF clock source is disabled .
// 1: Allows swtiching of sclk_hf source.
//
// Provided to prevent switching of the SCLK_HF source when running from flash
// (a long period during switching could corrupt flash). When sclk_hf  switching is
// disabled, a new source can be started when SCLK_HF_SRC_SEL is changed, but the
// switch will not occur until this bit is set.  This bit should be set to enable
// clock switching after STAT0.PENDINGSCLKHFSWITCHING indicates  the new HF clock
// is ready. When switching completes (also indicated by
// STAT0.PENDINGSCLKHFSWITCHING)  sclk_hf switching should be disabled to prevent
// flash corruption.  Switching should not be enabled when running from flash.
//
//
#define DDI_0_OSC_CTL0_ALLOW_SCLK_HF_SWITCHING                      0x00010000
#define DDI_0_OSC_CTL0_ALLOW_SCLK_HF_SWITCHING_BITN                 16
#define DDI_0_OSC_CTL0_ALLOW_SCLK_HF_SWITCHING_M                    0x00010000
#define DDI_0_OSC_CTL0_ALLOW_SCLK_HF_SWITCHING_S                    16

// Field: [14]    BAW_MODE_EN
//
// 0: Selects dtst_osc_clkin when OSC_DIG  is bypassed
// 1: Selects internal OSC_DIG _clk (RCOSC_HF or XOSC_HF derived 2 MHz)
#define DDI_0_OSC_CTL0_BAW_MODE_EN                                  0x00004000
#define DDI_0_OSC_CTL0_BAW_MODE_EN_BITN                             14
#define DDI_0_OSC_CTL0_BAW_MODE_EN_M                                0x00004000
#define DDI_0_OSC_CTL0_BAW_MODE_EN_S                                14

// Field: [12]    RCOSC_LF_TRIMMED
//
// Determines the accuracy at which RCOSC_LF_CLK is qualified. The RCOSC_LF_CLK is
// measured against the oscdig_clk (2MHz). This bit determines the acceptable ratio
// of  oscdig_clk periods per RCOSC_LF period.
//
// 0: RCOSC_LF_CLK is good if 6-200 2MHz periods per one RCOSC_LF period
// (intended for an untrimmed RCOSC_LF).
// 1: RCOSC_LF_CLK is good if 55-67 2MHz periods per one RCOSC_LF period
// (intended for a trimmed RCOSC_LF).
//
#define DDI_0_OSC_CTL0_RCOSC_LF_TRIMMED                             0x00001000
#define DDI_0_OSC_CTL0_RCOSC_LF_TRIMMED_BITN                        12
#define DDI_0_OSC_CTL0_RCOSC_LF_TRIMMED_M                           0x00001000
#define DDI_0_OSC_CTL0_RCOSC_LF_TRIMMED_S                           12

// Field: [11]    XOSC_HF_POWER_MODE
//
// XOSC/AMPCOMP mode.
//
// 0: High Power Mode (HPM)
// 1: Low Power Mode (LPM)
//
// Sets the XOSC_HF Power mode. If in the HPM_UPDATE (LPM_UPDATE)  state, the
// AMPCOMP FSM will transition to the (HPM_UPDATE) LPM_UPDATE state when this bit is
// set to a '1' ('0). If set to LPM when the XOSC_HF is started, the XOSC_HF will
// startup in High-Power Mode then transition to Low-Power Mode. STAT1.RAMPSTATE
// indicates the current FSM State. It can be polled to see that a transtion to the
// desired power mode has completed.
//
// Note well that XOSC_HF settings in AMPCOMPCTL, RADCEXTCFG,
// AMPCOMPTH1, and AMPCOMPTH2 must be set correctly, and are usually set by
// FW.
#define DDI_0_OSC_CTL0_XOSC_HF_POWER_MODE                           0x00000800
#define DDI_0_OSC_CTL0_XOSC_HF_POWER_MODE_BITN                      11
#define DDI_0_OSC_CTL0_XOSC_HF_POWER_MODE_M                         0x00000800
#define DDI_0_OSC_CTL0_XOSC_HF_POWER_MODE_S                         11

// Field: [10]    XOSC_LF_DIG_BYPASS
//
// Bypass XOSC_LF and use the digital input clock from AON for the xosc_lf clock..
//
// 0: Use 32kHz XOSC as xosc_lf clock source
// 1: Use digital input (from AON) as xosc_lf clock source.
//
// This bit will only have effect when SCLK_LF_SRC_SEL is selecting the
// xosc_lf as the sclk_lf source. The muxing performed by this bit is not glitch
// free. The following procedure should be followed when changing this field to
// avoid glitches on sclk_lf..
//
// 1) Set SCLK_LF_SRC_SEL to select any source other than the xosc_lf clock
// source.
// 2) Set or clear this bit to bypass or not bypass the xosc_lf.
// 3) Set SCLK_LF_SRC_SEL to use xosc_lf.
//
// It is recommended that either the rcosc_hf or xosc_hf (whichever is currently
// active) be selected as the source in step 1 above. This provides a faster clock
// change.
#define DDI_0_OSC_CTL0_XOSC_LF_DIG_BYPASS                           0x00000400
#define DDI_0_OSC_CTL0_XOSC_LF_DIG_BYPASS_BITN                      10
#define DDI_0_OSC_CTL0_XOSC_LF_DIG_BYPASS_M                         0x00000400
#define DDI_0_OSC_CTL0_XOSC_LF_DIG_BYPASS_S                         10

// Field: [9]    CLK_LOSS_EN
//
// Enable clock loss circuit and hence the indicators to system controller.  Checks
// both SCLK_HF and SCLK_LF clock loss indicators.
//
// 0: Disable
// 1: Enable
//
// Clock loss detection should be disabled when changing the sclk_lf source.
// STAT0.SCLK_LF_SRC can be polled to determine when a change to a new sclk_lf
// source has completed.
#define DDI_0_OSC_CTL0_CLK_LOSS_EN                                  0x00000200
#define DDI_0_OSC_CTL0_CLK_LOSS_EN_BITN                             9
#define DDI_0_OSC_CTL0_CLK_LOSS_EN_M                                0x00000200
#define DDI_0_OSC_CTL0_CLK_LOSS_EN_S                                9

// Field: [8:7] ACLK_TDC_SRC_SEL
//
// Source select for aclk_tdc.
//
// 00: rcosc_hf_clk
// 01: rcosc_hf_d24m_clk
// 10: xosc_hf_d24m_clk
// 11: Not valid
#define DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_M                           0x00000180
#define DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_S                           7

// Field: [6:5] ACLK_REF_SRC_SEL
//
// Source select for aclk_ref
//
// 00: rcosc_hf_dlf_clk
// 01: xosc_hf_dlf_clk
// 10: rcosc_lf_clk
// 11: xosc_lf_clk
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_M                           0x00000060
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_S                           5

// Field: [3:2] SCLK_LF_SRC_SEL
//
// Source select for sclk_lf
// ENUMs:
// RCOSCHFDLF           rcosc_hf_dlf_clk.
// Low frequency clock derived from High Frequency RCOSC
// XOSCHFDLF            xosc_hf_dlf_clk.
// Low frequency clock derived from High Frequency XOSC
// RCOSCLF              rcosc_lf_clk
// Low frequency RCOSC
// XOSCLF               xosc_lf_clk
// Low frequency XOSC
//
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_M                            0x0000000C
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_S                            2
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_RCOSCHFDLF                   0x00000000
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_XOSCHFDLF                    0x00000004
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_RCOSCLF                      0x00000008
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_XOSCLF                       0x0000000C

// Field: [1]    SCLK_MF_SRC_SEL
//
// Source select for sclk_mf
// ENUMs:
// RCOSCHFDMF           rcosc_hf_dmf_clk
// Medium frequency clock derived from high frequency RCOSC
// XCOSCHFDMF           xosc_hf_dmf_clk
// Medium frequency clock derived from high frequency XOSC or BAW.
//
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL                              0x00000002
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_BITN                         1
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_M                            0x00000002
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_S                            1
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_RCOSCHFDMF                   0x00000000
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL_XCOSCHFDMF                   0x00000002

// Field: [0]    SCLK_HF_SRC_SEL
//
// Source select for sclk_hf
// ENUMs:
// RCOSC                High frequency RCOSC clk
// XOSC                 High frequency XOSC or BAW clk (use BAW when
// BAW_MODE_EN = 1
//
#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL                              0x00000001
#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_BITN                         0
#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_M                            0x00000001
#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_S                            0
#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_RCOSC                        0x00000000
#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_XOSC                         0x00000001

//*****************************************************************************
//
// Register: DDI_0_OSC_O_CTL1
//
//*****************************************************************************
// Field: [22:18] RCOSCHFCTRIMFRACT
//
// Sets the fractional tuning of the RCOSC_HF capacitor trim. This field only has
// an effect if RCOSC_HF fractional trim is enabled via RCOSCHFCTRIMFRACT_EN.
// This field sets the duty cycle of the signal which enables the capacitor for
// fractional trimming. The field is an unsigned integer value. The duty cycle is
// RCOSCHFCTRIMFRACT/32. The fractional trim capacitor is 2X the size of the
// capacitors controlled by RCOSCHFCTL.RCOSCHF_CTRIM. The effective additional
// capacitance added to the RCOSC_HF is equal to the duty cycle times 2X. E.g. If
// this field is set to 16, then the duty cycle is 50% so the effective trim adds
// one capacitor to the RCOSC_HF capacitance. Setting this field to 8 gives a duty
// cycle of 25% which effectively adds 1/2 of a capacitor. The value of this field
// is calibrated and set via FW.  This field should only be changed when the
// fractional tuning is disabled or when the RCOSC_HF is off.
//
//
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_M                          0x007C0000
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_S                          18

// Field: [17]    RCOSCHFCTRIMFRACT_EN
//
// This field enables the fractional trimming of the RCOSC_HF captrim.
//
// 0: Disabled
// 1: Enabled
//
// When Fractional Trimming is enabled, the effective capacitance for tuning the
// RCOSC_HF frequency can be fractionally increased/decreased by defining the duty
// cycle of the signal that enables the RCOSC_HF Fractional Tuning Capacitor. The
// duty cycle is controlled by RCOSCHFCTRIMFRACT.
//
//
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN                         0x00020000
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_BITN                    17
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_M                       0x00020000
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_S                       17

// Field: [1:0] XOSC_HF_FAST_START
//
// Set precharge duration of fast startup of the XOSC_HF
//
// 00: Disable
// 01: 3 us
// 10: 5 us
// 11: 8 us
//
// This field enables and sets the duration of the XOSC_HF fast startup mode.
// This field will be set by firmware based on XTAL characteristics and other
// considerations.
#define DDI_0_OSC_CTL1_XOSC_HF_FAST_START_M                         0x00000003
#define DDI_0_OSC_CTL1_XOSC_HF_FAST_START_S                         0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_RADCEXTCFG
//
//*****************************************************************************
// Field: [31:22] HPM_IBIAS_WAIT_CNT
//
// AMPCOMP FSM waits for this count of OSC_DIG clock cycles in order to compensate
// the effect of slow crystal response. HPM_IBIAS_WAIT_CNT*0.5 us is absolute wait
// time and should be close to XTAL's response time towards ibias/cap change
#define DDI_0_OSC_RADCEXTCFG_HPM_IBIAS_WAIT_CNT_M                   0xFFC00000
#define DDI_0_OSC_RADCEXTCFG_HPM_IBIAS_WAIT_CNT_S                   22

// Field: [21:16] LPM_IBIAS_WAIT_CNT
//
// FSM waits for LPM_IBIAS_WAIT_CNT clock cycles in the IBIAS_INCREMENT state in
// order to compensate slow response of the xtal. Typical values = 64.
//
#define DDI_0_OSC_RADCEXTCFG_LPM_IBIAS_WAIT_CNT_M                   0x003F0000
#define DDI_0_OSC_RADCEXTCFG_LPM_IBIAS_WAIT_CNT_S                   16

// Field: [15:12] IDAC_STEP
//
// IDAC step size that will be used in IBIAS_CAP_UPDATE state. xosc_hf_idac is
// incremented IDAC_STEP times for each iteration of the loop that is performed
// during IBIAS_CAP_UPDATE. This setting is XTAL dependent and applied by FW.
//
//
#define DDI_0_OSC_RADCEXTCFG_IDAC_STEP_M                            0x0000F000
#define DDI_0_OSC_RADCEXTCFG_IDAC_STEP_S                            12

// Field: [11:6] RADC_DAC_TH
//
// RADC threshhold value when in comparator mode. Used when RADC_EXTERNAL_USE_EN
// = 1. RADC_DAC_TH is an unsigned integer input to the DAC that sets the voltage
// that goes to the compare input to the RADC comparator.
//
// The DAC voltage is ~ 0.0153*RADC_DAC_TH + 0.00765.
//
#define DDI_0_OSC_RADCEXTCFG_RADC_DAC_TH_M                          0x00000FC0
#define DDI_0_OSC_RADCEXTCFG_RADC_DAC_TH_S                          6

// Field: [5]    RADC_MODE_IS_SAR
//
// RADC mode when RADC_EXTERNAL_USE_EN = 1.
//
// 0: Comparator mode
// 1: SAR mode
//
#define DDI_0_OSC_RADCEXTCFG_RADC_MODE_IS_SAR                       0x00000020
#define DDI_0_OSC_RADCEXTCFG_RADC_MODE_IS_SAR_BITN                  5
#define DDI_0_OSC_RADCEXTCFG_RADC_MODE_IS_SAR_M                     0x00000020
#define DDI_0_OSC_RADCEXTCFG_RADC_MODE_IS_SAR_S                     5

// Field: [4]    RADC_START_CONV
//
// Start conversion signal when RADC_EXTERNAL_USE_EN = 1.
#define DDI_0_OSC_RADCEXTCFG_RADC_START_CONV                        0x00000010
#define DDI_0_OSC_RADCEXTCFG_RADC_START_CONV_BITN                   4
#define DDI_0_OSC_RADCEXTCFG_RADC_START_CONV_M                      0x00000010
#define DDI_0_OSC_RADCEXTCFG_RADC_START_CONV_S                      4

// Field: [1]    DDI_RADC_CLRZ
//
// Active low clrz for RADC_DIG.
//
// 0: Reset digital logic in the RADC.
// 1: Release reset of digital logic in RADC.
//
// Used when RADC_EXTERNAL_USE_EN = 1.
#define DDI_0_OSC_RADCEXTCFG_DDI_RADC_CLRZ                          0x00000002
#define DDI_0_OSC_RADCEXTCFG_DDI_RADC_CLRZ_BITN                     1
#define DDI_0_OSC_RADCEXTCFG_DDI_RADC_CLRZ_M                        0x00000002
#define DDI_0_OSC_RADCEXTCFG_DDI_RADC_CLRZ_S                        1

//*****************************************************************************
//
// Register: DDI_0_OSC_O_AMPCOMPCTL
//
//*****************************************************************************
// Field: [30]    AMPCOMP_REQ_MODE
//
// RADC mode during ampcomp_req from AON. FSM forces ADC to be in selected mode
// during ampcomp request (during amplitude maintainance).
//
// 0: Comparator
// 1: 1-time SAR mode
//
// This bit is not used during the ramping up or transitioning from one mode to
// other.
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_REQ_MODE                       0x40000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_REQ_MODE_BITN                  30
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_REQ_MODE_M                     0x40000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_REQ_MODE_S                     30

// Field: [29:28] AMPCOMP_FSM_UPDATE_RATE
//
// Run all XOSC input updates at 2M/1M/500K/250K.
//
// 00: 2 MHz
// 01: 1 MHz
// 10: 500KHz
// 11: 250KHz
// ENUMs:
// 2MHZ                 2 MHz ampcomp rate
// 1MHZ                 1 MHz ampcomp rate
// 500KHZ               500 kHz ampcomp rate
// 250KHZ               250 kHz ampcomp rate
//
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_FSM_UPDATE_RATE_M              0x30000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_FSM_UPDATE_RATE_S              28
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_FSM_UPDATE_RATE_2MHZ           0x00000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_FSM_UPDATE_RATE_1MHZ           0x10000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_FSM_UPDATE_RATE_500KHZ         0x20000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_FSM_UPDATE_RATE_250KHZ         0x30000000

// Field: [27]    AMPCOMP_SW_CTRL
//
// 0: OSC_DIG HW controls enabled amplitude compensation.
// 1: Give control to the software. SW is now responsible for start of ampcomp
// fsm.  Requires that AMPCOMP_SW_EN = 1.
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_CTRL                        0x08000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_CTRL_BITN                   27
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_CTRL_M                      0x08000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_CTRL_S                      27

// Field: [26]    AMPCOMP_SW_EN
//
// 0: Default
// 1: Starts ampcomp FSM
//
// Used when AMPCOMP_SW_CTRL = 1.
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_EN                          0x04000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_EN_BITN                     26
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_EN_M                        0x04000000
#define DDI_0_OSC_AMPCOMPCTL_AMPCOMP_SW_EN_S                        26

// Field: [25]    XOSC_HF_HP_BUF_SW_CTRL
//
// This field give SW control of the enabling of the XOSC_HF or BAW clock to the
// synth.
//
// 0: OSC_DIG  HW controls enabling the clock buffer to the synthesizer.
// 1: SW control enabling the clock buffer to the synthesizer.
//
// If this field = 1,. then XOSC_HF_HP_BUF_SW_EN  enables/disables the clock
// to the synthesizer. The user may find this useful if the synth clock is generated
// externally by a TXO on X48P pin.
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_CTRL                 0x02000000
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_CTRL_BITN            25
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_CTRL_M               0x02000000
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_CTRL_S               25

// Field: [24]    XOSC_HF_HP_BUF_SW_EN
//
// If XOSC_HF_HP_BUF_SW_CTRL = 0, then this bit has no effect.
//
// If XOSC_HF_HP_BUF_SW_CTRL = 1, this field controls the enable of the clock
// to the synthesizer.
//
// 0: Disable clock to the synthesizer.
// 1: Enable clock to the synthesizer.
//
// In XOSC_HF mode, this field activates the "high power" buffer in the XOSC_HF
// module to drive the clock to the synthesizer.
// In BAW mode, this field activates the BAW Clock Good signal to the BAW Module
// which enables the clock buffer that drives the clock to the synthesizer.
//
// The user may find this useful if the synth clock is generated externally by a
// TXO on X48P pin.
//
//
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_EN                   0x01000000
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_EN_BITN              24
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_EN_M                 0x01000000
#define DDI_0_OSC_AMPCOMPCTL_XOSC_HF_HP_BUF_SW_EN_S                 24

// Field: [23:20] IBIAS_OFFSET
//
// Offset (minimum) value of XOSC IBIAS trim.
// IBIAS trim value would always be greater than or equal to this offset in both
// HPM and LPM. The value is an unsigned integer. The setting is XTAL dependent and
// set by FW.
#define DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_M                         0x00F00000
#define DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S                         20

// Field: [19:16] IBIAS_INIT
//
// Value of XOSC IBIAS trim above the IBIAS_OFFSET for HPM. During ramp-up, IBIAS
// trim is set initially to the max value and then decreased to IBIAS_OFFSET +
// IBIAS_INIT on the way to HPM_UPDATE. The value is an unsigned integer. The
// setting is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_M                           0x000F0000
#define DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S                           16

// Field: [15:8] LPM_IBIAS_WAIT_CNT_FINAL
//
// FSM waits for LPM_IBIAS_WAIT_CNT_FINAL clock cycles in the
// IDAC_DECREMENT_WITH_MEASURE state in order to compensate slow response of the
// xtal. The value is an unsigned integer. The setting is XTAL dependent and set by
// FW.
#define DDI_0_OSC_AMPCOMPCTL_LPM_IBIAS_WAIT_CNT_FINAL_M             0x0000FF00
#define DDI_0_OSC_AMPCOMPCTL_LPM_IBIAS_WAIT_CNT_FINAL_S             8

// Field: [7:4] CAP_STEP
//
// Step size of XOSC capasitor trim (both Q1 and Q2) during XOSC mode transition.
// Can vary from 6 to 12. Other values are possible but not valid. The value is an
// unsigned integer. The setting is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPCTL_CAP_STEP_M                             0x000000F0
#define DDI_0_OSC_AMPCOMPCTL_CAP_STEP_S                             4

// Field: [3:0] IBIASCAP_HPTOLP_OL_CNT
//
// During a HPM to LPM transition a HW loop is entered that modifies cap, ibias and
// iDAC trims. This field sets the number of loop iterations. In each iteration the
// cap trim is decremented CAP_STEP times, then the iDAC trim is incremented
// RADCEXTCFG.IDAC_STEP times, and finally the ibias trim is decremented by 1.
// After IBIASCAP_HPTOLP_OL_CNT iterations, the cap trim is decremented to 0 (if
// greater than 0) and the iDAC trim is incremented to 96 (if less than 96). The
// setting of this field is XTAL dependant and set by FW.
//
#define DDI_0_OSC_AMPCOMPCTL_IBIASCAP_HPTOLP_OL_CNT_M               0x0000000F
#define DDI_0_OSC_AMPCOMPCTL_IBIASCAP_HPTOLP_OL_CNT_S               0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_AMPCOMPTH1
//
//*****************************************************************************
// Field: [23:18] HPMRAMP3_LTH
//
// HPM Ramp3 low amplitude threshhold.
//
// In HPM_UPDATE, if amplitude < HPMRAMP3_LTH increase iDAC trim, unless max
// trim has been reached.
//
// In IDAC_DECREMENT_WITH_MEASURE, exit the state if amplitude < HPMRAMP3_LTH.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_LTH_M                         0x00FC0000
#define DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_LTH_S                         18

// Field: [15:10] HPMRAMP3_HTH
//
// In HPM_RAMP3, if amp < HPMRAMP3_HTH then move to HPM_UPDATE.
//
// In HPM_UPDATE, if amplitude > HPMRAMP3_HTH decrease iDAC trim, unless min
// trim has been reached.
//
// In IDAC_DECREMENT_WITH_MEASURE, exit the state if amplitude < HPMRAMP3_HTH.
//
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_HTH_M                         0x0000FC00
#define DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_HTH_S                         10

// Field: [9:6] IBIASCAP_LPTOHP_OL_CNT
//
// During a LPM to HPM transition a HW loop is entered that modifies cap, and ibias
// trims. This field sets the number of loop iterations. In each iteration the cap
// trim is incremented AMPCOMPCTL.CAP_STEP times, then the ibias trim is
// decremented once.  After IBIASCAP_LPTOHP_OL_CNT iterations, the cap trim is
// incremented until the trim is equal to the setting defined by
// ANABYPASSVAL1.XOSC_HF_ROW_Q12 and ANABYPASSVAL1.XOSC_HF_COLUMN_Q12.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
//
//
//
#define DDI_0_OSC_AMPCOMPTH1_IBIASCAP_LPTOHP_OL_CNT_M               0x000003C0
#define DDI_0_OSC_AMPCOMPTH1_IBIASCAP_LPTOHP_OL_CNT_S               6

// Field: [5:0] HPMRAMP1_TH
//
// HPM Ramp1 amplitude threshhold.
// Wait in HPM_RAMP1 until amplitude > HPMRAMP1_TH.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
//
// The XOSCHF clock to the synth will be released after the amplitude of XOSC_HF
// passes HPMRAMP1_TH, as is the DOUBLER clock.
#define DDI_0_OSC_AMPCOMPTH1_HPMRAMP1_TH_M                          0x0000003F
#define DDI_0_OSC_AMPCOMPTH1_HPMRAMP1_TH_S                          0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_AMPCOMPTH2
//
//*****************************************************************************
// Field: [31:26] LPMUPDATE_LTH
//
// LPM Update low amplitude threshhold when RADC is in SAR mode.
//
// In the LPM_UPDATE state and RADC is in SAR Mode, increment iDAC trim (unless
// at max) if amplitude < LPMUPDATE_LTH.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
//
#define DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_LTH_M                        0xFC000000
#define DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_LTH_S                        26

// Field: [23:18] LPMUPDATE_HTH
//
// LPM update high amplitude threshhold when RADC is in SAR mode.
//
// In the LPM_UPDATE state  and RADC is in SAR Mode, decrement iDAC trim (unless
// at min) if amplitude > LPMUPDATE_HTH.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_HTH_M                        0x00FC0000
#define DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_HTH_S                        18

// Field: [15:10] ADC_COMP_AMPTH_LPM
//
// Low Power Mode Amplitude Threshold for Comparator mode.
// When ADC is in comparator mode, this field sets the amplitude threshold used
// during the LPM_UPDATE state (Low Power Mode amplitude maintenance).  The xosc_hf
// amplitude is periodically compared against the threshold. The iDAC bias is
// decremented if the amplitude is greater than the threshold, otherwise it is
// incremented.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_LPM_M                   0x0000FC00
#define DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_LPM_S                   10

// Field: [7:2] ADC_COMP_AMPTH_HPM
//
// High Power Mode Amplitude Threshold for Comparator mode.
// When ADC is in comparator mode, this field sets the amplitude threshold used
// during the HPM_UPDATE state (High Power Mode amplitude maintenance).  The xosc_hf
// amplitude is periodically compared against the threshold. The iDAC bias is
// decremented if the amplitude is greater than the threshold, otherwise it is
// incremented.
//
// The value is an unsigned integer.
// The settiing is XTAL dependent and set by FW.
#define DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_HPM_M                   0x000000FC
#define DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_HPM_S                   2

//*****************************************************************************
//
// Register: DDI_0_OSC_O_ANABYPASSVAL1
//
//*****************************************************************************
// Field: [19:16] XOSC_HF_ROW_Q12
//
// This field is used whether or not OSC_DIG  is bypassed.
//
// When OSC_DIG  is not bypassed this setting determines the cap_init_row value
// used in the High-Power Mode.
//
// This value is XTAL dependent and set by FW.
//
// When OSC_DIG  is bypassed this setting determines the xosc_hf capacitor bank
// row value
//
// The xosc_hf capacitor bank uses a row/column encoding. The row value is a
// thermometer encoded.
#define DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_ROW_Q12_M                   0x000F0000
#define DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_ROW_Q12_S                   16

// Field: [15:0] XOSC_HF_COLUMN_Q12
//
// This field is used whether or not OSC_DIG  is bypassed.
//
// When OSC_DIG  is not bypassed this setting determines the cap_init_column
// value used in HPM.
//
// This value is XTAL dependent and set by FW.
//
// When OSC_DIG  is bypassed this setting determines the target xosc_hf
// capacitor bank column value.
//
// The xosc_hf capacitor bank uses a row/column encoding. The column value is a
// thermometer encoded.
#define DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_COLUMN_Q12_M                0x0000FFFF
#define DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_COLUMN_Q12_S                0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_ANABYPASSVAL2
//
//*****************************************************************************
// Field: [13:0] XOSC_HF_IBIASTHERM
//
// Internal Register. Customers can control this through TI provided API
#define DDI_0_OSC_ANABYPASSVAL2_XOSC_HF_IBIASTHERM_M                0x00003FFF
#define DDI_0_OSC_ANABYPASSVAL2_XOSC_HF_IBIASTHERM_S                0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_ATESTCTL
//
//*****************************************************************************
// Field: [29]    SCLK_LF_AUX_EN
//
// Enable 32 kHz clock to SOC_AUX.
//
//
#define DDI_0_OSC_ATESTCTL_SCLK_LF_AUX_EN                           0x20000000
#define DDI_0_OSC_ATESTCTL_SCLK_LF_AUX_EN_BITN                      29
#define DDI_0_OSC_ATESTCTL_SCLK_LF_AUX_EN_M                         0x20000000
#define DDI_0_OSC_ATESTCTL_SCLK_LF_AUX_EN_S                         29

// Field: [28:20] ATEST_OSC_CTRL
//
// Select lines for top-level OSC_TOP ATEST mux. There are two outputs (ATEST1 and
// ATEST0)  and each has an independently controlled mux. The muxes use a one-hot
// encoding.
//
// Bits 23:20 select the source for the ATEST0 output as follows:
// 0x0: Not driven
// 0x1: OSCLFANA to ATEST0
// 0x2: OSCHFANA to ATEST0
// 0x4: RADC to ATEST0
// 0x8: Doubler to ATEST0
//
// Bits 28:24 select the source for the ATEST1 output as follows:
// 0x00: Not driven
// 0x01: OSCLFCLK to ATEST1  (output from OSCLF lower-level mux.
// ATESTLF_OSC_1P2V controls the lower-level mux)
// 0x02: OSCHFCLK to ATEST1 (ATEST_OSC_HF_SEL)
// 0x04: RADC to ATEST1
// 0x08: Doubler to ATEST1
// 0x10: Doubler clock to ATEST1
//
// ATEST is intended for debug and characterization. Programming ATEST correctly
// requires that paths are setup in the correct order. Incorrectly programmed paths
// can short signals together or to ground.
//
// A higher level of muxing (between OSC_TOP and other analog modules) and
// enabling exists and is controlled by ADI_4_AUX:ATEST. See the ATEST Design
// Memo for details.
//
//
#define DDI_0_OSC_ATESTCTL_ATEST_OSC_CTRL_M                         0x1FF00000
#define DDI_0_OSC_ATESTCTL_ATEST_OSC_CTRL_S                         20

// Field: [19:16] ATEST_OSC_HF_SEL
//
// Selects the source for high frequency oscillator test outputs.  Note, does not
// control muxing of BAW related test signals.  These are controled by
// ATESTLF_OSC_1P2V and ATESTLF_OSC_1P8V.
// Controls two test outputs ATEST_OSCHF_CLCKOUT_1P8V and ATEST_OSCHF_ANA_OUT.
//
// 0x0:
// ATEST_OSCHF_CLKOUT_1P8V is XOSC_HF LP clock output.
// ATEST_OSCHF_ANA_OUT is Not Driven.
//
// 0x1:
// ATEST_OSCHF_CLKOUT_1P8V is Not Driven.
// ATEST_OSCHF_ANA_OUT is XOSC_HF ATEST0 output.
//
// 0x2:
// ATEST_OSCHF_CLKOUT_1P8V is RCOSC_HF clock output.
// ATEST_OSCHF_ANA_OUT is Not Driven.
//
// 0x3:
// ATEST_OSCHF_CLKOUT_1P8V is RCOSC_HF clock output.
// ATEST_OSCHF_ANA_OUT is RCOSC_HF ATEST0 output.
//
// 0x4:
// ATEST_OSCHF_CLKOUT_1P8V is XOSC_HF HP clock output
// ATEST_OSCHF_ANA_OUT is Not Driven
//
// 0x5:
// ATEST_OSCHF_CLKOUT_1P8V is XOSC_HF HP clock output
// ATEST_OSCHF_ANA_OUT is XOSC ATEST0 output
//
//
#define DDI_0_OSC_ATESTCTL_ATEST_OSC_HF_SEL_M                       0x000F0000
#define DDI_0_OSC_ATESTCTL_ATEST_OSC_HF_SEL_S                       16

// Field: [7]    ATESTLF_RCOSCLF_IBIAS_TRIM
//
// Set high to increase the bias current to RCOSC_LF by 25 nA.  Nominal value is 50
// nA.  With this bit enabled current is 75 nA.
//
//
//
//
#define DDI_0_OSC_ATESTCTL_ATESTLF_RCOSCLF_IBIAS_TRIM               0x00000080
#define DDI_0_OSC_ATESTCTL_ATESTLF_RCOSCLF_IBIAS_TRIM_BITN          7
#define DDI_0_OSC_ATESTCTL_ATESTLF_RCOSCLF_IBIAS_TRIM_M             0x00000080
#define DDI_0_OSC_ATESTCTL_ATESTLF_RCOSCLF_IBIAS_TRIM_S             7

// Field: [6]    ATESTLF_UDIGLDO_IBIAS_TRIM
//
// Set high to enable an extra 25 nA of bias current to uDIGLDO (100% increase from
// nominal).
//
//
#define DDI_0_OSC_ATESTCTL_ATESTLF_UDIGLDO_IBIAS_TRIM               0x00000040
#define DDI_0_OSC_ATESTCTL_ATESTLF_UDIGLDO_IBIAS_TRIM_BITN          6
#define DDI_0_OSC_ATESTCTL_ATESTLF_UDIGLDO_IBIAS_TRIM_M             0x00000040
#define DDI_0_OSC_ATESTCTL_ATESTLF_UDIGLDO_IBIAS_TRIM_S             6

// Field: [5]    ATESTLF_SOXAUX_IBIAS_TRIM
//
// Set high to enable extra 25 nA to SOCAUX (100% increase from nominal).
//
//
#define DDI_0_OSC_ATESTCTL_ATESTLF_SOXAUX_IBIAS_TRIM                0x00000020
#define DDI_0_OSC_ATESTCTL_ATESTLF_SOXAUX_IBIAS_TRIM_BITN           5
#define DDI_0_OSC_ATESTCTL_ATESTLF_SOXAUX_IBIAS_TRIM_M              0x00000020
#define DDI_0_OSC_ATESTCTL_ATESTLF_SOXAUX_IBIAS_TRIM_S              5

// Field: [4:3] ATESTLF_OSC_1P2V
//
// Select 1.2V output test clock to drive the ATEST1 signal.
//
// 00: RCOSC_LF clock
// 01: BAW HF clock
// 10: XOSC_LF clock
//
// This field only selects the source to be driven. The output is enabled to
// drive ATEST1 by ATESTLF_EN.
//
//
#define DDI_0_OSC_ATESTCTL_ATESTLF_OSC_1P2V_M                       0x00000018
#define DDI_0_OSC_ATESTCTL_ATESTLF_OSC_1P2V_S                       3

// Field: [2:1] ATESTLF_OSC_1P8V
//
// Select the 1.8V test output to drive ATEST0.
//
// 00: BAW analog test
// 01: XOSC_LF analog test
// 10: NANOAMP_BIAS test current
// 11: RCOSC_LF VDD_LOCAL
//
// This field only selects the source to be driven. The output is enabled to
// drive ATEST0 by ATESTLF_EN.
//
//
#define DDI_0_OSC_ATESTCTL_ATESTLF_OSC_1P8V_M                       0x00000006
#define DDI_0_OSC_ATESTCTL_ATESTLF_OSC_1P8V_S                       1

// Field: [0]    ATESTLF_EN
//
// Enables the 1p8V test output selected by ATESTLF_OSC_1P8V.
//
// 0: Disable
// 1: Enable
//
//
#define DDI_0_OSC_ATESTCTL_ATESTLF_EN                               0x00000001
#define DDI_0_OSC_ATESTCTL_ATESTLF_EN_BITN                          0
#define DDI_0_OSC_ATESTCTL_ATESTLF_EN_M                             0x00000001
#define DDI_0_OSC_ATESTCTL_ATESTLF_EN_S                             0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL
//
//*****************************************************************************
// Field: [24]    NANOAMP_BIAS_ENABLE
//
// Enables NANOAMP_BIAS.
//
// 0: Disabled
// 1: Enabled
//
// Must be enabled for RCOSC_LF, XOSCLF, uDIGLDO, and SOCAUX to receive needed
// bias currents. This bit is normally set high after reset or after waking from
// Shutdown.
//
//
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_NANOAMP_BIAS_ENABLE          0x01000000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_NANOAMP_BIAS_ENABLE_BITN     24
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_NANOAMP_BIAS_ENABLE_M        0x01000000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_NANOAMP_BIAS_ENABLE_S        24

// Field: [23]    SPARE23
//
// Software should not rely on the value of a reserved. Writing any other value
// than the reset value may result in undefined behavior
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_SPARE23                      0x00800000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_SPARE23_BITN                 23
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_SPARE23_M                    0x00800000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_SPARE23_S                    23

// Field: [22]    DBLR_ATEST_SELECT
//
// Analog test select for Doubler
//
// 0: VCTRL Output (ATEST0=VCTRL, ATEST1=DOUBLER_RST)
// 1: Bias Output (ATEST0=VBP, ATEST1=VBN)
//
// Used when DBLR_ATEST_ENABLE = 1
// DOUBLER_RST allows control of DOUBLER RST signal via ATEST
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_SELECT            0x00400000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_SELECT_BITN       22
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_SELECT_M          0x00400000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_SELECT_S          22

// Field: [21]    DBLR_ATEST_ENABLE
//
// Enable ATEST for DBLR.  If enabled used DBLR_ATEST_SELECT to set ATEST bit.
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_ENABLE            0x00200000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_ENABLE_BITN       21
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_ENABLE_M          0x00200000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_ATEST_ENABLE_S          21

// Field: [20]    DBLR_TOOHI_MODE
//
// Not used.
//
//
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_TOOHI_MODE              0x00100000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_TOOHI_MODE_BITN         20
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_TOOHI_MODE_M            0x00100000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_TOOHI_MODE_S            20

// Field: [19]    DBLR_LOOP_FILTER_CAP
//
// Loop filter cap
//
// 0: 100% Nominal (17.5 pF)
// 1: 83% Nominal; (14.5 Pf)
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_CAP         0x00080000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_CAP_BITN    19
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_CAP_M       0x00080000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_CAP_S       19

// Field: [18:17] DBLR_LOOP_FILTER_RESET_VOLTAGE
//
// Loop filter reset voltage
//
// 10: 0.533*VDD = 0.666V
// 11: 0.615*VDD = 0.768V
// 00: 0.667*VDD = 0.833V
// 01: 0.800*VDD = 1.000V
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_RESET_VOLTAGE_M \
                                 0x00060000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_RESET_VOLTAGE_S 17

// Field: [16]    DOUBLER_BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DOUBLER_BIAS_DIS             0x00010000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DOUBLER_BIAS_DIS_BITN        16
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DOUBLER_BIAS_DIS_M           0x00010000
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DOUBLER_BIAS_DIS_S           16

// Field: [9]    RADC_BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_RADC_BIAS_DIS                0x00000200
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_RADC_BIAS_DIS_BITN           9
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_RADC_BIAS_DIS_M              0x00000200
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_RADC_BIAS_DIS_S              9

// Field: [8:7] ADC_ATEST_SRC
//
// Select ATEST0 muxed output from RADC mux. Mux is enabled by ADC_EN_ATEST.
//
// 0x0: Output of DAC - indicates if the current compare input is greater than
// the reference
// 0x1: COMP_IN (Comparator input signal after filtering and sample-hold)
// 0x2: FILTER_OUT (Comparator input signal after filtering - not sample-hold)
// 0x3: DAC_REF (DAC Reference Voltage - input is compared to this)
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_ATEST_SRC_M              0x00000180
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_ATEST_SRC_S              7

// Field: [6]    ADC_EN_ATEST
//
// Enable ATEST0 mux whose output is selected by the ADC_ATEST_SRC bitfield.
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_EN_ATEST                 0x00000040
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_EN_ATEST_BITN            6
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_EN_ATEST_M               0x00000040
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_EN_ATEST_S               6

// Field: [5]    ADC_SH_MODE_EN
//
// Enable S&H Mode
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_MODE_EN               0x00000020
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_MODE_EN_BITN          5
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_MODE_EN_M             0x00000020
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_MODE_EN_S             5

// Field: [4]    ADC_SH_VBUF_EN
//
// Enable S&H voltage buffer mode
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_VBUF_EN               0x00000010
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_VBUF_EN_BITN          4
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_VBUF_EN_M             0x00000010
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_VBUF_EN_S             4

// Field: [3:2] ADC_MUX_SEL
//
// Select line for 4-to-1 mux that feeds ADC input. Default is '00' Selects OSC_DIG
// input
//
// 00: Peak detector output
// 01: Vtemp
// 10: ATEST0
// 11: ATEST1
//
// Must be programmed to select the Peak detector output if xosc_hf is enabled
// so that amplitude compensation can be performed correctly so the clock is not
// lost.
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_MUX_SEL_M                0x0000000C
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_MUX_SEL_S                2

// Field: [1:0] ADC_IREF_CTRL
//
// Select RADC IBIAS source.
//
// 00: Use V2I
// 10: Connect external IREF to DAC
// 01: Use IREF 2uA from REFSYS for DAC
// 11: Reserved
//
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_IREF_CTRL_M              0x00000003
#define DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_IREF_CTRL_S              0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_XOSCHFCTL
//
//*****************************************************************************
// Field: [11]    HPBUFF_BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define DDI_0_OSC_XOSCHFCTL_HPBUFF_BIAS_DIS                         0x00000800
#define DDI_0_OSC_XOSCHFCTL_HPBUFF_BIAS_DIS_BITN                    11
#define DDI_0_OSC_XOSCHFCTL_HPBUFF_BIAS_DIS_M                       0x00000800
#define DDI_0_OSC_XOSCHFCTL_HPBUFF_BIAS_DIS_S                       11

// Field: [10]    PEAKDET_BIAS_DIS
//
// Disable dummy bias current.
//
// 0: Dummy bias current on (Default)
// 1: Dummy bias current off
//
//
#define DDI_0_OSC_XOSCHFCTL_PEAKDET_BIAS_DIS                        0x00000400
#define DDI_0_OSC_XOSCHFCTL_PEAKDET_BIAS_DIS_BITN                   10
#define DDI_0_OSC_XOSCHFCTL_PEAKDET_BIAS_DIS_M                      0x00000400
#define DDI_0_OSC_XOSCHFCTL_PEAKDET_BIAS_DIS_S                      10

// Field: [9:8] PEAK_DET_ITRIM
//
// Adjust Ibias trim of peak detector
//
// 00:  16 uA (default)
// 01:  20 uA
// 10:  24 uA
// 11:  28 uA
//
// This value is XTAL dependent and set by FW.
#define DDI_0_OSC_XOSCHFCTL_PEAK_DET_ITRIM_M                        0x00000300
#define DDI_0_OSC_XOSCHFCTL_PEAK_DET_ITRIM_S                        8

// Field: [7]    HP_ALT_BIAS
//
// Improve phase noise and reduce power consumption at the cost of reduced supply
// rejection.
//
// 0: Disable
// 1: Enable
#define DDI_0_OSC_XOSCHFCTL_HP_ALT_BIAS                             0x00000080
#define DDI_0_OSC_XOSCHFCTL_HP_ALT_BIAS_BITN                        7
#define DDI_0_OSC_XOSCHFCTL_HP_ALT_BIAS_M                           0x00000080
#define DDI_0_OSC_XOSCHFCTL_HP_ALT_BIAS_S                           7

// Field: [6]    BYPASS
//
// Bypass XOSC_HF core and pass through external clock from X48P.
//
//
#define DDI_0_OSC_XOSCHFCTL_BYPASS                                  0x00000040
#define DDI_0_OSC_XOSCHFCTL_BYPASS_BITN                             6
#define DDI_0_OSC_XOSCHFCTL_BYPASS_M                                0x00000040
#define DDI_0_OSC_XOSCHFCTL_BYPASS_S                                6

// Field: [5]    TESTMUX_EN
//
// Enable XOSC_HF ATEST Outputs
//
// 0: XOSC_HF ATEST outputs disabled
// 1: XOSC_HF ATEST outputs enabled.
//
// When enabled, the XOSC_HF COREREG bias current can be connected to drive
// ATEST0, and the HP Buffer clock or LP Buffer clock can be connected to drive
// ATEST1.
//
// This enable causes the XOSC_HF to drive the ATEST signals to a mux controlled
// by ATESTCTL.ATEST_OSC_HF_SEL. To drive these signals further on to the ATEST
// outputs, that mux must be programmed to select these inputs.
//
#define DDI_0_OSC_XOSCHFCTL_TESTMUX_EN                              0x00000020
#define DDI_0_OSC_XOSCHFCTL_TESTMUX_EN_BITN                         5
#define DDI_0_OSC_XOSCHFCTL_TESTMUX_EN_M                            0x00000020
#define DDI_0_OSC_XOSCHFCTL_TESTMUX_EN_S                            5

// Field: [4:2] HP_BUF_ITRIM
//
// Adjust Ibias trim for HP buffer.
//
// 0x4: Min (16 uA)
// 0x0: Default (64 uA)
// 0x3: Max (128 uA)
//
// This field uses a 2's complement encoding.
// This value is XTAL dependent and set by FW.
//
//
#define DDI_0_OSC_XOSCHFCTL_HP_BUF_ITRIM_M                          0x0000001C
#define DDI_0_OSC_XOSCHFCTL_HP_BUF_ITRIM_S                          2

// Field: [1:0] LP_BUF_ITRIM
//
// Adjust Ibias trim for LP buffer.
//
// 01: Min (700 nA)
// 00: Default (900 nA)
// 11: 1 step up (1.1 uA)
// 10: Max (2.0 uA)
//
// This value is XTAL dependent and set by FW
//
#define DDI_0_OSC_XOSCHFCTL_LP_BUF_ITRIM_M                          0x00000003
#define DDI_0_OSC_XOSCHFCTL_LP_BUF_ITRIM_S                          0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_LFOSCCTL
//
//*****************************************************************************
// Field: [26]    XOSCLF_BUFFER_TRIM
//
// Set high to double current mirror ratio in XOSCLF_LPBUFF (from 25 nA to 50 nA).
// Not normally used.
//
//
#define DDI_0_OSC_LFOSCCTL_XOSCLF_BUFFER_TRIM                       0x04000000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_BUFFER_TRIM_BITN                  26
#define DDI_0_OSC_LFOSCCTL_XOSCLF_BUFFER_TRIM_M                     0x04000000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_BUFFER_TRIM_S                     26

// Field: [25]    XOSCLF_TESTMUX_EN
//
// This enables ATEST0 and ATEST1 outputs from xosclf.
//
// 0: Diable (Default)
// 1: Enable
//
// This must be active to see the xosc_lf clock if it is selected by
// ATESTCTL.ATESTLF_OSC_1P2V onto ATEST1.
//
// This must be active to see the xosc_lf core regulator bias current on ATEST0.
//
//
#define DDI_0_OSC_LFOSCCTL_XOSCLF_TESTMUX_EN                        0x02000000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_TESTMUX_EN_BITN                   25
#define DDI_0_OSC_LFOSCCTL_XOSCLF_TESTMUX_EN_M                      0x02000000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_TESTMUX_EN_S                      25

// Field: [23:22] XOSCLF_REGULATOR_TRIM
//
// Trims resistor in constant gm bias
//
// 00: Default (225 k),
// 10: Min (425k),
// 01: Max (125k),
// 11: 1 step more than min (325k)
//
// This value is XTAL dependent and set by FW.
//
//
#define DDI_0_OSC_LFOSCCTL_XOSCLF_REGULATOR_TRIM_M                  0x00C00000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_REGULATOR_TRIM_S                  22

// Field: [21:18] XOSCLF_CMIRRWR_RATIO
//
// Adjust current mirror ratio into osc core
//
// 0x8: Min
// 0x0: Default
// 0x7: Max
//
// This field uses a 2's complement encoding.
// This value is XTAL dependent and set by FW
//
//
#define DDI_0_OSC_LFOSCCTL_XOSCLF_CMIRRWR_RATIO_M                   0x003C0000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_CMIRRWR_RATIO_S                   18

// Field: [17]    XOSCLF_ANA_AMP_CTRL
//
// Disables analog amplitude control
//
// 0: Enabled
// 1: Disabled
//
#define DDI_0_OSC_LFOSCCTL_XOSCLF_ANA_AMP_CTRL                      0x00020000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_ANA_AMP_CTRL_BITN                 17
#define DDI_0_OSC_LFOSCCTL_XOSCLF_ANA_AMP_CTRL_M                    0x00020000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_ANA_AMP_CTRL_S                    17

// Field: [16]    XOSCLF_RXTX_MODE
//
// Enables xosc_lf high-power buffer and disables the low-power-buffer.
//
// The high-power buffer provides better phase-noise but the use model doesn't
// require high performance of the xosc_lf.
//
// If the xosc_lf XTAL is not populated, an external clock can be driven on the
// xosc_lf_q1 pin to provide an external 32KHz source. This bit must be set to '1'
// to use this method. I.e. the external source must pass throught the xosc_lf
// high-power buffer.
//
// Note: This is a different method of providing a 32KHz external clock than the
// method enabled by CTL0.XOSC_LF_DIG_BYPASS
//
// The low power buffer is the buffer of choice and the use-model is that it is
// on whenever the xosc_lf is used. The high-power buffer is only expected to be
// used for the external clock.
//
//
#define DDI_0_OSC_LFOSCCTL_XOSCLF_RXTX_MODE                         0x00010000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_RXTX_MODE_BITN                    16
#define DDI_0_OSC_LFOSCCTL_XOSCLF_RXTX_MODE_M                       0x00010000
#define DDI_0_OSC_LFOSCCTL_XOSCLF_RXTX_MODE_S                       16

// Field: [11]    RCOSCLF_VDD_LOCAL_TRIM
//
// Not used.
//
//
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_VDD_LOCAL_TRIM                   0x00000800
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_VDD_LOCAL_TRIM_BITN              11
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_VDD_LOCAL_TRIM_M                 0x00000800
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_VDD_LOCAL_TRIM_S                 11

// Field: [10]    RCOSCLF_LOCAL_ATEST_EN
//
// Enable RCOSC_LF ATEST Outputs
//
// 0: RCOSC_LF ATEST outputs disabled
// 1: RCOSC_LF ATEST outputs enabled.
//
// When enabled, the RCOSC_LF 0P7V local supply and the RCOSC_LF clock can be
// connected to drive ATEST0 and ATEST1 (respectively).
//
// This enable causes the RCOSC_LF to drive the ATEST signals to muxes that do
// further selection. One mux selects the clock on ATEST1 and it is controled by
// ATESTCTL.ATESTLF_OSC_1P2V. The other mux selects the data and is controlled by
// ATESTCTL.ATESTLF_OSC_1P8V.
//
// To drive these signals further on to the ATEST outputs, these muxes must be
// programmed to select these inputs.
//
//
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_LOCAL_ATEST_EN                   0x00000400
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_LOCAL_ATEST_EN_BITN              10
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_LOCAL_ATEST_EN_M                 0x00000400
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_LOCAL_ATEST_EN_S                 10

// Field: [9:8] RCOSCLF_RTUNE_TRIM
//
// Trims the resistance in the RC 32 kHz osc to tune the osc frequency.
//
//
// ENUMs:
// 7P5MEG               7.5 MOhm (Default)
// 7P0MEG               7.0 MOhm
// 6P5MEG               6.5 MOhm
// 6P0MEG               6.0 MOhm
//
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_M                     0x00000300
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_S                     8
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_7P5MEG                0x00000000
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_7P0MEG                0x00000100
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_6P5MEG                0x00000200
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_6P0MEG                0x00000300

// Field: [7:0] RCOSCLF_CTUNE_TRIM
//
// Trim bits to tune the capacitance in the RC 32 kHz osc.
//
//
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_M                     0x000000FF
#define DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_S                     0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_RCOSCHFCTL
//
//*****************************************************************************
// Field: [15:8] RCOSCHF_CTRIM
//
// Capacitor Trim for RCOSC HF. The code 0xC0 gives the lowest frequency.
// The analog module inverts the two MSBs of this bit field internally.
//
// 1100000000: Min freq
// 0011111111: Max freq
//
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_M                        0x0000FF00
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_S                        8

// Field: [7]    SPARE7
//
// Software should not rely on the value of a reserved. Writing any other value
// than the reset value may result in undefined behavior
#define DDI_0_OSC_RCOSCHFCTL_SPARE7                                 0x00000080
#define DDI_0_OSC_RCOSCHFCTL_SPARE7_BITN                            7
#define DDI_0_OSC_RCOSCHFCTL_SPARE7_M                               0x00000080
#define DDI_0_OSC_RCOSCHFCTL_SPARE7_S                               7

// Field: [6]    ATEST_VDD_LOCAL_SEL
//
// Selects source of VDD_LOCAL - before or after the buffer
//
// 0: Selects buffered VDD_LOCAL
// 1: Selects unbuffered VDD_LOCAL directly from replica inverter
#define DDI_0_OSC_RCOSCHFCTL_ATEST_VDD_LOCAL_SEL                    0x00000040
#define DDI_0_OSC_RCOSCHFCTL_ATEST_VDD_LOCAL_SEL_BITN               6
#define DDI_0_OSC_RCOSCHFCTL_ATEST_VDD_LOCAL_SEL_M                  0x00000040
#define DDI_0_OSC_RCOSCHFCTL_ATEST_VDD_LOCAL_SEL_S                  6

// Field: [5]    RCOSCHF_ATEST_EN
//
// Enables ATEST outputs of RCOSC_HF.
//
// 0: RCOSC_HF does not drive ATEST signals.
// 1: RCOSC_HF_CLK drives ATEST1 and ATEST0 output is controlled by
// ATEST_VDD_LOCAL_SEL
//
// This enable causes the RCOSC_HF to drive the signals that go to a mux
// controlled by ATESTCTL.ATEST_OSC_HF_SEL.
// To drive these signals further on to the ATEST outputs, that mux must be
// programmed to select these inputs.
//
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_ATEST_EN                       0x00000020
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_ATEST_EN_BITN                  5
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_ATEST_EN_M                     0x00000020
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_ATEST_EN_S                     5

// Field: [3:0] RCOSCHF_ITUNE_TRIM
//
// Trim bias current used in local inverter in 500nA steps.
//
// This field uses a 2's complement encoding.
// 1000: 2.000 uA
// 1111: 5.500 uA
// 0000: 6.000 uA (default)
// 0001: 6.500 uA
// 0111: 9.500 uA
//
// Trims rcosc_hf vdd local regulator.
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_ITUNE_TRIM_M                   0x0000000F
#define DDI_0_OSC_RCOSCHFCTL_RCOSCHF_ITUNE_TRIM_S                   0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_STAT0
//
//*****************************************************************************
// Field: [30:29] SCLK_LF_SRC
//
// Indicates source for the sclk_lf
//
// 00: rcosc_hf_dlf
// 01: xosc_hf_dlf
// 10: rcosc_lf
// 11: xosc_lf
#define DDI_0_OSC_STAT0_SCLK_LF_SRC_M                               0x60000000
#define DDI_0_OSC_STAT0_SCLK_LF_SRC_S                               29

// Field: [28]    SCLK_HF_SRC
//
// Indicates source for the sclk_hf
//
// 0: rcosc_hf
// 1: xosc_hf
#define DDI_0_OSC_STAT0_SCLK_HF_SRC                                 0x10000000
#define DDI_0_OSC_STAT0_SCLK_HF_SRC_BITN                            28
#define DDI_0_OSC_STAT0_SCLK_HF_SRC_M                               0x10000000
#define DDI_0_OSC_STAT0_SCLK_HF_SRC_S                               28

// Field: [27]    BYPASS_OSCDIG
//
// indicates that OSC_DIG  is bypassed
#define DDI_0_OSC_STAT0_BYPASS_OSCDIG                               0x08000000
#define DDI_0_OSC_STAT0_BYPASS_OSCDIG_BITN                          27
#define DDI_0_OSC_STAT0_BYPASS_OSCDIG_M                             0x08000000
#define DDI_0_OSC_STAT0_BYPASS_OSCDIG_S                             27

// Field: [26]    BGAP_NEEDED
//
// bgap_needed request from OSC_DIG
#define DDI_0_OSC_STAT0_BGAP_NEEDED                                 0x04000000
#define DDI_0_OSC_STAT0_BGAP_NEEDED_BITN                            26
#define DDI_0_OSC_STAT0_BGAP_NEEDED_M                               0x04000000
#define DDI_0_OSC_STAT0_BGAP_NEEDED_S                               26

// Field: [25]    BGAP_RDY
//
// bgap_rdy -- input
#define DDI_0_OSC_STAT0_BGAP_RDY                                    0x02000000
#define DDI_0_OSC_STAT0_BGAP_RDY_BITN                               25
#define DDI_0_OSC_STAT0_BGAP_RDY_M                                  0x02000000
#define DDI_0_OSC_STAT0_BGAP_RDY_S                                  25

// Field: [24]    GBIAS_NEEDED
//
// gbias_needed request from OSC_DIG
#define DDI_0_OSC_STAT0_GBIAS_NEEDED                                0x01000000
#define DDI_0_OSC_STAT0_GBIAS_NEEDED_BITN                           24
#define DDI_0_OSC_STAT0_GBIAS_NEEDED_M                              0x01000000
#define DDI_0_OSC_STAT0_GBIAS_NEEDED_S                              24

// Field: [23]    GBIAS_RDY
//
// gbias_rdy -- input
#define DDI_0_OSC_STAT0_GBIAS_RDY                                   0x00800000
#define DDI_0_OSC_STAT0_GBIAS_RDY_BITN                              23
#define DDI_0_OSC_STAT0_GBIAS_RDY_M                                 0x00800000
#define DDI_0_OSC_STAT0_GBIAS_RDY_S                                 23

// Field: [22]    RCOSC_HF_EN
//
// RCOSC_HF_EN
#define DDI_0_OSC_STAT0_RCOSC_HF_EN                                 0x00400000
#define DDI_0_OSC_STAT0_RCOSC_HF_EN_BITN                            22
#define DDI_0_OSC_STAT0_RCOSC_HF_EN_M                               0x00400000
#define DDI_0_OSC_STAT0_RCOSC_HF_EN_S                               22

// Field: [21]    RCOSC_LF_EN
//
// RCOSC_LF_EN
#define DDI_0_OSC_STAT0_RCOSC_LF_EN                                 0x00200000
#define DDI_0_OSC_STAT0_RCOSC_LF_EN_BITN                            21
#define DDI_0_OSC_STAT0_RCOSC_LF_EN_M                               0x00200000
#define DDI_0_OSC_STAT0_RCOSC_LF_EN_S                               21

// Field: [20]    XOSC_LF_EN
//
// XOSC_LF_EN
#define DDI_0_OSC_STAT0_XOSC_LF_EN                                  0x00100000
#define DDI_0_OSC_STAT0_XOSC_LF_EN_BITN                             20
#define DDI_0_OSC_STAT0_XOSC_LF_EN_M                                0x00100000
#define DDI_0_OSC_STAT0_XOSC_LF_EN_S                                20

// Field: [19]    CLK_DCDC_RDY
//
// CLK_DCDC_RDY
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY                                0x00080000
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_BITN                           19
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_M                              0x00080000
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_S                              19

// Field: [18]    CLK_DCDC_RDY_ACK
//
// CLK_DCDC_RDY_ACK
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_ACK                            0x00040000
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_ACK_BITN                       18
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_ACK_M                          0x00040000
#define DDI_0_OSC_STAT0_CLK_DCDC_RDY_ACK_S                          18

// Field: [17]    SCLK_HF_LOSS
//
// Indicates sclk_hf is lost
#define DDI_0_OSC_STAT0_SCLK_HF_LOSS                                0x00020000
#define DDI_0_OSC_STAT0_SCLK_HF_LOSS_BITN                           17
#define DDI_0_OSC_STAT0_SCLK_HF_LOSS_M                              0x00020000
#define DDI_0_OSC_STAT0_SCLK_HF_LOSS_S                              17

// Field: [16]    SCLK_LF_LOSS
//
// Indicates sclk_lf is lost
#define DDI_0_OSC_STAT0_SCLK_LF_LOSS                                0x00010000
#define DDI_0_OSC_STAT0_SCLK_LF_LOSS_BITN                           16
#define DDI_0_OSC_STAT0_SCLK_LF_LOSS_M                              0x00010000
#define DDI_0_OSC_STAT0_SCLK_LF_LOSS_S                              16

// Field: [15]    XOSC_HF_EN
//
// Indicates that XOSC_HF is enable if not BAW Mode. Else indicates BAW is active.
#define DDI_0_OSC_STAT0_XOSC_HF_EN                                  0x00008000
#define DDI_0_OSC_STAT0_XOSC_HF_EN_BITN                             15
#define DDI_0_OSC_STAT0_XOSC_HF_EN_M                                0x00008000
#define DDI_0_OSC_STAT0_XOSC_HF_EN_S                                15

// Field: [14]    XOSC_HF_PEAK_DET_EN
//
// XOSC_HF_PEAK_DET_EN
#define DDI_0_OSC_STAT0_XOSC_HF_PEAK_DET_EN                         0x00004000
#define DDI_0_OSC_STAT0_XOSC_HF_PEAK_DET_EN_BITN                    14
#define DDI_0_OSC_STAT0_XOSC_HF_PEAK_DET_EN_M                       0x00004000
#define DDI_0_OSC_STAT0_XOSC_HF_PEAK_DET_EN_S                       14

// Field: [13]    XB_48M_CLK_EN
//
// Indicates that the 48MHz clock from the BAW or DOUBLER is enabled.
// If not in BAW Mode, indicates the DOUBLER is enabled. It will be enabled if
// 24 or 48 MHz chrystal is used (enabled in doulbler bypass for the 48MHz
// chrystal).
// If BAW Mode indicates that the 48MHz BAW clock is being used for a 48MHz
// clock source (SCLK_HF or CLK_DCDC).
//
//
#define DDI_0_OSC_STAT0_XB_48M_CLK_EN                               0x00002000
#define DDI_0_OSC_STAT0_XB_48M_CLK_EN_BITN                          13
#define DDI_0_OSC_STAT0_XB_48M_CLK_EN_M                             0x00002000
#define DDI_0_OSC_STAT0_XB_48M_CLK_EN_S                             13

// Field: [12]    ADC_EN
//
// ADC_EN
#define DDI_0_OSC_STAT0_ADC_EN                                      0x00001000
#define DDI_0_OSC_STAT0_ADC_EN_BITN                                 12
#define DDI_0_OSC_STAT0_ADC_EN_M                                    0x00001000
#define DDI_0_OSC_STAT0_ADC_EN_S                                    12

// Field: [11]    XOSC_HF_LP_BUF_EN
//
// XOSC_HF_LP_BUF_EN
#define DDI_0_OSC_STAT0_XOSC_HF_LP_BUF_EN                           0x00000800
#define DDI_0_OSC_STAT0_XOSC_HF_LP_BUF_EN_BITN                      11
#define DDI_0_OSC_STAT0_XOSC_HF_LP_BUF_EN_M                         0x00000800
#define DDI_0_OSC_STAT0_XOSC_HF_LP_BUF_EN_S                         11

// Field: [10]    XOSC_HF_HP_BUF_EN
//
// XOSC_HF_HP_BUF_EN
#define DDI_0_OSC_STAT0_XOSC_HF_HP_BUF_EN                           0x00000400
#define DDI_0_OSC_STAT0_XOSC_HF_HP_BUF_EN_BITN                      10
#define DDI_0_OSC_STAT0_XOSC_HF_HP_BUF_EN_M                         0x00000400
#define DDI_0_OSC_STAT0_XOSC_HF_HP_BUF_EN_S                         10

// Field: [9]    RADC_DIG_CLRZ
//
// RADC_DIG_CLRZ
#define DDI_0_OSC_STAT0_RADC_DIG_CLRZ                               0x00000200
#define DDI_0_OSC_STAT0_RADC_DIG_CLRZ_BITN                          9
#define DDI_0_OSC_STAT0_RADC_DIG_CLRZ_M                             0x00000200
#define DDI_0_OSC_STAT0_RADC_DIG_CLRZ_S                             9

// Field: [8]    ADC_THMET
//
// ADC_THMET
#define DDI_0_OSC_STAT0_ADC_THMET                                   0x00000100
#define DDI_0_OSC_STAT0_ADC_THMET_BITN                              8
#define DDI_0_OSC_STAT0_ADC_THMET_M                                 0x00000100
#define DDI_0_OSC_STAT0_ADC_THMET_S                                 8

// Field: [7]    ADC_DATA_READY
//
// indicates when adc_data is ready.
#define DDI_0_OSC_STAT0_ADC_DATA_READY                              0x00000080
#define DDI_0_OSC_STAT0_ADC_DATA_READY_BITN                         7
#define DDI_0_OSC_STAT0_ADC_DATA_READY_M                            0x00000080
#define DDI_0_OSC_STAT0_ADC_DATA_READY_S                            7

// Field: [6:1] ADC_DATA
//
// adc_data
#define DDI_0_OSC_STAT0_ADC_DATA_M                                  0x0000007E
#define DDI_0_OSC_STAT0_ADC_DATA_S                                  1

// Field: [0]    PENDINGSCLKHFSWITCHING
//
// Indicates when sclk_hf is ready to be swtiched
#define DDI_0_OSC_STAT0_PENDINGSCLKHFSWITCHING                      0x00000001
#define DDI_0_OSC_STAT0_PENDINGSCLKHFSWITCHING_BITN                 0
#define DDI_0_OSC_STAT0_PENDINGSCLKHFSWITCHING_M                    0x00000001
#define DDI_0_OSC_STAT0_PENDINGSCLKHFSWITCHING_S                    0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_STAT1
//
//*****************************************************************************
// Field: [31:28] RAMPSTATE
//
// AMPCOMP FSM State
// ENUMs:
// RESET                RESET
// INITIALIZATION       INITIALIZATION
// HPM_RAMP1            HPM_RAMP1
// HPM_RAMP2            HPM_RAMP2
// HPM_RAMP3            HPM_RAMP3
// HPM_UPDATE           HPM_UPDATE
// IDAC_INCREMENT       IDAC_INCREMENT
// IBIAS_CAP_UPDATE     IBIAS_CAP_UPDATE
// IBIAS_DEC_W_MEASURE  IBIAS_DECREMENT_WITH_MEASURE
// LPM_UPDATE           LPM_UPDATE
// IBIAS_INC            IBIAS_INCREMENT
// IDAC_DEC_W_MEASURE   IDAC_DECREMENT_WITH_MEASURE
// DUMMY_TO_INIT_1      DUMMY_TO_INIT_1
// FAST_START           FAST_START
// FAST_START_SETTLE    FAST_START_SETTLE
//
#define DDI_0_OSC_STAT1_RAMPSTATE_M                                 0xF0000000
#define DDI_0_OSC_STAT1_RAMPSTATE_S                                 28
#define DDI_0_OSC_STAT1_RAMPSTATE_RESET                             0x00000000
#define DDI_0_OSC_STAT1_RAMPSTATE_INITIALIZATION                    0x10000000
#define DDI_0_OSC_STAT1_RAMPSTATE_HPM_RAMP1                         0x20000000
#define DDI_0_OSC_STAT1_RAMPSTATE_HPM_RAMP2                         0x30000000
#define DDI_0_OSC_STAT1_RAMPSTATE_HPM_RAMP3                         0x40000000
#define DDI_0_OSC_STAT1_RAMPSTATE_HPM_UPDATE                        0x50000000
#define DDI_0_OSC_STAT1_RAMPSTATE_IDAC_INCREMENT                    0x60000000
#define DDI_0_OSC_STAT1_RAMPSTATE_IBIAS_CAP_UPDATE                  0x70000000
#define DDI_0_OSC_STAT1_RAMPSTATE_IBIAS_DEC_W_MEASURE               0x80000000
#define DDI_0_OSC_STAT1_RAMPSTATE_LPM_UPDATE                        0x90000000
#define DDI_0_OSC_STAT1_RAMPSTATE_IBIAS_INC                         0xA0000000
#define DDI_0_OSC_STAT1_RAMPSTATE_IDAC_DEC_W_MEASURE                0xB0000000
#define DDI_0_OSC_STAT1_RAMPSTATE_DUMMY_TO_INIT_1                   0xC0000000
#define DDI_0_OSC_STAT1_RAMPSTATE_FAST_START                        0xD0000000
#define DDI_0_OSC_STAT1_RAMPSTATE_FAST_START_SETTLE                 0xE0000000

// Field: [27:22] HMP_UPDATE_AMP
//
// OSC amplitude during HPM_UPDATE state.
// The vaue is an unsigned interger. It is used for debug only.
//
//
#define DDI_0_OSC_STAT1_HMP_UPDATE_AMP_M                            0x0FC00000
#define DDI_0_OSC_STAT1_HMP_UPDATE_AMP_S                            22

// Field: [21:16] LPM_UPDATE_AMP
//
// OSC amplitude during LPM_UPDATE state
// The vaue is an unsigned interger. It is used for debug only.
//
//
#define DDI_0_OSC_STAT1_LPM_UPDATE_AMP_M                            0x003F0000
#define DDI_0_OSC_STAT1_LPM_UPDATE_AMP_S                            16

// Field: [15]    FORCE_RCOSC_HF
//
// force_rcosc_hf
#define DDI_0_OSC_STAT1_FORCE_RCOSC_HF                              0x00008000
#define DDI_0_OSC_STAT1_FORCE_RCOSC_HF_BITN                         15
#define DDI_0_OSC_STAT1_FORCE_RCOSC_HF_M                            0x00008000
#define DDI_0_OSC_STAT1_FORCE_RCOSC_HF_S                            15

// Field: [14]    SCLK_HF_EN
//
// SCLK_HF_EN
#define DDI_0_OSC_STAT1_SCLK_HF_EN                                  0x00004000
#define DDI_0_OSC_STAT1_SCLK_HF_EN_BITN                             14
#define DDI_0_OSC_STAT1_SCLK_HF_EN_M                                0x00004000
#define DDI_0_OSC_STAT1_SCLK_HF_EN_S                                14

// Field: [13]    SCLK_MF_EN
//
// SCLK_MF_EN
#define DDI_0_OSC_STAT1_SCLK_MF_EN                                  0x00002000
#define DDI_0_OSC_STAT1_SCLK_MF_EN_BITN                             13
#define DDI_0_OSC_STAT1_SCLK_MF_EN_M                                0x00002000
#define DDI_0_OSC_STAT1_SCLK_MF_EN_S                                13

// Field: [12]    ACLK_ADC_EN
//
// ACLK_ADC_EN
#define DDI_0_OSC_STAT1_ACLK_ADC_EN                                 0x00001000
#define DDI_0_OSC_STAT1_ACLK_ADC_EN_BITN                            12
#define DDI_0_OSC_STAT1_ACLK_ADC_EN_M                               0x00001000
#define DDI_0_OSC_STAT1_ACLK_ADC_EN_S                               12

// Field: [11]    ACLK_TDC_EN
//
// ACLK_TDC_EN
#define DDI_0_OSC_STAT1_ACLK_TDC_EN                                 0x00000800
#define DDI_0_OSC_STAT1_ACLK_TDC_EN_BITN                            11
#define DDI_0_OSC_STAT1_ACLK_TDC_EN_M                               0x00000800
#define DDI_0_OSC_STAT1_ACLK_TDC_EN_S                               11

// Field: [10]    ACLK_REF_EN
//
// ACLK_REF_EN
#define DDI_0_OSC_STAT1_ACLK_REF_EN                                 0x00000400
#define DDI_0_OSC_STAT1_ACLK_REF_EN_BITN                            10
#define DDI_0_OSC_STAT1_ACLK_REF_EN_M                               0x00000400
#define DDI_0_OSC_STAT1_ACLK_REF_EN_S                               10

// Field: [9]    CLK_CHP_EN
//
// CLK_CHP_EN
#define DDI_0_OSC_STAT1_CLK_CHP_EN                                  0x00000200
#define DDI_0_OSC_STAT1_CLK_CHP_EN_BITN                             9
#define DDI_0_OSC_STAT1_CLK_CHP_EN_M                                0x00000200
#define DDI_0_OSC_STAT1_CLK_CHP_EN_S                                9

// Field: [8]    CLK_DCDC_EN
//
// CLK_DCDC_EN
#define DDI_0_OSC_STAT1_CLK_DCDC_EN                                 0x00000100
#define DDI_0_OSC_STAT1_CLK_DCDC_EN_BITN                            8
#define DDI_0_OSC_STAT1_CLK_DCDC_EN_M                               0x00000100
#define DDI_0_OSC_STAT1_CLK_DCDC_EN_S                               8

// Field: [7]    SCLK_HF_GOOD
//
// SCLK_HF_GOOD
#define DDI_0_OSC_STAT1_SCLK_HF_GOOD                                0x00000080
#define DDI_0_OSC_STAT1_SCLK_HF_GOOD_BITN                           7
#define DDI_0_OSC_STAT1_SCLK_HF_GOOD_M                              0x00000080
#define DDI_0_OSC_STAT1_SCLK_HF_GOOD_S                              7

// Field: [6]    SCLK_MF_GOOD
//
// SCLK_MF_GOOD
#define DDI_0_OSC_STAT1_SCLK_MF_GOOD                                0x00000040
#define DDI_0_OSC_STAT1_SCLK_MF_GOOD_BITN                           6
#define DDI_0_OSC_STAT1_SCLK_MF_GOOD_M                              0x00000040
#define DDI_0_OSC_STAT1_SCLK_MF_GOOD_S                              6

// Field: [5]    SCLK_LF_GOOD
//
// SCLK_LF_GOOD
#define DDI_0_OSC_STAT1_SCLK_LF_GOOD                                0x00000020
#define DDI_0_OSC_STAT1_SCLK_LF_GOOD_BITN                           5
#define DDI_0_OSC_STAT1_SCLK_LF_GOOD_M                              0x00000020
#define DDI_0_OSC_STAT1_SCLK_LF_GOOD_S                              5

// Field: [4]    ACLK_ADC_GOOD
//
// ACLK_ADC_GOOD
#define DDI_0_OSC_STAT1_ACLK_ADC_GOOD                               0x00000010
#define DDI_0_OSC_STAT1_ACLK_ADC_GOOD_BITN                          4
#define DDI_0_OSC_STAT1_ACLK_ADC_GOOD_M                             0x00000010
#define DDI_0_OSC_STAT1_ACLK_ADC_GOOD_S                             4

// Field: [3]    ACLK_TDC_GOOD
//
// ACLK_TDC_GOOD
#define DDI_0_OSC_STAT1_ACLK_TDC_GOOD                               0x00000008
#define DDI_0_OSC_STAT1_ACLK_TDC_GOOD_BITN                          3
#define DDI_0_OSC_STAT1_ACLK_TDC_GOOD_M                             0x00000008
#define DDI_0_OSC_STAT1_ACLK_TDC_GOOD_S                             3

// Field: [2]    ACLK_REF_GOOD
//
// ACLK_REF_GOOD
#define DDI_0_OSC_STAT1_ACLK_REF_GOOD                               0x00000004
#define DDI_0_OSC_STAT1_ACLK_REF_GOOD_BITN                          2
#define DDI_0_OSC_STAT1_ACLK_REF_GOOD_M                             0x00000004
#define DDI_0_OSC_STAT1_ACLK_REF_GOOD_S                             2

// Field: [1]    CLK_CHP_GOOD
//
// CLK_CHP_GOOD
#define DDI_0_OSC_STAT1_CLK_CHP_GOOD                                0x00000002
#define DDI_0_OSC_STAT1_CLK_CHP_GOOD_BITN                           1
#define DDI_0_OSC_STAT1_CLK_CHP_GOOD_M                              0x00000002
#define DDI_0_OSC_STAT1_CLK_CHP_GOOD_S                              1

// Field: [0]    CLK_DCDC_GOOD
//
// CLK_DCDC_GOOD
#define DDI_0_OSC_STAT1_CLK_DCDC_GOOD                               0x00000001
#define DDI_0_OSC_STAT1_CLK_DCDC_GOOD_BITN                          0
#define DDI_0_OSC_STAT1_CLK_DCDC_GOOD_M                             0x00000001
#define DDI_0_OSC_STAT1_CLK_DCDC_GOOD_S                             0

//*****************************************************************************
//
// Register: DDI_0_OSC_O_STAT2
//
//*****************************************************************************
// Field: [31:26] ADC_DCBIAS
//
// DC Bias read by RADC during SAR mode
// The vaue is an unsigned interger. It is used for debug only.
//
//
#define DDI_0_OSC_STAT2_ADC_DCBIAS_M                                0xFC000000
#define DDI_0_OSC_STAT2_ADC_DCBIAS_S                                26

// Field: [25]    HPM_RAMP1_THMET
//
// Indication of threshhold is met for hpm_ramp1
#define DDI_0_OSC_STAT2_HPM_RAMP1_THMET                             0x02000000
#define DDI_0_OSC_STAT2_HPM_RAMP1_THMET_BITN                        25
#define DDI_0_OSC_STAT2_HPM_RAMP1_THMET_M                           0x02000000
#define DDI_0_OSC_STAT2_HPM_RAMP1_THMET_S                           25

// Field: [24]    HPM_RAMP2_THMET
//
// Indication of threshhold is met for hpm_ramp2
#define DDI_0_OSC_STAT2_HPM_RAMP2_THMET                             0x01000000
#define DDI_0_OSC_STAT2_HPM_RAMP2_THMET_BITN                        24
#define DDI_0_OSC_STAT2_HPM_RAMP2_THMET_M                           0x01000000
#define DDI_0_OSC_STAT2_HPM_RAMP2_THMET_S                           24

// Field: [23]    HPM_RAMP3_THMET
//
// Indication of threshhold is met for hpm_ramp3
#define DDI_0_OSC_STAT2_HPM_RAMP3_THMET                             0x00800000
#define DDI_0_OSC_STAT2_HPM_RAMP3_THMET_BITN                        23
#define DDI_0_OSC_STAT2_HPM_RAMP3_THMET_M                           0x00800000
#define DDI_0_OSC_STAT2_HPM_RAMP3_THMET_S                           23

// Field: [22]    IBIAS_DEC_WITH_MEASURE_DONE
//
// Condition to exit ibias_dec_with_measure is met
#define DDI_0_OSC_STAT2_IBIAS_DEC_WITH_MEASURE_DONE                 0x00400000
#define DDI_0_OSC_STAT2_IBIAS_DEC_WITH_MEASURE_DONE_BITN            22
#define DDI_0_OSC_STAT2_IBIAS_DEC_WITH_MEASURE_DONE_M               0x00400000
#define DDI_0_OSC_STAT2_IBIAS_DEC_WITH_MEASURE_DONE_S               22

// Field: [21]    IBIAS_WAIT_CNTR_DONE
//
// Condition to exit ibias_wait_cntr is met
#define DDI_0_OSC_STAT2_IBIAS_WAIT_CNTR_DONE                        0x00200000
#define DDI_0_OSC_STAT2_IBIAS_WAIT_CNTR_DONE_BITN                   21
#define DDI_0_OSC_STAT2_IBIAS_WAIT_CNTR_DONE_M                      0x00200000
#define DDI_0_OSC_STAT2_IBIAS_WAIT_CNTR_DONE_S                      21

// Field: [20]    IDAC_INCREMENT_DONE
//
// Condition to exit idac_increment is met
#define DDI_0_OSC_STAT2_IDAC_INCREMENT_DONE                         0x00100000
#define DDI_0_OSC_STAT2_IDAC_INCREMENT_DONE_BITN                    20
#define DDI_0_OSC_STAT2_IDAC_INCREMENT_DONE_M                       0x00100000
#define DDI_0_OSC_STAT2_IDAC_INCREMENT_DONE_S                       20

// Field: [19]    IBIAS_CAP_UPDATE_DONE
//
// Condition to exit ibias_cap_updated is met
#define DDI_0_OSC_STAT2_IBIAS_CAP_UPDATE_DONE                       0x00080000
#define DDI_0_OSC_STAT2_IBIAS_CAP_UPDATE_DONE_BITN                  19
#define DDI_0_OSC_STAT2_IBIAS_CAP_UPDATE_DONE_M                     0x00080000
#define DDI_0_OSC_STAT2_IBIAS_CAP_UPDATE_DONE_S                     19

// Field: [18]    IDAC_DECREMENT_WITH_MEASURE_DONE
//
// Condition to exit idac_dec_with_measure is met
#define DDI_0_OSC_STAT2_IDAC_DECREMENT_WITH_MEASURE_DONE            0x00040000
#define DDI_0_OSC_STAT2_IDAC_DECREMENT_WITH_MEASURE_DONE_BITN       18
#define DDI_0_OSC_STAT2_IDAC_DECREMENT_WITH_MEASURE_DONE_M          0x00040000
#define DDI_0_OSC_STAT2_IDAC_DECREMENT_WITH_MEASURE_DONE_S          18

// Field: [17]    IBIAS_INCREMENT_DONE
//
// Condition to exit ibias_inc is met
#define DDI_0_OSC_STAT2_IBIAS_INCREMENT_DONE                        0x00020000
#define DDI_0_OSC_STAT2_IBIAS_INCREMENT_DONE_BITN                   17
#define DDI_0_OSC_STAT2_IBIAS_INCREMENT_DONE_M                      0x00020000
#define DDI_0_OSC_STAT2_IBIAS_INCREMENT_DONE_S                      17

// Field: [16]    RAMP_DOWN_TO_INIT_DONE
//
// counter for ramp down to initi is done. Use for debug
#define DDI_0_OSC_STAT2_RAMP_DOWN_TO_INIT_DONE                      0x00010000
#define DDI_0_OSC_STAT2_RAMP_DOWN_TO_INIT_DONE_BITN                 16
#define DDI_0_OSC_STAT2_RAMP_DOWN_TO_INIT_DONE_M                    0x00010000
#define DDI_0_OSC_STAT2_RAMP_DOWN_TO_INIT_DONE_S                    16

// Field: [15:12] RAMPSTATE
//
// xosc_hf amplitude compensation FSM
//
// This is identical to STAT1.RAMPSTATE. See that description for encoding.
//
#define DDI_0_OSC_STAT2_RAMPSTATE_M                                 0x0000F000
#define DDI_0_OSC_STAT2_RAMPSTATE_S                                 12

// Field: [11:9] ADCSTATE
//
// ADC FSM State.
// Used by amplitude compensation FSM.
// ENUMs:
// ADC_RESET            ADC_RESET
// ADC_IDLE             ADC_IDLE
// ADC_SC               ADC_SC
// ADC_RDDCB            ADC_RDDCB
// ADC_RDAMP            ADC_RDAMP
// ADC_CALC             ADC_CALC
//
#define DDI_0_OSC_STAT2_ADCSTATE_M                                  0x00000E00
#define DDI_0_OSC_STAT2_ADCSTATE_S                                  9
#define DDI_0_OSC_STAT2_ADCSTATE_ADC_RESET                          0x00000000
#define DDI_0_OSC_STAT2_ADCSTATE_ADC_IDLE                           0x00000200
#define DDI_0_OSC_STAT2_ADCSTATE_ADC_SC                             0x00000400
#define DDI_0_OSC_STAT2_ADCSTATE_ADC_RDDCB                          0x00000600
#define DDI_0_OSC_STAT2_ADCSTATE_ADC_RDAMP                          0x00000800
#define DDI_0_OSC_STAT2_ADCSTATE_ADC_CALC                           0x00000A00

// Field: [8]    ADC_COMP_P
//
// LDO Status in BAW mode and comparator output in regular mode. Should be ignored
// in regular mode
//
//
#define DDI_0_OSC_STAT2_ADC_COMP_P                                  0x00000100
#define DDI_0_OSC_STAT2_ADC_COMP_P_BITN                             8
#define DDI_0_OSC_STAT2_ADC_COMP_P_M                                0x00000100
#define DDI_0_OSC_STAT2_ADC_COMP_P_S                                8

// Field: [7]    ADC_COMP_M
//
// BAW_CLKGOOD in BAW mode and not used in regular mode
//
//
#define DDI_0_OSC_STAT2_ADC_COMP_M                                  0x00000080
#define DDI_0_OSC_STAT2_ADC_COMP_M_BITN                             7
#define DDI_0_OSC_STAT2_ADC_COMP_M_M                                0x00000080
#define DDI_0_OSC_STAT2_ADC_COMP_M_S                                7

// Field: [6:4] AMPCOMP_OF_UF
//
// overflow and underflow status in FSM
//
// 0x0: No overlow or underflow
// 0x1: IDAC underflow in HPM_RAMP3
// 0x2: IDAC underflow in IDAC_DECREMENT_WITH_MEASURE.
// 0x4: IDAC underflow or overflow in HPM_UPDATE or LPM_UPDATE
#define DDI_0_OSC_STAT2_AMPCOMP_OF_UF_M                             0x00000070
#define DDI_0_OSC_STAT2_AMPCOMP_OF_UF_S                             4

// Field: [3]    AMPCOMP_REQ
//
// ampcomp_req
#define DDI_0_OSC_STAT2_AMPCOMP_REQ                                 0x00000008
#define DDI_0_OSC_STAT2_AMPCOMP_REQ_BITN                            3
#define DDI_0_OSC_STAT2_AMPCOMP_REQ_M                               0x00000008
#define DDI_0_OSC_STAT2_AMPCOMP_REQ_S                               3

// Field: [2]    XOSC_HF_AMPGOOD
//
// amplitude of xosc_hf is within the required threshold (set by DDI). Not used for
// anything just for debug/status
#define DDI_0_OSC_STAT2_XOSC_HF_AMPGOOD                             0x00000004
#define DDI_0_OSC_STAT2_XOSC_HF_AMPGOOD_BITN                        2
#define DDI_0_OSC_STAT2_XOSC_HF_AMPGOOD_M                           0x00000004
#define DDI_0_OSC_STAT2_XOSC_HF_AMPGOOD_S                           2

// Field: [1]    XOSC_HF_FREQGOOD
//
// frequency of xosc_hf is good to use for the digital clocks
#define DDI_0_OSC_STAT2_XOSC_HF_FREQGOOD                            0x00000002
#define DDI_0_OSC_STAT2_XOSC_HF_FREQGOOD_BITN                       1
#define DDI_0_OSC_STAT2_XOSC_HF_FREQGOOD_M                          0x00000002
#define DDI_0_OSC_STAT2_XOSC_HF_FREQGOOD_S                          1

// Field: [0]    XOSC_HF_RF_FREQGOOD
//
// frequency of xosc_hf is within +/- 20 ppm and xosc_hf is good for radio
// operations. Used for SW to start synthesizer.
#define DDI_0_OSC_STAT2_XOSC_HF_RF_FREQGOOD                         0x00000001
#define DDI_0_OSC_STAT2_XOSC_HF_RF_FREQGOOD_BITN                    0
#define DDI_0_OSC_STAT2_XOSC_HF_RF_FREQGOOD_M                       0x00000001
#define DDI_0_OSC_STAT2_XOSC_HF_RF_FREQGOOD_S                       0

#endif // __HW_DDI_0_OSC_H__
