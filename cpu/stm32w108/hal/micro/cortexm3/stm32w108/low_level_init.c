/**************************************************
 *
 * This module contains the function `__low_level_init', a function
 * that is called before the `main' function of the program.  Normally
 * low-level initializations - such as setting the prefered interrupt
 * level or setting the watchdog - can be performed here.
 *
 * Note that this function is called before the data segments are
 * initialized, this means that this function cannot rely on the
 * values of global or static variables.
 *
 * When this function returns zero, the startup code will inhibit the
 * initialization of the data segments. The result is faster startup,
 * the drawback is that neither global nor static data will be
 * initialized.
 *
 * Copyright 1999-2004 IAR Systems. All rights reserved.
 * Customized by STMicroelectronics for STM32W
 *
 **************************************************/

#include PLATFORM_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#pragma language=extended

#include "hal/micro/cortexm3/memmap.h"
__root __no_init const HalFixedAddressTableType halFixedAddressTable @ __FAT__;

extern const HalVectorTableType __vector_table[];
extern void  halInternalSwitchToXtal(void);

#define IAP_BOOTLOADER_APP_SWITCH_SIGNATURE  0xb001204d
#define IAP_BOOTLOADER_MODE_UART  0

__interwork int __low_level_init(void);

static void setStackPointer(int32u address)
{
  asm("MOVS SP, r0");
}

static const int16u blOffset[] = {
  0x0715 - 0x03ad - 0x68,
  0x0719 - 0x03ad - 0x6C
};


__interwork int __low_level_init(void)
{
  //Ensure there is enough margin on VREG_1V8 for stable RAM reads by
  //setting it to a code of 6.  VREG_1V2 can be left at its reset value.
  VREG = 0x00000307;
  
  // This code should be careful about the use of local variables in case the
  // reset type happens to be a deep sleep reset.  If the reset is not from
  // deep sleep, then locals can be freely used

  //When the Cortex-M3 exits reset, interrupts are enable.  Explicitely
  //disable them immediately using the standard set PRIMASK instruction.
  //Injecting an assembly instruction this early does not effect optimization.
  asm("CPSID i");

  //It is quite possible that when the Cortex-M3 begins executing code the
  //Core Reset Vector Catch is still left enabled.  Because this VC would
  //cause us to halt at reset if another reset event tripped, we should
  //clear it as soon as possible.  If a debugger wants to halt at reset,
  //it will set this bit again.
  DEBUG_EMCR &= ~DEBUG_EMCR_VC_CORERESET;

  //Configure flash access for optimal current consumption early
  //during boot to save as much current as we can.
  FLASH_ACCESS = (FLASH_ACCESS_PREFETCH_EN          |
                  (1<<FLASH_ACCESS_CODE_LATENCY_BIT));

  ////---- Always remap the vector table ----////
  // We might be coming from a bootloader at the base of flash, or even in the
  //  NULL_BTL case, the BAT/AAT will be at the beginning of the image
  SCS_VTOR = (int32u)__vector_table;

  ////---- Always Configure Interrupt Priorities ----////
  //The STM32W support 5 bits of priority configuration.
  //  The cortex allows this to be further divided into preemption and a
  //  "tie-breaker" sub-priority.
  //We configure a scheme that allows for 3 bits (8 values) of preemption and
  //   2 bits (4 values) of tie-breaker by using the value 4 in PRIGROUP.
  //The value 0x05FA0000 is a special key required to write to this register.
  SCS_AIRCR = (0x05FA0000 | (4 <<SCS_AIRCR_PRIGROUP_BIT));
    
  //A few macros to help with interrupt priority configuration.  Really only 
  //  uses 6 of the possible levels, and ignores the tie-breaker sub-priority
  //  for now.
  //Don't forget that the priority level values need to be shifted into the
  //  top 5 bits of the 8 bit priority fields. (hence the <<3)
  //
  // NOTE: The ATOMIC and DISABLE_INTERRUPTS macros work by setting the 
  //       current priority to a value of 12, which still allows CRITICAL and 
  //       HIGH priority interrupts to fire, while blocking MED and LOW.
  //       If a different value is desired, spmr.s79 will need to be edited.
  #define CRITICAL (0  <<3)
  #define HIGH     (8  <<3)
  #define MED      (16 <<3)
  #define LOW      (28 <<3)
  #define NONE     (31 <<3)

  //With optimization turned on, the compiler will indentify all the values
  //and variables used here as constants at compile time and will truncate
  //this entire block of code to 98 bytes, comprised of 7 load-load-store
  //operations.
                    //vect00 is fixed                        //Stack pointer
                    //vect01 is fixed                        //Reset Vector
                    //vect02 is fixed                        //NMI Handler
                    //vect03 is fixed                        //Hard Fault Handler
  SCS_SHPR_7to4   = ((CRITICAL <<SCS_SHPR_7to4_PRI_4_BIT) |  //Memory Fault Handler 
                     (CRITICAL <<SCS_SHPR_7to4_PRI_5_BIT) |  //Bus Fault Handler
                     (CRITICAL <<SCS_SHPR_7to4_PRI_6_BIT) |  //Usage Fault Handler
                     (NONE <<SCS_SHPR_7to4_PRI_7_BIT));      //Reserved
  SCS_SHPR_11to8  = ((NONE <<SCS_SHPR_11to8_PRI_8_BIT)  |    //Reserved
                     (NONE <<SCS_SHPR_11to8_PRI_9_BIT)  |    //Reserved
                     (NONE <<SCS_SHPR_11to8_PRI_10_BIT) |    //Reserved
                     (HIGH <<SCS_SHPR_11to8_PRI_11_BIT));    //SVCall Handler
  SCS_SHPR_15to12 = ((MED  <<SCS_SHPR_15to12_PRI_12_BIT) |   //Debug Monitor Handler
                     (NONE <<SCS_SHPR_15to12_PRI_13_BIT) |   //Reserved
                     (HIGH <<SCS_SHPR_15to12_PRI_14_BIT) |   //PendSV Handler
                     (MED  <<SCS_SHPR_15to12_PRI_15_BIT));   //SysTick Handler
  NVIC_IPR_3to0   = ((MED  <<NVIC_IPR_3to0_PRI_0_BIT) |      //Timer 1 Handler
                     (MED  <<NVIC_IPR_3to0_PRI_1_BIT) |      //Timer 2 Handler
                     (HIGH <<NVIC_IPR_3to0_PRI_2_BIT) |      //Management Handler
                     (MED  <<NVIC_IPR_3to0_PRI_3_BIT));      //BaseBand Handler
  NVIC_IPR_7to4   = ((MED  <<NVIC_IPR_7to4_PRI_4_BIT) |      //Sleep Timer Handler
                     (MED  <<NVIC_IPR_7to4_PRI_5_BIT) |      //SC1 Handler
                     (MED  <<NVIC_IPR_7to4_PRI_6_BIT) |      //SC2 Handler
                     (MED  <<NVIC_IPR_7to4_PRI_7_BIT));      //Security Handler
  NVIC_IPR_11to8  = ((MED  <<NVIC_IPR_11to8_PRI_8_BIT)  |    //MAC Timer Handler
                     (MED  <<NVIC_IPR_11to8_PRI_9_BIT)  |    //MAC TX Handler
                     (MED  <<NVIC_IPR_11to8_PRI_10_BIT) |    //MAC RX Handler
                     (MED  <<NVIC_IPR_11to8_PRI_11_BIT));    //ADC Handler
  NVIC_IPR_15to12 = ((MED  <<NVIC_IPR_15to12_PRI_12_BIT) |   //GPIO IRQA Handler
                     (MED  <<NVIC_IPR_15to12_PRI_13_BIT) |   //GPIO IRQB Handler
                     (MED  <<NVIC_IPR_15to12_PRI_14_BIT) |   //GPIO IRQC Handler
                     (MED  <<NVIC_IPR_15to12_PRI_15_BIT));   //GPIO IRQD Handler
  NVIC_IPR_19to16 = ((LOW  <<NVIC_IPR_19to16_PRI_16_BIT));   //Debug Handler
                    //vect33 not implemented
                    //vect34 not implemented
                    //vect35 not implemented

  ////---- Always Configure System Handlers Control and Configuration ----////
  SCS_CCR = SCS_CCR_DIV_0_TRP_MASK;
  SCS_SHCSR = ( SCS_SHCSR_USGFAULTENA_MASK
                | SCS_SHCSR_BUSFAULTENA_MASK
                | SCS_SHCSR_MEMFAULTENA_MASK );


  if((RESET_EVENT&RESET_DSLEEP) == RESET_DSLEEP) {
    //Since the 13 NVIC registers above are fixed values, they are restored
    //above (where they get set anyways during normal boot sequences) instead
    //of inside of the halInternalSleep code:

    void halTriggerContextRestore(void);
    extern volatile boolean halPendSvSaveContext;
    halPendSvSaveContext = 0;       //0 means restore context
    SCS_ICSR |= SCS_ICSR_PENDSVSET; //pend halPendSvIsr to enable later
    halTriggerContextRestore();     //sets MSP, enables interrupts
    //if the context restore worked properly, we should never return here
    while(1) { ; }
  }

  //USART bootloader software activation check
  if ((*((int32u *)RAM_BOTTOM) == IAP_BOOTLOADER_APP_SWITCH_SIGNATURE) && (*((int8u *)(RAM_BOTTOM+4)) == IAP_BOOTLOADER_MODE_UART)){
       int8u cut = *(volatile int8u *) 0x08040798;
       int16u offset = 0;
       typedef void (*EntryPoint)(void);     
       offset = (halFixedAddressTable.baseTable.version == 3) ? blOffset[cut - 2] : 0;
       *((int32u *)RAM_BOTTOM) = 0;
       if (offset) {
         halInternalSwitchToXtal();
       }
       EntryPoint entryPoint = (EntryPoint)(*(int32u *)(FIB_BOTTOM+4) - offset);
       setStackPointer(*(int32u *)FIB_BOTTOM);
       entryPoint();
  }

  INTERRUPTS_OFF();
  asm("CPSIE i");

  /*==================================*/
  /* Choose if segment initialization */
  /* should be done or not.           */
  /* Return: 0 to omit seg_init       */
  /*         1 to run seg_init        */
  /*==================================*/
  return 1;
}

#pragma language=default

#ifdef __cplusplus
}
#endif
