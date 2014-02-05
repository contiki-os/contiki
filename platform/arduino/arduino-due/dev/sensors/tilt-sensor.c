/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "platform-conf.h"
#include "tilt-sensor.h"
#include "wire_digital.h"

#ifdef TILT_SENSOR_CONF_PIN_NUMBER
#define TILT_SENSOR_PIN_NUMBER TILT_SENSOR_CONF_PIN_NUMBER
#else
#define TILT_SENSOR_PIN_NUMBER	17 // Default PIN for the tilt sensor
#endif
/*---------------------------------------------------------------------------*/
static void tilt_handler() {
	
	printf("Tilt\n");
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
	/* Configure tilt sensor. */
	configure_input_pin_interrupt_enable(TILT_SENSOR_PIN_NUMBER,PIO_PULLUP,PIO_IT_RISE_EDGE,tilt_handler);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
	/* Tilt sensor returns no type. */
	return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
	switch(type){
		case SENSORS_HW_INIT:
			init();
			return 1;
		case SENSORS_ACTIVE:
			return 1;
	}

	return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
	switch(type) {
		
		case SENSORS_READY:
		return 1;
	}
	
	return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(tilt_sensor, TILT_SENSOR,
value, configure, status);