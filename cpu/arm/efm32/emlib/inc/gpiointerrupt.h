/***************************************************************************//**
 * @file
 * @brief General Purpose IO (GPIO) interrupt dispatcher API
 * @author Energy Micro AS
 * @version 3.20.2
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
#ifndef __EMDRV_GPIOINTERRUPT_H
#define __EMDRV_GPIOINTERRUPT_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup GPIOINT
 * @brief General Purpose Input/Output (GPIO) Interrupt Dispatcher API
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   TYPEDEFS   **********************************
 ******************************************************************************/

/**
 * @brief
 *  GPIO interrupt callback function pointer.
 * @details
 *   Parameters:
 *   @li pin - The pin index the callback function is invoked for.
 */
typedef void (*GPIOINT_IrqCallbackPtr_t)(uint8_t pin);

/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/
void GPIOINT_Init(void);
void GPIOINT_CallbackRegister(uint8_t pin, GPIOINT_IrqCallbackPtr_t callbackPtr);
static __INLINE void GPIOINT_CallbackUnRegister(uint8_t pin);

/***************************************************************************//**
 * @brief
 *   Unregisters user callback for given pin number.
 *
 * @details
 *   Use this function to unregister a callback.
 *
 * @param[in] pin
 *   Pin number for the callback.
 *
 ******************************************************************************/
static __INLINE void GPIOINT_CallbackUnRegister(uint8_t pin)
{
  GPIOINT_CallbackRegister(pin,0);
}

/** @} (end addtogroup GPIOINT */
/** @} (end addtogroup EM_Drivers) */
#ifdef __cplusplus
}
#endif

#endif /* __EMDRV_GPIOINTERRUPT_H */
