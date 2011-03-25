/** @file hal/micro/generic/compiler/platform-common.h
 * See @ref platform_common for detailed documentation.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

/** @addtogroup platform_common
 * @brief Compiler and Platform specific definitions and typedefs common to
 * all platforms.  
 *
 * platform-common.h provides PLATFORM_HEADER defaults and common definitions.
 * This head should never be included directly, it should only be included
 * by the specific PLATFORM_HEADER used by your platform.
 *
 * See platform-common.h for source code.
 *@{
 */
 
#ifndef PLATCOMMONOKTOINCLUDE
  //  This header should only be included by a PLATFORM_HEADER
  #error  platform-common.h should not be included directly
#endif

#ifndef __PLATFORMCOMMON_H__
#define __PLATFORMCOMMON_H__
////////////////////////////////////////////////////////////////////////////////
// Many of the common definitions must be explicitly enabled by the 
//  particular PLATFORM_HEADER being used
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
#ifdef _HAL_USE_COMMON_PGM_
  /** \name Master Program Memory Declarations
   * These are a set of defines for simple declarations of program memory.
   */
  //@{
  /**
   * @brief Standard program memory delcaration.
   */
  #define PGM     const

  /**
   * @brief Char pointer to program memory declaration.
   */
  #define PGM_P   const char *

  /**
   * @brief Unsigned char pointer to program memory declaration.
   */
  #define PGM_PU  const unsigned char *


  /**
   * @brief Sometimes a second PGM is needed in a declaration.  Having two
   * 'const' declarations generates a warning so we have a second PGM that turns
   * into nothing under gcc.
   */
  #define PGM_NO_CONST
  //@} \\END MASTER PROGRAM MEMORY DECLARATIONS
#endif //_HAL_USE_COMMON_PGM_


////////////////////////////////////////////////////////////////////////////////
#ifdef _HAL_USE_COMMON_DIVMOD_
  /** \name Divide and Modulus Operations
   * Some platforms can perform divide and modulus operations on 32 bit 
   * quantities more efficiently when the divisor is only a 16 bit quantity.
   * C compilers will always promote the divisor to 32 bits before performing the
   * operation, so the following utility functions are instead required to take 
   * advantage of this optimisation.
   */
  //@{
  /**
   * @brief Provide a portable name for the int32u by int16u division
   * library function (which can perform the division with only a single 
   * assembly instruction on some platforms)
   */
  #define halCommonUDiv32By16(x, y) ((int16u) (((int32u) (x)) / ((int16u) (y))))

  /**
   * @brief Provide a portable name for the int32s by int16s division
   * library function (which can perform the division with only a single
   * assembly instruction on some platforms)
   */
  #define halCommonSDiv32By16(x, y) ((int16s) (((int32s) (x)) / ((int16s) (y))))

  /**
   * @brief Provide a portable name for the int32u by int16u modulo
   * library function (which can perform the division with only a single
   * assembly instruction on some platforms)
   */
  #define halCommonUMod32By16(x, y) ((int16u) (((int32u) (x)) % ((int16u) (y))))

  /**
   * @brief Provide a portable name for the int32s by int16s modulo
   * library function (which can perform the division with only a single
   * assembly instruction on some platforms)
   */
  #define halCommonSMod32By16(x, y) ((int16s) (((int32s) (x)) % ((int16s) (y))))
  //@} \\END DIVIDE and MODULUS OPERATIONS
#endif //_HAL_USE_COMMON_DIVMOD_


////////////////////////////////////////////////////////////////////////////////
#ifdef _HAL_USE_COMMON_MEMUTILS_
  /** \name C Standard Library Memory Utilities
   * These should be used in place of the standard library functions.
   * 
   * These functions have the same parameters and expected results as their C
   * Standard Library equivalents but may take advantage of certain implementation
   * optimizations.
   * 
   * Unless otherwise noted, these functions are utilized by the StStack and are 
   * therefore required to be implemented in the HAL. Additionally, unless otherwise
   * noted, applications that find these functions useful may utilze them.
   */
  //@{

  /**
   * @brief Refer to the C stdlib memcpy().
   */
  void halCommonMemCopy(void *dest, const void *src, int8u bytes);


  /**
   * @brief Refer to the C stdlib memset().
   */
  void halCommonMemSet(void *dest, int8u val, int16u bytes);


  /**
   * @brief Refer to the C stdlib memcmp().
   */
  int8s halCommonMemCompare(const void *source0, const void *source1, int8u bytes);


  /**
   * @brief Friendly convenience macro pointing to the full HAL function.
   */
  #define MEMSET(d,v,l)  halCommonMemSet(d,v,l)
  #define MEMCOPY(d,s,l) halCommonMemCopy(d,s,l)
  #define MEMCOMPARE(s0,s1,l) halCommonMemCompare(s0, s1, l)
  #define MEMPGMCOMPARE(s0,s1,l) halCommonMemPGMCompare(s0, s1, l)

  //@}  // end of C Standard Library Memory Utilities
#endif //_HAL_USE_COMMON_MEMUTILS_









////////////////////////////////////////////////////////////////////////////////
//  The following sections are common on all platforms
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @name Generic Types
 *@{
 */

/**
 * @brief An alias for one, used for clarity.
 */
#define TRUE  1

/**
 * @brief An alias for zero, used for clarity.
 */
#define FALSE 0

#ifndef NULL
/**
 * @brief The null pointer.
 */
#define NULL ((void *)0)
#endif

//@} \\END Generic Types


/**
 * @name  Bit Manipulation Macros
 */
//@{

/**
 * @brief Useful to reference a single bit of a byte.
 */
#define BIT(x) (1U << (x))  // Unsigned avoids compiler warnings re BIT(15)

/**
 * @brief Useful to reference a single bit of an int32u type.
 */
#define BIT32(x) (((int32u) 1) << (x))

/**
 * @brief Sets \c bit in the \c reg register or byte.
 * @note Assuming \c reg is an IO register, some platforms
 * can implement this in a single atomic operation.
*/
#define SETBIT(reg, bit)      reg |= BIT(bit)

/**
 * @brief Sets the bits in the \c reg register or the byte 
 * as specified in the bitmask \c bits. 
 * @note This is never a single atomic operation.
 */
#define SETBITS(reg, bits)    reg |= (bits)

/**
 * @brief Clears a bit in the \c reg register or byte. 
 * @note Assuming \c reg is an IO register, some platforms (such as the AVR) 
 * can implement this in a single atomic operation.
 */
#define CLEARBIT(reg, bit)    reg &= ~(BIT(bit))

/**
 * @brief Clears the bits in the \c reg register or byte 
 * as specified in the bitmask \c bits. 
 * @note This is never a single atomic operation.
 */
#define CLEARBITS(reg, bits)  reg &= ~(bits)

/**
 * @brief Returns the value of \c bit within the register or byte \c reg.
*/
#define READBIT(reg, bit)     (reg & (BIT(bit)))

/**
 * @brief Returns the value of the bitmask \c bits within 
 * the register or byte \c reg.
 */
#define READBITS(reg, bits)   (reg & (bits))

//@} \\END Bit Manipulation Macros


////////////////////////////////////////////////////////////////////////////////
/**
 * @name  Byte Manipulation Macros
 */
//@{

/**
 * @brief Returns the low byte of the 16-bit value \c n as an \c int8u.
 */
#define LOW_BYTE(n)                     ((int8u)((n) & 0xFF))

/**
 * @brief Returns the high byte of the 16-bit value \c n as an \c int8u.
 */
#define HIGH_BYTE(n)                    ((int8u)(LOW_BYTE((n) >> 8)))

/**
 * @brief Returns the value built from the two \c int8u 
 * values \c high and \c low.
 */
#define HIGH_LOW_TO_INT(high, low) (                              \
                                    (( (int16u) (high) ) << 8) +  \
                                    (  (int16u) ( (low) & 0xFF))  \
                                   )                                

/**
 * @brief Returns the low byte of the 32-bit value \c n as an \c int8u.
 */
#define BYTE_0(n)                    ((int8u)((n) & 0xFF))

/**
 * @brief Returns the second byte of the 32-bit value \c n as an \c int8u.
 */
#define BYTE_1(n)                    ((int8u)(BYTE_0((n) >> 8)))

/**
 * @brief Returns the third byte of the 32-bit value \c n as an \c int8u.
 */
#define BYTE_2(n)                    ((int8u)(BYTE_0((n) >> 16)))

/**
 * @brief Returns the high byte of the 32-bit value \c n as an \c int8u.
 */
#define BYTE_3(n)                    ((int8u)(BYTE_0((n) >> 24)))

//@} \\END Byte manipulation macros


////////////////////////////////////////////////////////////////////////////////
/**
 * @name  Time Manipulation Macros
 */
//@{

/**
 * @brief Returns the elapsed time between two 8 bit values.  
 *        Result may not be valid if the time samples differ by more than 127
 */
#define elapsedTimeInt8u(oldTime, newTime)       \
  ((int8u) ((int8u)(newTime) - (int8u)(oldTime)))

/**
 * @brief Returns the elapsed time between two 16 bit values.  
 *        Result may not be valid if the time samples differ by more than 32767
 */
#define elapsedTimeInt16u(oldTime, newTime)      \
  ((int16u) ((int16u)(newTime) - (int16u)(oldTime)))

/**
 * @brief Returns the elapsed time between two 32 bit values.  
 *   Result may not be valid if the time samples differ by more than 2147483647
 */
#define elapsedTimeInt32u(oldTime, newTime)      \
  ((int32u) ((int32u)(newTime) - (int32u)(oldTime)))

/**
 * @brief Returns TRUE if t1 is greater than t2.  Can only account for 1 wrap
 * around of the variable before it is wrong.
 */
#define MAX_INT8U_VALUE 0xFF
#define timeGTorEqualInt8u(t1, t2)           \
  (elapsedTimeInt8u(t2, t1) <= ((MAX_INT8U_VALUE + 1) / 2))

/**
 * @brief Returns TRUE if t1 is greater than t2.  Can only account for 1 wrap
 * around of the variable before it is wrong.
 */
#define MAX_INT16U_VALUE 0xFFFF
#define timeGTorEqualInt16u(t1, t2)           \
  (elapsedTimeInt16u(t2, t1) <= ((MAX_INT16U_VALUE + 1) / 2))

/**
 * @brief Returns TRUE if t1 is greater than t2.  Can only account for 1 wrap
 * around of the variable before it is wrong.
 */
#define MAX_INT32U_VALUE 0xFFFFFFFF
#define timeGTorEqualInt32u(t1, t2)           \
  (elapsedTimeInt32u(t2, t1) <= ((MAX_INT32U_VALUE + 1) / 2))

//@} \\END Time manipulation macros



#endif //__PLATFORMCOMMON_H__

/** @}  END addtogroup */

