/*
 * Copyright (c) 2013, Kerlink
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup efm32-gpio
 * @{
 */

/**
 * \file
 *         EFM32 GPIO routines
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdint.h>
#include <stdio.h>
#include "platform-conf.h"
#include "gpio.h"
#include "gpiointerrupt.h"
#include "em_cmu.h"

#include "irq.h"
#include "core_cmInstr.h"

#define _MASK2IDX(mask) (__CLZ(__RBIT(mask)))

/**
 * \brief       Set mode for pin
 *
 * \param gpio  Gpio number to set as output
 * \param value Value to set for gpio
 *
 *              This function set gpio as default output :
 *               - Open-drain
 *               - no pullup
 */
inline void gpio_set_mode(uint8_t gpio, GPIO_Mode_TypeDef mode, uint8_t value)
{
  GPIO_PinModeSet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio), mode, value);
}


/*---------------------------------------------------------------------------*/
/**
 * \brief     Get GPIO value
 *
 * \param gpio  Gpio number to get
 *
 * \return     The pin value, 0 or 1
 */
inline uint8_t gpio_get_value(uint8_t gpio)
{
  return GPIO_PinInGet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio));
}

/*---------------------------------------------------------------------------*/
/**
 * \brief       Set output value for GPIO
 *
 * \param gpio  Gpio number to set
 * \param value Output value to set for gpio
 */
inline void gpio_set_value(uint8_t gpio, uint8_t value)
{
  if(value)
  {
    GPIO_PinOutSet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio));
  }
  else
  {
    GPIO_PinOutClear(GPIO_PORTNO(gpio), GPIO_PINNO(gpio));
  }
}

/*---------------------------------------------------------------------------*/
/**
 * \brief       Disable Pin (no connect)
 *
 * \param gpio  Gpio to disable
 */
inline void gpio_disable(uint8_t gpio)
{
  GPIO_PinModeSet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio), gpioModeDisabled, 0);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief       Set pin as input
 *
 * \param gpio  Gpio to set as output
 */
inline void gpio_set_input(uint8_t gpio)
{
  GPIO_PinModeSet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio), gpioModeInput, 0);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief       Set pin as output
 *
 * \param gpio  Gpio number to set as output
 * \param value Value to set for gpio
 *
 *              This function set gpio as default output :
 *               - Open-drain
 *               - no pullup
 */
inline void gpio_set_output(uint8_t gpio, uint8_t value)
{
  GPIO_PinModeSet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio), gpioModeWiredAnd, value);
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/**
 * \brief       Set pin as Pushpull output
 *
 * \param gpio  Gpio number to set as output
 * \param value Output to set for gpio
 */
inline void gpio_set_output_pushpull(uint8_t gpio, uint8_t value)
{
  GPIO_PinModeSet(GPIO_PORTNO(gpio), GPIO_PINNO(gpio), gpioModePushPull, value);
}



/*---------------------------------------------------------------------------*/
/*---------         P O W E R        M A N A G E M E N T            ---------*/
/*---------------------------------------------------------------------------*/

/**
 * Enable or disable a GPIO as wakeup source
 * Corresponding IRQ must be registered (external or port match)
 *
 * \param enable   1 to enable, 0 to disable
 * \param level    logical level to catch
 * \return         0 if OK
 *                 -1 if GPIO can't be a wakeup source
 *
 */
int gpio_set_wakeup(uint8_t gpio, char enable, char level)
{
	// TBD
  return -1;
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

int gpio_suspend(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int gpio_resume(void)
{
	//TBD

	return 0;
}

/*---------------------------------------------------------------------------*/
typedef struct{
    uint8_t gpio;
    uint8_t irqtype;
    void *priv;
    irq_handler_t handler;
} gpioirq;

static gpioirq gpioirq_list[GPIO_MAX_IRQS];

/*---------------------------------------------------------------------------*/
static inline void gpio_edge_irqhandler(uint32_t iflags)
{
  uint32_t irqIdx;

  /* check for all flags set in IF register */
  while(iflags)
  {
    irqIdx = _MASK2IDX(iflags);

    /* clear flag*/
    iflags &= ~(1 << irqIdx);

    if (gpioirq_list[irqIdx].handler != NULL)
    {
      /* call user callback */
      gpioirq_list[irqIdx].handler(irqIdx, gpioirq_list[irqIdx].priv);
    }
  }
}

void GPIO_EVEN_IRQHandler(void)
{
  uint32_t iflags;

  /* Get all even interrupts. */
  iflags = GPIO_IntGetEnabled() & 0x00005555;

  /* Clean only even interrupts. */
  GPIO_IntClear(iflags);

  gpio_edge_irqhandler(iflags);
}

void GPIO_ODD_IRQHandler(void)
{
  uint32_t iflags;

  /* Get all odd interrupts. */
  iflags = GPIO_IntGetEnabled() & 0x0000AAAA;

  /* Clean only even interrupts. */
  GPIO_IntClear(iflags);

  gpio_edge_irqhandler(iflags);
}

/*---------------------------------------------------------------------------*/
int gpio_register_irq(uint8_t gpio, irq_edge_type_t type, irq_handler_t handler, void *priv)
{
  int i = 0;
  uint8_t gpioirq_no = GPIO_PINNO(gpio);

  irq_disable();


  GPIO_IntConfig(GPIO_PORTNO(gpio), GPIO_PINNO(gpio),
                 (type & IRQ_TYPE_EDGE_RISING)?true:false,
                 (type & IRQ_TYPE_EDGE_FALLING)?true:false,
                 false);

  gpioirq_list[gpioirq_no].gpio    = gpio;
  gpioirq_list[gpioirq_no].irqtype = type;
  gpioirq_list[gpioirq_no].priv    = priv;
  gpioirq_list[gpioirq_no].handler = handler;

  irq_enable();

  return gpioirq_no;
}

inline void gpio_enable_irq(uint8_t gpioirq_id)
{
  GPIO_IntEnable( 1 << gpioirq_id );
}
inline void gpio_disable_irq(uint8_t gpioirq_id)
{
  GPIO_IntDisable( 1 << gpioirq_id );
}

/*---------------------------------------------------------------------------*/
/**
 * \brief       Initialize
 *
 * \param gpio  Gpio number to set as output
 * \param value Output to set for gpio
 */
void gpio_init(void)
{
  // Enable Port Bank 0 Clock to access registers
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

/** @} */
