/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-ac-dimmer
 * @{
 *
 * \file
 *  Driver for the Krida Electronics AC light dimmer with zero-crossing, using
 *  a 50Hz frequency as reference (1/50Hz) ~20ms and 10ms half-cycle
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "ac-dimmer.h"
#include "dev/gpio.h"
#include "lib/sensors.h"
#include "dev/ioc.h"
/*---------------------------------------------------------------------------*/
#define DIMMER_SYNC_PORT_BASE          GPIO_PORT_TO_BASE(DIMMER_SYNC_PORT)
#define DIMMER_SYNC_PIN_MASK           GPIO_PIN_MASK(DIMMER_SYNC_PIN)
#define DIMMER_GATE_PORT_BASE          GPIO_PORT_TO_BASE(DIMMER_GATE_PORT)
#define DIMMER_GATE_PIN_MASK           GPIO_PIN_MASK(DIMMER_GATE_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
static uint8_t dimming;
/*---------------------------------------------------------------------------*/
PROCESS(ac_dimmer_int_process, "AC Dimmer zero-cross interrupt process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ac_dimmer_int_process, ev, data)
{
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

  int dimtime;

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    dimtime = (uint8_t)(100 - dimming);
    dimtime *= 100;

    /* Off cycle */
    clock_delay_usec(dimtime);
    GPIO_SET_PIN(DIMMER_GATE_PORT_BASE, DIMMER_GATE_PIN_MASK);
    /* Triac on propagation delay */
    clock_delay_usec(DIMMER_DEFAULT_GATE_PULSE_US);
    GPIO_CLR_PIN(DIMMER_GATE_PORT_BASE, DIMMER_GATE_PIN_MASK);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
dimmer_zero_cross_int_handler(uint8_t port, uint8_t pin)
{
  if((port == DIMMER_SYNC_PORT) && (pin == DIMMER_SYNC_PIN)) {
    process_poll(&ac_dimmer_int_process);
  }
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
    return dimming;
  case SENSORS_READY:
    return enabled;
  }
  return DIMMER_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if(!enabled) {
    return DIMMER_ERROR;
  }

  dimming = (uint8_t)type;
  return DIMMER_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return DIMMER_ERROR;
  }

  if(value) {
    /* This is the Triac's gate pin */
    GPIO_SOFTWARE_CONTROL(DIMMER_GATE_PORT_BASE, DIMMER_GATE_PIN_MASK);
    GPIO_SET_OUTPUT(DIMMER_GATE_PORT_BASE, DIMMER_GATE_PIN_MASK);
    ioc_set_over(DIMMER_GATE_PORT, DIMMER_GATE_PIN, IOC_OVERRIDE_OE);
    GPIO_CLR_PIN(DIMMER_GATE_PORT_BASE, DIMMER_GATE_PIN_MASK);

    /* This is the zero-crossing pin and interrupt */
    GPIO_SOFTWARE_CONTROL(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
    GPIO_SET_INPUT(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);

    /* Pull-up resistor, detect rising edge */
    GPIO_DETECT_EDGE(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
    GPIO_TRIGGER_SINGLE_EDGE(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
    GPIO_DETECT_RISING(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
    gpio_register_callback(dimmer_zero_cross_int_handler, DIMMER_SYNC_PORT,
                           DIMMER_SYNC_PIN);

    /* Spin process until an interrupt is received */
    process_start(&ac_dimmer_int_process, NULL);

    /* Enable interrupts */
    GPIO_ENABLE_INTERRUPT(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
    // ioc_set_over(DIMMER_SYNC_PORT, DIMMER_SYNC_PIN, IOC_OVERRIDE_PUE);
    NVIC_EnableIRQ(DIMMER_INT_VECTOR);

    enabled = 1;
    dimming = DIMMER_DEFAULT_START_VALUE;
    return DIMMER_SUCCESS;
  }

  /* Disable interrupt and pins */
  
  GPIO_DISABLE_INTERRUPT(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
  GPIO_SET_INPUT(DIMMER_GATE_PORT_BASE, DIMMER_GATE_PIN_MASK);
  GPIO_SET_OUTPUT(DIMMER_SYNC_PORT_BASE, DIMMER_SYNC_PIN_MASK);
  process_exit(&ac_dimmer_int_process);

  enabled = 0;
  dimming = 0;
  return DIMMER_SUCCESS;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ac_dimmer, AC_DIMMER_ACTUATOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
