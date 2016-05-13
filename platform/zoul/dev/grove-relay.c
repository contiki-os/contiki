/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-grove-relay
 * @{
 *
 * \file
 *      HLS8-T73 GROVE_RELAY example   
 * \author
 *      Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/grove-relay.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
/*---------------------------------------------------------------------------*/
#define RELAY_DEBUG 0
#if RELAY_DEBUG
#define PRINTF_RELAY(...) printf(__VA_ARGS__)
#else
#define PRINTF_RELAY(...)
#endif
/*---------------------------------------------------------------------------*/
#define GROVE_RELAY_PORT_BASE   GPIO_PORT_TO_BASE(GROVE_RELAY_CTRL_PORT)
#define GROVE_RELAY_PIN_MASK    GPIO_PIN_MASK(GROVE_RELAY_CTRL_PIN)
/*---------------------------------------------------------------------------*/
void
grove_relay_configure(void)
{
  /* Set as output, used as relay activation */
  GPIO_SOFTWARE_CONTROL(GROVE_RELAY_PORT_BASE, GROVE_RELAY_PIN_MASK);
  ioc_set_over(GROVE_RELAY_CTRL_PORT, GROVE_RELAY_CTRL_PIN, IOC_OVERRIDE_DIS); 
  GPIO_SET_OUTPUT(GROVE_RELAY_PORT_BASE, GROVE_RELAY_PIN_MASK);
  GPIO_CLR_PIN(GROVE_RELAY_PORT_BASE, GROVE_RELAY_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
int
grove_relay_set(int state)
{
  if(state > GROVE_RELAY_ON) {
    return GROVE_RELAY_ERROR;
  }

  if(state) {
    /* Set ON relay (closed) */
    GPIO_SET_PIN(GROVE_RELAY_PORT_BASE, GROVE_RELAY_PIN_MASK);
    PRINTF_RELAY("Relay Closed!\n");
  } else {
    /* Set OFF relay (open) */
    GPIO_CLR_PIN(GROVE_RELAY_PORT_BASE, GROVE_RELAY_PIN_MASK);
    PRINTF_RELAY("Relay Opened!\n");
  }
  
  return GROVE_RELAY_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/** @} */
