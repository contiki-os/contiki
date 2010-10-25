/**
 * @file error.h
 * @brief Return codes for API functions and module definitions.
 *
 * See @ref status_codes for documentation.
 * 
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef __ERRORS_H__
#define __ERRORS_H__

/**
 * @brief  Return type for St functions.
 */
#ifndef __STSTATUS_TYPE__
#define __STSTATUS_TYPE__
  typedef int8u StStatus;
#endif //__STSTATUS_TYPE__

/**
 * @addtogroup status_codes
 * @{
 */

/**
 * @brief Macro used by error-def.h to define all of the return codes.
 *
 * @param symbol  The name of the constant being defined. All St returns
 *                begin with ST_. For example, ::ST_CONNECTION_OPEN.
 * 
 * @param value   The value of the return code. For example, 0x61.
 */
#define DEFINE_ERROR(symbol, value) \
 ST_ ## symbol = value,


enum {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "error-def.h"
#endif //DOXYGEN_SHOULD_SKIP_THIS
  /** Gets defined as a count of all the possible return codes in the 
  * StZNet stack API. 
  */
  ST_ERROR_CODE_COUNT

};

#undef DEFINE_ERROR

#endif // __ERRORS_H__

/**@} // End of addtogroup
 */
 

