/******************************************************************************
*  Filename:       setup.c
*  Revised:        2015-01-16 10:44:49 +0100 (fr, 16 jan 2015)
*  Revision:       42411
*
*  Description:    Setup file for CC26xx PG2 device family.
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

// Hardware headers
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_adi.h>
#include <inc/hw_adi_0_rf.h>
#include <inc/hw_adi_1_synth.h>
#include <inc/hw_adi_2_refsys.h>
#include <inc/hw_adi_3_refsys.h>
#include <inc/hw_adi_4_aux.h>
#include <inc/hw_aon_ioc.h>
#include <inc/hw_aon_sysctl.h>
#include <inc/hw_aon_wuc.h>
#include <inc/hw_aux_wuc.h>
#include <inc/hw_ccfg.h>
#include <inc/hw_chip_def.h>
#include <inc/hw_ddi.h>
#if ( CC_GET_CHIP_OPTION == CC_CHIP_OPTION_OTP )
#else
#include <inc/hw_flash.h>
#endif
#include <inc/hw_fcfg1.h>
#include <inc/hw_ddi_0_osc.h>
#include <inc/hw_prcm.h>
#include <inc/hw_vims.h>
#include <inc/hw_aon_batmon.h>
#include <inc/hw_aon_rtc.h>
// Driverlib headers
#include <driverlib/adi.h>
#include <driverlib/cpu.h>
#include <driverlib/chipinfo.h>
#include <driverlib/ddi.h>
#include <driverlib/prcm.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/aon_batmon.h>

// We need intrinsic functions for IAR (if used in source code)
#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif

// Due to a misunderstanding when probing the first wafer this VDDR trim
// was not written to the EFUSE registers (only in the EFUSE shadow in FCFG1)
// The VDDR trim is therefore not done by the boot code and must be done here.
// This code can be removed when chips from the first wafer is no longer supported.
#define INCLUDE_VDDR_TEMPORARILY    1

// Bit defines for CUSTOMER_CFG_O_MODE_CONF:SCLK_LF_OPTION
#define CLK_LF_RCOSC_LF 0xC00000
#define CLK_LF_XOSC_LF  0x800000
#define CLK_LF_XOSC_HF  0x000000

//*****************************************************************************
//
// Function declarations
//
//*****************************************************************************
void   trimDevice( void );
static uint32_t GetTrimForAdcShModeEn( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForAdcShVbufEn( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForAmpcompCtrl( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForAmpcompTh1( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForAmpcompTh2( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForAnabypassValue1( uint32_t ui32Fcfg1Revision, uint32_t ccfg_ModeConfReg );
static uint32_t GetTrimForDblrLoopFilterResetVoltage( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForRadcExtCfg( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForRcOscLfIBiasTrim( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForRcOscLfRtuneCtuneTrim( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForXoscHfCtl( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForXoscHfFastStart( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForXoscHfIbiastherm( uint32_t ui32Fcfg1Revision );
static uint32_t GetTrimForXoscLfRegulatorAndCmirrwrRatio( uint32_t ui32Fcfg1Revision );

#if ( INCLUDE_VDDR_TEMPORARILY )
static int32_t  SignExtendVddrTrimValue( uint32_t ui32VddrTrimVal );
#endif
static void     HapiTrimDeviceColdReset( uint32_t ui32Fcfg1Revision );
static void     HapiTrimDeviceShutDown( uint32_t ui32Fcfg1Revision );
static void     HapiTrimDevicePowerDown( uint32_t ui32Fcfg1Revision );


//*****************************************************************************
//
//! Defined time delay assuming 48 MHz clock and resolution of 3 cycles
//
//*****************************************************************************
#define DELAY_20_USEC           0x140


//*****************************************************************************
//
// Defined CPU delay macro with microseconds as input
// Quick check shows: (Tob be further investigated)
// At 48 MHz RCOSC and VIMS.CONTROL.PREFETCH = 0, there is 5 cycles
// At 48 MHz RCOSC and VIMS.CONTROL.PREFETCH = 1, there is 4 cycles
// At 24 MHz RCOSC and VIMS.CONTROL.PREFETCH = 0, there is 3 cycles
//
//*****************************************************************************
#define CPU_DELAY_MICRO_SECONDS( x ) \
   CPUdelay(((uint32_t)((( x ) * 48.0 ) / 5.0 )) - 1 )


//*****************************************************************************
//
//! Perform the necessary trim of the device which is not done in boot code
//!
//! This function should only execute coming from ROM boot. The current
//! implementation does not take soft reset into account. However, it does no
//! damage to execute it again. It only consumes time.
//
//*****************************************************************************
void
trimDevice(void)
{
    uint32_t ui32Fcfg1Revision;
    uint32_t ui32AonSysResetctl;

    //
    // Get layout revision of the factory configuration area
    // (Handle undefined revision as revision = 0)
    //
    ui32Fcfg1Revision = HWREG(FCFG1_BASE + FCFG1_O_FCFG1_REVISION);
    if ( ui32Fcfg1Revision == 0xFFFFFFFF ) {
        ui32Fcfg1Revision = 0;
    }

#if defined( CHECK_AT_STARTUP_FOR_CORRECT_FAMILY_ONLY )
    //
    // This driverlib version and setup file is for CC26xx PG2.0 and later
    // (Keeping this check at HwRev2.0 independent of "#else" check)
    // Halt if violated
    //
    ThisCodeIsBuiltForCC26xxHwRev20AndLater_HaltIfViolated();
#else

    //
    // This driverlib version and setup file is for CC26xx PG2.2 and later
    // Halt if violated
    //
    ThisCodeIsBuiltForCC26xxHwRev22AndLater_HaltIfViolated();

#endif

    //
    // Enable standby in flash bank
    //
    HWREGBITW(FLASH_BASE + FLASH_O_CFG, FLASH_CFG_DIS_STANDBY_BITN ) = 0;

    //
    // Clock must always be enabled for the semaphore module (due to ADI/DDI HW workaround)
    //
    HWREG( AUX_WUC_BASE + AUX_WUC_O_MODCLKEN1 ) = AUX_WUC_MODCLKEN1_SMPH;

    // 1. Check for powerdown
    // 2. Check for shutdown
    // 3. Assume cold reset if none of the above.
    //
    // It is always assumed that the application will freeze the latches in
    // AON_IOC when going to powerdown in order to retain the values on the IOs.
    //
    // NB. If this bit is not cleared before proceeding to powerdown, the IOs
    //     will all default to the reset configuration when restarting.
    if(((HWREG(AON_IOC_BASE + AON_IOC_O_IOCLATCH) &
         AON_IOC_IOCLATCH_EN_M) == AON_IOC_IOCLATCH_EN_STATIC))
    {
        //
        // NB. This should be calling a ROM implementation of required trim and
        // compensation
        // e.g. HapiTrimDevicePowerDown()
        HapiTrimDevicePowerDown(ui32Fcfg1Revision);
    }
    // Check for shutdown
    //
    // When device is going to shutdown the hardware will automatically clear
    // the SLEEPDIS bit in the SLEEP register in the AON_SYSCTRL12 module.
    // It is left for the application to assert this bit when waking back up,
    // but not before the desired IO configuration has been re-established.
    else if(!(HWREG(AON_SYSCTL_BASE + AON_SYSCTL_O_SLEEPCTL) &
            AON_SYSCTL_SLEEPCTL_IO_PAD_SLEEP_DIS))
    {
        //
        // NB. This should be calling a ROM implementation of required trim and
        // compensation
        // e.g. HapiTrimDeviceShutDown()    -->
        //      HapiTrimDevicePowerDown();
        HapiTrimDeviceShutDown(ui32Fcfg1Revision);
        HapiTrimDevicePowerDown(ui32Fcfg1Revision);
    }
    else
    {
        // Consider adding a check for soft reset to allow debugging to skip
        // this section!!!
        //
        // NB. This should be calling a ROM implementation of required trim and
        // compensation
        // e.g. HapiTrimDeviceColdReset()   -->
        //      HapiTrimDeviceShutDown()    -->
        //      HapiTrimDevicePowerDown()
        HapiTrimDeviceColdReset(ui32Fcfg1Revision);
        HapiTrimDeviceShutDown(ui32Fcfg1Revision);
        HapiTrimDevicePowerDown(ui32Fcfg1Revision);

    }

    //
    // Make sure to enable agressive VIMS clock gating for power optimization
    // Only for PG2 devices.
    //
    HWREG(VIMS_BASE + VIMS_O_CTL) |= VIMS_CTL_DYN_CG_EN;

#if ( CC_GET_CHIP_OPTION != CC_CHIP_OPTION_OTP )
    //
    // Configure optimal wait time for flash FSM in cases where flash pump
    // wakes up from sleep
    //
    HWREG(FLASH_BASE + FLASH_O_FPAC1) = (HWREG(FLASH_BASE + FLASH_O_FPAC1) &
                                         ~FLASH_FPAC1_PSLEEPTDIS_M) |
                                        (0x139<<FLASH_FPAC1_PSLEEPTDIS_S);
#endif

    //
    // And finally at the end of the flash boot process:
    // SET BOOT_DET bits in AON_SYSCTL to 3 if already found to be 1
    // Note: The BOOT_DET_x_CLR/SET bits must be maually cleared
    //
    if ((( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) &
        ( AON_SYSCTL_RESETCTL_BOOT_DET_1_M | AON_SYSCTL_RESETCTL_BOOT_DET_0_M )) >>
        AON_SYSCTL_RESETCTL_BOOT_DET_0_S ) == 1 )
    {
        ui32AonSysResetctl = ( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) &
        ~( AON_SYSCTL_RESETCTL_BOOT_DET_1_CLR_M | AON_SYSCTL_RESETCTL_BOOT_DET_0_CLR_M |
           AON_SYSCTL_RESETCTL_BOOT_DET_1_SET_M | AON_SYSCTL_RESETCTL_BOOT_DET_0_SET_M ));
        HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) = ui32AonSysResetctl | AON_SYSCTL_RESETCTL_BOOT_DET_1_SET_M;
        HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) = ui32AonSysResetctl;
    }
}

static void
HapiTrimDevicePowerDown(uint32_t ui32Fcfg1Revision)
{
    //
    // Currently no specific trim for Powerdown
    //
}

static void
HapiTrimDeviceShutDown(uint32_t ui32Fcfg1Revision)
{
    uint32_t ui32Trim         ;
    uint32_t ccfg_ModeConfReg ;
    uint32_t sclk_lf          ;

    //
    // Force AUX on and enable clocks
    //
    // No need to save the current status of the power/clock registers.
    // At this point both AUX and AON should have been reset to 0x0.
    //
    HWREG(AON_WUC_BASE + AON_WUC_O_AUXCTL) = AON_WUC_AUXCTL_AUX_FORCE_ON;

    //
    // Wait for power on on the AUX domain
    //
    while(!(HWREG(AON_WUC_BASE + AON_WUC_O_PWRSTAT) & AON_WUC_PWRSTAT_AUX_PD_ON))
    { }

    //
    // Enable the clock
    //
    HWREG(AUX_WUC_BASE + AUX_WUC_O_MODCLKEN0) = AUX_WUC_MODCLKEN0_AUX_DDI0_OSC |
                                               AUX_WUC_MODCLKEN0_AUX_ADI4;

    //
    // read the MODE_CONF register in CCFG
    //
    ccfg_ModeConfReg = HWREG( CCFG_BASE + CCFG_O_MODE_CONF );

    //
    // setup the LF clock based upon CCFG:MODE_CONF:SCLK_LF_OPTION
    //
    sclk_lf = ccfg_ModeConfReg & CCFG_MODE_CONF_SCLK_LF_OPTION_M;

    if (      sclk_lf == CLK_LF_XOSC_LF  ) OSCClockSourceSet( OSC_SRC_CLK_LF, OSC_XOSC_LF  );
    else if ( sclk_lf == CLK_LF_XOSC_HF  ) OSCClockSourceSet( OSC_SRC_CLK_LF, OSC_XOSC_HF  );
    else                                   OSCClockSourceSet( OSC_SRC_CLK_LF, OSC_RCOSC_LF );

    //
    // It's found to be optimal to override the FCFG1..DCDC_IPEAK setting as follows:
    // if ( alternative DCDC setting in CCFG is enabled )  ADI3..IPEAK = CCFG..DCDC_IPEAK
    // else                                                ADI3..IPEAK = 2
    //
    if (( HWREG( CCFG_BASE + CCFG_O_SIZE_AND_DIS_FLAGS ) & CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING ) == 0 ) {
        //
        // ADI_3_REFSYS:DCDCCTL5[3]  (=DITHER_EN) = CCFG_MODE_CONF_1[19]   (=DCDC_DITHER_EN)
        // ADI_3_REFSYS:DCDCCTL5[2:0](=IPEAK    ) = CCFG_MODE_CONF_1[18:16](=DCDC_IPEAK    )
        // Using a single 4-bit masked write since layout is equal for both source and destination
        //
        HWREGB( ADI3_BASE + ADI_O_MASK4B + ( ADI_3_REFSYS_O_DCDCCTL5 * 2 )) = ( 0xF0 |
            ( HWREG( CCFG_BASE + CCFG_O_MODE_CONF_1 ) >> CCFG_MODE_CONF_1_ALT_DCDC_IPEAK_S ));
    } else {
        HWREGB( ADI3_BASE + ADI_O_MASK4B + ( ADI_3_REFSYS_O_DCDCCTL5 * 2 )) = 0x72;
    }

    //
    // set the RECHARGE source based upon CCFG:MODE_CONF:DCDC_RECHARGE
    // Note: Inverse polarity
    //
    HWREGBITW( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL, AON_SYSCTL_PWRCTL_DCDC_EN_BITN ) =
        ((( ccfg_ModeConfReg >> CCFG_MODE_CONF_DCDC_RECHARGE_S ) & 1 ) ^ 1 );

    //
    // set the ACTIVE source based upon CCFG:MODE_CONF:DCDC_ACTIVE
    // Note: Inverse polarity
    //
    HWREGBITW( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL, AON_SYSCTL_PWRCTL_DCDC_ACTIVE_BITN ) =
        ((( ccfg_ModeConfReg >> CCFG_MODE_CONF_DCDC_ACTIVE_S ) & 1 ) ^ 1 );

    //
    // Following sequence is required for using XOSCHF, if not included
    // devices crashes when trying to switch to XOSCHF.
    //
    // Trim CAP settings. Get and set trim value for the ANABYPASS_VALUE1
    // register
    ui32Trim = GetTrimForAnabypassValue1( ui32Fcfg1Revision, ccfg_ModeConfReg );
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ANABYPASSVAL1, ui32Trim);

    // Trim RCOSC_LF. Get and set trim values for the RCOSCLF_RTUNE_TRIM and
    // RCOSCLF_CTUNE_TRIM fields in the XOSCLF_RCOSCLF_CTRL register.
    ui32Trim = GetTrimForRcOscLfRtuneCtuneTrim(ui32Fcfg1Revision);
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_LFOSCCTL,
                       (DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_M |
                        DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_M),
                       DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_S,
                       ui32Trim);

    // Trim XOSCHF IBIAS THERM. Get and set trim value for the
    // XOSCHF IBIAS THERM bit field in the ANABYPASS_VALUE2 register. Other
    // register bit fields are set to 0.
    ui32Trim = GetTrimForXoscHfIbiastherm(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ANABYPASSVAL2,
                  ui32Trim<<DDI_0_OSC_ANABYPASSVAL2_XOSC_HF_IBIASTHERM_S);

    // Trim AMPCOMP settings required before switch to XOSCHF
    ui32Trim = GetTrimForAmpcompTh2(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_AMPCOMPTH2, ui32Trim);
    ui32Trim = GetTrimForAmpcompTh1(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_AMPCOMPTH1, ui32Trim);
    ui32Trim = GetTrimForAmpcompCtrl(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_AMPCOMPCTL, ui32Trim);

    //
    // Set trim for DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_MODE_EN in acordance to FCFG1 setting
    // This is bit[5] in the DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL register
    // Using MASK4 write + 1 => writing to bits[7:4]
    //
    ui32Trim = GetTrimForAdcShModeEn( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL * 2 ) + 1 ) =
      ( 0x20 | ( ui32Trim << 1 ));

    //
    // Set trim for DDI_0_OSC_ADCDOUBLERNANOAMPCTL_ADC_SH_VBUF_EN in acordance to FCFG1 setting
    // This is bit[4] in the DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL register
    // Using MASK4 write + 1 => writing to bits[7:4]
    //
    ui32Trim = GetTrimForAdcShVbufEn( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL * 2 ) + 1 ) =
      ( 0x10 | ( ui32Trim ));

    //
    // Set trim for the PEAK_DET_ITRIM, HP_BUF_ITRIM and LP_BUF_ITRIM bit fields
    // in the DDI0_OSC_O_XOSCHFCTL register in accordance to FCFG1 setting.
    // Remaining register bit fields are set to their reset values of 0.
    //
    ui32Trim = GetTrimForXoscHfCtl(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_XOSCHFCTL, ui32Trim);

    //
    // Set trim for DBLR_LOOP_FILTER_RESET_VOLTAGE in accordance to FCFG1 setting
    // (This is bits [18:17] in DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL)
    // (Using MASK4 write + 4 => writing to bits[19:16] => (4*4))
    // (Assuming: DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_RESET_VOLTAGE_S = 17 and
    //  that DDI_0_OSC_ADCDOUBLERNANOAMPCTL_DBLR_LOOP_FILTER_RESET_VOLTAGE_M = 0x00060000)
    //
    ui32Trim = GetTrimForDblrLoopFilterResetVoltage( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_ADCDOUBLERNANOAMPCTL * 2 ) + 4 ) =
      ( 0x60 | ( ui32Trim << 1 ));

    //
    // Update DDI_0_OSC_ATESTCTL_ATESTLF_RCOSCLF_IBIAS_TRIM with data from
    // FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM
    // This is DDI_0_OSC_O_ATESTCTL bit[7]
    // ( DDI_0_OSC_O_ATESTCTL is currently hidden (but=0x00000020))
    // Using MASK4 write + 1 => writing to bits[7:4]
    //
    ui32Trim = GetTrimForRcOscLfIBiasTrim( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( 0x00000020 * 2 ) + 1 ) =
      ( 0x80 | ( ui32Trim << 3 ));

    //
    // Update DDI_0_OSC_LFOSCCTL_XOSCLF_REGULATOR_TRIM and
    //        DDI_0_OSC_LFOSCCTL_XOSCLF_CMIRRWR_RATIO in one write
    // This can be simplified since the registers are packed together in the same
    // order both in FCFG1 and in the HW register.
    // This spans DDI_0_OSC_O_LFOSCCTL bits[23:18]
    // Using MASK8 write + 4 => writing to bits[23:16]
    //
    ui32Trim = GetTrimForXoscLfRegulatorAndCmirrwrRatio( ui32Fcfg1Revision );
    HWREGH( AUX_DDI0_OSC_BASE + DDI_O_MASK8B + ( DDI_0_OSC_O_LFOSCCTL * 2 ) + 4 ) =
      ( 0xFC00 | ( ui32Trim << 2 ));

    //
    // Set trim the HPM_IBIAS_WAIT_CNT, LPM_IBIAS_WAIT_CNT and IDAC_STEP bit
    // fields in the DDI0_OSC_O_RADCEXTCFG register in accordance to FCFG1 setting.
    // Remaining register bit fields are set to their reset values of 0.
    //
    ui32Trim = GetTrimForRadcExtCfg(ui32Fcfg1Revision);
    DDI32RegWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_RADCEXTCFG, ui32Trim);

    // Setting FORCE_KICKSTART_EN (ref. CC26_V1_BUG00261). Should also be done for PG2
    // (This is bit 22 in DDI_0_OSC_O_CTL0)
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_CTL0 * 2 ) + 5 ) = 0x44;

    // Set bit DDI_0_OSC_CTL0_XTAL_IS_24M (this is bit 31 in DDI_0_OSC_O_CTL0)
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( DDI_0_OSC_O_CTL0 * 2 ) + 7 ) = 0x88;

    // Setting DDI_0_OSC_CTL1_XOSC_HF_FAST_START according to value found in FCFG1
    ui32Trim = GetTrimForXoscHfFastStart( ui32Fcfg1Revision );
    HWREGB( AUX_DDI0_OSC_BASE + DDI_O_MASK4B + ( 0x00000004 * 2 )) = ( 0x30 | ui32Trim );

    //
    // Update ADI_4_AUX_ADCREF1_VTRIM with value from FCFG1
    //
    HWREGB( AUX_ADI4_BASE + ADI_4_AUX_O_ADCREF1  ) =
      ((( HWREG( FCFG1_BASE + FCFG1_O_SOC_ADC_REF_TRIM_AND_OFFSET_EXT ) >>
      FCFG1_SOC_ADC_REF_TRIM_AND_OFFSET_EXT_SOC_ADC_REF_VOLTAGE_TRIM_TEMP1_S ) <<
      ADI_4_AUX_ADCREF1_VTRIM_S ) &
      ADI_4_AUX_ADCREF1_VTRIM_M );

    //
    // Set ADI_4_AUX:ADC0.SMPL_CYCLE_EXP to it's default minimum value (=3)
    // (Note: Using MASK8B requires that the bits to be modified must be within the same
    //        byte boundary whivh is the case for the ADI_4_AUX_ADC0_SMPL_CYCLE_EXP field)
    //
    HWREGH( AUX_ADI4_BASE + ADI_O_MASK8B + ( ADI_4_AUX_O_ADC0 * 2 )) =
      ( ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_M << 8 ) | ( 3 << ADI_4_AUX_ADC0_SMPL_CYCLE_EXP_S );

    //
    // Sync with AON
    //
    SysCtrlAonSync();

    //
    // Disable clock for OSC_DIG and release power on AUX
    //
    HWREG(AUX_WUC_BASE + AUX_WUC_O_MODCLKEN0) = 0x0;

    //
    // Release "Force AUX on"
    //
    // No need to save the current status of the power/clock registers.
    // At this point both AUX and AON should have been reset to 0x0.
    //
    HWREG(AON_WUC_BASE + AON_WUC_O_AUXCTL) = 0;

#if ( CC_GET_CHIP_OPTION != CC_CHIP_OPTION_OTP )
    // Disable EFUSE clock
    HWREGBITW( FLASH_BASE + FLASH_O_CFG, FLASH_CFG_DIS_EFUSECLK_BITN ) = 1;
#endif
}

#if ( INCLUDE_VDDR_TEMPORARILY )
static int32_t
SignExtendVddrTrimValue( uint32_t ui32VddrTrimVal )
{
    //
    // The VDDR trim value is 5 bits representing the range from -10 to +21
    // (where -10=0x16, -1=0x1F, 0=0x00, 1=0x01 and +21=0x15)
    //
    int32_t i32SignedVddrVal = ui32VddrTrimVal;
    if ( i32SignedVddrVal > 0x15 ) {
        i32SignedVddrVal -= 0x20;
    }
    return ( i32SignedVddrVal );
}
#endif

static void
HapiTrimDeviceColdReset(uint32_t ui32Fcfg1Revision)
{
#if ( INCLUDE_VDDR_TEMPORARILY )
    int32_t i32TargetTrim   ; // Target trim sign extended
    int32_t i32CurrentTrim  ; // Current trim sign extended
    int32_t i32DeltaVal     ; // Delta Value for each 1/64 loop

    //
    // Check if trim of Global LDO is required. Only required on PG1 devices
    //
    //
    // PG1 device. Trim the Global LDO in staircase steps to avoid
    // system reset while all the BOD reset sources are disabled
    //
    i32TargetTrim = SignExtendVddrTrimValue((
      HWREG( FCFG1_BASE + FCFG1_O_SHDW_ANA_TRIM ) &
         FCFG1_SHDW_ANA_TRIM_VDDR_TRIM_M ) >>
         FCFG1_SHDW_ANA_TRIM_VDDR_TRIM_S );

    i32CurrentTrim = SignExtendVddrTrimValue((
        HWREGB( ADI3_BASE + ADI_3_REFSYS_O_DCDCCTL0 ) &
        ADI_3_REFSYS_DCDCCTL0_VDDR_TRIM_M ) >>
        ADI_3_REFSYS_DCDCCTL0_VDDR_TRIM_S );

    if ( i32TargetTrim != i32CurrentTrim ) {
        int32_t i32Cnt; // Counter for trim loop
        //
        // Disbale all BOD reset sources
        //
        HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) = 0x00;

        i32DeltaVal = i32TargetTrim - i32CurrentTrim;
        if ( i32DeltaVal > 0 ) {
            i32DeltaVal -= 1;
            i32CurrentTrim  = ( i32CurrentTrim + 1 ) << 6;
        } else { // ( i32DeltaVal < 0 ) (will never become == 0)
            i32DeltaVal += 1;
            i32CurrentTrim  = ( i32CurrentTrim << 6 ) - 1;
        }

        for ( i32Cnt = 0 ; i32Cnt < 64 ; i32Cnt++ ) {
            i32CurrentTrim += i32DeltaVal;

            HWREGB( ADI3_BASE + ADI_O_DIR + ADI_3_REFSYS_O_DCDCCTL0 ) =
                (( i32CurrentTrim >> 6 ) <<
                ADI_3_REFSYS_DCDCCTL0_VDDR_TRIM_S ) &
                ADI_3_REFSYS_DCDCCTL0_VDDR_TRIM_M;

            //
            // Delay a total of approx. 300 uS for all 64 rounds in the loop
            //
            CPU_DELAY_MICRO_SECONDS( 300.0 / 64.0 );
        }

        //
        // Delay additionally 700 uS if going more than 1 step down
        // (i32DeltaVal is less than 0 only when going more than one step down)
        //
        if ( i32DeltaVal < 0 ) {
            CPU_DELAY_MICRO_SECONDS( 700.0 );
        }

        //
        // Enable all BOD reset sources
        //
        HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) = (
            AON_SYSCTL_RESETCTL_CLK_LOSS_EN  |
            AON_SYSCTL_RESETCTL_VDD_LOSS_EN  |
            AON_SYSCTL_RESETCTL_VDDR_LOSS_EN |
            AON_SYSCTL_RESETCTL_VDDS_LOSS_EN   );
    }
#endif
}

//*****************************************************************************
//
//! Returns the trim value to be used for the ANABYPASS_VALUE1 register in
//! OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForAnabypassValue1( uint32_t ui32Fcfg1Revision, uint32_t ccfg_ModeConfReg )
{
    uint32_t ui32Fcfg1Value            ;
    uint32_t ui32XoscHfRow             ;
    uint32_t ui32XoscHfCol             ;
    int32_t  i32CustomerDeltaAdjust    ;
    uint32_t ui32TrimValue             ;

    if ( ui32Fcfg1Revision == 0 ) {
        // Set hardcoded trim values for all bit fields in the
        // ANABYPASS_VALUE1 register
        ui32TrimValue = 0x000F0FFF;
    } else {
        // Use device specific trim values located in factory configuration
        // area for the XOSC_HF_COLUMN_Q12 and XOSC_HF_ROW_Q12 bit fields in
        // the ANABYPASS_VALUE1 register. Value for the other bit fields
        // are set to 0.

        ui32Fcfg1Value = HWREG(FCFG1_BASE + FCFG1_O_CONFIG_OSC_TOP);
        ui32XoscHfRow = (( ui32Fcfg1Value &
            FCFG1_CONFIG_OSC_TOP_XOSC_HF_ROW_Q12_M ) >>
            FCFG1_CONFIG_OSC_TOP_XOSC_HF_ROW_Q12_S );
        ui32XoscHfCol = (( ui32Fcfg1Value &
            FCFG1_CONFIG_OSC_TOP_XOSC_HF_COLUMN_Q12_M ) >>
            FCFG1_CONFIG_OSC_TOP_XOSC_HF_COLUMN_Q12_S );

        i32CustomerDeltaAdjust = 0;
        if (( ccfg_ModeConfReg & CCFG_MODE_CONF_XOSC_CAP_MOD ) == 0 ) {
            // XOSC_CAP_MOD = 0 means: CAP_ARRAY_DELTA is in use -> Apply compensation
            // XOSC_CAPARRAY_DELTA is located in bit[15:8] of ccfg_ModeConfReg
            // Note: HW_REV_DEPENDENT_IMPLEMENTATION. Field width is not given by
            // a define and sign extention must therefore be hardcoded.
            // ( A small testprogram is created vrifying the code lines below:
            //   Ref.: ..\test\small_standalone_test_programs\CapArrayDeltaAdjust_test.c)
            i32CustomerDeltaAdjust = ((int32_t)ccfg_ModeConfReg << 16 ) >> 24;

            while ( i32CustomerDeltaAdjust < 0 ) {
                ui32XoscHfCol >>= 1;                              // COL 1 step down
                if ( ui32XoscHfCol == 0 ) {                       // if COL below minimum
                    ui32XoscHfCol = 0xFFFF;                       //   Set COL to maximum
                    ui32XoscHfRow >>= 1;                          //   ROW 1 step down
                    if ( ui32XoscHfRow == 0 ) {                   // if ROW below minimum
                       ui32XoscHfRow = 1;                         //   Set both ROW and COL
                       ui32XoscHfCol = 1;                         //   to minimum
                    }
                }
                i32CustomerDeltaAdjust++;
            }
            while ( i32CustomerDeltaAdjust > 0 ) {
                ui32XoscHfCol = ( ui32XoscHfCol << 1 ) | 1;       // COL 1 step up
                if ( ui32XoscHfCol > 0xFFFF ) {                   // if COL abowe maximum
                    ui32XoscHfCol = 1;                            //   Set COL to minimum
                    ui32XoscHfRow = ( ui32XoscHfRow << 1 ) | 1;   //   ROW 1 step up
                    if ( ui32XoscHfRow > 0xF ) {                  // if ROW abowe maximum
                       ui32XoscHfRow = 0xF;                       //   Set both ROW and COL
                       ui32XoscHfCol = 0xFFFF;                    //   to maximum
                    }
                }
                i32CustomerDeltaAdjust--;
            }
        }

        ui32TrimValue = (( ui32XoscHfRow << DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_ROW_Q12_S    ) |
                         ( ui32XoscHfCol << DDI_0_OSC_ANABYPASSVAL1_XOSC_HF_COLUMN_Q12_S )   );
    }

    return (ui32TrimValue);
}

//*****************************************************************************
//
//! Returns the trim value to be used for the RCOSCLF_RTUNE_TRIM and the
//! RCOSCLF_CTUNE_TRIM bit fields in the XOSCLF_RCOSCLF_CTRL register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForRcOscLfRtuneCtuneTrim(uint32_t ui32Fcfg1Revision)
{
    uint32_t ui32TrimValue;

    if ( ui32Fcfg1Revision == 0 ) {
        // Use hardcoded trim value
        ui32TrimValue = 0x00D8;
    } else {
        // Use device specific trim values located in factory configuration
        // area
        ui32TrimValue =
            ((HWREG(FCFG1_BASE + FCFG1_O_CONFIG_OSC_TOP) &
              FCFG1_CONFIG_OSC_TOP_RCOSCLF_CTUNE_TRIM_M)>>
              FCFG1_CONFIG_OSC_TOP_RCOSCLF_CTUNE_TRIM_S)<<
                DDI_0_OSC_LFOSCCTL_RCOSCLF_CTUNE_TRIM_S;

        ui32TrimValue |=
            ((HWREG(FCFG1_BASE + FCFG1_O_CONFIG_OSC_TOP) &
              FCFG1_CONFIG_OSC_TOP_RCOSCLF_RTUNE_TRIM_M)>>
              FCFG1_CONFIG_OSC_TOP_RCOSCLF_RTUNE_TRIM_S)<<
                DDI_0_OSC_LFOSCCTL_RCOSCLF_RTUNE_TRIM_S;
    }

    return(ui32TrimValue);
}

//*****************************************************************************
//
//! Returns the trim value to be used for the XOSC_HF_IBIASTHERM bit field in
//! the ANABYPASS_VALUE2 register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForXoscHfIbiastherm(uint32_t ui32Fcfg1Revision)
{
    uint32_t ui32TrimValue;

    if ( ui32Fcfg1Revision == 0 ) {
        // Use hardcoded trim value
        ui32TrimValue = 0x03FF;
    } else {
        // Use device specific trim value located in factory configuration
        // area
        ui32TrimValue =
            (HWREG(FCFG1_BASE + FCFG1_O_ANABYPASS_VALUE2) &
             FCFG1_ANABYPASS_VALUE2_XOSC_HF_IBIASTHERM_M)>>
             FCFG1_ANABYPASS_VALUE2_XOSC_HF_IBIASTHERM_S;
    }

    return(ui32TrimValue);
}

//*****************************************************************************
//
//! Returns the trim value to be used for the AMPCOMP_TH2 register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForAmpcompTh2(uint32_t ui32Fcfg1Revision)
{
    uint32_t ui32TrimValue;
    uint32_t ui32Fcfg1Value;

    if ( ui32Fcfg1Revision == 0 ) {
        // Use hardcoded trim values
        ui32TrimValue = 0x68880000;
    } else {
        // Use device specific trim value located in factory configuration
        // area. All defined register bit fields have corresponding trim
        // value in the factory configuration area
        ui32Fcfg1Value = HWREG(FCFG1_BASE + FCFG1_O_AMPCOMP_TH2);
        ui32TrimValue = ((ui32Fcfg1Value &
                          FCFG1_AMPCOMP_TH2_LPMUPDATE_LTH_M)>>
                          FCFG1_AMPCOMP_TH2_LPMUPDATE_LTH_S)<<
                       DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_LTH_S;
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH2_LPMUPDATE_HTM_M)>>
                            FCFG1_AMPCOMP_TH2_LPMUPDATE_HTM_S)<<
                         DDI_0_OSC_AMPCOMPTH2_LPMUPDATE_HTH_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_LPM_M)>>
                            FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_LPM_S)<<
                         DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_LPM_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_HPM_M)>>
                            FCFG1_AMPCOMP_TH2_ADC_COMP_AMPTH_HPM_S)<<
                         DDI_0_OSC_AMPCOMPTH2_ADC_COMP_AMPTH_HPM_S);
    }

    return(ui32TrimValue);
}

//*****************************************************************************
//
//! Returns the trim value to be used for the AMPCOMP_TH1 register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForAmpcompTh1(uint32_t ui32Fcfg1Revision)
{
    uint32_t ui32TrimValue;
    uint32_t ui32Fcfg1Value;

    if ( ui32Fcfg1Revision == 0 ) {
        // Use hardcoded trim values
        ui32TrimValue = 0x0068768A;
    } else {
        // Use device specific trim values located in factory configuration
        // area. All defined register bit fields have a corresponding trim
        // value in the factory configuration area
        ui32Fcfg1Value = HWREG(FCFG1_BASE + FCFG1_O_AMPCOMP_TH1);
        ui32TrimValue = (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH1_HPMRAMP3_LTH_M)>>
                            FCFG1_AMPCOMP_TH1_HPMRAMP3_LTH_S)<<
                         DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_LTH_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH1_HPMRAMP3_HTH_M)>>
                            FCFG1_AMPCOMP_TH1_HPMRAMP3_HTH_S)<<
                         DDI_0_OSC_AMPCOMPTH1_HPMRAMP3_HTH_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH1_IBIASCAP_LPTOHP_OL_CNT_M)>>
                            FCFG1_AMPCOMP_TH1_IBIASCAP_LPTOHP_OL_CNT_S)<<
                         DDI_0_OSC_AMPCOMPTH1_IBIASCAP_LPTOHP_OL_CNT_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_TH1_HPMRAMP1_TH_M)>>
                            FCFG1_AMPCOMP_TH1_HPMRAMP1_TH_S)<<
                         DDI_0_OSC_AMPCOMPTH1_HPMRAMP1_TH_S);
    }

    return(ui32TrimValue);
}

//*****************************************************************************
//
//! Returns the trim value to be used for the AMPCOMP_CTRL register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForAmpcompCtrl(uint32_t ui32Fcfg1Revision)
{
    uint32_t ui32TrimValue    ;
    uint32_t ui32Fcfg1Value   ;
    uint32_t ibiasOffset      ;
    uint32_t ibiasInit        ;
    uint32_t modeConf1        ;
    int32_t  deltaAdjust      ;

    if ( ui32Fcfg1Revision == 0 ) {
        // Use hardcoded trim values
        ui32TrimValue = 0x00713F27;
    } else {
        // Use device specific trim valuse loacted in factory configuration
        // area. Register bit fields without trim values in the factory
        // configuration area will be set to the value of 0.
        ui32Fcfg1Value = HWREG( FCFG1_BASE + FCFG1_O_AMPCOMP_CTRL1 );

        ibiasOffset    = ( ui32Fcfg1Value &
                           FCFG1_AMPCOMP_CTRL1_IBIAS_OFFSET_M ) >>
                           FCFG1_AMPCOMP_CTRL1_IBIAS_OFFSET_S ;
        ibiasInit      = ( ui32Fcfg1Value &
                           FCFG1_AMPCOMP_CTRL1_IBIAS_INIT_M ) >>
                           FCFG1_AMPCOMP_CTRL1_IBIAS_INIT_S ;

        if (( HWREG( CCFG_BASE + CCFG_O_SIZE_AND_DIS_FLAGS ) & CCFG_SIZE_AND_DIS_FLAGS_DIS_XOSC_OVR_M ) == 0 ) {
            // Adjust with DELTA_IBIAS_OFFSET and DELTA_IBIAS_INIT from CCFG
            modeConf1   = HWREG( CCFG_BASE + CCFG_O_MODE_CONF_1 );

            // Both fields are signed 4-bit values. This is an assumption when doing the sign extention.
            deltaAdjust = (int32_t)modeConf1 << ( 32 - CCFG_MODE_CONF_1_DELTA_IBIAS_OFFSET_S - 4 ) >> 28;
            deltaAdjust += (int32_t)ibiasOffset;
            if ( deltaAdjust < 0 ) {
               deltaAdjust = 0;
            }
            if ( deltaAdjust > ( DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S )) {
                deltaAdjust  = ( DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S );
            }
            ibiasOffset = (uint32_t)deltaAdjust;

            deltaAdjust = (int32_t)modeConf1 << ( 32 - CCFG_MODE_CONF_1_DELTA_IBIAS_INIT_S - 4 ) >> 28;
            deltaAdjust += (int32_t)ibiasInit;
            if ( deltaAdjust < 0 ) {
               deltaAdjust = 0;
            }
            if ( deltaAdjust > ( DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S )) {
                deltaAdjust  = ( DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_M >> DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S );
            }
            ibiasInit = (uint32_t)deltaAdjust;
        }
        ui32TrimValue = ( ibiasOffset << DDI_0_OSC_AMPCOMPCTL_IBIAS_OFFSET_S ) |
                        ( ibiasInit   << DDI_0_OSC_AMPCOMPCTL_IBIAS_INIT_S   ) ;

        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_CTRL1_LPM_IBIAS_WAIT_CNT_FINAL_M)>>
                            FCFG1_AMPCOMP_CTRL1_LPM_IBIAS_WAIT_CNT_FINAL_S)<<
                           DDI_0_OSC_AMPCOMPCTL_LPM_IBIAS_WAIT_CNT_FINAL_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_CTRL1_CAP_STEP_M)>>
                            FCFG1_AMPCOMP_CTRL1_CAP_STEP_S)<<
                           DDI_0_OSC_AMPCOMPCTL_CAP_STEP_S);
        ui32TrimValue |= (((ui32Fcfg1Value &
                            FCFG1_AMPCOMP_CTRL1_IBIASCAP_HPTOLP_OL_CNT_M)>>
                            FCFG1_AMPCOMP_CTRL1_IBIASCAP_HPTOLP_OL_CNT_S)<<
                           DDI_0_OSC_AMPCOMPCTL_IBIASCAP_HPTOLP_OL_CNT_S);

        if ( ui32Fcfg1Revision >= 0x00000022 ) {
            ui32TrimValue |= ((( ui32Fcfg1Value &
                FCFG1_AMPCOMP_CTRL1_AMPCOMP_REQ_MODE_M ) >>
                FCFG1_AMPCOMP_CTRL1_AMPCOMP_REQ_MODE_S ) <<
               DDI_0_OSC_AMPCOMPCTL_AMPCOMP_REQ_MODE_S );
        }
    }

    return(ui32TrimValue);
}

//*****************************************************************************
//
//! Returns the trim value from FCFG1 to be used as DBLR_LOOP_FILTER_RESET_VOLTAGE setting
//
//*****************************************************************************
static uint32_t
GetTrimForDblrLoopFilterResetVoltage( uint32_t ui32Fcfg1Revision )
{
   uint32_t dblrLoopFilterResetVoltageValue = 0; // Reset value

   if ( ui32Fcfg1Revision >= 0x00000020 ) {
      dblrLoopFilterResetVoltageValue = ( HWREG( FCFG1_BASE + FCFG1_O_MISC_OTP_DATA_1 ) &
         FCFG1_MISC_OTP_DATA_1_DBLR_LOOP_FILTER_RESET_VOLTAGE_M ) >>
         FCFG1_MISC_OTP_DATA_1_DBLR_LOOP_FILTER_RESET_VOLTAGE_S;
   }

   return ( dblrLoopFilterResetVoltageValue );
}

//*****************************************************************************
//
//! Returns the trim value from FCFG1 to be used as ADC_SH_MODE_EN setting
//
//*****************************************************************************
static uint32_t
GetTrimForAdcShModeEn( uint32_t ui32Fcfg1Revision )
{
   uint32_t getTrimForAdcShModeEnValue = 1; // Recommanded default setting

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      getTrimForAdcShModeEnValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_ADC_SH_MODE_EN_M ) >>
         FCFG1_OSC_CONF_ADC_SH_MODE_EN_S;
   }

   return ( getTrimForAdcShModeEnValue );
}

//*****************************************************************************
//
//! Returns the trim value from FCFG1 to be used as ADC_SH_VBUF_EN setting
//
//*****************************************************************************
static uint32_t
GetTrimForAdcShVbufEn( uint32_t ui32Fcfg1Revision )
{
   uint32_t getTrimForAdcShVbufEnValue = 1; // Recommanded default setting

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      getTrimForAdcShVbufEnValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_ADC_SH_VBUF_EN_M ) >>
         FCFG1_OSC_CONF_ADC_SH_VBUF_EN_S;
   }

   return ( getTrimForAdcShVbufEnValue );
}

//*****************************************************************************
//
//! Returns the trim value to be used for the XOSCHFCTL register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForXoscHfCtl( uint32_t ui32Fcfg1Revision )
{
   uint32_t getTrimForXoschfCtlValue = 0; // Recommanded default setting
   uint32_t fcfg1Data;

   if ( ui32Fcfg1Revision >= 0x00000020 ) {
      fcfg1Data = HWREG( FCFG1_BASE + FCFG1_O_MISC_OTP_DATA_1 );
      getTrimForXoschfCtlValue =
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_PEAK_DET_ITRIM_M ) >>
             FCFG1_MISC_OTP_DATA_1_PEAK_DET_ITRIM_S ) <<
           DDI_0_OSC_XOSCHFCTL_PEAK_DET_ITRIM_S);

      getTrimForXoschfCtlValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_HP_BUF_ITRIM_M ) >>
             FCFG1_MISC_OTP_DATA_1_HP_BUF_ITRIM_S ) <<
           DDI_0_OSC_XOSCHFCTL_HP_BUF_ITRIM_S);

      getTrimForXoschfCtlValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_LP_BUF_ITRIM_M ) >>
             FCFG1_MISC_OTP_DATA_1_LP_BUF_ITRIM_S ) <<
           DDI_0_OSC_XOSCHFCTL_LP_BUF_ITRIM_S);
   }

   return ( getTrimForXoschfCtlValue );
}

//*****************************************************************************
//
//! Returns the trim value to be used as OSC_DIG:CTL1.XOSC_HF_FAST_START
//
//*****************************************************************************
static uint32_t
GetTrimForXoscHfFastStart( uint32_t ui32Fcfg1Revision )
{
   uint32_t ui32XoscHfFastStartValue   ;

   if ( ui32Fcfg1Revision == 0 ) {
      // Use reset value
      ui32XoscHfFastStartValue = 0;
   } else {
      // Get value from FCFG1
      ui32XoscHfFastStartValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_XOSC_HF_FAST_START_M ) >>
         FCFG1_OSC_CONF_XOSC_HF_FAST_START_S;
   }

   return ( ui32XoscHfFastStartValue );
}

//*****************************************************************************
//
//! Returns the trim value to be used for the RADCEXTCFG register in OSC_DIG
//
//*****************************************************************************
static uint32_t
GetTrimForRadcExtCfg( uint32_t ui32Fcfg1Revision )
{
   uint32_t getTrimForRadcExtCfgValue = 0x403F8000; // Recommanded default setting
   uint32_t fcfg1Data;

   if ( ui32Fcfg1Revision >= 0x00000020 ) {
      fcfg1Data = HWREG( FCFG1_BASE + FCFG1_O_MISC_OTP_DATA_1 );
      getTrimForRadcExtCfgValue =
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_HPM_IBIAS_WAIT_CNT_M ) >>
             FCFG1_MISC_OTP_DATA_1_HPM_IBIAS_WAIT_CNT_S ) <<
           DDI_0_OSC_RADCEXTCFG_HPM_IBIAS_WAIT_CNT_S);

      getTrimForRadcExtCfgValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_LPM_IBIAS_WAIT_CNT_M ) >>
             FCFG1_MISC_OTP_DATA_1_LPM_IBIAS_WAIT_CNT_S ) <<
           DDI_0_OSC_RADCEXTCFG_LPM_IBIAS_WAIT_CNT_S);

      getTrimForRadcExtCfgValue |=
         ( ( ( fcfg1Data & FCFG1_MISC_OTP_DATA_1_IDAC_STEP_M ) >>
             FCFG1_MISC_OTP_DATA_1_IDAC_STEP_S ) <<
           DDI_0_OSC_RADCEXTCFG_IDAC_STEP_S);
   }

   return ( getTrimForRadcExtCfgValue );
}

//*****************************************************************************
//
// Returns the FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM
//
//*****************************************************************************
static uint32_t
GetTrimForRcOscLfIBiasTrim( uint32_t ui32Fcfg1Revision )
{
   uint32_t trimForRcOscLfIBiasTrimValue = 0; // Default value

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      trimForRcOscLfIBiasTrimValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM_M ) >>
         FCFG1_OSC_CONF_ATESTLF_RCOSCLF_IBIAS_TRIM_S ;
   }

   return ( trimForRcOscLfIBiasTrimValue );
}

//*****************************************************************************
//
// Returns XOSCLF_REGULATOR_TRIM and XOSCLF_CMIRRWR_RATIO as one packet
// spanning bits [5:0] in the returned value.
//
//*****************************************************************************
static uint32_t
GetTrimForXoscLfRegulatorAndCmirrwrRatio( uint32_t ui32Fcfg1Revision )
{
   uint32_t trimForXoscLfRegulatorAndCmirrwrRatioValue = 0; // Default value for both fields

   if ( ui32Fcfg1Revision >= 0x00000022 ) {
      trimForXoscLfRegulatorAndCmirrwrRatioValue = ( HWREG( FCFG1_BASE + FCFG1_O_OSC_CONF ) &
         ( FCFG1_OSC_CONF_XOSCLF_REGULATOR_TRIM_M |
           FCFG1_OSC_CONF_XOSCLF_CMIRRWR_RATIO_M  )) >>
           FCFG1_OSC_CONF_XOSCLF_CMIRRWR_RATIO_S  ;
   }

   return ( trimForXoscLfRegulatorAndCmirrwrRatioValue );
}
