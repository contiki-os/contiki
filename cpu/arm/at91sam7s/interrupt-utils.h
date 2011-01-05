/*
 *  Defines and Macros for Interrupt-Service-Routines
 *  collected and partly created by
 *  Martin Thomas <mthomas@rhrk.uni-kl.de>
 *
 *  Copyright 2005 M. Thomas
 *  No guarantees, warrantees, or promises, implied or otherwise.
 *  May be used for hobby or commercial purposes provided copyright
 *  notice remains intact.
 */

#ifndef interrupt_utils_
#define interrupt_utils_

/* 
   The following defines are usefull for 
   interrupt service routine declarations.
*/

/* 
   RAMFUNC 
   Attribute which defines a function to be located 
   in memory section .fastrun and called via "long calls". 
   See linker-skript and startup-code to see how the
   .fastrun-section is handled.
   The definition is not only useful for ISRs but since
   ISRs should be executed fast the macro is defined in
   this header.
*/
#define RAMFUNC __attribute__ ((long_call, section (".fastrun")))


/*
  INTFUNC
  standard attribute for arm-elf-gcc which marks
  a function as ISR (for the VIC). Since gcc seems
  to produce wrong code if this attribute is used in
  thumb/thumb-interwork the attribute should only be
  used for "pure ARM-mode" binaries.
*/
#define INTFUNC __attribute__ ((interrupt("IRQ"))) 


/*
  NACKEDFUNC
  gcc will not add any code to a function declared
  "nacked". The user has to take care to save registers
  and add the needed code for ISR functions. Some
  macros for this tasks are provided below.
*/
#define NACKEDFUNC __attribute__((naked))


/******************************************************************************
 *
 * MACRO Name: ISR_STORE()
 *
 * Description:
 *    This MACRO is used upon entry to an ISR with interrupt nesting.  
 *    Should be used together with ISR_ENABLE_NEST(). The MACRO
 *    performs the following steps:
 *
 *    1 - Save the non-banked registers r0-r12 and lr onto the IRQ stack.
 *
 *****************************************************************************/
#define ISR_STORE() asm volatile( \
 "STMDB SP!,{R0-R12,LR}\n" )
 
 /******************************************************************************
 *
 * MACRO Name: ISR_RESTORE()
 *
 * Description:
 *    This MACRO is used upon exit from an ISR with interrupt nesting.  
 *    Should be used together with ISR_DISABLE_NEST(). The MACRO
 *    performs the following steps:
 *
 *    1 - Load the non-banked registers r0-r12 and lr from the IRQ stack.
 *    2 - Adjusts resume adress
 *
 *****************************************************************************/
#define ISR_RESTORE()  asm volatile( \
 "LDMIA SP!,{R0-R12,LR}\n" \
 "SUBS  R15,R14,#0x0004\n" ) 

/******************************************************************************
 *
 * MACRO Name: ISR_ENABLE_NEST()
 *
 * Description:
 *    This MACRO is used upon entry from an ISR with interrupt nesting.  
 *    Should be used after ISR_STORE. 
 *
 *****************************************************************************/

#define ISR_ENABLE_NEST() asm volatile( \
 "MRS     LR, SPSR \n"  \
 "STMFD   SP!, {LR} \n" \
 "MSR     CPSR_c, #0x1f \n" \
 "STMFD   SP!, {LR} " )

/******************************************************************************
 *
 * MACRO Name: ISR_DISABLE_NEST()
 *
 * Description:
 *    This MACRO is used upon entry from an ISR with interrupt nesting.  
 *    Should be used before ISR_RESTORE. 
 *
 *****************************************************************************/

#define ISR_DISABLE_NEST() asm volatile(  \
 "LDMFD   SP!, {LR} \n" \
 "MSR     CPSR_c, #0x92 \n" \
 "LDMFD   SP!, {LR} \n" \
 "MSR     SPSR_cxsf, LR \n" )


 
/*
 * The following marcos are from the file "armVIC.h" by:
 *
 * Copyright 2004, R O SoftWare
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 * 
 */ 
 
/******************************************************************************
 *
 * MACRO Name: ISR_ENTRY()
 *
 * Description:
 *    This MACRO is used upon entry to an ISR.  The current version of
 *    the gcc compiler for ARM does not produce correct code for
 *    interrupt routines to operate properly with THUMB code.  The MACRO
 *    performs the following steps:
 *
 *    1 - Adjust address at which execution should resume after servicing
 *        ISR to compensate for IRQ entry
 *    2 - Save the non-banked registers r0-r12 and lr onto the IRQ stack.
 *    3 - Get the status of the interrupted program is in SPSR.
 *    4 - Push it onto the IRQ stack as well.
 *
 *****************************************************************************/
#define ISR_ENTRY() asm volatile(" sub   lr, lr,#4\n" \
                                 " stmfd sp!,{r0-r12,lr}\n" \
                                 " mrs   r1, spsr\n" \
                                 " stmfd sp!,{r1}")

/******************************************************************************
 *
 * MACRO Name: ISR_EXIT()
 *
 * Description:
 *    This MACRO is used to exit an ISR.  The current version of the gcc
 *    compiler for ARM does not produce correct code for interrupt
 *    routines to operate properly with THUMB code.  The MACRO performs
 *    the following steps:
 *
 *    1 - Recover SPSR value from stack       
 *    2 - and restore  its value                   
 *    3 - Pop the return address & the saved general registers from
 *        the IRQ stack & return
 *
 *****************************************************************************/
#define ISR_EXIT()  asm volatile(" ldmfd sp!,{r1}\n" \
                                 " msr   spsr_c,r1\n" \
                                 " ldmfd sp!,{r0-r12,pc}^") 
  
/******************************************************************************
 *
 * Function Name: disableIRQ()
 *
 * Description:
 *    This function sets the IRQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned disableIRQ(void);

/******************************************************************************
 *
 * Function Name: enableIRQ()
 *
 * Description:
 *    This function clears the IRQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned enableIRQ(void);

/******************************************************************************
 *
 * Function Name: restoreIRQ()
 *
 * Description:
 *    This function restores the IRQ disable bit in the status register
 *    to the value contained within passed oldCPSR
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned restoreIRQ(unsigned oldCPSR);

/******************************************************************************
 *
 * Function Name: disableFIQ()
 *
 * Description:
 *    This function sets the FIQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned disableFIQ(void);

/******************************************************************************
 *
 * Function Name: enableFIQ()
 *
 * Description:
 *    This function clears the FIQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned enableFIQ(void);

/******************************************************************************
 *
 * Function Name: restoreFIQ()
 *
 * Description:
 *    This function restores the FIQ disable bit in the status register
 *    to the value contained within passed oldCPSR
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned restoreFIQ(unsigned oldCPSR);


#endif

