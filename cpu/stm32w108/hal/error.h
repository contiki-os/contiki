/**
 * @file cpu/stm32w108/hal/error.h
 * @brief Return codes for API functions and module definitions.
 *
 * See @ref status_codes for documentation.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef ERRORS_H_
#define ERRORS_H_

#ifndef __STSTATUS_TYPE__
#define __STSTATUS_TYPE__
/**
 * @brief  Return type for St functions.
 */
typedef uint8_t StStatus;
#endif //__STSTATUS_TYPE__

/**
 * @addtogroup status_codes
 * @{
 */

/**
 * @brief Macro used by error-def.h to define all of the return codes.
 *
 * @param symbol  The name of the constant being defined. All St returns
 *                begin with ST_. For example, ::ST_ERR_FATAL.
 *
 * @param value   The value of the return code. For example, 0x61.
 */
#define DEFINE_ERROR(symbol, value) \
 ST_ ## symbol = value,


enum {
#include "error-def.h"
  /** Gets defined as a count of all the possible return codes in the
  * StZNet stack API.
  */
  ST_ERROR_CODE_COUNT

};

#undef DEFINE_ERROR

#endif // ERRORS_H_

/**@} // End of addtogroup
 */
