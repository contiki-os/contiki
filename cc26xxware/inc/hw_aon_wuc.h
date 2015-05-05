/******************************************************************************
*  Filename:       hw_aon_wuc.h
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

#ifndef __HW_AON_WUC_H__
#define __HW_AON_WUC_H__


//*****************************************************************************
//
// This section defines the register offsets of
// AON_WUC component
//
//*****************************************************************************
// MCU Clock Management
#define AON_WUC_O_MCUCLK                                            0x00000000

// AUX Clock Management
#define AON_WUC_O_AUXCLK                                            0x00000004

// MCU Configuration
#define AON_WUC_O_MCUCFG                                            0x00000008

// AUX Configuration
#define AON_WUC_O_AUXCFG                                            0x0000000C

// AUX Control
#define AON_WUC_O_AUXCTL                                            0x00000010

// Power Status Register
#define AON_WUC_O_PWRSTAT                                           0x00000014

// Shutdown Control
#define AON_WUC_O_SHUTDOWN                                          0x00000018

// General Configuration
#define AON_WUC_O_CTL0                                              0x00000020

// General Control Register
#define AON_WUC_O_CTL1                                              0x00000024

// Recharge Controller Configuration
#define AON_WUC_O_RECHARGECFG                                       0x00000030

// Recharge Controller Status
#define AON_WUC_O_RECHARGESTAT                                      0x00000034

// Oscillator Configuration
#define AON_WUC_O_OSCCFG                                            0x00000038

// JTAG Configuration
#define AON_WUC_O_JTAGCFG                                           0x00000040

// JTAG USERCODE
#define AON_WUC_O_JTAGUSERCODE                                      0x00000044

//*****************************************************************************
//
// Register: AON_WUC_O_MCUCLK
//
//*****************************************************************************
// Field: [2]    RCOSC_HF_CAL_DONE
//
// MCU bootcode will set this bit when RCOSC_HF is calibrated.  The FLASH can not
// be used until this bit is set.
//
// 1: RCOSC_HF is calibrated to 48 MHz, allowing FLASH to power up.
// 0: RCOSC_HF is not yet calibrated, ie FLASH must not assume that the SCLK_HF
// is safe
//
//
#define AON_WUC_MCUCLK_RCOSC_HF_CAL_DONE                            0x00000004
#define AON_WUC_MCUCLK_RCOSC_HF_CAL_DONE_BITN                       2
#define AON_WUC_MCUCLK_RCOSC_HF_CAL_DONE_M                          0x00000004
#define AON_WUC_MCUCLK_RCOSC_HF_CAL_DONE_S                          2

// Field: [1:0] PWR_DWN_SRC
//
// Controls the clock source for the entire MCU domain while MCU is requesting
// powerdown.
//
// When MCU requests powerdown with SCLK_HF as source, then WUC will switch over
// to this clock source during powerdown, and automatically switch back to SCLK_HF
// when MCU is no longer requesting powerdown and system is back in active mode.
//
//
// ENUMs:
// NONE                 No clock in Powerdown
// SCLK_LF              Use SCLK_LF in Powerdown
// SCLK_MF              Use SCLK_MF in Powerdown
//
#define AON_WUC_MCUCLK_PWR_DWN_SRC_M                                0x00000003
#define AON_WUC_MCUCLK_PWR_DWN_SRC_S                                0
#define AON_WUC_MCUCLK_PWR_DWN_SRC_NONE                             0x00000000
#define AON_WUC_MCUCLK_PWR_DWN_SRC_SCLK_LF                          0x00000001
#define AON_WUC_MCUCLK_PWR_DWN_SRC_SCLK_MF                          0x00000002

//*****************************************************************************
//
// Register: AON_WUC_O_AUXCLK
//
//*****************************************************************************
// Field: [12:11] PWR_DWN_SRC
//
// When AUX requests powerdown with SCLK_HF as source, then WUC will switch over to
// this clock source during powerdown, and automatically switch back to SCLK_HF when
// AUX system is back in active mode
//
//
// ENUMs:
// NONE                 No clock in Powerdown
// SCLK_LF              Use SCLK_LF in Powerdown
// SCLK_MF              Use SCLK_MF in Powerdown
//
#define AON_WUC_AUXCLK_PWR_DWN_SRC_M                                0x00001800
#define AON_WUC_AUXCLK_PWR_DWN_SRC_S                                11
#define AON_WUC_AUXCLK_PWR_DWN_SRC_NONE                             0x00000000
#define AON_WUC_AUXCLK_PWR_DWN_SRC_SCLK_LF                          0x00000800
#define AON_WUC_AUXCLK_PWR_DWN_SRC_SCLK_MF                          0x00001000

// Field: [10:8] SCLK_HF_DIV
//
// Select the AUX clock divider for SCLK_HF
//
// NB: It is not supported to change the AUX clock divider while SCLK_HF is
// active source for AUX
//
// ENUMs:
// DIV2                 Divide by 2
// DIV4                 Divide by 4
// DIV8                 Divide by 8
// DIV16                Divide by 16
// DIV32                Divide by 32
// DIV64                Divide by 64
// DIV128               Divide by 128
// DIV256               Divide by 256
//
#define AON_WUC_AUXCLK_SCLK_HF_DIV_M                                0x00000700
#define AON_WUC_AUXCLK_SCLK_HF_DIV_S                                8
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV2                             0x00000000
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV4                             0x00000100
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV8                             0x00000200
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV16                            0x00000300
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV32                            0x00000400
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV64                            0x00000500
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV128                           0x00000600
#define AON_WUC_AUXCLK_SCLK_HF_DIV_DIV256                           0x00000700

// Field: [2:0] SRC
//
// Selects the clock source for AUX:
//
// NB: Switching the clock source is guaranteed to be glitchless
// ENUMs:
// SCLK_HF              HF Clock (SCLK_HF)
// SCLK_MF              MF Clock (SCLK_MF)
// SCLK_LF              LF Clock (SCLK_LF)
//
#define AON_WUC_AUXCLK_SRC_M                                        0x00000007
#define AON_WUC_AUXCLK_SRC_S                                        0
#define AON_WUC_AUXCLK_SRC_SCLK_HF                                  0x00000001
#define AON_WUC_AUXCLK_SRC_SCLK_MF                                  0x00000002
#define AON_WUC_AUXCLK_SRC_SCLK_LF                                  0x00000004

//*****************************************************************************
//
// Register: AON_WUC_O_MCUCFG
//
//*****************************************************************************
// Field: [17]    VIRT_OFF
//
// Controls action taken when receiving a request to power-off MCU domain:
//
// 0: Power-off MCU
// 1: Assert reset and stop clock, but keep MCU powered (virtual power-off)
//
//
#define AON_WUC_MCUCFG_VIRT_OFF                                     0x00020000
#define AON_WUC_MCUCFG_VIRT_OFF_BITN                                17
#define AON_WUC_MCUCFG_VIRT_OFF_M                                   0x00020000
#define AON_WUC_MCUCFG_VIRT_OFF_S                                   17

// Field: [16]    FIXED_WU_EN
//
// Controls timing of MCU wakeup:
//
// 0: MCU wake-up is immediately following triggering event
// 1: MCU wake-up is forced to happen a fixed number of 32 KHz clocks following
// the triggering event.
//
// The number of 32 KHz clock cycles is configured in CTL0.FIXED_WU_PER
// register
// This bit is used to ensure that the MCU is always woken up an exact number of
// clocks following the triggering event by compensating for any  variable delays
// caused by other activities going on at the time of wakeup (such as a recharge
// event etc.)
//
#define AON_WUC_MCUCFG_FIXED_WU_EN                                  0x00010000
#define AON_WUC_MCUCFG_FIXED_WU_EN_BITN                             16
#define AON_WUC_MCUCFG_FIXED_WU_EN_M                                0x00010000
#define AON_WUC_MCUCFG_FIXED_WU_EN_S                                16

// Field: [3:0] SRAM_RET_EN
//
// MCU SRAM is partitioned into 4 banks . This register controls which of the banks
// that has retention during MCU power off
//
// ENUMs:
// RET_NONE             Retention is disabled
// RET_LEVEL1           Retention on for SRAM:BANK0
// RET_LEVEL2           Retention on for SRAM:BANK0 and SRAM:BANK1
// RET_LEVEL3           Retention on for SRAM:BANK0, SRAM:BANK1 and
// SRAM:BANK2
// RET_FULL             Retention on for all banks (SRAM:BANK0,
// SRAM:BANK1 ,SRAM:BANK2 and SRAM:BANK3)
//
#define AON_WUC_MCUCFG_SRAM_RET_EN_M                                0x0000000F
#define AON_WUC_MCUCFG_SRAM_RET_EN_S                                0
#define AON_WUC_MCUCFG_SRAM_RET_EN_RET_NONE                         0x00000000
#define AON_WUC_MCUCFG_SRAM_RET_EN_RET_LEVEL1                       0x00000001
#define AON_WUC_MCUCFG_SRAM_RET_EN_RET_LEVEL2                       0x00000003
#define AON_WUC_MCUCFG_SRAM_RET_EN_RET_LEVEL3                       0x00000007
#define AON_WUC_MCUCFG_SRAM_RET_EN_RET_FULL                         0x0000000F

//*****************************************************************************
//
// Register: AON_WUC_O_AUXCFG
//
//*****************************************************************************
// Field: [17]    VIRT_OFF
//
// Controls action taken when receiving a request to power AUX domain off:
//
// 0: Power-off AUX
// 1: Assert reset and stop clock, but keep AUX powered (virtual power-off)
//
//
#define AON_WUC_AUXCFG_VIRT_OFF                                     0x00020000
#define AON_WUC_AUXCFG_VIRT_OFF_BITN                                17
#define AON_WUC_AUXCFG_VIRT_OFF_M                                   0x00020000
#define AON_WUC_AUXCFG_VIRT_OFF_S                                   17

// Field: [16]    FIXED_WU_EN
//
// Controls timing of AUX wakeup:
//
// 0: AUX wake-up is immediately following triggering event
// 1: AUX wake-up is forced to happen a fixed number of 32 KHz clocks following
// the triggering event.
//
// The number of  32KHz clock cycles is configured in CTL0.FIXED_WU_PER
// register.
// This bit is used to ensure that the AUX is always woken up an exact number of
// clocks following the triggering event by compensating for any  variable delays
// caused by other activities going on at the time of wakeup (such as a recharge
// event etc.)
// NB: Waking up AUX by AUXCTL.AUX_FORCE_ON and AUXCTL.SWEV is not affected
// by this bit
//
#define AON_WUC_AUXCFG_FIXED_WU_EN                                  0x00010000
#define AON_WUC_AUXCFG_FIXED_WU_EN_BITN                             16
#define AON_WUC_AUXCFG_FIXED_WU_EN_M                                0x00010000
#define AON_WUC_AUXCFG_FIXED_WU_EN_S                                16

// Field: [0]    RAM_RET_EN
//
// This bit controls retention mode for the AUX_RAM:BANK0:
//
// 0: Retention is disabled
// 1: Retention is enabled
//
// NB: If retention is disabled, the AUX_RAM will be powered off when it would
// otherwise be put in retention mode
//
#define AON_WUC_AUXCFG_RAM_RET_EN                                   0x00000001
#define AON_WUC_AUXCFG_RAM_RET_EN_BITN                              0
#define AON_WUC_AUXCFG_RAM_RET_EN_M                                 0x00000001
#define AON_WUC_AUXCFG_RAM_RET_EN_S                                 0

//*****************************************************************************
//
// Register: AON_WUC_O_AUXCTL
//
//*****************************************************************************
// Field: [31]    RESET_REQ
//
// Reset request for AUX. Writing 1 to this register will assert reset to AUX. The
// reset will be held until the bit is cleared again.
//
// 0: AUX reset pin will be deasserted
// 1: AUX reset pin will be asserted
//
//
#define AON_WUC_AUXCTL_RESET_REQ                                    0x80000000
#define AON_WUC_AUXCTL_RESET_REQ_BITN                               31
#define AON_WUC_AUXCTL_RESET_REQ_M                                  0x80000000
#define AON_WUC_AUXCTL_RESET_REQ_S                                  31

// Field: [2]    SCE_RUN_EN
//
// Enables (1) or disables (0) AUX_SCE execution. AUX_SCE execution will begin when
// AUX Domain is powered and either this or AUX_SCE:CTL.CLK_EN is set.
//
// Setting this bit will assure that AUX_SCE execution starts as soon as AUX
// power domain is woken up. (  AUX_SCE:CTL.CLK_EN will be reset to 0 if AUX power
// domain has been off)
//
// 0: AUX_SCE execution will be disabled if AUX_SCE:CTL.CLK_EN is 0
// 1: AUX_SCE execution is enabled.
//
//
#define AON_WUC_AUXCTL_SCE_RUN_EN                                   0x00000004
#define AON_WUC_AUXCTL_SCE_RUN_EN_BITN                              2
#define AON_WUC_AUXCTL_SCE_RUN_EN_M                                 0x00000004
#define AON_WUC_AUXCTL_SCE_RUN_EN_S                                 2

// Field: [1]    SWEV
//
// Writing 1 sets the software event to the AUX domain, which can be read through
// AUX_WUC:WUEVFLAGS.AON_SW.
//
// This event is normally cleared by AUX_SCE through the
// AUX_WUC:WUEVCLR.AON_SW. It can also be cleared by writing 0 to this register.
//
// Reading 0 means that there is no outstanding software event for AUX.
//
// Note that it can take up to 1,5 SCLK_LF clock cycles to clear the event from
// AUX.
//
#define AON_WUC_AUXCTL_SWEV                                         0x00000002
#define AON_WUC_AUXCTL_SWEV_BITN                                    1
#define AON_WUC_AUXCTL_SWEV_M                                       0x00000002
#define AON_WUC_AUXCTL_SWEV_S                                       1

// Field: [0]    AUX_FORCE_ON
//
// Forces the AUX domain into active mode, overriding the requests from
// AUX_WUC:PWROFFREQ, AUX_WUC:PWRDWNREQ and AUX_WUC:MCUBUSCTL.
// Note that an ongoing AUX_WUC:PWROFFREQ will complete before this bit will
// set the AUX domain into active mode.
//
// MCU must set this bit in order to access the AUX peripherals.
// The AUX domain status can be read from PWRSTAT.AUX_PD_ON
//
// 0: AUX is allowed to Power Off, Power Down or Disconnect.
// 1: AUX  Power OFF, Power Down or Disconnect requests will be overruled
//
//
#define AON_WUC_AUXCTL_AUX_FORCE_ON                                 0x00000001
#define AON_WUC_AUXCTL_AUX_FORCE_ON_BITN                            0
#define AON_WUC_AUXCTL_AUX_FORCE_ON_M                               0x00000001
#define AON_WUC_AUXCTL_AUX_FORCE_ON_S                               0

//*****************************************************************************
//
// Register: AON_WUC_O_PWRSTAT
//
//*****************************************************************************
// Field: [31:30] SW
//
// General purpose Read/Write register that can be read through the test subsystem
// tap. This register is dedicated for the Power Profiler tool.
#define AON_WUC_PWRSTAT_SW_M                                        0xC0000000
#define AON_WUC_PWRSTAT_SW_S                                        30

// Field: [25]    VDDS_OK
//
// Observation of VDDS_BOD_OK signal from VDDS brown out detection circuit
//
// 0: VDDS_BOD_OK signal is low.
// 1: VDDS_BOD_OK signal is high indication that VDDS voltage is OK.
//
//
#define AON_WUC_PWRSTAT_VDDS_OK                                     0x02000000
#define AON_WUC_PWRSTAT_VDDS_OK_BITN                                25
#define AON_WUC_PWRSTAT_VDDS_OK_M                                   0x02000000
#define AON_WUC_PWRSTAT_VDDS_OK_S                                   25

// Field: [24]    VDDR_OK
//
// Observation of VDDR_OK from VDDR brown out detection circuit
//
// 1: VDDR_OK signal is high indication that VDDR voltage is OK.
// 0: VDDR_OK signal is low.
//
//
#define AON_WUC_PWRSTAT_VDDR_OK                                     0x01000000
#define AON_WUC_PWRSTAT_VDDR_OK_BITN                                24
#define AON_WUC_PWRSTAT_VDDR_OK_M                                   0x01000000
#define AON_WUC_PWRSTAT_VDDR_OK_S                                   24

// Field: [23]    VDD_OK
//
// Observation of VDD_BOD_OK signal from VDD brown out detection circuit
//
// 0: VDD_BOD_OK signal is low
// 1: VDD_BOD_OK signal is high indicating that VDD voltage is good
//
//
#define AON_WUC_PWRSTAT_VDD_OK                                      0x00800000
#define AON_WUC_PWRSTAT_VDD_OK_BITN                                 23
#define AON_WUC_PWRSTAT_VDD_OK_M                                    0x00800000
#define AON_WUC_PWRSTAT_VDD_OK_S                                    23

// Field: [22]    OSC_GBIAS
//
// Indicates OSC GBIAS state:
//
// 0: OSC is not requesting GBIAS
// 1: OSC is requesting GBIAS
//
//
#define AON_WUC_PWRSTAT_OSC_GBIAS                                   0x00400000
#define AON_WUC_PWRSTAT_OSC_GBIAS_BITN                              22
#define AON_WUC_PWRSTAT_OSC_GBIAS_M                                 0x00400000
#define AON_WUC_PWRSTAT_OSC_GBIAS_S                                 22

// Field: [21]    AUX_GBIAS
//
// Indicates AUX GBIAS state:
//
// 0: AUX is not requesting GBIAS
// 1: AUX is requesting GBIAS
//
//
#define AON_WUC_PWRSTAT_AUX_GBIAS                                   0x00200000
#define AON_WUC_PWRSTAT_AUX_GBIAS_BITN                              21
#define AON_WUC_PWRSTAT_AUX_GBIAS_M                                 0x00200000
#define AON_WUC_PWRSTAT_AUX_GBIAS_S                                 21

// Field: [20]    MCU_GBIAS
//
// Indicates MCU GBIAS state:
//
// 0: MCU is not requesting GBIAS
// 1: MCU is requesting GBIAS
//
//
#define AON_WUC_PWRSTAT_MCU_GBIAS                                   0x00100000
#define AON_WUC_PWRSTAT_MCU_GBIAS_BITN                              20
#define AON_WUC_PWRSTAT_MCU_GBIAS_M                                 0x00100000
#define AON_WUC_PWRSTAT_MCU_GBIAS_S                                 20

// Field: [18]    OSC_BGAP
//
// Indicates OSC BGAP state:
//
// 0: OSC is not requesting BGAP
// 1: OSC is requesting BGAP
//
//
#define AON_WUC_PWRSTAT_OSC_BGAP                                    0x00040000
#define AON_WUC_PWRSTAT_OSC_BGAP_BITN                               18
#define AON_WUC_PWRSTAT_OSC_BGAP_M                                  0x00040000
#define AON_WUC_PWRSTAT_OSC_BGAP_S                                  18

// Field: [17]    AUX_BGAP
//
// Indicates AUX BGAP state:
//
// 0: AUX is not requesting BGAP
// 1: AUX is requesting BGAP
//
//
#define AON_WUC_PWRSTAT_AUX_BGAP                                    0x00020000
#define AON_WUC_PWRSTAT_AUX_BGAP_BITN                               17
#define AON_WUC_PWRSTAT_AUX_BGAP_M                                  0x00020000
#define AON_WUC_PWRSTAT_AUX_BGAP_S                                  17

// Field: [16]    MCU_BGAP
//
// Indicates MCU BGAP state:
//
// 0: MCU is not requesting BGAP
// 1: MCU is requesting BGAP
//
//
#define AON_WUC_PWRSTAT_MCU_BGAP                                    0x00010000
#define AON_WUC_PWRSTAT_MCU_BGAP_BITN                               16
#define AON_WUC_PWRSTAT_MCU_BGAP_M                                  0x00010000
#define AON_WUC_PWRSTAT_MCU_BGAP_S                                  16

// Field: [14]    GBIAS_ON99
//
// Indicates GBIAS state:
//
// 0: GBIAS is powered off
// 1: GBIAS is powered on and steady within 99%
//
//
#define AON_WUC_PWRSTAT_GBIAS_ON99                                  0x00004000
#define AON_WUC_PWRSTAT_GBIAS_ON99_BITN                             14
#define AON_WUC_PWRSTAT_GBIAS_ON99_M                                0x00004000
#define AON_WUC_PWRSTAT_GBIAS_ON99_S                                14

// Field: [13]    GBIAS_ON
//
// Indicates GBIAS state:
//
// 0: GBIAS is powered off
// 1: GBIAS is powered on and steady within 95%
//
//
#define AON_WUC_PWRSTAT_GBIAS_ON                                    0x00002000
#define AON_WUC_PWRSTAT_GBIAS_ON_BITN                               13
#define AON_WUC_PWRSTAT_GBIAS_ON_M                                  0x00002000
#define AON_WUC_PWRSTAT_GBIAS_ON_S                                  13

// Field: [12]    BGAP_ON
//
// Indicates BGAP state:
//
// 0: BGAP is powered off
// 1: BGAP is powered on
//
//
#define AON_WUC_PWRSTAT_BGAP_ON                                     0x00001000
#define AON_WUC_PWRSTAT_BGAP_ON_BITN                                12
#define AON_WUC_PWRSTAT_BGAP_ON_M                                   0x00001000
#define AON_WUC_PWRSTAT_BGAP_ON_S                                   12

// Field: [9]    AUX_PWR_DWN
//
// Indicates the AUX powerdown state when AUX domain is powered up.
//
// 0: Active mode
// 1: AUX Powerdown request has been granted
//
//
#define AON_WUC_PWRSTAT_AUX_PWR_DWN                                 0x00000200
#define AON_WUC_PWRSTAT_AUX_PWR_DWN_BITN                            9
#define AON_WUC_PWRSTAT_AUX_PWR_DWN_M                               0x00000200
#define AON_WUC_PWRSTAT_AUX_PWR_DWN_S                               9

// Field: [8]    MCU_PWR_DWN
//
// Indicates the MCU powerdown state when MCU is powered up.
//
// 0: Active mode
// 1: MCU Powerdown request has been granted. ( MCU in STANDBY)
//
// Note: MCU can only read 0 from this register due to the nature of the signal.
//
#define AON_WUC_PWRSTAT_MCU_PWR_DWN                                 0x00000100
#define AON_WUC_PWRSTAT_MCU_PWR_DWN_BITN                            8
#define AON_WUC_PWRSTAT_MCU_PWR_DWN_M                               0x00000100
#define AON_WUC_PWRSTAT_MCU_PWR_DWN_S                               8

// Field: [6]    JTAG_PD_ON
//
// Indicates JTAG power state:
//
// 0: JTAG is powered off
// 1: JTAG is powered on
//
//
#define AON_WUC_PWRSTAT_JTAG_PD_ON                                  0x00000040
#define AON_WUC_PWRSTAT_JTAG_PD_ON_BITN                             6
#define AON_WUC_PWRSTAT_JTAG_PD_ON_M                                0x00000040
#define AON_WUC_PWRSTAT_JTAG_PD_ON_S                                6

// Field: [5]    AUX_PD_ON
//
// Indicates AUX power state:
//
// 0: AUX is not ready for use ( may be powered off or in power state transition
// )
// 1: AUX is powered on, connected to bus and ready for use,
//
//
#define AON_WUC_PWRSTAT_AUX_PD_ON                                   0x00000020
#define AON_WUC_PWRSTAT_AUX_PD_ON_BITN                              5
#define AON_WUC_PWRSTAT_AUX_PD_ON_M                                 0x00000020
#define AON_WUC_PWRSTAT_AUX_PD_ON_S                                 5

// Field: [4]    MCU_PD_ON
//
// Indicates MCU power state:
//
// 0: MCU Power sequencing is not yet finalized and MCU_AONIF registers may not
// be reliable
// 1: MCU Power sequencing is finalized and all MCU_AONIF registers are reliable
//
//
#define AON_WUC_PWRSTAT_MCU_PD_ON                                   0x00000010
#define AON_WUC_PWRSTAT_MCU_PD_ON_BITN                              4
#define AON_WUC_PWRSTAT_MCU_PD_ON_M                                 0x00000010
#define AON_WUC_PWRSTAT_MCU_PD_ON_S                                 4

// Field: [3]    AUX_BUS_RESET_DONE
//
// Indicates Reset Done from AUX Bus:
//
// 0: AUX Bus is being reset
// 1: AUX Bus reset is released
//
//
#define AON_WUC_PWRSTAT_AUX_BUS_RESET_DONE                          0x00000008
#define AON_WUC_PWRSTAT_AUX_BUS_RESET_DONE_BITN                     3
#define AON_WUC_PWRSTAT_AUX_BUS_RESET_DONE_M                        0x00000008
#define AON_WUC_PWRSTAT_AUX_BUS_RESET_DONE_S                        3

// Field: [2]    AUX_BUS_CONNECTED
//
// Indicates that AUX Bus is connected:
//
// 0: AUX bus is not connected
// 1: AUX bus is connected ( idle_ack = 0 )
//
//
#define AON_WUC_PWRSTAT_AUX_BUS_CONNECTED                           0x00000004
#define AON_WUC_PWRSTAT_AUX_BUS_CONNECTED_BITN                      2
#define AON_WUC_PWRSTAT_AUX_BUS_CONNECTED_M                         0x00000004
#define AON_WUC_PWRSTAT_AUX_BUS_CONNECTED_S                         2

// Field: [1]    AUX_RESET_DONE
//
// Indicates Reset Done from AUX:
//
// 0: AUX is being reset
// 1: AUX reset is released
//
//
#define AON_WUC_PWRSTAT_AUX_RESET_DONE                              0x00000002
#define AON_WUC_PWRSTAT_AUX_RESET_DONE_BITN                         1
#define AON_WUC_PWRSTAT_AUX_RESET_DONE_M                            0x00000002
#define AON_WUC_PWRSTAT_AUX_RESET_DONE_S                            1

// Field: [0]    PWR_DWN
//
// Indicates state of power supply:
//
// 0: Active mode
// 1: System is running of uLDO
//
// Note MCU can normally only read 0 from this register due to the nature of
// signal
//
//
#define AON_WUC_PWRSTAT_PWR_DWN                                     0x00000001
#define AON_WUC_PWRSTAT_PWR_DWN_BITN                                0
#define AON_WUC_PWRSTAT_PWR_DWN_M                                   0x00000001
#define AON_WUC_PWRSTAT_PWR_DWN_S                                   0

//*****************************************************************************
//
// Register: AON_WUC_O_SHUTDOWN
//
//*****************************************************************************
// Field: [1]    JTAG_OVR
//
// This register is ignored. Shutdown requests can be overridden directly from
// WUC_TAP
//
#define AON_WUC_SHUTDOWN_JTAG_OVR                                   0x00000002
#define AON_WUC_SHUTDOWN_JTAG_OVR_BITN                              1
#define AON_WUC_SHUTDOWN_JTAG_OVR_M                                 0x00000002
#define AON_WUC_SHUTDOWN_JTAG_OVR_S                                 1

// Field: [0]    EN
//
// Writing a 1 to this bit forces a shutdown request to be registered and all I/O
// values to be latched - in the PAD ring, possibly enabling I/O wakeup. Writing 0
// will cancel a registered shutdown request and open th I/O latches residing in the
// PAD ring.
//
// A registered shutdown request takes effect the next time power down
// conditions exists. At this time, the will not enter Powerdown mode, but instead
// it will turn off all internal powersupplies, effectively putting the device into
// Shutdown mode.
#define AON_WUC_SHUTDOWN_EN                                         0x00000001
#define AON_WUC_SHUTDOWN_EN_BITN                                    0
#define AON_WUC_SHUTDOWN_EN_M                                       0x00000001
#define AON_WUC_SHUTDOWN_EN_S                                       0

//*****************************************************************************
//
// Register: AON_WUC_O_CTL0
//
//*****************************************************************************
// Field: [8]    PWR_DWN_DIS
//
// Controls whether MCU and AUX requesting to be powered off will enable a
// transition to powerdown:
//
// 0: Enabled
// 1: Disabled
//
//
#define AON_WUC_CTL0_PWR_DWN_DIS                                    0x00000100
#define AON_WUC_CTL0_PWR_DWN_DIS_BITN                               8
#define AON_WUC_CTL0_PWR_DWN_DIS_M                                  0x00000100
#define AON_WUC_CTL0_PWR_DWN_DIS_S                                  8

// Field: [7:4] FIXED_WU_PER
//
// Select  a wakeup period for AUX and MCU wakeup. Wakeup time will take at least
// the number of SCLK_LF cycles specified in this register, when enabed by
// MCUCFG.FIXED_WU_EN and/or AUXCFG.FIXED_WU_EN
//
#define AON_WUC_CTL0_FIXED_WU_PER_M                                 0x000000F0
#define AON_WUC_CTL0_FIXED_WU_PER_S                                 4

//*****************************************************************************
//
// Register: AON_WUC_O_CTL1
//
//*****************************************************************************
// Field: [23:16] TAP_SECURITY_CTL
//
// Readback of security ctrl word of WUC_TAP
//
// Bit 0:  t2c_wuc_sec_totalerase
// Bit 1:  t2c_wuc_sec_chiperase
// Bit 2:  t2c_wuc_sec_bddat
// Bit 3:  t2c_wuc_sec_bdrdy
// Bit 4:  t2c_wuc_sec_aux_reset_req
// Bit 5:  t2c_wuc_sec_mcu_reset_req
// Bit 6:  t2c_wuc_sec_shutdown_waitforjtagdisconnect
// Bit 7:  t2c_wuc_sec_sys_reset_req ( note:  always reads 0 )
//
//
#define AON_WUC_CTL1_TAP_SECURITY_CTL_M                             0x00FF0000
#define AON_WUC_CTL1_TAP_SECURITY_CTL_S                             16

// Field: [8]    CHIP_ERASE
//
// Request set by WUC_TAP to request ROM FW to perform a Chip erase
//
// Read 0: No Chip Erase request is registered
// Read 1: A Chip Erase has been requested via WUC_TAP.
// Write 0: No effect.
// Write 1: Will clear a registered Chip Erase request. The WUC_TAP CHIP_ERASE
// request must be cleared and set again before a new requst is registered.
//
// A Chip Erase will be effectuated by the ROM FW if this bit is set when MCU is
// booting. User must therefore issue a MCU reset request ( via WUC_TAP) before the
// Chip Erase takes effect
//
//
#define AON_WUC_CTL1_CHIP_ERASE                                     0x00000100
#define AON_WUC_CTL1_CHIP_ERASE_BITN                                8
#define AON_WUC_CTL1_CHIP_ERASE_M                                   0x00000100
#define AON_WUC_CTL1_CHIP_ERASE_S                                   8

// Field: [1]    MCU_RESET_SRC
//
// Indicates source of last MCU Voltage Domain warm reset request:
//
// 0: MCU SW reset
// 1: JTAG reset
//
// This bit can only be cleared by writing a 1 to it
//
#define AON_WUC_CTL1_MCU_RESET_SRC                                  0x00000002
#define AON_WUC_CTL1_MCU_RESET_SRC_BITN                             1
#define AON_WUC_CTL1_MCU_RESET_SRC_M                                0x00000002
#define AON_WUC_CTL1_MCU_RESET_SRC_S                                1

// Field: [0]    MCU_WARM_RESET
//
// Indicates type of last MCU Voltage Domain reset:
//
// 0: Last MCU reset was not a warm reset
// 1: Last MCU reset was a warm reset (requested from MCU or JTAG as indicated
// in MCU_RESET_SRC)
//
// This bit can only be cleared by writing a 1 to it
//
#define AON_WUC_CTL1_MCU_WARM_RESET                                 0x00000001
#define AON_WUC_CTL1_MCU_WARM_RESET_BITN                            0
#define AON_WUC_CTL1_MCU_WARM_RESET_M                               0x00000001
#define AON_WUC_CTL1_MCU_WARM_RESET_S                               0

//*****************************************************************************
//
// Register: AON_WUC_O_RECHARGECFG
//
//*****************************************************************************
// Field: [31]    ADAPTIVE_EN
//
// Enable adaptive recharge
//
// Note: Recharge can be turned completely of by setting MAX_PER_E=7 and
// MAX_PER_M=31 and this bitfield to 0
//
#define AON_WUC_RECHARGECFG_ADAPTIVE_EN                             0x80000000
#define AON_WUC_RECHARGECFG_ADAPTIVE_EN_BITN                        31
#define AON_WUC_RECHARGECFG_ADAPTIVE_EN_M                           0x80000000
#define AON_WUC_RECHARGECFG_ADAPTIVE_EN_S                           31

// Field: [23:20] C2
//
// Gain factor for adaptive recharge algorithm
//
// period_new=period * ( 1+/-(2^-C1+2^-C2) )
// Valid values for C2 is 2 to 10
//
// Note: Rounding may cause adaptive recharge not to start for very small values
// of both Gain and Initial period. Criteria for algorithm to start is
// MAX(PERIOD*2^-C1,PERIOD*2^-C2) >= 1
#define AON_WUC_RECHARGECFG_C2_M                                    0x00F00000
#define AON_WUC_RECHARGECFG_C2_S                                    20

// Field: [19:16] C1
//
// Gain factor for adaptive recharge algorithm
//
// period_new=period * ( 1+/-(2^-C1+2^-C2) )
// Valid values for C1 is 1 to 10
//
// Note: Rounding may cause adaptive recharge not to start for very small values
// of both Gain and Initial period. Criteria for algorithm to start is
// MAX(PERIOD*2^-C1,PERIOD*2^-C2) >= 1
#define AON_WUC_RECHARGECFG_C1_M                                    0x000F0000
#define AON_WUC_RECHARGECFG_C1_S                                    16

// Field: [15:11] MAX_PER_M
//
// This register defines the maximum period that the recharge algorithm can take,
// i.e. it defines  the maximum number of cycles between 2 recharges.
// The maximum number of cycles is specified with a 5 bit mantissa and 3 bit
// exponent:
// MAXCYCLES=(MAX_PER_M*16+15)*2^MAX_PER_E
// This field sets the mantissa of MAXCYCLES
//
#define AON_WUC_RECHARGECFG_MAX_PER_M_M                             0x0000F800
#define AON_WUC_RECHARGECFG_MAX_PER_M_S                             11

// Field: [10:8] MAX_PER_E
//
// This register defines the maximum period that the recharge algorithm can take,
// i.e. it defines  the maximum number of cycles between 2 recharges.
// The maximum number of cycles is specified with a 5 bit mantissa and 3 bit
// exponent:
// MAXCYCLES=(MAX_PER_M*16+15)*2^MAX_PER_E
// This field sets the exponent MAXCYCLES
//
#define AON_WUC_RECHARGECFG_MAX_PER_E_M                             0x00000700
#define AON_WUC_RECHARGECFG_MAX_PER_E_S                             8

// Field: [7:3] PER_M
//
// Number of 32 KHz clocks between activation of recharge controller
// For recharge algorithm, PERIOD is the initial period when entering powerdown
// mode. The adaptive recharge algorithm will not change this register
// PERIOD will effectively be a 16 bit value coded in a 5 bit mantissa and 3 bit
// exponent:
// This field sets the Mantissa of the Period.
// PERIOD=(PER_M*16+15)*2^PER_E
//
#define AON_WUC_RECHARGECFG_PER_M_M                                 0x000000F8
#define AON_WUC_RECHARGECFG_PER_M_S                                 3

// Field: [2:0] PER_E
//
// Number of 32 KHz clocks between activation of recharge controller
// For recharge algorithm, PERIOD is the initial period when entering powerdown
// mode. The adaptive recharge algorithm will not change this register
// PERIOD will effectively be a 16 bit value coded in a 5 bit mantissa and 3 bit
// exponent:
// This field sets the Exponent of the Period.
// PERIOD=(PER_M*16+15)*2^PER_E
//
#define AON_WUC_RECHARGECFG_PER_E_M                                 0x00000007
#define AON_WUC_RECHARGECFG_PER_E_S                                 0

//*****************************************************************************
//
// Register: AON_WUC_O_RECHARGESTAT
//
//*****************************************************************************
// Field: [19:16] VDDR_SMPLS
//
// The last 4 VDDR samples, bit 0 being the newest.
//
// The register is being updated in every recharge period with a shift left, and
// bit 0 is updated with the last VDDR sample, ie a 1 is shiftet in in case VDDR >
// VDDR_threshold just before recharge starts. Otherwise a 0 will be shifted in.
//
//
#define AON_WUC_RECHARGESTAT_VDDR_SMPLS_M                           0x000F0000
#define AON_WUC_RECHARGESTAT_VDDR_SMPLS_S                           16

// Field: [15:0] MAX_USED_PER
//
// The maximum value of recharge period seen with VDDR>threshold.
//
// The VDDR voltage is compared against the threshold voltage at  just before
// each recharge. If VDDR is above threshold, MAX_USED_PER is updated with max (
// current recharge peride; MAX_USED_PER )  This way MAX_USED_PER can track the
// recharge period where VDDR is decharged to the threshold value. We can therefore
// use the value as an indication of the leakage current during recharge.
//
// This bitfield is cleared to 0 when writing this register.
//
#define AON_WUC_RECHARGESTAT_MAX_USED_PER_M                         0x0000FFFF
#define AON_WUC_RECHARGESTAT_MAX_USED_PER_S                         0

//*****************************************************************************
//
// Register: AON_WUC_O_OSCCFG
//
//*****************************************************************************
// Field: [7:3] PER_M
//
// Number of 32 KHz clocks between oscillator amplitude calibrations.
// When this counter expires, an oscillator amplitude compensation is triggered
// immediately in Active mode. When this counter expires in Powerdown mode an
// internal flag is set such that the amplitude compensation is postponed until the
// next recharge occurs.
//
// The Period will effectively be a 16 bit value coded in a 5 bit mantissa and 3
// bit exponent
// PERIOD=(PER_M*16+15)*2^PER_E
// This field sets the mantissa
// Note: Oscillator amplitude calibration is turned of when both this bitfield
// and PER_E are set to 0
//
#define AON_WUC_OSCCFG_PER_M_M                                      0x000000F8
#define AON_WUC_OSCCFG_PER_M_S                                      3

// Field: [2:0] PER_E
//
// Number of 32 KHz clocks between oscillator amplitude calibrations.
// When this counter expires, an oscillator amplitude compensation is triggered
// immediately in Active mode. When this counter expires in Powerdown mode an
// internal flag is set such that the amplitude compensation is postponed until the
// next recharge occurs.
// The Period will effectively be a 16 bit value coded in a 5 bit mantissa and 3
// bit exponent
// PERIOD=(PER_M*16+15)*2^PER_E
// This field sets the exponent
// Note: Oscillator amplitude calibration is turned of when both  PER_M and
// this bitfield are set to 0
//
#define AON_WUC_OSCCFG_PER_E_M                                      0x00000007
#define AON_WUC_OSCCFG_PER_E_S                                      0

//*****************************************************************************
//
// Register: AON_WUC_O_JTAGCFG
//
//*****************************************************************************
// Field: [8]    JTAG_PD_FORCE_ON
//
// Controls JTAG PowerDomain power state:
//
// 0: Controlled exclusively by debug subsystem. (JTAG Powerdomain will be
// powered off unless a debugger is attached)
// 1: JTAG Power Domain is forced on, independent of debug subsystem.
//
// NB: The reset value causes JTAG Power Domain to be powered on by default.
// Software must clear this bit to turn off the JTAG Power Domain
//
#define AON_WUC_JTAGCFG_JTAG_PD_FORCE_ON                            0x00000100
#define AON_WUC_JTAGCFG_JTAG_PD_FORCE_ON_BITN                       8
#define AON_WUC_JTAGCFG_JTAG_PD_FORCE_ON_M                          0x00000100
#define AON_WUC_JTAGCFG_JTAG_PD_FORCE_ON_S                          8

// Field: [6]    PBIST2_TAP
//
// PBIST2 TAP access control:
//
// 0: Disabled
// 1: Enabled
//
// NB: This bit can be locked
//
#define AON_WUC_JTAGCFG_PBIST2_TAP                                  0x00000040
#define AON_WUC_JTAGCFG_PBIST2_TAP_BITN                             6
#define AON_WUC_JTAGCFG_PBIST2_TAP_M                                0x00000040
#define AON_WUC_JTAGCFG_PBIST2_TAP_S                                6

// Field: [5]    PBIST1_TAP
//
// PBIST1 TAP access control:
//
// 0: Disabled
// 1: Enabled
//
// NB: This bit can be locked
//
#define AON_WUC_JTAGCFG_PBIST1_TAP                                  0x00000020
#define AON_WUC_JTAGCFG_PBIST1_TAP_BITN                             5
#define AON_WUC_JTAGCFG_PBIST1_TAP_M                                0x00000020
#define AON_WUC_JTAGCFG_PBIST1_TAP_S                                5

// Field: [3]    TEST_TAP
//
// Test TAP access control:
//
// 0: Disabled
// 1: Enabled
//
// NB: This bit can be locked
//
#define AON_WUC_JTAGCFG_TEST_TAP                                    0x00000008
#define AON_WUC_JTAGCFG_TEST_TAP_BITN                               3
#define AON_WUC_JTAGCFG_TEST_TAP_M                                  0x00000008
#define AON_WUC_JTAGCFG_TEST_TAP_S                                  3

// Field: [2]    WUC_TAP
//
// WUC_TAP debug access control:
//
// 0: Disabled
// 1: Enabled
//
// Note that basic WUC TAP functions such has TMS drive strength setting and
// reset requests are available even with this bitfield set to 0. The WUC TAP will
// therefore always appear as available seen from the debug subsystem.
// NB: This bit can be locked
//
#define AON_WUC_JTAGCFG_WUC_TAP                                     0x00000004
#define AON_WUC_JTAGCFG_WUC_TAP_BITN                                2
#define AON_WUC_JTAGCFG_WUC_TAP_M                                   0x00000004
#define AON_WUC_JTAGCFG_WUC_TAP_S                                   2

// Field: [1]    PRCM_TAP
//
// PRCM TAP access control:
//
// 0: Disabled
// 1: Enabled
//
// NB: This bit can be locked
//
#define AON_WUC_JTAGCFG_PRCM_TAP                                    0x00000002
#define AON_WUC_JTAGCFG_PRCM_TAP_BITN                               1
#define AON_WUC_JTAGCFG_PRCM_TAP_M                                  0x00000002
#define AON_WUC_JTAGCFG_PRCM_TAP_S                                  1

// Field: [0]    CPU_DAP
//
// Main CPU DAP access control:
//
// 0: Disabled
// 1: Enabled
//
// NB: This bit can be locked
//
#define AON_WUC_JTAGCFG_CPU_DAP                                     0x00000001
#define AON_WUC_JTAGCFG_CPU_DAP_BITN                                0
#define AON_WUC_JTAGCFG_CPU_DAP_M                                   0x00000001
#define AON_WUC_JTAGCFG_CPU_DAP_S                                   0

//*****************************************************************************
//
// Register: AON_WUC_O_JTAGUSERCODE
//
//*****************************************************************************
// Field: [31:0] USER_CODE
//
// 32-bit JTAG USERCODE register feeding main JTAG TAP
// NB: This field can be locked
//
#define AON_WUC_JTAGUSERCODE_USER_CODE_M                            0xFFFFFFFF
#define AON_WUC_JTAGUSERCODE_USER_CODE_S                            0

#endif // __HW_AON_WUC_H__
