/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 * Header file for the reed sensor
 *
 * The Reed sensor allows to be used either by polling the sensor status or by
 * setting up a timer on the background ticking every REED_CHECK_PERIOD, posting
 * a reed_sensor_event_changed event, informing the application about a change
 * in the sensor status (basically open or closed).  To enable each mode
 * (default is polling) call the configure() function with REED_SENSOR_MODE
 * using REED_SENSOR_EVENT_MODE or REED_SENSOR_POLL_MODE, after having
 * initialized the device using SENSORS_ACTIVATE(reed_sensor).
 *
 * \file
 *         Reed sensor header file
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
#include "lib/sensors.h"

#ifndef REED_SENSOR_H_
#define REED_SENSOR_H_
/* -------------------------------------------------------------------------- */
#define REED_SENSOR_ERROR       -1
#define REED_SENSOR_SUCCESS     0x00
#define REED_SENSOR_VAL         0x01
/* -------------------------------------------------------------------------- */
#define REED_OPEN               0x00
#define REED_CLOSED             0x01
/* -------------------------------------------------------------------------- */
#define REED_SENSOR_MODE        0x01
#define REED_SENSOR_EVENT_MODE  0x0A
#define REED_SENSOR_EVENT_POLL  0x0B
/* -------------------------------------------------------------------------- */
#define REED_PORT_DIR           P4DIR
#define REED_PORT_SEL           P4SEL
#define REED_PORT_REN           P4REN
#define REED_PORT_READ          P4IN
#define REED_PORT_PRES          P4OUT
#define REED_READ_PIN           (1 << 2)
/* -------------------------------------------------------------------------- */
#define REED_SENSOR             "Reed Sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor reed_sensor;
extern process_event_t reed_sensor_event_changed;
/* -------------------------------------------------------------------------- */
#endif /* ifndef REED_SENSOR_H_ */
