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
 * handshake but that is not implemented yet.
 */
#ifndef __CTSRTS_SENSOR_H__
#define __CTSRTS_SENSOR_H__

#include "lib/sensors.h"

extern const struct sensors_sensor ctsrts_sensor;

#define CTSRTS_SENSOR "CTSRTS"

/**
 * Indicate to host/client we are NOT ready to receive data. Sets the RTS pin
 * to low.
 */
void ctsrts_rts_clear(void);

/**
 * Request host/client to send data. Sets the RTS pin to high.
 */
void ctsrts_rts_set(void);

#endif /* __CTSRTS_SENSOR_H__ */
