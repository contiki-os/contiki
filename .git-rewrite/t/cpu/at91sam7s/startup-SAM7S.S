/***********************************************************************/
/*                                                                     */
/*  startup_SAM7S.S:  Startup file for Atmel AT91SAM7S device series   */
/*                                                                     */
/***********************************************************************/
/*  ported to arm-elf-gcc / WinARM by Martin Thomas, KL, .de           */
/*  <eversmith@heizung-thomas.de>                                      */
/*  modifications Copyright Martin Thomas 2005                         */
/*                                                                     */
/*  Based on a file that has been a part of the uVision/ARM            */
/*  development tools, Copyright KEIL ELEKTRONIK GmbH 2002-2004        */
/***********************************************************************/

/* 
  Modifications by Martin Thomas:
  - added handling of execption vectors in RAM ("ramfunc")
  - added options to remap the interrupt vectors to RAM
    (see makefile for switch-option)
  - replaced all ";" and "#" for comments with // or / *  * /
  - added C++ ctor handling
  - .text in RAM for debugging (RAM_RUN)
*/
/*
  Modifications by Simon Berg
  - added stack segment
  - running program as system by defining RUN_AS_SYSTEM
*/

// mt: this file should not be used with the Configuration Wizard
// since a lot of changes have been done for the WinARM/gcc example
/* 
// *** <<< Use Configuration Wizard in Context Menu >>> ***
*/



// *** Startup Code (executed after Reset) ***


// Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

        .equ    Mode_USR,       0x10
        .equ    Mode_FIQ,       0x11
        .equ    Mode_IRQ,       0x12
        .equ    Mode_SVC,       0x13
        .equ    Mode_ABT,       0x17
        .equ    Mode_UND,       0x1B
        .equ    Mode_SYS,       0x1F

        .equ    I_Bit,          0x80    /* when I bit is set, IRQ is disabled */
        .equ    F_Bit,          0x40    /* when F bit is set, FIQ is disabled */


// Internal Memory Base Addresses
        .equ    FLASH_BASE,     0x00100000   
        .equ    RAM_BASE,       0x00200000


/*
// <h> Stack Configuration
//   <o>  Top of Stack Address  <0x0-0xFFFFFFFF:4>
//   <h>  Stack Sizes (in Bytes)
//     <o1> Undefined Mode      <0x0-0xFFFFFFFF:4>
//     <o2> Supervisor Mode     <0x0-0xFFFFFFFF:4>
//     <o3> Abort Mode          <0x0-0xFFFFFFFF:4>
//     <o4> Fast Interrupt Mode <0x0-0xFFFFFFFF:4>
//     <o5> Interrupt Mode      <0x0-0xFFFFFFFF:4>
//     <o6> User/System Mode    <0x0-0xFFFFFFFF:4>
//   </h>
// </h>
*/
        .equ    Top_Stack,      0x00204000
        .equ    UND_Stack_Size, 0x00000004
        .equ    SVC_Stack_Size, 0x00000200
        .equ    ABT_Stack_Size, 0x00000004
        .equ    FIQ_Stack_Size, 0x00000004
        .equ    IRQ_Stack_Size, 0x00000400
        .equ    USR_Stack_Size, 0x00000400


	.bss
	.section .stack , "aw", %nobits
	
USR_Stack_Start:
	.skip USR_Stack_Size
USR_Stack_End:
IRQ_Stack_Start:
	.skip IRQ_Stack_Size
IRQ_Stack_End:
FIQ_Stack_Start:
	.skip FIQ_Stack_Size
FIQ_Stack_End:
ABT_Stack_Start:
	.skip ABT_Stack_Size
ABT_Stack_End:
SVC_Stack_Start:
	.skip SVC_Stack_Size
SVC_Stack_End:
UND_Stack_Start:
	.skip UND_Stack_Size
UND_Stack_End:
	
// Embedded Flash Controller (EFC) definitions
        .equ    EFC_BASE,       0xFFFFFF00  /* EFC Base Address */
        .equ    EFC_FMR,        0x60        /* EFC_FMR Offset */

/*
// <e> Embedded Flash Controller (EFC)
//   <o1.16..23> FMCN: Flash Microsecond Cycle Number <0-255>
//               <i> Number of Master Clock Cycles in 1us
//   <o1.8..9>   FWS: Flash Wait State
//               <0=> Read: 1 cycle / Write: 2 cycles
//               <1=> Read: 2 cycle / Write: 3 cycles
//               <2=> Read: 3 cycle / Write: 4 cycles
//               <3=> Read: 4 cycle / Write: 4 cycles
// </e>
*/
        .equ    EFC_SETUP,      1
        .equ    EFC_FMR_Val,    0x00320100


// Watchdog Timer (WDT) definitions
        .equ    WDT_BASE,       0xFFFFFD40  /* WDT Base Address */
        .equ    WDT_MR,         0x04        /* WDT_MR Offset */

/*
// <e> Watchdog Timer (WDT)
//   <o1.0..11>  WDV: Watchdog Counter Value <0-4095>
//   <o1.16..27> WDD: Watchdog Delta Value <0-4095>
//   <o1.12>     WDFIEN: Watchdog Fault Interrupt Enable
//   <o1.13>     WDRSTEN: Watchdog Reset Enable
//   <o1.14>     WDRPROC: Watchdog Reset Processor
//   <o1.28>     WDDBGHLT: Watchdog Debug Halt
//   <o1.29>     WDIDLEHLT: Watchdog Idle Halt
//   <o1.15>     WDDIS: Watchdog Disable
// </e>
*/
        .equ    WDT_SETUP,      1
        .equ    WDT_MR_Val,     0x00008000 // Disable watchdog


// Power Mangement Controller (PMC) definitions
        .equ    PMC_BASE,       0xFFFFFC00  /* PMC Base Address */
        .equ    PMC_MOR,        0x20        /* PMC_MOR Offset */
        .equ    PMC_MCFR,       0x24        /* PMC_MCFR Offset */
        .equ    PMC_PLLR,       0x2C        /* PMC_PLLR Offset */
        .equ    PMC_MCKR,       0x30        /* PMC_MCKR Offset */
        .equ    PMC_SR,         0x68        /* PMC_SR Offset */
        .equ    PMC_MOSCEN,     (1<<0)      /* Main Oscillator Enable */
        .equ    PMC_OSCBYPASS,  (1<<1)      /* Main Oscillator Bypass */
        .equ    PMC_OSCOUNT,    (0xFF<<8)   /* Main OScillator Start-up Time */
        .equ    PMC_DIV,        (0xFF<<0)   /* PLL Divider */
        .equ    PMC_PLLCOUNT,   (0x3F<<8)   /* PLL Lock Counter */
        .equ    PMC_OUT,        (0x03<<14)  /* PLL Clock Frequency Range */
        .equ    PMC_MUL,        (0x7FF<<16) /* PLL Multiplier */
        .equ    PMC_USBDIV,     (0x03<<28)  /* USB Clock Divider */
        .equ    PMC_CSS,        (3<<0)      /* Clock Source Selection */
        .equ    PMC_PRES,       (7<<2)      /* Prescaler Selection */
        .equ    PMC_MOSCS,      (1<<0)      /* Main Oscillator Stable */
        .equ    PMC_LOCK,       (1<<2)      /* PLL Lock Status */

/*
// <e> Power Mangement Controller (PMC)
//   <h> Main Oscillator
//     <o1.0>      MOSCEN: Main Oscillator Enable
//     <o1.1>      OSCBYPASS: Oscillator Bypass
//     <o1.8..15>  OSCCOUNT: Main Oscillator Startup Time <0-255>
//   </h>
//   <h> Phase Locked Loop (PLL)
//     <o2.0..7>   DIV: PLL Divider <0-255>
//     <o2.16..26> MUL: PLL Multiplier <0-2047>
//                 <i> PLL Output is multiplied by MUL+1
//     <o2.14..15> OUT: PLL Clock Frequency Range
//                 <0=> 80..160MHz  <1=> Reserved
//                 <2=> 150..220MHz <3=> Reserved
//     <o2.8..13>  PLLCOUNT: PLL Lock Counter <0-63>
//     <o2.28..29> USBDIV: USB Clock Divider
//                 <0=> None  <1=> 2  <2=> 4  <3=> Reserved
//   </h>
//   <o3.0..1>   CSS: Clock Source Selection
//               <0=> Slow Clock
//               <1=> Main Clock
//               <2=> Reserved
//               <3=> PLL Clock
//   <o3.2..4>   PRES: Prescaler
//               <0=> None
//               <1=> Clock / 2    <2=> Clock / 4
//               <3=> Clock / 8    <4=> Clock / 16
//               <5=> Clock / 32   <6=> Clock / 64
//               <7=> Reserved
// </e>
*/
        .equ    PMC_SETUP,      1
        .equ    PMC_MOR_Val,    0x00000601	/* Enable main oscilator,
						   48 cycles startup */
        .equ    PMC_PLLR_Val,   0x00191C05	/* 28 cycles startup,
						   PLL = 5.2* * main clock */ 
        .equ    PMC_MCKR_Val,   0x0000000B	/* MCK = PLL/4 */

/* Reset controller */
	.equ	RSTC_BASE,	0xfffffd00
	.equ	RSTC_CR,	0x00
	.equ	RSTC_SR,	0x04
	.equ	RSTC_MR,	0x08
	
	.equ	RSTC_SETUP,	1
	.equ	RSTC_MR_Val,	0xa5000001	/* Enable user reset */
	

/* Advanced interrupt controller */
	.equ	AIC_SETUP,	1
	.equ	AIC_BASE,	0xfffff000
	.equ	AIC_EOICR,	0x130
	.equ	AIC_SPU,	0x134

#if (defined(VECTORS_IN_RAM) && defined(ROM_RUN)) || defined(USE_SAMBA)

/* 
 Exception Vectors to be placed in RAM - added by mt
 -> will be used after remapping in ROM_RUN
 -> not needed for RAM_RUN
 -> moved to address 0 after remapping	
 Mapped to Address 0 after remapping in ROM_RUN
 Absolute addressing mode must be used.
 Dummy Handlers are implemented as infinite loops which can be modified.
 VECTORS_IN_RAM defined in makefile/by commandline 
*/
			.text
			.arm
			.section .vectram, "ax"
			
VectorsRAM:     LDR     PC,Reset_AddrR
                LDR     PC,Undef_AddrR
                LDR     PC,SWI_AddrR
                LDR     PC,PAbt_AddrR
                LDR     PC,DAbt_AddrR
                NOP                            /* Reserved Vector */
                LDR     PC,[PC,#-0xF20]        /* Vector From AIC_IVR */
                LDR     PC,[PC,#-0xF20]        /* Vector From AIC_FVR */

Reset_AddrR:     .word   Reset_Handler
Undef_AddrR:     .word   Undef_HandlerR
SWI_AddrR:       .word   SWI_HandlerR
PAbt_AddrR:      .word   PAbt_HandlerR
DAbt_AddrR:      .word   DAbt_HandlerR
//               .word   0xdeadbeef     /* Test Reserved Address */
                 .word   0     /* Reserved Address */
IRQ_AddrR:       .word   IRQ_HandlerR
FIQ_AddrR:       .word   FIQ_HandlerR

Undef_HandlerR:  B       Undef_HandlerR
SWI_HandlerR:    B       SWI_HandlerR
PAbt_HandlerR:   B       PAbt_HandlerR
DAbt_HandlerR:   B       DAbt_HandlerR
IRQ_HandlerR:    B       IRQ_HandlerR
FIQ_HandlerR:    B       FIQ_HandlerR

VectorsRAM_end:	
#endif /* VECTORS_IN_RAM && ROM_RUN */



#ifndef USE_SAMBA
	
/*
 Exception Vectors 
 - for ROM_RUN: placed in 0x00000000 
 - for RAM_RUN: placed at 0x00200000 (on AT91SAM7S64)
 - for USE_SAMBA: not used	
 -> will be used during startup before remapping with target ROM_RUN
 -> will be used "always" in code without remapping or with target RAM_RUN
 Mapped to Address relative address 0 of .text
 Absolute addressing mode must be used.
 Dummy Handlers are implemented as infinite loops which can be modified.
*/
			.text
			.arm
			.section .vectrom, "ax"

Vectors:        LDR     PC,Reset_Addr         
                LDR     PC,Undef_Addr
                LDR     PC,SWI_Addr
                LDR     PC,PAbt_Addr
                LDR     PC,DAbt_Addr
                NOP                            /* Reserved Vector */
//                LDR     PC,IRQ_Addr
                LDR     PC,[PC,#-0xF20]        /* Vector From AIC_IVR */
//                LDR     PC,FIQ_Addr
                LDR     PC,[PC,#-0xF20]        /* Vector From AIC_FVR */

Reset_Addr:     .word   Reset_Handler
Undef_Addr:     .word   Undef_Handler
SWI_Addr:       .word   SWI_Handler
PAbt_Addr:      .word   PAbt_Handler
DAbt_Addr:      .word   DAbt_Handler
                .word   0                      /* Reserved Address */
IRQ_Addr:       .word   IRQ_Handler
FIQ_Addr:       .word   FIQ_Handler

Undef_Handler:  B       Undef_Handler
SWI_Handler:    B       SWI_Handler
PAbt_Handler:   B       PAbt_Handler
DAbt_Handler:   B       DAbt_Handler
IRQ_Handler:    B       IRQ_Handler
FIQ_Handler:    B       FIQ_Handler

#endif

// Spurious interrupt handler
SPU_Handler:
		STMDB   SP!, {R0}
		LDR	R0, =AIC_BASE
		STR	R0, [R0, #AIC_EOICR]
		LDMIA   SP!, {R0}
		SUBS    PC, LR, #4
	

// Starupt Code must be linked first at Address at which it expects to run.

		.text
		.arm
		.section .init, "ax"
	
		.global _startup
		.func   _startup
_startup:


// Reset Handler
                LDR     pc, =Reset_Handler
Reset_Handler:

// Setup EFC
.if EFC_SETUP
                LDR     R0, =EFC_BASE
                LDR     R1, =EFC_FMR_Val
                STR     R1, [R0, #EFC_FMR]
.endif


// Setup WDT
.if WDT_SETUP
                LDR     R0, =WDT_BASE
                LDR     R1, =WDT_MR_Val
                STR     R1, [R0, #WDT_MR]
.endif

// Setup reset controller
.if RSTC_SETUP
		LDR     R0, =RSTC_BASE
                LDR     R1, =RSTC_MR_Val
                STR     R1, [R0, #RSTC_MR]
.endif			

// Setup PMC
.if PMC_SETUP
                LDR     R0, =PMC_BASE

//  Setup Main Oscillator
                LDR     R1, =PMC_MOR_Val
                STR     R1, [R0, #PMC_MOR]

//  Wait until Main Oscillator is stablilized
.if (PMC_MOR_Val & PMC_MOSCEN)
MOSCS_Loop:     LDR     R2, [R0, #PMC_SR]
                ANDS    R2, R2, #PMC_MOSCS
                BEQ     MOSCS_Loop
.endif

//  Setup the PLL
.if (PMC_PLLR_Val & PMC_MUL)
                LDR     R1, =PMC_PLLR_Val
                STR     R1, [R0, #PMC_PLLR]

//  Wait until PLL is stabilized
PLL_Loop:       LDR     R2, [R0, #PMC_SR]
                ANDS    R2, R2, #PMC_LOCK
                BEQ     PLL_Loop
.endif

//  Select Clock
                LDR     R1, =PMC_MCKR_Val
                STR     R1, [R0, #PMC_MCKR]
.endif


// Setup Stack for each mode

                LDR     R0, =Top_Stack

//  Enter Undefined Instruction Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_UND|I_Bit|F_Bit
                LDR     SP, =UND_Stack_End

//  Enter Abort Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_ABT|I_Bit|F_Bit
                LDR     SP, =ABT_Stack_End

//  Enter FIQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_FIQ|I_Bit|F_Bit
                LDR     SP, =FIQ_Stack_End

//  Enter IRQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_IRQ|I_Bit|F_Bit
                LDR     SP, =IRQ_Stack_End

//  Enter Supervisor Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SVC|I_Bit|F_Bit
                LDR     SP, =SVC_Stack_End
	
//  Enter User Mode and set its Stack Pointer
#ifdef RUN_AS_SYSTEM
		MSR     CPSR_c, #Mode_SYS
#else
                MSR     CPSR_c, #Mode_USR
#endif
                LDR     SP, =USR_Stack_End
	
// Setup a default Stack Limit (when compiled with "-mapcs-stack-check")
                LDR     SL, =USR_Stack_End

#ifdef ROM_RUN
// Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =_etext
                LDR     R2, =_data
                LDR     R3, =_edata
LoopRel:        CMP     R2, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel
#endif
	

// Clear .bss section (Zero init)
                MOV     R0, #0
                LDR     R1, =__bss_start__
                LDR     R2, =__bss_end__
LoopZI:         CMP     R1, R2
                STRLO   R0, [R1], #4
                BLO     LoopZI


#if defined(VECTORS_IN_RAM) || defined(RAM_RUN)
/* 
   *** Remap ***
   ROM_RUN: exception vectors for RAM have been already copied 
     to 0x00200000 by the .data copy-loop 
   RAM_RUN: exception vectors are already placed at 0x0020000 by
     linker settings
*/
				.equ    MC_BASE,0xFFFFFF00  /* MC Base Address */
				.equ    MC_RCR, 0x00        /* MC_RCR Offset */

				LDR     R0, =MC_BASE
				MOV     R1, #1
				STR     R1, [R0, #MC_RCR]   // Remap
#endif /* VECTORS_IN_RAM || RAM_RUN */
	
#ifdef USE_SAMBA
// Copy interrupt vectors to RAM, that has previously been mapped to 0
		MOV     R1, #0
                LDR     R2, = VectorsRAM
                LDR     R3, = VectorsRAM_end
LoopVectCopy:   CMP     R2, R3
                LDRLO   R0, [R2], #4
                STRLO   R0, [R1], #4
                BLO     LoopVectCopy
#endif	

.if AIC_SETUP
		LDR	R1, =AIC_BASE
		LDR	R0, = SPU_Handler
		STR	R0, [R1, #AIC_SPU]
.endif

/*
   Call C++ constructors (for objects in "global scope")
   added by Martin Thomas based on a Anglia Design 
   example-application for STR7 ARM
*/

			LDR 	r0, =__ctors_start__
			LDR 	r1, =__ctors_end__
ctor_loop:
			CMP 	r0, r1
			BEQ 	ctor_end
			LDR 	r2, [r0], #4   /* this ctor's address */
			STMFD 	sp!, {r0-r1}   /* save loop counters  */
			MOV 	lr, pc         /* set return address  */
//			MOV 	pc, r2
			BX      r2             /* call ctor */
			LDMFD 	sp!, {r0-r1}   /* restore loop counters */
			B 		ctor_loop
ctor_end:

       
// Enter the C code
				mov   r0,#0            // no arguments (argc = 0)
				mov   r1,r0
				mov   r2,r0
				mov   fp,r0            // null frame pointer
				mov   r7,r0            // null frame pointer for thumb
				ldr   r10,=main
				adr   lr, __main_exit
				bx    r10              // enter main()

__main_exit:    B       __main_exit


				.size   _startup, . - _startup
				.endfunc

.end




