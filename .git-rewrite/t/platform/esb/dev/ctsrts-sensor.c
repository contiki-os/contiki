/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: ctsrts-sensor.c,v 1.1 2006/06/18 07:49:33 adamdunkels Exp $
 */

/**
 * RTS/CTS (Request to Send/Clear to Send) are the signals used for hardware
 * flow control. By setting the RTS line to "ON" the host tells the connected
 * device that it is ready to receive data. Hardware flow control is not
 * implemented yet. This implementation is just so some application can use
 * the pins, it would also be possible for rs232.c to use it for hardware
 * handshake but as said, that is not implemented yet.
 */

#include "contiki-esb.h"

const struct sensors_sensor ctsrts_sensor;

HWCONF_PIN(RS232RTS, 1, 7);

HWCONF_PIN(RS232CTS, 1, 6);
HWCONF_IRQ(RS232CTS, 1, 6);


/*---------------------------------------------------------------------------*/
static void
init(void)
{
  RS232RTS_SELECT();
  RS232RTS_MAKE_OUTPUT();
  RS232RTS_CLEAR();
  RS232CTS_SELECT();
  RS232CTS_MAKE_INPUT();
}

/**
 * Indicate to host/client we are NOT ready to receive data. Sets the RTS pin
 * to low.
 */
void ctsrts_rts_clear(void) {
  RS232RTS_CLEAR();
}

/**
 * Request host/client to send data. Sets the RTS pin to high.
 */
void ctsrts_rts_set(void) {
  RS232RTS_SET();
}

/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  if(RS232CTS_CHECK_IRQ()) {
    /* Change the flank triggering for the irq so we will detect next
       shift. */
    if(RS232CTS_READ()) {
      RS232CTS_IRQ_EDGE_SELECTD();
    } else {
      RS232CTS_IRQ_EDGE_SELECTU();
    }

    sensors_changed(&ctsrts_sensor);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  /*
   * Check current status on the CTS pin and set IRQ flank so we will detect
   * a shift.
   */
  if(RS232CTS_READ()) {
    RS232CTS_IRQ_EDGE_SELECTD();
  } else {
    RS232CTS_IRQ_EDGE_SELECTU();
  }
  
  sensors_add_irq(&ctsrts_sensor, RS232CTS_IRQ_PORT());
  RS232CTS_ENABLE_IRQ();
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  RS232CTS_DISABLE_IRQ();
  sensors_remove_irq(&ctsrts_sensor, RS232CTS_IRQ_PORT());
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return RS232CTS_IRQ_ENABLED();
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  /*
   * Invert the bit and return.
   * This is strange, accordingly to the MSP430 manual section 9.2.1, Input
   * Register PxIN the bit should be low when input is low. In RealTerm on
   * the PC I set RTS which is coupled to the CTS on the esb and I read a 0.
   * Maybe RTS is defined active LOW on the PC? //Kalle
   */
  return RS232CTS_READ()?0:1;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, void *c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void *
status(int type)
{
  return NULL;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ctsrts_sensor, CTSRTS_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
