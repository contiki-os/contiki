#ifndef ASM_H_
#define ASM_H_

#ifdef __IAR_SYSTEMS_ASM__

  // IAR V5 Definitions.
  #define __END__                  END
  #define __EXPORT__               EXPORT
  #define __IMPORT__               IMPORT
  #define __SPACE__                DS8
  #define __EQU__(a, b)            a EQU b
  #define __WEAK__                 PUBWEAK
  #define __THUMB__                THUMB

  #if __VER__ >= 5000000

    // IAR V5 Definitions.
          RSEG RESETINFO:DATA
          RSEG CSTACK:DATA
    #define __CODE__                    SECTION .text:CODE:REORDER:NOROOT(2)
    #define __BSS__                     SECTION .bss:DATA:NOROOT(2)
    #define __BEGIN_RESETINFO__(offset) SFB(RESETINFO + offset)
    #define __END_RESETINFO__(offset)   SFE(RESETINFO + offset)
    #define __BEGIN_STACK__(offset)     SFB(CSTACK + offset)
    #define __END_STACK__(offset)       SFE(CSTACK + offset)
    #define __CFI__(x)

  #else // __VER__
    
    #error IAR versions less that 5.xx are not supported

  #endif // __VER__

#endif // __IAR_SYSTEMS_ASM__

#ifdef __GNUC__

  // GCC Definitions.
      .syntax unified
      .thumb
  #define __CODE__                 .text
  #define __THUMB__                .thumb_func
  #define __BSS__                  .bss
  #define __END__                  .end
  #define __EXPORT__               .global
  #define __IMPORT__               .extern
  #define __SPACE__                .space
  #define __EQU__(a, b)            .equ a, b
  #define __WEAK__                 .weak
  #define __BEGIN_NVDATA__(offset) (_noinit + offset)
  #define __BEGIN_STACK__(offset)  (_stack + offset)
  #define __END_STACK__(offset)    (_estack + offset)
  #define __CFI__(x)

#endif // __GNUC__

#endif // ASM_H_
