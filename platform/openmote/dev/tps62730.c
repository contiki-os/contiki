/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
 *
 */

/**
 * \addtogroup platform
 * @{
 *
 * \defgroup openmote The OpenMote Platform
 *
 * \file
 * Driver for the TPS62730 voltage regulator on the OpenMote-CC2538.
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */

/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/gpio.h"
#include "tps62730.h"
/*---------------------------------------------------------------------------*/
#define BSP_TPS62730_BASE           (GPIO_B_BASE)
#define BSP_TPS62730_ON             (1 << 1)
#define BSP_TPS62730_STATUS         (1 << 0)
/*---------------------------------------------------------------------------*/
static void
gpio_set(int port, int bit)
{
  REG((port | GPIO_DATA) + (bit << 2)) = bit;
}
/*---------------------------------------------------------------------------*/
static void
gpio_reset(int port, int bit)
{
  REG((port | GPIO_DATA) + (bit << 2)) = 0;
}
/*---------------------------------------------------------------------------*/
/**
 * Initializes the TPS62730 voltage regulator
 * By default it is in bypass mode, Vout = Vin, Iq < 1 uA
 */
void
tps62730_init(void)
{
  GPIO_SET_OUTPUT(BSP_TPS62730_BASE, BSP_TPS62730_ON);
  GPIO_SET_INPUT(BSP_TPS62730_BASE, BSP_TPS62730_STATUS);

  tps62730_bypass();
}
/*---------------------------------------------------------------------------*/
/**
 * Enables the TPS62730, Vout = 2.2V, Iq = 30 uA
 */
void
tps62730_on(void)
{
  gpio_set(BSP_TPS62730_BASE, BSP_TPS62730_ON);
}
/*---------------------------------------------------------------------------*/
/**
 * Disables the TPS62730, Vout = Vin, Iq < 1 uA
 */
void
tps62730_bypass(void)
{
  gpio_reset(BSP_TPS62730_BASE, BSP_TPS62730_ON);
}
/*---------------------------------------------------------------------------*/
/** @} */
