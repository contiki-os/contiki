/** @file hal/hal.h
 * @brief Generic set of HAL includes for all platforms.
 *
 * See also @ref hal for more documentation.
 *  
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
  
/** @addtogroup hal 
 *  @if STM32W10XX
 *    <center><h1>STM32W108xx Microprocessors</h1></center>
 *  @endif
 *
 * HAL function names have the following conventions:
 *
 *  <b>HAL which SimpleMAC library depends upon:</b>  API that is required for proper operation of the SimpleMAC library.  As with all of the HAL, it is provided as source and it is possible for the end customer to modify this code.  However, unlike other portions of the HAL, if the customer does modify this code,  it must ensure that equivalent functionality is still provided to ensure proper operation of the SimpleMAC library.
 *
 *
 *  <b>HAL for other chip capabilities:</b>   API which SimpleMAC does not directly depend on. However, much of this functionality may be required for proper operation of all features of the chip. 
 *
 *  <b>Additional HAL for sample applications:</b>  API which is included to assist the development of the included sample applications. This functionality may be very useful, but is not required for proper operation of the chip. This code and functionality may be freely modified by the end customer.
 *
 * <br><br>
 *
 * See also hal.h.
 */


#ifndef __HAL_H__
#define __HAL_H__

// Keep micro first for specifics used by other headers
#include "micro/micro-common.h"
#include "micro/led.h"
#include "micro/button.h"
#include "micro/system-timer.h"
#include "micro/cortexm3/nvm.h"
#include "hal/micro/cortexm3/uart.h"
#include "hal/micro/adc.h"

#endif //__HAL_H__

