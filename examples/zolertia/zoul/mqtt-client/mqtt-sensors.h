/*
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
#ifndef MQTT_SENSORS_H_
#define MQTT_SENSORS_H_
/*---------------------------------------------------------------------------*/
#define SENSOR_NAME_STRING  20

typedef struct sensor_val {
  int16_t value;                            /* Sensor value */
  int16_t threshold;                        /* Threshold (over) */
  int16_t min;                              /* Minimum allowed value */
  int16_t max;                              /* Maximum allowed value */
  uint8_t pres;                             /* Number of precision digits */
  char sensor_name[SENSOR_NAME_STRING];     /* Sensor name (string) */
  char alarm_name[SENSOR_NAME_STRING];      /* Alarm name (string) */
  char sensor_config[SENSOR_NAME_STRING];   /* Configuration name (string) */
} sensor_val_t;

typedef struct sensors_values {
  uint8_t num;                              /* Number of sensors */
  sensor_val_t sensor[DEFAULT_SENSORS_NUM]; /* Array of sensor_val_t */
} sensor_values_t;
/*---------------------------------------------------------------------------*/
/* Auxiliary function to fill the sensor_values_t structure */
int mqtt_sensor_register(sensor_values_t *reg, uint8_t i, uint16_t val,
                         char *name, char *alarm, char *config, uint16_t min,
                         uint16_t max, uint16_t thres, uint16_t pres);

/* Auxiliary function to check the sensor values and send alarms or publish
 * periodic sensor events */
void mqtt_sensor_check(sensor_values_t *reg, process_event_t alarm,
                       process_event_t data);
/*---------------------------------------------------------------------------*/
#endif /* MQTT_SENSORS_H_ */
/** @} */

