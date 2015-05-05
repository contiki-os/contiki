/******************************************************************************
*  Filename:       sys_ctrl.c
*  Revised:        2015-01-15 14:12:37 +0100 (to, 15 jan 2015)
*  Revision:       42392
*
*  Description:    Driver for the System Control.
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
#include <inc/hw_types.h>
#include <inc/hw_ccfg.h>
// Driverlib headers
#include <driverlib/aon_batmon.h>
#include <driverlib/sys_ctrl.h>

//*****************************************************************************
//
// Handle support for DriverLib in ROM:
// This section will undo prototype renaming made in the header file
//
//*****************************************************************************
#ifndef DRIVERLIB_GENERATE_ROM
    #undef  SysCtrlPowerEverything
    #define SysCtrlPowerEverything          NOROM_SysCtrlPowerEverything
    #undef  SysCtrlStandby
    #define SysCtrlStandby                  NOROM_SysCtrlStandby
    #undef  SysCtrlPowerdown
    #define SysCtrlPowerdown                NOROM_SysCtrlPowerdown
    #undef  SysCtrlShutdown
    #define SysCtrlShutdown                 NOROM_SysCtrlShutdown
    #undef  SysCtrlClockGet
    #define SysCtrlClockGet                 NOROM_SysCtrlClockGet
    #undef  SysCtrlPeripheralClockGet
    #define SysCtrlPeripheralClockGet       NOROM_SysCtrlPeripheralClockGet
    #undef  SysCtrlResetSourceGet
    #define SysCtrlResetSourceGet           NOROM_SysCtrlResetSourceGet
#endif

//*****************************************************************************
//
// Recharge calculator defines and globals
//
//*****************************************************************************

#define  PD_STATE_CACHE_RET      1
#define  PD_STATE_RFMEM_RET      2
#define  PD_STATE_XOSC_LPM       4

typedef struct {
   uint32_t    pdTime               ;
   uint16_t    pdRechargePeriod     ;
   uint8_t     pdState              ;
   int8_t      pdTemp               ;
} PowerQualGlobals_t;

static PowerQualGlobals_t powerQualGlobals;


//*****************************************************************************
//
// Arrays that maps the "peripheral set" number (which is stored in the
// third nibble of the PRCM_PERIPH_* defines) to the PRCM register that
// contains the relevant bit for that peripheral.
//
//*****************************************************************************

// Run mode registers
static const uint32_t g_pui32ModuleCG[] =
{
    PRCM_PERIPH_TIMER0,
    PRCM_PERIPH_TIMER1,
    PRCM_PERIPH_TIMER2,
    PRCM_PERIPH_TIMER3,
    PRCM_PERIPH_SSI0,
    PRCM_PERIPH_SSI1,
    PRCM_PERIPH_UART0,
    PRCM_PERIPH_I2C0,
    PRCM_PERIPH_UDMA,
    PRCM_PERIPH_TRNG,
    PRCM_PERIPH_CRYPTO,
    PRCM_PERIPH_GPIO,
    PRCM_PERIPH_I2S
};

//*****************************************************************************
//
//! Power up everything
//
//*****************************************************************************
void
SysCtrlPowerEverything(void)
{
    uint32_t ui32Idx;
    uint32_t ui32AuxClocks;

    //
    // Force power on AUX
    //
    AONWUCAuxWakeupEvent(AONWUC_AUX_WAKEUP);
    while(!(AONWUCPowerStatus() & AONWUC_AUX_POWER_ON))
    { }

    //
    // Enable all the AUX domain clocks and wait for them to be ready
    //
    ui32AuxClocks = AUX_WUC_ADI_CLOCK | AUX_WUC_OSCCTRL_CLOCK |
                    AUX_WUC_TDCIF_CLOCK | AUX_WUC_SOC_CLOCK |
                    AUX_WUC_TIMER_CLOCK | AUX_WUC_AIODIO0_CLOCK |
                    AUX_WUC_AIODIO1_CLOCK | AUX_WUC_SMPH_CLOCK |
                    AUX_WUC_TDC_CLOCK | AUX_WUC_ADC_CLOCK |
                    AUX_WUC_REF_CLOCK;
    AUXWUCClockEnable(ui32AuxClocks);
    while(AUXWUCClockStatus(ui32AuxClocks) != AUX_WUC_CLOCK_READY)
    { }

    //
    // Request to switch to the crystal to enable radio operation.
    // It takes a while for the XTAL to be ready so it is possible to
    // perform other tasks while waiting.
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_XOSC_HF);
    OSCClockSourceSet(OSC_SRC_CLK_LF, OSC_XOSC_LF);

    //
    // Switch the HF source to XTAL - must be performed safely out of ROM to
    // avoid flash issues when switching the clock.
    //
    // NB. If already running XTAL on HF clock source the ROM will wait forever
    // on a flag that will never be set - need to check.
    //
    if(OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_XOSC_HF) {
        OSCHfSourceSwitch();
    }

    //
    // Turn on all the MCU power domains
    // If the CPU is running and executing code the SYSBUS, VIMS and CPU are
    // automatically on as well.
    //
    PRCMPowerDomainOn(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_SERIAL |
                      PRCM_DOMAIN_PERIPH);
    //
    // Wait for power to be on
    //
    while(PRCMPowerDomainStatus(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_SERIAL |
                                PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON);

    //
    // Minimize all domain clock dividers and wait for the clock settings to
    // take effect
    //
    PRCMClockConfigureSet(PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_CPU |
                          PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_SERIAL |
                          PRCM_DOMAIN_TIMER, PRCM_CLOCK_DIV_1);
    PRCMLoadSet();
    while(!PRCMLoadGet());

    //
    // Ensure the domain clocks are running and wait for the clock settings to
    // take effect
    //
    PRCMDomainEnable(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_VIMS);
    PRCMLoadSet();
    while(!PRCMLoadGet())
    { }

    //
    // Enable all the RF Core clocks
    // TBD Encapsulate in an API
    //
    // Do not read back to check, for two reasons:
    // 1. CPE will update the PWMCLKENABLE register right after boot
    // 2. The PWMCLKENABLE register always reads back what is written
    HWREG(RFC_PWR_NONBUF_BASE + RFC_PWR_O_PWMCLKEN) = 0x7FF;

    //
    // Enable all peripheral clocks in CM3 run/sleep/deep-sleep mode.
    //
    for(ui32Idx = 0; ui32Idx < sizeof(g_pui32ModuleCG) / sizeof(uint32_t);
        ui32Idx++)
    {
        PRCMPeripheralRunEnable(g_pui32ModuleCG[ui32Idx]);
        PRCMPeripheralSleepEnable(g_pui32ModuleCG[ui32Idx]);
        PRCMPeripheralDeepSleepEnable(g_pui32ModuleCG[ui32Idx]);
    }
    PRCMLoadSet();
    while(!PRCMLoadGet())
    { }
}

//*****************************************************************************
//
//! Force the system in to standby mode
//
//*****************************************************************************
void SysCtrlStandby(void)
{
    //
    // Enable the oscillator configuration interface
    //
    OSCInterfaceEnable();

    //
    // Ensure the low frequency clock source is sourced from a low frequency
    // oscillator. The XTAL will provide the most accurate real time clock.
    //
    OSCClockSourceSet(OSC_SRC_CLK_LF,OSC_XOSC_LF);

    //
    // Enable the oscillator configuration interface
    //
    OSCInterfaceDisable();

    //
    // Execute the transition to standby
    //
    PowerCtrlStateSet(PWRCTRL_STANDBY);
}

//*****************************************************************************
//
//! Force the system in to power down.
//
//*****************************************************************************
void
SysCtrlPowerdown(void)
{
    //
    // Make sure the oscillator interface is enabled
    //
    OSCInterfaceEnable();

    //
    // Source the LF clock from the low frequency XTAL_OSC.
    // HF and MF are sourced from the high frequency RC_OSC.
    //
    OSCClockSourceSet(OSC_SRC_CLK_LF, OSC_XOSC_LF);
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_RCOSC_HF);

    //
    // Check if already sourcing the HF clock from RC_OSC.
    // If a switch of the clock source is not required, then the call to ROM
    // will loop forever.
    //
    if(OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_RCOSC_HF)
    {
        OSCHfSourceSwitch();
    }

    //
    // Disable the oscillator interface
    //
    OSCInterfaceDisable();

    //
    // Execute the transition to power down.
    //
    PowerCtrlStateSet(PWRCTRL_POWER_DOWN);
}

//*****************************************************************************
//
//! Force the system in to shutdown.
//
//*****************************************************************************
void
SysCtrlShutdown(void)
{
    //
    // Make sure the oscillator interface is enabled
    //
    OSCInterfaceEnable();

    //
    // Source the LF clock from the low frequency RC_OSC.
    // HF and MF are sourced from the high frequency RC_OSC.
    // TBD - Modify to source LF clock from XTAL
    //
    OSCClockSourceSet(OSC_SRC_CLK_LF, OSC_RCOSC_LF);
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_RCOSC_HF);

    //
    // Check if already sourcing the HF clock from RC_OSC.
    // If a switch of the clock source is not required, then the call to ROM
    // will loop forever.
    //
    if(OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_RCOSC_HF)
    {
        OSCHfSourceSwitch();
    }

    //
    // Disable the oscillator interface
    //
    OSCInterfaceDisable();

    //
    // Execute transition to shutdown.
    //
    PowerCtrlStateSet(PWRCTRL_SHUTDOWN);
}

//*****************************************************************************
//
//! Get the CPU core clock frequency.
//
//*****************************************************************************
uint32_t
SysCtrlClockGet(void)
{
    uint32_t ui32Div;

    //
    // Get the current clock divider for the CPU.
    //
    ui32Div = HWREG(PRCM_BASE + PRCM_O_CPUCLKDIV) ? 2 : 1;

    //
    // Get the current clock divider for the SYSBUS.
    //
    ui32Div <<= HWREG(PRCM_BASE + PRCM_O_SYSBUSCLKDIV);

    //
    // Return the current value of the divided clock.
    //
    return (GET_MCU_CLOCK / ui32Div);
}

//*****************************************************************************
//
//! Get the clock for a peripheral
//
//*****************************************************************************
uint32_t
SysCtrlPeripheralClockGet(uint32_t ui32Peripheral, uint32_t ui32BusMode)
{
    uint32_t ui32SysBusDiv;
    uint32_t ui32PerDiv;
    uint32_t ui32Div;
    uint32_t ui32ClkFreq;

    //
    // Check the arguments.
    //
    ASSERT((ui32BusMode == SYSCTRL_SYSBUS_ON) ||
           (ui32BusMode == SYSCTRL_SYSBUS_OFF));

    //
    // Get the system bus clock divider.
    //
    ui32SysBusDiv = PRCMClockConfigureGet(PRCM_DOMAIN_SYSBUS);

    //
    // Get the Peripheral clock divider.
    //
    ui32PerDiv = PRCMClockConfigureGet(PRCM_DOMAIN_PERIPH);

    //
    // Check clock mode.
    //
    ui32PerDiv = (ui32BusMode == SYSCTRL_SYSBUS_ON) ? ui32SysBusDiv : ui32PerDiv;

    //
    // Check type of peripheral and return correct clock frequency.
    //
    switch(ui32Peripheral)
    {
    case PRCM_PERIPH_TIMER0 :
    case PRCM_PERIPH_TIMER1 :
    case PRCM_PERIPH_TIMER2 :
    case PRCM_PERIPH_TIMER3 :
        //
        // Get the Timer clock divider and calculate the frequency.
        //
        ui32Div = PRCMClockConfigureGet(PRCM_DOMAIN_TIMER);

        if(ui32PerDiv < ui32Div)
        {
            ui32ClkFreq = GET_MCU_CLOCK >> ui32Div;
        }
        else
        {
            ui32ClkFreq = GET_MCU_CLOCK >> ui32PerDiv;
        }
        break;
    case PRCM_PERIPH_SSI0 :
    case PRCM_PERIPH_SSI1 :
    case PRCM_PERIPH_UART0 :
    case PRCM_PERIPH_UART1 :
        //
        // Get the Serial clock divider and calculate the frequency.
        //
        ui32Div = PRCMClockConfigureGet(PRCM_DOMAIN_SERIAL);
        ui32ClkFreq = GET_MCU_CLOCK >> ui32Div;
        break;
    case PRCM_PERIPH_I2C0 :
    case PRCM_PERIPH_I2C1 :
    case PRCM_PERIPH_UDMA :
    case PRCM_PERIPH_TRNG :
    case PRCM_PERIPH_CRYPTO :
    case PRCM_PERIPH_GPIO :
    case PRCM_PERIPH_I2S :
        ui32ClkFreq = GET_MCU_CLOCK >> ui32PerDiv;
        break;
    default :
        ui32ClkFreq = 0;
        break;
    }

    //
    // Return the clock frequency.
    //
    return (ui32ClkFreq);
}


//*****************************************************************************
//
// SysCtrlSetRechargeBeforePowerDown( xoscPowerMode )
//
//*****************************************************************************
void
SysCtrlSetRechargeBeforePowerDown( XoscPowerMode_t xoscPowerMode )
{
   int32_t           curTemp           ;
   int32_t           shiftedTemp       ;
   uint32_t          curState          ;
   uint32_t          prcmRamRetention  ;
   uint32_t          di                ;
   uint32_t          dii               ;
   uint32_t          ti                ;
   uint32_t          cd                ;
   uint32_t          cl                ;
   uint32_t          load              ;
   uint32_t          k                 ;
   uint32_t          vddrCap           ;
   uint32_t          newRechargePeriod ;
   uint32_t          perE              ;
   uint32_t          perM              ;
   const uint32_t  * pLookupTable      ;

   //--- Spec. point 1 ---
   curTemp  = AON_BatmonTempGetDegC();
   curState = 0;
   prcmRamRetention = HWREG( PRCM_BASE + PRCM_O_RAMRETEN );
   if ( prcmRamRetention & PRCM_RAMRETEN_VIMS_M ) {
      curState |= PD_STATE_CACHE_RET;
   }
   if ( prcmRamRetention & PRCM_RAMRETEN_RFC ) {
      curState |= PD_STATE_RFMEM_RET;
   }
   if ( xoscPowerMode != XoscInHighPowerMode ) {
      curState |= PD_STATE_XOSC_LPM;
   }

   pLookupTable = (uint32_t *)( FCFG1_BASE + FCFG1_O_PWD_CURR_20C );

   //--- Spec. point 2 ---
   if ((( curTemp - powerQualGlobals.pdTemp ) >= 5 ) || ( curState != powerQualGlobals.pdState )) {
      //--- Spec. point 3 ---
      shiftedTemp = curTemp - 15;

      //--- Spec point 4 ---
      //4.	Check for external VDDR load option (may not be supported): ext_load = (VDDR_EXT_LOAD=0 in CCFG)
      // Currently not implementing external load handling
      // if ( __ccfg.ulModeConfig & MODE_CONF_VDDR_EXT_LOAD ) {
      // }

      //--- Spec point 5 ---
      di    = 0;
      ti    = 0;
      if ( shiftedTemp >= 0 ) {
         //--- Spec point 5.a ---
         shiftedTemp += ( shiftedTemp << 4 );

         //--- Spec point 5.b ---
         ti = ( shiftedTemp >> 8 );
         if ( ti > 7 ) {
            ti = 7;
         }
         dii = ti;
         if ( dii > 6 ) {
            dii = 6;
         }

         //--- Spec point 5.c ---
         cd = pLookupTable[ dii + 1 ] - pLookupTable[ dii ];

         //--- Spec point 5.d ---
         di = cd & 0xFF;

         //--- Spec point 5.e ---
         if ( curState & PD_STATE_XOSC_LPM ) {
            di += (( cd >> 8 ) & 0xFF );
         }
         if ( curState & PD_STATE_RFMEM_RET ) {
            di += (( cd >> 16 ) & 0xFF );
         }
         if ( curState & PD_STATE_CACHE_RET ) {
            di += (( cd >> 24 ) & 0xFF );
         }

         //--- Spec point 5.f ---
         // Currently not implementing external load handling
      }

      //--- Spec. point 6 ---
      cl = pLookupTable[ ti ];

      //--- Spec. point 7 ---
      load = cl & 0xFF;

      //--- Spec. point 8 ---
      if ( curState & PD_STATE_XOSC_LPM ) {
         load += (( cl >> 8 ) & 0xFF );
      }
      if ( curState & PD_STATE_RFMEM_RET ) {
         load += (( cl >> 16 ) & 0xFF );
      }
      if ( curState & PD_STATE_CACHE_RET ) {
         load += (( cl >> 24 ) & 0xFF );
      }

      //--- Spec. point 9 ---
      load += ((( di * ( shiftedTemp - ( ti << 8 ))) + 128 ) > 8 );

      // Currently not implementing external load handling
      // if ( __ccfg.ulModeConfig & MODE_CONF_VDDR_EXT_LOAD ) {
         //--- Spec. point 10 ---
      // } else {
         //--- Spec. point 11 ---
         k = 468;
      // }

      //--- Spec. point 12 ---

      vddrCap = ( HWREG( CCFG_BASE + CCFG_O_MODE_CONF ) & CCFG_MODE_CONF_VDDR_CAP_M ) >> CCFG_MODE_CONF_VDDR_CAP_S;
      newRechargePeriod = ( vddrCap * k ) / load;
      if ( newRechargePeriod > 0xFFFF ) {
         newRechargePeriod = 0xFFFF;
      }
      powerQualGlobals.pdRechargePeriod = newRechargePeriod;
   }

   //--- Spec. point 13 ---
   if ( curTemp >  127 ) curTemp =  127;
   if ( curTemp < -128 ) curTemp = -128;
   powerQualGlobals.pdTemp    = curTemp;
   powerQualGlobals.pdState   = curState;
   powerQualGlobals.pdTime    = HWREG( AON_RTC_BASE + AON_RTC_O_SEC );

   // Calculate PER_E and PER_M (based on powerQualGlobals.pdRechargePeriod)
   // Round downwards but make sure PER_E=0 and PER_M=1 is the minimum possible setting.
   perE = 0;
   perM = powerQualGlobals.pdRechargePeriod;
   if ( perM < 31 ) {
      perM = 31;
      powerQualGlobals.pdRechargePeriod = 31;
   }
   while ( perM > 511 ) {
      perM >>= 1;
      perE  += 1;
   }
   perM = ( perM - 15 ) >> 4;

   HWREG( AON_WUC_BASE + AON_WUC_O_RECHARGECFG ) =
      ( 0x80A4FF00                          ) |
      ( perM << AON_WUC_RECHARGECFG_PER_M_S ) |
      ( perE << AON_WUC_RECHARGECFG_PER_E_S ) ;
   HWREG( AON_WUC_BASE + AON_WUC_O_RECHARGESTAT ) = 0;
}


//*****************************************************************************
//
// SysCtrlAdjustRechargeAfterPowerDown()
//
//*****************************************************************************
void
SysCtrlAdjustRechargeAfterPowerDown( void )
{
   int32_t     curTemp                 ;
   uint32_t    longestRechargePeriod   ;
   uint32_t    newRechargePeriod       ;

   //--- Spec. point 1 ---
   curTemp = AON_BatmonTempGetDegC();
   if ( curTemp < powerQualGlobals.pdTemp ) {
      if ( curTemp < -128 ) {
         curTemp = -128;
      }
      powerQualGlobals.pdTemp = curTemp;
   }

   //--- Spec. point 2 ---
   longestRechargePeriod = ( HWREG( AON_WUC_BASE + AON_WUC_O_RECHARGESTAT ) &
      AON_WUC_RECHARGESTAT_MAX_USED_PER_M ) >>
      AON_WUC_RECHARGESTAT_MAX_USED_PER_S ;

   if ( longestRechargePeriod == 0 ) {
      //--- Spec. point 3 ---
      powerQualGlobals.pdRechargePeriod >>= 1;
   } else {
      //--- Spec. point 4 ---
      if ( longestRechargePeriod < powerQualGlobals.pdRechargePeriod ) {
         powerQualGlobals.pdRechargePeriod = longestRechargePeriod;
      } else {
         //--- Spec. point 5 ---
         uint32_t deltaTime = HWREG( AON_RTC_BASE + AON_RTC_O_SEC ) - powerQualGlobals.pdTime + 1;
         if ( deltaTime > 31 ) {
            deltaTime = 31;
         }
         newRechargePeriod = powerQualGlobals.pdRechargePeriod + (( longestRechargePeriod - powerQualGlobals.pdRechargePeriod ) >> deltaTime );
         if ( newRechargePeriod > 0xFFFF ) {
            newRechargePeriod = 0xFFFF;
         }
         powerQualGlobals.pdRechargePeriod = newRechargePeriod;
      }
   }
}


//*****************************************************************************
//
// SysCtrl_DCDC_VoltageConditionalControl()
//
//*****************************************************************************
void
SysCtrl_DCDC_VoltageConditionalControl( void )
{
   uint32_t batThreshold     ;
   uint32_t aonBatmonBat     ;
   uint32_t aonSysctlPwrctl  ;
   uint32_t ccfg_ModeConfReg = HWREG( CCFG_BASE + CCFG_O_MODE_CONF );

   //
   // Check if Voltage Conditional Control is enabled
   // It is enabled if both:
   // - DCDC in use (either in active or recharge mode), (in use if one of the corresponding CCFG bits are zero).
   // - Alternative DCDC settings are enabled ( DIS_ALT_DCDC_SETTING == 0 )
   //
   if (((( ccfg_ModeConfReg & CCFG_MODE_CONF_DCDC_RECHARGE_M ) == 0                                            ) ||
        (( ccfg_ModeConfReg & CCFG_MODE_CONF_DCDC_ACTIVE_M   ) == 0                                            )    ) &&
       (( HWREG( CCFG_BASE + CCFG_O_SIZE_AND_DIS_FLAGS ) & CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING ) == 0      )    )
   {
      aonSysctlPwrctl = HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL );
      aonBatmonBat    = HWREG( AON_BATMON_BASE + AON_BATMON_O_BAT    );
      batThreshold    = (((( HWREG( CCFG_BASE + CCFG_O_MODE_CONF_1 ) &
         CCFG_MODE_CONF_1_ALT_DCDC_VMIN_M ) >>
         CCFG_MODE_CONF_1_ALT_DCDC_VMIN_S ) + 28 ) << 4 );

      if ( aonSysctlPwrctl & ( AON_SYSCTL_PWRCTL_DCDC_EN_M | AON_SYSCTL_PWRCTL_DCDC_ACTIVE_M )) {
         //
         // DCDC is ON, check if it should be switched off
         //
         if ( aonBatmonBat < batThreshold ) {
            aonSysctlPwrctl &= ~( AON_SYSCTL_PWRCTL_DCDC_EN_M | AON_SYSCTL_PWRCTL_DCDC_ACTIVE_M );

            HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL ) = aonSysctlPwrctl;
         }
      } else {
         //
         // DCDC is OFF, check if it should be switched on
         //
         if ( aonBatmonBat > batThreshold ) {
            if (( ccfg_ModeConfReg & CCFG_MODE_CONF_DCDC_RECHARGE_M ) == 0 ) aonSysctlPwrctl |= AON_SYSCTL_PWRCTL_DCDC_EN_M     ;
            if (( ccfg_ModeConfReg & CCFG_MODE_CONF_DCDC_ACTIVE_M   ) == 0 ) aonSysctlPwrctl |= AON_SYSCTL_PWRCTL_DCDC_ACTIVE_M ;

            HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_PWRCTL ) = aonSysctlPwrctl;
         }
      }
   }
}


//*****************************************************************************
//
// SysCtrlResetSourceGet()
//
//*****************************************************************************
uint32_t
SysCtrlResetSourceGet( void )
{
   if ( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) & AON_SYSCTL_RESETCTL_WU_FROM_SD_M ) {
      return ( RSTSRC_WAKEUP_FROM_SHUTDOWN );
   } else {
      return (( HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) &
                AON_SYSCTL_RESETCTL_RESET_SRC_M ) >>
                AON_SYSCTL_RESETCTL_RESET_SRC_S ) ;
   }
}
