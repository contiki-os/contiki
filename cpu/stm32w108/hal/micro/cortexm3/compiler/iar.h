/** @file hal/micro/cortexm3/compiler/iar.h
 * @brief iar for detailed documentation.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

/**
 * @addtogroup stm32w-cpu
 * @{ */

/** @defgroup iar
 * @brief Compiler and Platform specific definitions and typedefs for the
 *  IAR ARM C compiler.
 *
 * @note iar.h should be included first in all source files by setting the
 *  preprocessor macro PLATFORM_HEADER to point to it.  iar.h automatically
 *  includes platform-common.h.
 *
 *  See iar.h and platform-common.h for source code.
 *@{
 */

#ifndef IAR_H_
#define IAR_H_

#ifndef __ICCARM__
  #error Improper PLATFORM_HEADER
#endif

#if (__VER__ < 5040005)
  #error Only IAR EWARM versions later than 5.40.5 are supported
#endif // __VER__


#ifndef DOXYGEN_SHOULD_SKIP_THIS
  #include <intrinsics.h>
  #include <stdarg.h>
  #if defined (CORTEXM3_STM32W108)
    #include "micro/cortexm3/stm32w108/regs.h"
    #include "micro/cortexm3/stm32w108/stm32w108-type.h"
  #elif defined (CORTEXM3_STM32F103)
    #include "stm32f10x.h"
  #else
    #error Unknown CORTEXM3 micro
  #endif
  //Provide a default NVIC configuration file.  The build process can
  //override this if it needs to.
  #ifndef NVIC_CONFIG
    #define NVIC_CONFIG "hal/micro/cortexm3/nvic-config.h"
  #endif
//[[
#ifdef  ST_EMU_TEST
  #ifdef  I_AM_AN_EMULATOR
    // This register is defined for both the chip and the emulator with
    // with distinct reset values.  Need to undefine to avoid preprocessor
    // collision.
    #undef DATA_EMU_REGS_BASE
    #undef DATA_EMU_REGS_END
    #undef DATA_EMU_REGS_SIZE
    #undef I_AM_AN_EMULATOR
    #undef I_AM_AN_EMULATOR_REG
    #undef I_AM_AN_EMULATOR_ADDR
    #undef I_AM_AN_EMULATOR_RESET
    #undef I_AM_AN_EMULATOR_I_AM_AN_EMULATOR
    #undef I_AM_AN_EMULATOR_I_AM_AN_EMULATOR_MASK
    #undef I_AM_AN_EMULATOR_I_AM_AN_EMULATOR_BIT
    #undef I_AM_AN_EMULATOR_I_AM_AN_EMULATOR_BITS
  #endif//I_AM_AN_EMULATOR
 
  #error MICRO currently not supported for emulator builds.
 
#endif//ST_EMU_TEST
//]]

// suppress warnings about unknown pragmas
//  (as they may be pragmas known to other platforms)
#pragma diag_suppress = pe161

#endif  // DOXYGEN_SHOULD_SKIP_THIS

// Define that the minimal hal is being used.
#define MINIMAL_HAL


/** \name Master Variable Types
 * These are a set of typedefs to make the size of all variable declarations
 * explicitly known.
 */
//@{
/**
 * @brief A typedef to make the size of the variable explicitly known.
 */
typedef unsigned char  boolean;
typedef unsigned char  uint8_t;
typedef signed   char  int8_t;
typedef unsigned short uint16_t;
typedef signed   short int16_t;
typedef unsigned int   uint32_t;
typedef signed   int   int32_t;
typedef unsigned int   PointerType;
//@} \\END MASTER VARIABLE TYPES

/**
 * @brief Use the Master Program Memory Declarations from platform-common.h
 */
#define _HAL_USE_COMMON_PGM_



////////////////////////////////////////////////////////////////////////////////
/** \name Miscellaneous Macros
 */
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * @brief A convenient method for code to know what endiannes processor
 * it is running on.  For the Cortex-M3, we are little endian.
 */
#define BIGENDIAN_CPU  FALSE


/**
 * @brief A friendlier name for the compiler's intrinsic for not
 * stripping.
 */
#define NO_STRIPPING  __root


/**
 * @brief A friendlier name for the compiler's intrinsic for eeprom
 * reference.
 */
#define EEPROM  errorerror


#ifndef __SOURCEFILE__
  /**
   * @brief The __SOURCEFILE__ macro is used by asserts to list the
   * filename if it isn't otherwise defined, set it to the compiler intrinsic
   * which specifies the whole filename and path of the sourcefile
   */
  #define __SOURCEFILE__ __FILE__
#endif


#include <assert.h>


#ifndef BOOTLOADER
  #undef __delay_cycles
  /**
   * @brief __delay_cycles() is an intrinsic IAR call; however, we
   * have explicity disallowed it since it is too specific to the system clock.
   * \note Please use halCommonDelayMicroseconds() instead, because it correctly
   * accounts for various system clock speeds.
   */
  #define __delay_cycles(x)  please_use_halCommonDelayMicroseconds_instead_of_delay_cycles
#endif

/**
 * @brief Set debug level based on whether or DEBUG is defined.
 * For the STM32W108xx, basic debugging support is included if DEBUG is not defined.
 */
#ifndef DEBUG_LEVEL
  #ifdef DEBUG
    #define DEBUG_LEVEL FULL_DEBUG
  #else
    #define DEBUG_LEVEL BASIC_DEBUG
  #endif
#endif

/**
 * @brief Set the application start and end address.
 * This are useful to detect whether an image is for bootloader mode or not.
 * This can be used also to clone image to another node via bootloader.
 */
#define APPLICATION_IMAGE_START ((u32) __section_begin("FLASH_IMAGE"))
#define APPLICATION_IMAGE_END ((u32) __section_end ("FLASH_IMAGE"))


/**
 * @brief Macro to reset the watchdog timer.  Note:  be very very
 * careful when using this as you can easily get into an infinite loop if you
 * are not careful.
 */
void halInternalResetWatchDog(void);
#define halResetWatchdog()  halInternalResetWatchDog()


/**
 * @brief Define __attribute__ to nothing since it isn't handled by IAR.
 */
#define __attribute__(nothing)


/**
 * @brief Declare a variable as unused to avoid a warning.  Has no effect
 * in IAR builds
 */
#define UNUSED

/**
 * @brief Some platforms need to cast enum values that have the high bit set.
 */
#define SIGNED_ENUM


/**
 * @brief Define the magic value that is interpreted by IAR C-SPY's Stack View.
 */
#define STACK_FILL_VALUE  0xCDCDCDCD

/**
 * @brief Define a generic RAM function identifier to a compiler specific one.
 */
#ifdef RAMEXE
  //If the whole build is running out of RAM, as chosen by the RAMEXE build
  //define, then define RAMFUNC to nothing since it's not needed.
  #define RAMFUNC
#else //RAMEXE
  #define RAMFUNC __ramfunc
#endif //RAMEXE

/**
 * @brief Define a generic no operation identifier to a compiler specific one.
 */
#define NO_OPERATION() __no_operation()

/**
 * @brief A convenience macro that makes it easy to change the field of a
 * register to any value.
 */
#define SET_REG_FIELD(reg, field, value)                      \
  do{                                                         \
    reg = ((reg & (~field##_MASK)) | (value << field##_BIT)); \
  }while(0)

/**
 * @brief Stub for code not running in simulation.
 */
#define simulatedTimePasses()
/**
 * @brief Stub for code not running in simulation.
 */
#define simulatedTimePassesMs(x)
/**
 * @brief Stub for code not running in simulation.
 */
#define simulatedSerialTimePasses()


/**
 * @brief Use the Divide and Modulus Operations from platform-common.h
 */
#define _HAL_USE_COMMON_DIVMOD_


/**
 * @brief Provide a portable way to specify the segment where a variable
 * lives.
 */
#define VAR_AT_SEGMENT(__variableDeclaration, __segmentName) \
    __variableDeclaration @ __segmentName

#define _QUOTEME(a) #a
#define QUOTEME(a) _QUOTEME(a)
#define ALIGN_VAR(__variableDeclaration, alignment) _Pragma(QUOTEME(data_alignment=alignment)) \
 __variableDeclaration

////////////////////////////////////////////////////////////////////////////////
//@}  // end of Miscellaneous Macros
////////////////////////////////////////////////////////////////////////////////

/** @name Portable segment names
 *@{
 */
/**
 * @brief Portable segment names
 */
#define __NO_INIT__       ".noinit"
#define __INTVEC__ ".intvec"
#define __CSTACK__ "CSTACK"
#define __DATA_INIT__ ".data_init"
#define __DATA__ ".data"
#define __BSS__ ".bss"
#define __CONST__ ".rodata"
#define __TEXT__ ".text"
#define __TEXTRW_INIT__ ".textrw_init"
#define __TEXTRW__ ".textrw"
#define __FAT__ "FAT" // Fixed address table
#define __NVM__ "NVM" //Non-Volatile Memory data storage

//=============================================================================
// The '#pragma segment=' declaration must be used before attempting to access
// the segments so the compiler properly handles the __segment_*() functions.
//
// The segment names used here are the default segment names used by IAR. Refer
// to the IAR Compiler Reference Guide for a proper description of these
// segments.
//=============================================================================
#pragma segment=__NO_INIT__
#pragma segment=__INTVEC__
#pragma segment=__CSTACK__
#pragma segment=__DATA_INIT__
#pragma segment=__DATA__
#pragma segment=__BSS__
#pragma segment=__CONST__
#pragma segment=__TEXT__
#pragma segment=__TEXTRW_INIT__
#pragma segment=__TEXTRW__
#pragma segment=__FAT__
#pragma segment=__NVM__
// Special pragma to get the application image start and end address
#pragma segment="FLASH_IMAGE"
/**@} */

//A utility function for inserting barrier instructions.  These
//instructions should be used whenever the MPU is enabled or disabled so
//that all memory/instruction accesses can complete before the MPU changes
//state.  
void _executeBarrierInstructions(void);
// MPU is unused with this platform header variant
#define _HAL_MPU_UNUSED_

////////////////////////////////////////////////////////////////////////////////
/** \name Global Interrupt Manipulation Macros
 *
 * \b Note: The special purpose BASEPRI register is used to enable and disable
 * interrupts while permitting faults.
 * When BASEPRI is set to 1 no interrupts can trigger. The configurable faults
 * (usage, memory management, and bus faults) can trigger if enabled as well as 
 * the always-enabled exceptions (reset, NMI and hard fault).
 * When BASEPRI is set to 0, it is disabled, so any interrupt can triggger if 
 * its priority is higher than the current priority.
 */
////////////////////////////////////////////////////////////////////////////////
//@{

#define ATOMIC_LITE(blah) ATOMIC(blah)
#define DECLARE_INTERRUPT_STATE_LITE DECLARE_INTERRUPT_STATE
#define DISABLE_INTERRUPTS_LITE() DISABLE_INTERRUPTS()
#define RESTORE_INTERRUPTS_LITE() RESTORE_INTERRUPTS()

#ifdef BOOTLOADER
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
    // The bootloader does not use interrupts
    #define DECLARE_INTERRUPT_STATE
    #define DISABLE_INTERRUPTS() do { } while(0)
    #define RESTORE_INTERRUPTS() do { } while(0)
    #define INTERRUPTS_ON() do { } while(0)
    #define INTERRUPTS_OFF() do { } while(0)
    #define INTERRUPTS_ARE_OFF() (FALSE)
    #define ATOMIC(blah) { blah }
    #define HANDLE_PENDING_INTERRUPTS() do { } while(0)
    #define SET_BASE_PRIORITY_LEVEL(basepri) do { } while(0)
  #endif  // DOXYGEN_SHOULD_SKIP_THIS
#else  // BOOTLOADER
  
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
      /**
       * @brief This macro should be called in the local variable
       * declarations section of any function which calls DISABLE_INTERRUPTS()
       * or RESTORE_INTERRUPTS().
       */
      #define DECLARE_INTERRUPT_STATE uint8_t _emIsrState
    
    // Prototypes for the BASEPRI and PRIMASK access functions.  They are very
    // basic and instantiated in assembly code in the file spmr.s37 (since
    // there are no C functions that cause the compiler to emit code to access
    // the BASEPRI/PRIMASK). This will inhibit the core from taking interrupts
    // with a priority equal to or less than the BASEPRI value.
    // Note that the priority values used by these functions are 5 bits and 
    // right-aligned
    extern uint8_t _readBasePri(void);
    extern void _writeBasePri(uint8_t priority);

    // Prototypes for BASEPRI functions used to disable and enable interrupts
    // while still allowing enabled faults to trigger.
    extern void _enableBasePri(void);
    extern uint8_t _disableBasePri(void);
    extern boolean _basePriIsDisabled(void);
    
    // Prototypes for setting and clearing PRIMASK for global interrupt
    // enable/disable.
    extern void _setPriMask(void);
    extern void _clearPriMask(void);
  #endif  // DOXYGEN_SHOULD_SKIP_THIS

  //The core Global Interrupt Manipulation Macros start here.
  
  /**
   * @brief Disable interrupts, saving the previous state so it can be
   * later restored with RESTORE_INTERRUPTS().
   * \note Do not fail to call RESTORE_INTERRUPTS().
   * \note It is safe to nest this call.
   */
  #define DISABLE_INTERRUPTS()                    \
    do {                                          \
      _emIsrState = _disableBasePri();            \
    } while(0)
  
  
  /** 
   * @brief Restore the global interrupt state previously saved by
   * DISABLE_INTERRUPTS()
   * \note Do not call without having first called DISABLE_INTERRUPTS()
   * to have saved the state.
   * \note It is safe to nest this call.
   */
  #define RESTORE_INTERRUPTS()      \
    do {                            \
      _writeBasePri(_emIsrState); \
    } while(0)
  
  
  /**
   * @brief Enable global interrupts without regard to the current or
   * previous state.
   */
  #define INTERRUPTS_ON()               \
    do {                                \
      _enableBasePri();                 \
    } while(0)
  
  
  /**
   * @brief Disable global interrupts without regard to the current or
   * previous state.
   */
  #define INTERRUPTS_OFF()      \
    do {                        \
      (void)_disableBasePri();  \
    } while(0)
  
  
  /**
   * @returns TRUE if global interrupts are disabled.
   */
  #define INTERRUPTS_ARE_OFF() ( _basePriIsDisabled() )
  
  /**
   * @returns TRUE if global interrupt flag was enabled when 
   * ::DISABLE_INTERRUPTS() was called.
   */
  #define INTERRUPTS_WERE_ON() (_emIsrState == 0)
  
  /**
   * @brief A block of code may be made atomic by wrapping it with this
   * macro.  Something which is atomic cannot be interrupted by interrupts.
   */
  #define ATOMIC(blah)         \
  {                            \
    DECLARE_INTERRUPT_STATE;   \
    DISABLE_INTERRUPTS();      \
    { blah }                   \
    RESTORE_INTERRUPTS();      \
  }
  
  
  /**
   * @brief Allows any pending interrupts to be executed. Usually this
   * would be called at a safe point while interrupts are disabled (such as
   * within an ISR).
   * 
   * Takes no action if interrupts are already enabled.
   */
  #define HANDLE_PENDING_INTERRUPTS() \
    do {                              \
      if (INTERRUPTS_ARE_OFF()) {     \
        INTERRUPTS_ON();              \
        INTERRUPTS_OFF();             \
      }                               \
   } while (0)
  
  
  /**
   * @brief Sets the base priority mask (BASEPRI) to the value passed,
   * bit shifted up by PRIGROUP_POSITION+1.  This will inhibit the core from
   * taking all interrupts with a preemptive priority equal to or less than
   * the BASEPRI mask.  This macro is dependent on the value of
   * PRIGROUP_POSITION in nvic-config.h. Note that the value 0 disables the
   * the base priority mask.
   *
   * Refer to the "PRIGROUP" table in nvic-config.h to know the valid values
   * for this macro depending on the value of PRIGROUP_POSITION.  With respect
   * to the table, this macro can only take the preemptive priority group
   * numbers denoted by the parenthesis.
   */
  #define SET_BASE_PRIORITY_LEVEL(basepri) \
    do {                                   \
      _writeBasePri(basepri);              \
    } while(0)
  
#endif // BOOTLOADER
////////////////////////////////////////////////////////////////////////////////
//@}  // end of Global Interrupt Manipulation Macros
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Use the C Standard Library Memory Utilities from platform-common.h
 */
#define _HAL_USE_COMMON_MEMUTILS_

////////////////////////////////////////////////////////////////////////////////
/** \name External Declarations
 * These are routines that are defined in certain header files that we don't
 * want to include, e.g. stdlib.h
 */
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * @brief Returns the absolute value of I (also called the magnitude of I).
 * That is, if I is negative, the result is the opposite of I, but if I is
 * nonnegative the result is I.
 *
 * @param I  An integer.
 *
 * @return A nonnegative integer.
 */
int abs(int I);

////////////////////////////////////////////////////////////////////////////////
//@}  // end of External Declarations
////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Include platform-common.h last to pick up defaults and common definitions.
 */
#define PLATCOMMONOKTOINCLUDE
  #include "hal/micro/generic/compiler/platform-common.h"
#undef PLATCOMMONOKTOINCLUDE

#endif // IAR_H_

/** @}  END addtogroup */
/** @} */

