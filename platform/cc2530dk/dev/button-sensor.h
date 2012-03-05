/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 * \file
 *         Override core/dev/button-sensor.h
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef __BUTTON_SENSOR_H__
#define __BUTTON_SENSOR_H__

#include "contiki-conf.h"
#include "lib/sensors.h"

#define BUTTON_PORT 0
#define BUTTON_PIN  1

#define BUTTON_SENSOR "Button"

/*
 * SmartRF Buttons
 * B1: P0_1, B2: Not Connected
 */
#ifdef BUTTON_SENSOR_CONF_ON
#define BUTTON_SENSOR_ON BUTTON_SENSOR_CONF_ON
#endif /* BUTTON_SENSOR_CONF_ON */

#if BUTTON_SENSOR_ON
extern const struct sensors_sensor button_sensor;
/* Button 1: P0_1 - Port 0 ISR needed */
void port_0_isr(void) __interrupt(P0INT_VECTOR);
#define   BUTTON_SENSOR_ACTIVATE() button_sensor.configure(SENSORS_ACTIVE, 1)
#else
#define   BUTTON_SENSOR_ACTIVATE()
#endif /* BUTTON_SENSOR_ON */

/* Define macros for button 1 */
#define BUTTON_READ()           PORT_READ(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_FUNC_GPIO()      PORT_FUNC_GPIO(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_DIR_INPUT()      PORT_DIR_INPUT(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_ENABLED()    PORT_IRQ_ENABLED(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_CHECK()      PORT_IRQ_CHECK(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_ENABLE()     PORT_IRQ_ENABLE(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_DISABLE()    PORT_IRQ_DISABLE(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_FLAG_OFF()   PORT_IRQ_FLAG_OFF(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_ON_PRESS()   PORT_IRQ_EDGE_RISE(BUTTON_PORT, BUTTON_PIN)
#define BUTTON_IRQ_ON_RELEASE() PORT_IRQ_EDGE_FALL(BUTTON_PORT, BUTTON_PIN)

#endif /* __BUTTON_SENSOR_H__ */
