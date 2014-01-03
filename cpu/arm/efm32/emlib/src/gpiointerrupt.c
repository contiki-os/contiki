/***************************************************************************//**
 * @file
 * @brief General Purpose IO (GPIO) interrupt dispatcher.
 * @author Energy Micro AS
 * @version 3.20.2
 *
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
#include "em_gpio.h"
#include "em_int.h"
#include "gpiointerrupt.h"
#include "em_assert.h"

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup GPIOINT
 * @brief General Purpose Input/Output (GPIO) Interrupt Dispatcher API
 * @details
 * This is a GPIO interrupt dispatcher module. It consists of gpiointerrupt.c
 * and gpiointerrupt.h. EFM32 has two GPIO interrupts (Odd and Even). If more
 * than two interrupts are used then interrupt routine must dispatch. This
 * driver provides small dispatcher for both GPIO interrupts enabling
 * handling of up to 16 GPIO pin interrupts.
 *
 * It is up to the user to set up and enable interrupt on given pin. Dispatcher
 * handles cleaning of interrupt flags.
 *
 * In order to use GPIO Interrupt Dispatcher it has to be initialized first by
 * calling GPIOINT_Init(). Then each pin must be configured by first registering
 * the callback for given pin (GPIOINT_CallbackRegister()) and then setting up
 * and enabling the interrupt in GPIO module.
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   MACROS   ***********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/* Macro return index of the LSB flag which is set. */
#define GPIOINT_MASK2IDX(mask) (__CLZ(__RBIT(mask)))

/** @endcond */

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

typedef struct
{
  /* Pin number in range of 0 to 15 */
  uint32_t pin;

  /* Pointer to the callback function */
  GPIOINT_IrqCallbackPtr_t callback;

} GPIOINT_CallbackDesc_t;


/*******************************************************************************
 ********************************   GLOBALS   **********************************
 ******************************************************************************/

/* Array of user callbacks. One for each pin. */
static GPIOINT_IrqCallbackPtr_t gpioCallbacks[16] = {0};

/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/
static void GPIOINT_IRQDispatcher(uint32_t iflags);

/** @endcond */

/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS   ******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialization of GPIOINT module.
 *
 ******************************************************************************/
void GPIOINT_Init(void)
{
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}


/***************************************************************************//**
 * @brief
 *   Registers user callback for given pin number.
 *
 * @details
 *   Use this function to register a callback which shall be called upon
 *   interrupt generated from given pin number (port is irrelevant). Interrupt
 *   itself must be configured externally. Function overwrites previously
 *   registered callback.
 *
 * @param[in] pin
 *   Pin number for the callback.
 * @param[in] callbackPtr
 *   A pointer to callback function.
 ******************************************************************************/
void GPIOINT_CallbackRegister(uint8_t pin, GPIOINT_IrqCallbackPtr_t callbackPtr)
{
  INT_Disable();

  /* Dispatcher is used */
  gpioCallbacks[pin] = callbackPtr;

  INT_Enable();
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *   Function calls users callback for registered pin interrupts.
 *
 * @details
 *   This function is called when GPIO interrupts are handled by the dispatcher.
 *   Function gets even or odd interrupt flags and calls user callback
 *   registered for that pin. Function iterates on flags starting from MSB.
 *
 * @param iflags
 *  Interrupt flags which shall be handled by the dispatcher.
 *
 ******************************************************************************/
static void GPIOINT_IRQDispatcher(uint32_t iflags)
{
  uint32_t irqIdx;

  /* check for all flags set in IF register */
  while(iflags)
  {
    irqIdx = GPIOINT_MASK2IDX(iflags);

    /* clear flag*/
    iflags &= ~(1 << irqIdx);

    if (gpioCallbacks[irqIdx])
    {
      /* call user callback */
      gpioCallbacks[irqIdx](irqIdx);
    }
  }
}

/***************************************************************************//**
 * @brief
 *   GPIO EVEN interrupt handler. Interrupt handler clears all IF even flags and
 *   call the dispatcher passing the flags which triggered the interrupt.
 *
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  uint32_t iflags;

  /* Get all even interrupts. */
  iflags = GPIO_IntGetEnabled() & 0x00005555;

  /* Clean only even interrupts. */
  GPIO_IntClear(iflags);

  GPIOINT_IRQDispatcher(iflags);
}


/***************************************************************************//**
 * @brief
 *   GPIO ODD interrupt handler. Interrupt handler clears all IF odd flags and
 *   call the dispatcher passing the flags which triggered the interrupt.
 *
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  uint32_t iflags;

  /* Get all odd interrupts. */
  iflags = GPIO_IntGetEnabled() & 0x0000AAAA;

  /* Clean only even interrupts. */
  GPIO_IntClear(iflags);

  GPIOINT_IRQDispatcher(iflags);
}

/** @endcond */

/** @} (end addtogroup GPIOINT */
/** @} (end addtogroup EM_Drivers) */
