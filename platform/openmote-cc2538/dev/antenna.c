/*
 * Copyright (c) 2014, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup openmote-antenna
 * @{
 *
 * Driver for the OpenMote-CC2538 RF switch.
 * INT is the internal antenna (chip) configured through ANT1_SEL (V1)
 * EXT is the external antenna (connector) configured through ANT2_SEL (V2)
 * @{
 *
 * \file
 * Driver implementation for the OpenMote-CC2538 antenna switch
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/gpio.h"
#include "dev/antenna.h"
/*---------------------------------------------------------------------------*/
#define BSP_RADIO_BASE              GPIO_PORT_TO_BASE(GPIO_D_NUM)
#define BSP_RADIO_INT               GPIO_PIN_MASK(5)
#define BSP_RADIO_EXT               GPIO_PIN_MASK(4)
/*---------------------------------------------------------------------------*/
void
antenna_init(void)
{
  /* Configure the ANT1 and ANT2 GPIO as output */
  GPIO_SET_OUTPUT(BSP_RADIO_BASE, BSP_RADIO_INT);
  GPIO_SET_OUTPUT(BSP_RADIO_BASE, BSP_RADIO_EXT);

  /* Select external antenna by default. */
  antenna_external();
}
/*---------------------------------------------------------------------------*/
void
antenna_external(void)
{
  GPIO_WRITE_PIN(BSP_RADIO_BASE, BSP_RADIO_INT, 0);
  GPIO_WRITE_PIN(BSP_RADIO_BASE, BSP_RADIO_EXT, 1);
}
/*---------------------------------------------------------------------------*/
void
antenna_internal(void)
{
  GPIO_WRITE_PIN(BSP_RADIO_BASE, BSP_RADIO_EXT, 0);
  GPIO_WRITE_PIN(BSP_RADIO_BASE, BSP_RADIO_INT, 1);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
