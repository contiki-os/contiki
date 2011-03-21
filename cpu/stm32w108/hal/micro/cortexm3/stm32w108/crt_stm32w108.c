/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stm32f10x_vector.c
* Author             : MCD Tools Team
* Date First Issued  : 05/14/2007
* Description        : This file contains the vector table for STM32F10x.
*                     	 After Reset the Cortex-M3 processor is in Thread mode,
*                     	 priority is Privileged, and the Stack is set to Main.
********************************************************************************
* History:
* 05/14/2007: V0.2
*
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#define RESERVED 0
#define IAP_BOOTLOADER_APP_SWITCH_SIGNATURE  0xb001204d
#define IAP_BOOTLOADER_MODE_UART  0
/* Includes ----------------------------------------------------------------------*/
#include PLATFORM_HEADER
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMonitor_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void halTimer1Isr(void);
void halTimer2Isr(void);
void halManagementIsr(void);
void halBaseBandIsr(void);
void halSleepTimerIsr(void);
void halSc1Isr(void);
void halSc2Isr(void);
void halSecurityIsr(void);
void halStackMacTimerIsr(void);
void stmRadioTransmitIsr(void);
void stmRadioReceiveIsr(void);
void halAdcIsr(void);
void halIrqAIsr(void);
void halIrqBIsr(void);
void halIrqCIsr(void);
void halIrqDIsr(void);
void halDebugIsr(void);

/* Exported types --------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
extern unsigned long _etext;
extern unsigned long _sidata;		/* start address for the initialization values of the .data section. defined in linker script */
extern unsigned long _sdata;		/* start address for the .data section. defined in linker script */
extern unsigned long _edata;		/* end address for the .data section. defined in linker script */

extern unsigned long _sbss;			/* start address for the .bss section. defined in linker script */
extern unsigned long _ebss;			/* end address for the .bss section. defined in linker script */

extern void _estack;		/* init value for the stack pointer. defined in linker script */

#include "hal/micro/cortexm3/memmap.h"
VAR_AT_SEGMENT(const HalFixedAddressTableType halFixedAddressTable, __FAT__);


/* Private typedef -----------------------------------------------------------*/

/* function prototypes ------------------------------------------------------*/
void Reset_Handler(void) __attribute__((__interrupt__));
extern int main(void);
extern void  halInternalSwitchToXtal(void);

/******************************************************************************
*
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/


__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
  &_estack,            // The initial stack pointer
  Reset_Handler,             // 1 The reset handler
  NMI_Handler,              // 2
  HardFault_Handler,         // 3
  MemManage_Handler,         // 4
  BusFault_Handler,          // 5
  UsageFault_Handler,        // 6
  RESERVED,                  // 7
  RESERVED,                  // 8
  RESERVED,                  // 9
  RESERVED,                  // 10
  SVC_Handler,               // 11
  DebugMonitor_Handler,      // 12
  RESERVED,                  // 13
  PendSV_Handler,            // 14
  SysTick_Handler,           // 15
  halTimer1Isr,              // 16
  halTimer2Isr,              // 17
  halManagementIsr,          // 18
  halBaseBandIsr,            // 19
  halSleepTimerIsr,          // 20
  halSc1Isr,                 // 21
  halSc2Isr,                 // 22
  halSecurityIsr,            // 23
  halStackMacTimerIsr,       // 24
  stmRadioTransmitIsr,       // 25
  stmRadioReceiveIsr,        // 26
  halAdcIsr,                 // 27
  halIrqAIsr,                // 28
  halIrqBIsr,                // 29
  halIrqCIsr,                // 30
  halIrqDIsr,                // 31
  halDebugIsr,               // 32
};

static  void setStackPointer(int32u address) __attribute__((noinline));
static void setStackPointer(int32u address)
{
  // This code is needed to generate the instruction below
  // that GNU ASM is refusing to add
  // asm("MOVS SP, r0");
  asm(".short 0x4685");
}

static const int16u blOffset[] = {
  0x0715 - 0x03ad - 0x68,
  0x0719 - 0x03ad - 0x6C
};

/*******************************************************************************
* Function Name  : Reset_Handler
* Description    : This is the code that gets called when the processor first starts execution
*		       following a reset event.  Only the absolutely necessary set is performed,
*		       after which the application supplied main() routine is called. 
* Input          :
* Output         :
* Return         :
*******************************************************************************/
void Reset_Handler(void)
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
  //return 1;
  unsigned long *pulSrc, *pulDest;

  //
  // Copy the data segment initializers from flash to SRAM.
  //
  pulSrc = &_sidata;
  for(pulDest = &_sdata; pulDest < &_edata; )
    {
      *(pulDest++) = *(pulSrc++);
    }
  
  //
  // Zero fill the bss segment.
  //
  for(pulDest = &_sbss; pulDest < &_ebss; )
    {
      *(pulDest++) = 0;
    }
  
  //
  // Call the application's entry point.
  //
  main();
}
#ifdef USE_HEAP
static unsigned char __HEAP_START[1024*3-560+0x200];
caddr_t _sbrk ( int incr )
{
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  //printf ("_sbrk (%d)\n\r", incr);
  if (heap == NULL) {
    heap = (unsigned char *)__HEAP_START;
  }
  prev_heap = heap;
  /* check removed to show basic approach */
  if ((heap + incr) > (__HEAP_START + sizeof(__HEAP_START))) {
    prev_heap = NULL;
  } else {
    heap += incr;
  }
  if (prev_heap == NULL) {
    printf ("_sbrk %d return %p\n\r", incr, prev_heap);
  }
  return (caddr_t) prev_heap;
}
#else
caddr_t _sbrk ( int incr )
{
    return NULL;
}
#endif
int _lseek (int file,
	int ptr,
	int dir)
{
  return 0;
}
int _close (int file)
{
  return -1;
}

void _exit (int n)
{
  /* FIXME: return code is thrown away.  */
  while(1);
}



int _kill (int n, int m)
{
   return -1;
}
int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}
int _isatty (int fd)
{
  return 1;
  fd = fd;
}
int _getpid	(int n)
{
   return -1;
}
int _open (const char * path,
       int          flags,
       ...)
{
  return -1;
}
int _fflush_r(struct _reent *r, FILE *f)
{
  return 0;
}

/********************* (C) COPYRIGHT 2007 STMicroelectronics  *****END OF FILE****/


