/** @file hal/micro/cortexm3/sleep.c
 * 
 * @brief STM32W108 micro specific sleep functions.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"





















































































//We don't have a real register to hold this composite information.
//Pretend we do so halGetWakeInfo can operate like halGetResetInfo.
//This "register" is only ever set by halInternalSleep.
// [31] = WakeInfoValid
// [30] = SleepSkipped
// [29] = CSYSPWRUPREQ
// [28] = CDBGPWRUPREQ
// [27] = WAKE_CORE
// [26] = TIMER_WAKE_WRAP
// [25] = TIMER_WAKE_COMPB
// [24] = TIMER_WAKE_COMPA
// [23:0] = corresponding GPIO activity
#define WAKEINFOVALID_INTERNAL_WAKE_EVENT_BIT 31
#define SLEEPSKIPPED_INTERNAL_WAKE_EVENT_BIT  30
#define CSYSPWRUPREQ_INTERNAL_WAKE_EVENT_BIT  29
#define CDBGPWRUPREQ_INTERNAL_WAKE_EVENT_BIT  28
#define WAKE_CORE_INTERNAL_WAKE_EVENT_BIT     27
#define WRAP_INTERNAL_WAKE_EVENT_BIT          26
#define CMPB_INTERNAL_WAKE_EVENT_BIT          25
#define CMPA_INTERNAL_WAKE_EVENT_BIT          24
//This define shifts events from the PWRUP_EVENT register into the proper
//place in the halInternalWakeEvent variable
#define INTERNAL_WAKE_EVENT_BIT_SHIFT         20

static uint32_t halInternalWakeEvent=0;

uint32_t halGetWakeInfo(void)
{
  return halInternalWakeEvent;
}
  
void halInternalSleep(SleepModes sleepMode)
{
  //Timer restoring always takes place during the wakeup sequence.  We save
  //the state here in case SLEEPMODE_NOTIMER is invoked, which would disable
  //the clocks.
  uint32_t SLEEPTMR_CLKEN_SAVED = SLEEPTMR_CLKEN;
  
  //This code assumes all wake source registers are properly configured.
  //As such, it should be called from halSleepWithOptions() or from
  // halSleepForQsWithOptions() which configues the wake sources.
  
  //The parameter gpioWakeSel is a bitfield composite of the GPIO wake
  //sources derived from the 3 ports, indicating which of the 24 GPIO
  //are configured as a wake source.
  uint32_t gpioWakeSel  = (GPIO_PAWAKE<<0);
         gpioWakeSel |= (GPIO_PBWAKE<<8);
         gpioWakeSel |= (GPIO_PCWAKE<<16);
  
  //PB2 is also WAKE_SC1.  Set this wake source if PB2's GPIO wake is set.
  if(GPIO_PBWAKE & PB2) {
    WAKE_SEL |= WAKE_SC1;
  }
  
  //PA2 is also WAKE_SC2.  Set this wake source if PA2's GPIO wake is set.
  if(GPIO_PAWAKE & PA2) {
    WAKE_SEL |= WAKE_SC2;
  }
  
  //The WAKE_IRQD source can come from any pin based on IRQD's sel register.
  if(gpioWakeSel & BIT(GPIO_IRQDSEL)) {
    WAKE_SEL |= WAKE_IRQD;
  }
  
  halInternalWakeEvent = 0; //clear old wake events
  
  switch(sleepMode)
  {
    case SLEEPMODE_NOTIMER:
      //The sleep timer clock sources (both RC and XTAL) are turned off.
      //Wakeup is possible from only GPIO.  System time is lost.
      //NOTE: Timer restoring always takes place during the wakeup sequence.
      SLEEPTMR_CLKEN = 0;
      goto deepSleepCore;
    
    case SLEEPMODE_WAKETIMER:
      //The sleep timer clock sources remain running.  The RC is always
      //running and the 32kHz XTAL depends on the board header.  Wakeup
      //is possible from both GPIO and the sleep timer.  System time
      //is maintained.  The sleep timer is assumed to be configured
      //properly for wake events.
      //NOTE: This mode assumes the caller has configured the *entire*
      //      sleep timer properly.
      
      if(INT_SLEEPTMRCFG&INT_SLEEPTMRWRAP) {
        WAKE_SEL |= WAKE_SLEEPTMRWRAP;
      }
      if(INT_SLEEPTMRCFG&INT_SLEEPTMRCMPB) {
        WAKE_SEL |= WAKE_SLEEPTMRCMPB;
      }
      if(INT_SLEEPTMRCFG&INT_SLEEPTMRCMPA) {
        WAKE_SEL |= WAKE_SLEEPTMRCMPA;
      }
      //fall into SLEEPMODE_MAINTAINTIMER's sleep code:
      
    case SLEEPMODE_MAINTAINTIMER:
      //The sleep timer clock sources remain running.  The RC is always
      //running and the 32kHz XTAL depends on the board header.  Wakeup
      //is possible from only GPIO.  System time is maintained.
      //NOTE: System time is maintained without any sleep timer interrupts
      //      because the hardware sleep timer counter is large enough
      //      to hold the entire count value and not need a RAM counter.
      
    ////////////////////////////////////////////////////////////////////////////
    // Core deep sleep code
    ////////////////////////////////////////////////////////////////////////////
deepSleepCore:
      // Interrupts *must* be/stay disabled for DEEP SLEEP operation
      // INTERRUPTS_OFF will use BASEPRI to disable all interrupts except
      // fault handlers and PendSV.
      INTERRUPTS_OFF();
      // This is the point of no return.  From here on out, only the interrupt
      // sources available in WAKE_SEL will be captured and propagated across
      // deep sleep.
      //stick all our saved info onto stack since it's only temporary
      {
        boolean restoreWatchdog = halInternalWatchDogEnabled();
        boolean skipSleep = FALSE;
        
        // Only three register blocks keep power across deep sleep:
        //  CM_HV, GPIO, SLOW_TIMERS
        //
        // All other register blocks lose their state across deep sleep:
        //  BASEBAND, MAC, SECURITY, SERIAL, TMR1, TMR2, EVENT, CM_LV, RAM_CTRL,
        //  AUX_ADC, CAL_ADC, FLASH_CONTROL, ITM, DWT, FPB, NVIC, TPIU
        //
        // The sleep code will only save and restore registers where it is
        // meaningful and necessary to do so.  In most cases, there must still
        // be a powerup function to restore proper state.
        //
        // NOTE: halPowerUp() and halPowerDown() will always be called before
        // and after this function.  halPowerDown and halPowerUp should leave
        // the modules in a safe state and then restart the modules.
        // (For example, shutting down and restarting Timer1)
        //
        //----BASEBAND
        //        reinitialized by stStackPowerUp()
        //----MAC
        //        reinitialized by stStackPowerUp()
        //----SECURITY
        //        reinitialized by stStackPowerUp()
        //----SERIAL
        //        reinitialized by halPowerUp() or similar
        //----TMR1
        //        reinitialized by halPowerUp() or similar
        //----TMR2
        //        reinitialized by halPowerUp() or similar
        //----EVENT
        //SRC or FLAG interrupts are not saved or restored
        //MISS interrupts are not saved or restored
        //MAC_RX_INT_MASK - reinitialized by stStackPowerUp()
        //MAC_TX_INT_MASK - reinitialized by stStackPowerUp()
        //MAC_TIMER_INT_MASK - reinitialized by stStackPowerUp()
        //BB_INT_MASK - reinitialized by stStackPowerUp()
        //SEC_INT_MASK - reinitialized by stStackPowerUp()
        uint32_t INT_SLEEPTMRCFG_SAVED = INT_SLEEPTMRCFG_REG;
        uint32_t INT_MGMTCFG_SAVED = INT_MGMTCFG_REG;
        //INT_TIM1CFG - reinitialized by halPowerUp() or similar
        //INT_TIM2CFG - reinitialized by halPowerUp() or similar
        //INT_SC1CFG - reinitialized by halPowerUp() or similar
        //INT_SC2CFG - reinitialized by halPowerUp() or similar
        //INT_ADCCFG - reinitialized by halPowerUp() or similar
        uint32_t GPIO_INTCFGA_SAVED = GPIO_INTCFGA_REG;
        uint32_t GPIO_INTCFGB_SAVED = GPIO_INTCFGB_REG;
        uint32_t GPIO_INTCFGC_SAVED = GPIO_INTCFGC_REG;
        uint32_t GPIO_INTCFGD_SAVED = GPIO_INTCFGD_REG;
        //SC1_INTMODE - reinitialized by halPowerUp() or similar
        //SC2_INTMODE - reinitialized by halPowerUp() or similar
        //----CM_LV
        uint32_t OSC24M_BIASTRIM_SAVED = OSC24M_BIASTRIM_REG;
        uint32_t OSCHF_TUNE_SAVED = OSCHF_TUNE_REG;
        uint32_t DITHER_DIS_SAVED = DITHER_DIS_REG;
        //OSC24M_CTRL - reinitialized by halPowerUp() or similar
        //CPU_CLKSEL  - reinitialized by halPowerUp() or similar
        //TMR1_CLK_SEL - reinitialized by halPowerUp() or similar
        //TMR2_CLK_SEL - reinitialized by halPowerUp() or similar
        uint32_t PCTRACE_SEL_SAVED = PCTRACE_SEL_REG;
        //----RAM_CTRL
        uint32_t MEM_PROT_0_SAVED = MEM_PROT_0_REG;
        uint32_t MEM_PROT_1_SAVED = MEM_PROT_1_REG;
        uint32_t MEM_PROT_2_SAVED = MEM_PROT_2_REG;
        uint32_t MEM_PROT_3_SAVED = MEM_PROT_3_REG;
        uint32_t MEM_PROT_4_SAVED = MEM_PROT_4_REG;
        uint32_t MEM_PROT_5_SAVED = MEM_PROT_5_REG;
        uint32_t MEM_PROT_6_SAVED = MEM_PROT_6_REG;
        uint32_t MEM_PROT_7_SAVED = MEM_PROT_7_REG;
        uint32_t MEM_PROT_EN_SAVED = MEM_PROT_EN_REG;
        //----AUX_ADC
        //        reinitialized by halPowerUp() or similar
        //----CAL_ADC
        //        reinitialized by stStackPowerUp()
        //----FLASH_CONTROL
        //        configured on the fly by the flash library
        //----ITM
        //        reinitialized by halPowerUp() or similar
        //----DWT
        //        not used by software on chip
        //----FPB
        //        not used by software on chip
        //----NVIC
        //ST_CSR - fixed, restored by cstartup when exiting deep sleep
        //ST_RVR - fixed, restored by cstartup when exiting deep sleep
        uint32_t INT_CFGSET_SAVED = INT_CFGSET_REG; //mask against wake sources
        //INT_PENDSET - used below when overlapping interrupts and wake sources
        //NVIC_IPR_3to0 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_7to4 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_11to8 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_15to12 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_19to16 - fixed, restored by cstartup when exiting deep sleep
        uint32_t SCS_VTOR_SAVED = SCS_VTOR_REG;
        //SCS_CCR - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHPR_7to4 - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHPR_11to8 - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHPR_15to12 - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHCSR - fixed, restored by cstartup when exiting deep sleep
        //----TPIU
        //        reinitialized by halPowerUp() or similar
        
        //stmDebugPowerDown() should have shutdown the DWT/ITM/TPIU already.
        
        //freeze input to the GPIO from LV (alternate output functions freeze)
        EVENT_CTRL = LV_FREEZE;
        //record GPIO state for wake monitoring purposes
        //By having a snapshot of GPIO state, we can figure out after waking
        //up exactly which GPIO could have woken us up.
        //Reading the three IN registers is done separately to avoid warnings
        //about undefined order of volatile access.
        uint32_t GPIO_IN_SAVED =   GPIO_PAIN;
               GPIO_IN_SAVED |= (GPIO_PBIN<<8);
               GPIO_IN_SAVED |= (GPIO_PCIN<<16);
        //reset the power up events by writing 1 to all bits.
        PWRUP_EVENT = 0xFFFFFFFF;



        //By clearing the events, the wake up event capturing is activated.
        //At this point we can safely check our interrupt flags since event
        //capturing is now overlapped.  Up to now, interrupts indicate
        //activity, after this point, powerup events indicate activity.
        //If any of the interrupt flags are set, that means we saw a wake event
        //sometime while entering sleep, so we need to skip over sleeping
        //
        //--possible interrupt sources for waking:
        //  IRQA, IRQB, IRQC, IRQD
        //  SleepTMR CMPA, CMPB, Wrap
        //  WAKE_CORE (DebugIsr)
        //
        //check for IRQA interrupt and if IRQA (PB0) is wake source
        if((INT_PENDSET&INT_IRQA) &&
           (GPIO_PBWAKE&PB0) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQA as a wake event
          halInternalWakeEvent |= BIT(PORTB_PIN(0));



        }
        //check for IRQB interrupt and if IRQB (PB6) is wake source
        if((INT_PENDSET&INT_IRQB) &&
           (GPIO_PBWAKE&PB6) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQB as a wake event
          halInternalWakeEvent |= BIT(PORTB_PIN(6));



        }
        //check for IRQC interrupt and if IRQC (GPIO_IRQCSEL) is wake source
        if((INT_PENDSET&INT_IRQC) &&
           (gpioWakeSel&BIT(GPIO_IRQCSEL)) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQC as a wake event
          halInternalWakeEvent |= BIT(GPIO_IRQCSEL);



        }
        //check for IRQD interrupt and if IRQD (GPIO_IRQDSEL) is wake source
        if((INT_PENDSET&INT_IRQD) &&
           (gpioWakeSel&BIT(GPIO_IRQDSEL)) &&
           ((WAKE_SEL&GPIO_WAKE) ||
            (WAKE_SEL&WAKE_IRQD))) {
          skipSleep = TRUE;
          //log IRQD as a wake event
          halInternalWakeEvent |= BIT(GPIO_IRQDSEL);



        }
        //check for SleepTMR CMPA interrupt and if SleepTMR CMPA is wake source
        if((INT_SLEEPTMR&INT_SLEEPTMRCMPA) && (WAKE_SEL&WAKE_SLEEPTMRCMPA)) {
          skipSleep = TRUE;
          //log SleepTMR CMPA as a wake event
          halInternalWakeEvent |= BIT32(CMPA_INTERNAL_WAKE_EVENT_BIT);



        }
        //check for SleepTMR CMPB interrupt and if SleepTMR CMPB is wake source
        if((INT_SLEEPTMR&INT_SLEEPTMRCMPB) && (WAKE_SEL&WAKE_SLEEPTMRCMPB)) {
          skipSleep = TRUE;
          //log SleepTMR CMPB as a wake event
          halInternalWakeEvent |= BIT32(CMPB_INTERNAL_WAKE_EVENT_BIT);



        }
        //check for SleepTMR WRAP interrupt and if SleepTMR WRAP is wake source
        if((INT_SLEEPTMR&INT_SLEEPTMRWRAP) && (WAKE_SEL&WAKE_SLEEPTMRWRAP)) {
          skipSleep = TRUE;
          //log SleepTMR WRAP as a wake event
          halInternalWakeEvent |= BIT32(WRAP_INTERNAL_WAKE_EVENT_BIT);



        }
        //check for Debug interrupt and if WAKE_CORE is wake source
        if((INT_PENDSET&INT_DEBUG) && (WAKE_SEL&WAKE_WAKE_CORE)) {
          skipSleep = TRUE;
          //log WAKE_CORE as a wake event
          halInternalWakeEvent |= BIT32(WAKE_CORE_INTERNAL_WAKE_EVENT_BIT);



        }

        //only propagate across deep sleep the interrupts that are both
        //enabled and possible wake sources
        {
          uint32_t wakeSourceInterruptMask = 0;
          
          if(GPIO_PBWAKE&PB0) {
            wakeSourceInterruptMask |= INT_IRQA;



          }
          if(GPIO_PBWAKE&PB6) {
            wakeSourceInterruptMask |= INT_IRQB;



          }
          if(gpioWakeSel&BIT(GPIO_IRQCSEL)) {
            wakeSourceInterruptMask |= INT_IRQC;



          }
          if(gpioWakeSel&BIT(GPIO_IRQDSEL)) {
            wakeSourceInterruptMask |= INT_IRQD;



          }
          if( (WAKE_SEL&WAKE_SLEEPTMRCMPA) ||
              (WAKE_SEL&WAKE_SLEEPTMRCMPB) ||
              (WAKE_SEL&WAKE_SLEEPTMRWRAP) ) {
            wakeSourceInterruptMask |= INT_SLEEPTMR;



          }
          if(WAKE_SEL&WAKE_WAKE_CORE) {
            wakeSourceInterruptMask |= INT_DEBUG;



          }
          
          INT_CFGSET_SAVED &= wakeSourceInterruptMask;
        }
        















        
        //disable watchdog while sleeping (since we can't reset it asleep)
        halInternalDisableWatchDog(MICRO_DISABLE_WATCH_DOG_KEY);
        
        //The chip is not allowed to enter a deep sleep mode (which could
        //cause a core reset cycle) while CSYSPWRUPREQ is set.  CSYSPWRUPREQ
        //indicates that the debugger is trying to access sections of the
        //chip that would get reset during deep sleep.  Therefore, a reset
        //cycle could very easily cause the debugger to error and we don't
        //want that.  While the power management state machine will stall
        //if CSYSPWRUPREQ is set (to avoid the situation just described),
        //in this stalled state the chip will not be responsive to wake
        //events.  To be sensitive to wake events, we must handle them in
        //software instead.  To accomplish this, we request that the
        //CSYSPWRUPACK be inhibited (which will indicate the debugger is not
        //connected).  But, we cannot induce deep sleep until CSYSPWRUPREQ/ACK
        //go low and these are under the debuggers control, so we must stall
        //and wait here.  If there is a wake event during this time, break
        //out and wake like normal.  If the ACK eventually clears,
        //we can proceed into deep sleep.  The CSYSPWRUPACK_INHIBIT
        //functionality will hold off the debugger (by holding off the ACK)
        //until we are safely past and out of deep sleep.  The power management
        //state machine then becomes responsible for clearing
        //CSYSPWRUPACK_INHIBIT and responding to a CSYSPWRUPREQ with a
        //CSYSPWRUPACK at the right/safe time.
        CSYSPWRUPACK_INHIBIT = CSYSPWRUPACK_INHIBIT_CSYSPWRUPACK_INHIBIT;
        {
          //Use a local copy of WAKE_SEL to avoid warnings from the compiler
          //about order of volatile accesses
          uint32_t wakeSel = WAKE_SEL;
          //stall until a wake event or CSYSPWRUPREQ/ACK clears
          while( (CSYSPWRUPACK_STATUS) && (!(PWRUP_EVENT&wakeSel)) ) {}
          //if there was a wake event, allow CSYSPWRUPACK and skip sleep
          if(PWRUP_EVENT&wakeSel) {
            CSYSPWRUPACK_INHIBIT = CSYSPWRUPACK_INHIBIT_RESET;
            skipSleep = TRUE;
          }
        }
        
        



        if(!skipSleep) {



          //FogBugz 7283 states that we must switch to the OSCHF when entering
          //deep sleep since using the 24MHz XTAL could result in RAM
          //corruption.  This switch must occur at least 2*24MHz cycles before
          //sleeping.
          //FogBugz 8858 states that we cannot go into deep-sleep when the
          //chip is clocked with the 24MHz XTAL with a duty cycle as low as
          //70/30 since this causes power_down generation timing to fail.
          OSC24M_CTRL &= ~OSC24M_CTRL_OSC24M_SEL;
          //If DS12 needs to be forced regardless of state, clear
          //REGEN_DSLEEP here.  This is hugely dangerous and
          //should only be done in very controlled chip tests.
          SCS_SCR |= SCS_SCR_SLEEPDEEP;      //enable deep sleep
          extern volatile boolean halPendSvSaveContext;
          halPendSvSaveContext = 1;          //1 means save context
          //The INTERRUPTS_OFF used at the beginning of this function set
          //BASEPRI such that the only interrupts that will fire are faults
          //and PendSV.  Trigger PendSV now to induce a context save.
          SCS_ICSR |= SCS_ICSR_PENDSVSET;    //pend the context save and Dsleep
          //Since the interrupt will not fire immediately it is possible to
          //execute a few lines of code.  To stay halted in this spot until the
          //WFI instruction, spin on the context flag (which will get cleared
          //during the startup sequence when restoring context).
          while(halPendSvSaveContext) {}
          //I AM ASLEEP.  WHEN EXECUTION RESUMES, CSTARTUP WILL RESTORE TO HERE
        } else {
          //Record the fact that we skipped sleep
          halInternalWakeEvent |= BIT32(SLEEPSKIPPED_INTERNAL_WAKE_EVENT_BIT);
          //If this was a true deep sleep, we would have executed cstartup and
          //PRIMASK would be set right now.  If we skipped sleep, PRIMASK is not
          //set so we explicitely set it to guarantee the powerup sequence
          //works cleanly and consistently with respect to interrupt
          //dispatching and enabling.
          _setPriMask();
        }



        



        
        //Clear the interrupt flags for all wake sources.  This
        //is necessary because if we don't execute an actual deep sleep cycle
        //the interrupt flags will never be cleared.  By clearing the flags,
        //we always mimick a real deep sleep as closely as possible and
        //guard against any accidental interrupt triggering coming out
        //of deep sleep.  (The interrupt dispatch code coming out of sleep
        //is responsible for translating wake events into interrupt events,
        //and if we don't clear interrupt flags here it's possible for an
        //interrupt to trigger even if it wasn't the true wake event.)
        INT_SLEEPTMRFLAG = (INT_SLEEPTMRCMPA |
                            INT_SLEEPTMRCMPB |
                            INT_SLEEPTMRWRAP);
        INT_GPIOFLAG = (INT_IRQAFLAG |
                        INT_IRQBFLAG |
                        INT_IRQCFLAG |
                        INT_IRQDFLAG);
        
        //immediately restore the registers we saved before sleeping
        //so IRQ and SleepTMR capture can be reenabled as quickly as possible
        //this is safe because our global interrupts are still disabled
        //other registers will be restored later







        SLEEPTMR_CLKEN_REG = SLEEPTMR_CLKEN_SAVED;
        INT_SLEEPTMRCFG_REG = INT_SLEEPTMRCFG_SAVED;
        INT_MGMTCFG_REG = INT_MGMTCFG_SAVED;
        GPIO_INTCFGA_REG = GPIO_INTCFGA_SAVED;
        GPIO_INTCFGB_REG = GPIO_INTCFGB_SAVED;
        GPIO_INTCFGC_REG = GPIO_INTCFGC_SAVED;
        GPIO_INTCFGD_REG = GPIO_INTCFGD_SAVED;
        OSC24M_BIASTRIM_REG = OSC24M_BIASTRIM_SAVED;
        OSCHF_TUNE_REG = OSCHF_TUNE_SAVED;
        DITHER_DIS_REG = DITHER_DIS_SAVED;
        PCTRACE_SEL_REG = PCTRACE_SEL_SAVED;
        MEM_PROT_0_REG = MEM_PROT_0_SAVED;
        MEM_PROT_1_REG = MEM_PROT_1_SAVED;
        MEM_PROT_2_REG = MEM_PROT_2_SAVED;
        MEM_PROT_3_REG = MEM_PROT_3_SAVED;
        MEM_PROT_4_REG = MEM_PROT_4_SAVED;
        MEM_PROT_5_REG = MEM_PROT_5_SAVED;
        MEM_PROT_6_REG = MEM_PROT_6_SAVED;
        MEM_PROT_7_REG = MEM_PROT_7_SAVED;
        MEM_PROT_EN_REG = MEM_PROT_EN_SAVED;
        INT_CFGSET_REG = INT_CFGSET_SAVED;
        SCS_VTOR_REG = SCS_VTOR_SAVED;
        
        //WAKE_CORE/INT_DEBUG and INT_IRQx is cleared by INT_PENDCLR below
        INT_PENDCLR = 0xFFFFFFFF;
        
        //Now that we're awake, normal interrupts are operational again
        //Take a snapshot of the new GPIO state and the EVENT register to
        //record our wake event
        uint32_t GPIO_IN_NEW =   GPIO_PAIN;
               GPIO_IN_NEW |= (GPIO_PBIN<<8);
               GPIO_IN_NEW |= (GPIO_PCIN<<16);
        //Only operate on power up events that are also wake events.  Power
        //up events will always trigger like an interrupt flag, so we have
        //to check them against events that are enabled for waking. (This is
        //a two step process because we're accessing two volatile values.)
        uint32_t powerUpEvents = PWRUP_EVENT;
               powerUpEvents &= WAKE_SEL;
        halInternalWakeEvent |= ((GPIO_IN_SAVED^GPIO_IN_NEW)&gpioWakeSel);
        //PWRUP_SC1 is PB2 which is bit 10
        halInternalWakeEvent |= (!!(powerUpEvents&PWRUP_SC1))<<((1*8)+2);
        //PWRUP_SC2 is PA2 which is bit 2
        halInternalWakeEvent |= (!!(powerUpEvents&PWRUP_SC2))<<((0*8)+2);
        //PWRUP_IRQD is chosen by GPIO_IRQDSEL
        halInternalWakeEvent |= (!!(powerUpEvents&PWRUP_IRQD))<<(GPIO_IRQDSEL);
        halInternalWakeEvent |= ((powerUpEvents & 
                                  (PWRUP_CSYSPWRUPREQ_MASK  |
                                   PWRUP_CDBGPWRUPREQ_MASK  |
                                   PWRUP_WAKECORE_MASK      |
                                   PWRUP_SLEEPTMRWRAP_MASK  |
                                   PWRUP_SLEEPTMRCOMPB_MASK |
                                   PWRUP_SLEEPTMRCOMPA_MASK ))
                                          <<INTERNAL_WAKE_EVENT_BIT_SHIFT);
        //at this point wake events are fully captured and interrupts have
        //taken over handling all new events
        




        
        //Bring limited interrupts back online.  INTERRUPTS_OFF will use
        //BASEPRI to disable all interrupts except fault handlers and PendSV.
        //PRIMASK is still set though (global interrupt disable) so we need
        //to clear that next.
        INTERRUPTS_OFF();
        



        
        //Now that BASEPRI has taken control of interrupt enable/disable,
        //we can clear PRIMASK to reenable global interrupt operation.
        _clearPriMask();
        



        
        //wake events are saved and interrupts are back on track,
        //disable gpio freeze
        EVENT_CTRL = EVENT_CTRL_RESET;
        
        //restart watchdog if it was running when we entered sleep
        //do this before dispatching interrupts while we still have tight
        //control of code execution
        if(restoreWatchdog) {
          halInternalEnableWatchDog();
        }
        



        
        //Pend any interrupts associated with deep sleep wake sources.  The
        //restoration of INT_CFGSET above and the changing of BASEPRI below
        //is responsible for proper dispatching of interrupts at the end of
        //halSleepWithOptions.
        //
        //
        //The WAKE_CORE wake source triggers a Debug Interrupt.  If INT_DEBUG
        //interrupt is enabled and WAKE_CORE is a wake event, then pend the
        //Debug interrupt (using the wake_core bit).
        if( (INT_CFGSET&INT_DEBUG) &&
            (halInternalWakeEvent&BIT(WAKE_CORE_INTERNAL_WAKE_EVENT_BIT)) ) {
          WAKE_CORE = WAKE_CORE_FIELD;



        }
        //
        //
        //The SleepTMR CMPA is linked to a real ISR.  If the SleepTMR CMPA
        //interrupt is enabled and CMPA is a wake event, then pend the CMPA
        //interrupt (force the second level interrupt).
        if( (INT_SLEEPTMRCFG&INT_SLEEPTMRCMPA) &&
            (halInternalWakeEvent&BIT(CMPA_INTERNAL_WAKE_EVENT_BIT)) ) {
          INT_SLEEPTMRFORCE = INT_SLEEPTMRCMPA;



        }
        //
        //The SleepTMR CMPB is linked to a real ISR.  If the SleepTMR CMPB
        //interrupt is enabled and CMPB is a wake event, then pend the CMPB
        //interrupt (force the second level interrupt).
        if( (INT_SLEEPTMRCFG&INT_SLEEPTMRCMPB) &&
            (halInternalWakeEvent&BIT(CMPB_INTERNAL_WAKE_EVENT_BIT)) ) {
          INT_SLEEPTMRFORCE = INT_SLEEPTMRCMPB;



        }
        //
        //The SleepTMR WRAP is linked to a real ISR.  If the SleepTMR WRAP
        //interrupt is enabled and WRAP is a wake event, then pend the WRAP
        //interrupt (force the second level interrupt).
        if( (INT_SLEEPTMRCFG&INT_SLEEPTMRWRAP) &&
            (halInternalWakeEvent&BIT(WRAP_INTERNAL_WAKE_EVENT_BIT)) ) {
          INT_SLEEPTMRFORCE = INT_SLEEPTMRWRAP;



        }
        //
        //
        //The four IRQs are linked to a real ISR.  If any of the four IRQs
        //triggered, then pend their ISR
        //
        //If the IRQA interrupt mode is enabled and IRQA (PB0) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGA&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(PORTB_PIN(0))) ) {
          INT_PENDSET = INT_IRQA;



        }
        //If the IRQB interrupt mode is enabled and IRQB (PB6) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGB&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(PORTB_PIN(6))) ) {
          INT_PENDSET = INT_IRQB;



        }
        //If the IRQC interrupt mode is enabled and IRQC (GPIO_IRQCSEL) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGC&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(GPIO_IRQCSEL)) ) {
          INT_PENDSET = INT_IRQC;



        }
        //If the IRQD interrupt mode is enabled and IRQD (GPIO_IRQDSEL) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGD&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(GPIO_IRQDSEL)) ) {
          INT_PENDSET = INT_IRQD;



        }
      }
      




      
      //Mark the wake events valid just before exiting
      halInternalWakeEvent |= BIT32(WAKEINFOVALID_INTERNAL_WAKE_EVENT_BIT);
      
      //We are now reconfigured, appropriate ISRs are pended, and ready to go,
      //so enable interrupts!
      INTERRUPTS_ON();
      



      
      break; //and deep sleeping is done!
    
    case SLEEPMODE_IDLE:
      //Only the CPU is idled.  The rest of the chip continues runing
      //normally.  The chip will wake from any interrupt.
      {
        boolean restoreWatchdog = halInternalWatchDogEnabled();
        //disable watchdog while sleeping (since we can't reset it asleep)
        halInternalDisableWatchDog(MICRO_DISABLE_WATCH_DOG_KEY);
        //Normal ATOMIC/INTERRUPTS_OFF/INTERRUPTS_ON uses the BASEPRI mask
        //to juggle priority levels so that the fault handlers can always
        //be serviced.  But, the WFI instruction is only capable of
        //working with the PRIMASK bit.  Therefore, we have to switch from
        //using BASEPRI to PRIMASK to keep interrupts disabled so that the
        //WFI can return on an interrupt
        //Globally disable interrupts with PRIMASK
        _setPriMask();
        //Bring the BASEPRI up to 0 to allow interrupts (but still disabled
        //with PRIMASK)
        INTERRUPTS_ON();
        //an internal function call is made here instead of injecting the
        //"WFI" assembly instruction because injecting assembly code will
        //cause the compiler's optimizer to reduce efficiency.
        halInternalIdleSleep();
        //The WFI instruction does not actually clear the PRIMASK bit, it
        //only allows the PRIMASK bit to be bypassed.  Therefore, we must
        //manually clear PRIMASK to reenable all interrupts.
        _clearPriMask();
        //restart watchdog if it was running when we entered sleep
        if(restoreWatchdog)
          halInternalEnableWatchDog();
      }
      break;
      
    default:
      //Oops!  Invalid sleepMode parameter.
      assert(0);
  }
}


void halSleepWithOptions(SleepModes sleepMode, uint32_t gpioWakeBitMask)
{
  //configure all GPIO wake sources
  GPIO_PAWAKE = (gpioWakeBitMask>>0)&0xFF;
  GPIO_PBWAKE = (gpioWakeBitMask>>8)&0xFF;
  GPIO_PCWAKE = (gpioWakeBitMask>>16)&0xFF;
  
  //use the defines found in the board file to choose our wakeup source(s)
  WAKE_SEL = 0;  //start with no wake sources
  
  //if any of the GPIO wakeup monitor bits are set, enable the top level
  //GPIO wakeup monitor
  if((GPIO_PAWAKE)||(GPIO_PBWAKE)||(GPIO_PCWAKE)) {
    WAKE_SEL |= GPIO_WAKE;
  }
  //always wakeup when the debugger is connected
  WAKE_SEL |= WAKE_CDBGPWRUPREQ;
  //always wakeup when the debugger attempts to access the chip
  WAKE_SEL |= WAKE_CSYSPWRUPREQ;
  //always wakeup when the debug channel attempts to access the chip
  WAKE_SEL |= WAKE_WAKE_CORE;
  //the timer wakeup sources are enabled below in POWERSAVE, if needed
  
  //wake sources are configured so do the actual sleeping
  halInternalSleep(sleepMode);
}
