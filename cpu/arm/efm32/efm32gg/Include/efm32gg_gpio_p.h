/**************************************************************************//**
 * @file
 * @brief efm32gg_gpio_p Register and Bit Field definitions
 * @author Energy Micro AS
 * @version 3.20.2
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
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
 *****************************************************************************/
/**************************************************************************//**
 * @brief GPIO_P EFM32GG GPIO P
 *****************************************************************************/
typedef struct
{
  __IO uint32_t CTRL;     /**< Port Control Register  */
  __IO uint32_t MODEL;    /**< Port Pin Mode Low Register  */
  __IO uint32_t MODEH;    /**< Port Pin Mode High Register  */
  __IO uint32_t DOUT;     /**< Port Data Out Register  */
  __O uint32_t  DOUTSET;  /**< Port Data Out Set Register  */
  __O uint32_t  DOUTCLR;  /**< Port Data Out Clear Register  */
  __O uint32_t  DOUTTGL;  /**< Port Data Out Toggle Register  */
  __I uint32_t  DIN;      /**< Port Data In Register  */
  __IO uint32_t PINLOCKN; /**< Port Unlocked Pins Register  */
} GPIO_P_TypeDef;

