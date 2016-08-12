/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * Copyright (c) 2015, University of Bristol - http://www.bristol.ac.uk
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-button-sensor Zoul User Button Driver
 *
 * Driver for the Zoul user button
 *
 * The user button (on Zoul-based platforms like the RE-Mote and the Firefly)
 *  will generate a sensors_changed event on press as well as on release.
 *
 * Unlike many other platforms, the user button has the ability to
 * generate events when the user keeps the button pressed. The user can
 * configure the button driver with a timer interval in clock ticks. When the
 * button is kept pressed, the driver will then generate a broadcast event
 * each time the interval passes. For example the driver can be configured to
 * generate an event every second while the button is kept pressed. This
 * functionality can be enabled through the configure() function, by passing
 * BUTTON_SENSOR_CONFIG_TYPE_INTERVAL as the type argument.
 * @{
 *
 * \file
 * Header file for the Zoul User Button Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef BUTTON_SENSOR_H_
#define BUTTON_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR "Button"

extern const struct sensors_sensor button_sensor;
/*---------------------------------------------------------------------------*/
extern process_event_t button_press_duration_exceeded;
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR_CONFIG_TYPE_INTERVAL      0x0100

#define BUTTON_SENSOR_VALUE_TYPE_LEVEL          0
#define BUTTON_SENSOR_VALUE_TYPE_PRESS_DURATION 1

#define BUTTON_SENSOR_PRESSED_LEVEL             0
#define BUTTON_SENSOR_RELEASED_LEVEL            8
/*---------------------------------------------------------------------------*/
#endif /* BUTTON_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
