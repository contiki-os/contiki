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
 */

/**
 * RTS/CTS (Request to Send/Clear to Send) are the signals used for hardware
 * flow control. By setting the RTS line to "ON" the host tells the connected
 * device that it is ready to receive data. Hardware flow control is not
 * implemented yet. This implementation is just so some application can use
 * the pins, it would also be possible for rs232.c to use it for hardware
 * handshake but as said, that is not implemented yet.
 */

#include "dev/ctsrts-sensor.h"
#include "dev/irq.h"
#include "dev/hwconf.h"

const struct sensors_sensor ctsrts_sensor;

HWCONF_PIN(RS232RTS, 1, 7);

#define RS232CTS_IRQ() 6
HWCONF_PIN(RS232CTS, 1, RS232CTS_IRQ());
HWCONF_IRQ(RS232CTS, 1, RS232CTS_IRQ());

/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  /* Change the flank triggering for the irq so we will detect next shift. */
  if(RS232CTS_READ()) {
    RS232CTS_IRQ_EDGE_SELECTD();
  } else {
    RS232CTS_IRQ_EDGE_SELECTU();
  }
  sensors_changed(&ctsrts_sensor);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /*
   * Invert the bit and return.
   * This is strange, accordingly to the MSP430 manual section 9.2.1, Input
   * Register PxIN the bit should be low when input is low. In RealTerm on
   * the PC I set RTS which is coupled to the CTS on the esb and I read a 0.
   * Maybe RTS is defined active LOW on the PC? //Kalle
   */
  return RS232CTS_READ() ? 0 : 1;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    RS232RTS_SELECT();
    RS232RTS_MAKE_OUTPUT();
    RS232RTS_CLEAR();
    RS232CTS_SELECT();
    RS232CTS_MAKE_INPUT();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!RS232CTS_IRQ_ENABLED()) {

        /*
         * Check current status on the CTS pin and set IRQ flank so we
         * will detect a shift.
         */
        if(RS232CTS_READ()) {
          RS232CTS_IRQ_EDGE_SELECTD();
        } else {
          RS232CTS_IRQ_EDGE_SELECTU();
        }

        irq_port1_activate(RS232CTS_IRQ(), irq);
        RS232CTS_ENABLE_IRQ();
      }
    } else {
      RS232CTS_DISABLE_IRQ();
      irq_port1_deactivate(RS232CTS_IRQ());
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return RS232CTS_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * Indicate to host/client we are NOT ready to receive data. Sets the RTS pin
 * to low.
 */
void ctsrts_rts_clear(void) {
  RS232RTS_CLEAR();
}
/*---------------------------------------------------------------------------*/
/**
 * Request host/client to send data. Sets the RTS pin to high.
 */
void ctsrts_rts_set(void) {
  RS232RTS_SET();
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ctsrts_sensor, CTSRTS_SENSOR,
               value, configure, status);
