/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *         Example to demonstrate-test the sensors functionality on
 *         sensinode/cc2430 devices.
 *
 *         B1 turns L2 on and off.
 *         B2 reboots the node via the watchdog.
 *
 *         The node takes readings from the various sensors every x seconds and
 *         prints out the results.
 *
 *         We use floats here to translate the AD conversion results to
 *         meaningful values. However, our printf does not have %f support so
 *         we use an ugly hack to print out the value by extracting the integral
 *         part and then the fractional part. Don't try this at home.
 *
 *         Temperature:
 *           Math is correct, the sensor needs calibration per device.
 *           I currently use default values for the math which may result in
 *           very incorrect values in degrees C.
 *           See TI Design Note DN102 about the offset calibration.
 *
 *         Supply Voltage (VDD) and Battery Sensor:
 *           For VDD, math is correct, conversion is correct. See DN101 for details if
 *           interested.
 *           Battery reports different values when we run it many times
 *           in succession. The cause is unknown.
 *           I am fairly confident that I have captured the connections on the
 *           device correctly. I am however accepting input/feedback
 *
 *         Light Sensor (Vishay Semiconductors TEPT4400):
 *           I am uncertain about the math. This needs testing. All I know is
 *           that 600lux = 0.9V and that the relation is linear. See inline for
 *           more details
 *
 *         Accelerometer (Freescale Semiconductor MMA7340L):
 *           Math is correct but the sensor needs calibration. I've not
 *           attempted one cause the reported values differ per device.
 *           Place the N740 with the logo facing down to get 1g on the Z axis.
 *           Place the antenna side facing down to get 1g on the Y axis
 *           Place the N740 on its longer side while looking at the antenna and
 *           the D connector. Antenna on the bottom, D connector on the top.
 *           This should give you 1g on the X axis.
 *
 *         Make sure you enable/disable things in contiki-conf.h
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "net/rime.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "lib/random.h"

#if CONTIKI_TARGET_SENSINODE
#include "dev/sensinode-sensors.h"
#else
#include "lib/sensors.h"
#endif

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#if CONTIKI_TARGET_SENSINODE
#include "debug.h"
#endif /* CONTIKI_TARGET_SENSINODE */
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
/* We overwrite (read as annihilate) all output functions here */
#define PRINTF(...)
#define putstring(...)
#define putchar(...)
#endif /* DEBUG */


#define SEND_BATTERY_INFO 0
#if SEND_BATTERY_INFO
#include "sensors-example.h"
static void bc_rx(struct broadcast_conn *c, const rimeaddr_t *from) {
  return;
}

static const struct broadcast_callbacks bc_cb = { bc_rx };
static struct broadcast_conn bc_con;
#endif

#if BUTTON_SENSOR_ON
extern const struct sensors_sensor button_1_sensor, button_2_sensor;
#endif

/*---------------------------------------------------------------------------*/
PROCESS(sensors_test_process, "Sensor Test Process");
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
PROCESS(buttons_test_process, "Button Test Process");
AUTOSTART_PROCESSES(&sensors_test_process, &buttons_test_process);
#else
AUTOSTART_PROCESSES(&sensors_test_process);
#endif
/*---------------------------------------------------------------------------*/
#if BUTTON_SENSOR_ON
PROCESS_THREAD(buttons_test_process, ev, data)
{
  struct sensors_sensor *sensor;

  PROCESS_BEGIN();

  while (1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

    /* If we woke up after a sensor event, inform what happened */
    sensor = (struct sensors_sensor *)data;
    if(sensor == &button_1_sensor) {
      leds_toggle(LEDS_GREEN);
    } else if(sensor == &button_2_sensor) {
      watchdog_reboot();
    }
  }

  PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_test_process, ev, data)
{
  static struct etimer et;
#if SEND_BATTERY_INFO
  /* Node Time */
  static struct sensor_data sd;
#endif

  /* Sensor Values */
  static int rv;
  static struct sensors_sensor * sensor;
  static float sane = 0;
  static int dec;
  static float frac;

#if SEND_BATTERY_INFO
  PROCESS_EXITHANDLER(broadcast_close(&bc_con);)
#endif

  PROCESS_BEGIN();

  putstring("========================\n");
  putstring("Starting Sensor Example.\n");
  putstring("========================\n");

#if SEND_BATTERY_INFO
  broadcast_open(&bc_con, BATTERY_RIME_CHANNEL, &bc_cb);
#endif

  /* Set an etimer. We take sensor readings when it expires and reset it. */
  etimer_set(&et, CLOCK_SECOND * 2);

  while (1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /*
     * Request some ADC conversions
     * Return value -1 means sensor not available or turned off in conf
     */
    sensor = sensors_find(ADC_SENSOR);
    if (sensor) {
      putstring("------------------\n");
      leds_on(LEDS_RED);
      /*
       * Temperature:
       * Using 1.25V ref. voltage (1250mV).
       * Typical Voltage at 0°C : 743 mV
       * Typical Co-efficient   : 2.45 mV/°C
       * Offset at 25°C         : 30 (this varies and needs calibration)
       *
       * Thus, at 12bit resolution:
       *
       *     ADC x 1250 / 2047 - (743 + 30)    0.61065 x ADC - 773
       * T = ------------------------------ ~= ------------------- °C
       *                 2.45                         2.45
       */
      rv = sensor->value(ADC_SENSOR_TYPE_TEMP);
      if(rv != -1) {
        sane = ((rv * 0.61065 - 773) / 2.45);
        dec = sane;
        frac = sane - dec;
        PRINTF("  Temp=%d.%02u C (%d)\n", dec, (unsigned int)(frac*100), rv);
      }
      /*
       * Accelerometer: Freescale Semiconductor MMA7340L
       * Using 1.25V ref. voltage.
       * Sensitivity: 0.44 mV/g in ±3g mode.
       *              0.1175 mV/g in ±11g mode.
       * Typical 0g Vout = 1.65V (both modes, Vdd=3.3V, T=25°C)
       * ADC Input Voltage is 1/3 Accelerometer Output Voltage
       *
       * +3g -> 2.97V Acc Out -> 0.9900V ADC Input -> 1621
       * +1g -> 2.09V Acc Out -> 0.6967V ADC Input -> 1141
       *  0g -> 1.65V Acc Out -> 0.5500V ADC Input -> 901
       * -1g -> 1.21V Acc Out -> 0.4033V ADC Input -> 660
       * -3g -> 0.33V Acc Out -> 0.1100V ADC Input -> 180
       *
       * Thus, at 12bit resolution, ±3g mode:
       *        ADC x 1.25 x 3
       * Vout = -------------- V
       *             2047
       *
       *         Vout - 0g    Vout - 1.65
       * Acc  = ----------- = ----------- g
       *        Sensitivity       0.44
       *
       * Similar calc. for ±11g with 0.1175V increments
       *
       * This is only valid if you set ACC_SENSOR_CONF_GSEL 0 in contiki-conf.h
       */
      rv = sensor->value(ADC_SENSOR_TYPE_ACC_X);
      if(rv != -1) {
        sane = ((rv * 3.75 / 2047) - 1.65) / 0.44;
        dec = sane;
        frac = sane - dec;
        frac = (frac < 0) ? -frac : frac;

        /*
         * This will fail for numbers like -0.xyz (since there is no such thing
         * as -0. We manually add a minus sign in the printout if sane is neg
         * and dec is 0.
         * This is the wrong way to do it...
         */
        putstring("  AccX=");
        if(sane < 0 && dec == 0) {
          putchar('-');
        }
        PRINTF("%d.%02ug (%d)\n", dec, (unsigned int)(frac*100), rv);
      }
      rv = sensor->value(ADC_SENSOR_TYPE_ACC_Y);
      if(rv != -1) {
        sane = ((rv * 3.75 / 2047) - 1.65) / 0.44;
        dec = sane;
        frac = sane - dec;
        frac = (frac < 0) ? -frac : frac;
        putstring("  AccY=");
        if(sane < 0 && dec == 0) {
          putchar('-');
        }
        PRINTF("%d.%02ug (%d)\n", dec, (unsigned int)(frac*100), rv);
      }
      rv = sensor->value(ADC_SENSOR_TYPE_ACC_Z);
      if(rv != -1) {
        sane = ((rv * 3.75 / 2047) - 1.65) / 0.44;
        dec = sane;
        frac = sane - dec;
        frac = (frac < 0) ? -frac : frac;
        putstring("  AccZ=");
        if(sane < 0 && dec == 0) {
          putchar('-');
        }
        PRINTF("%d.%02ug (%d)\n", dec, (unsigned int)(frac*100), rv);
      }
      /*
       * Light: Vishay Semiconductors TEPT4400
       * Using 1.25V ref. voltage.
       * For 600 Lux illuminance, the sensor outputs 1mA current (0.9V ADC In)
       * 600 lux = 1mA output => 1473 ADC value at 12 bit resolution)
       *
       * Thus, at 12bit resolution:
       *       600 x 1.25 x ADC
       * Lux = ---------------- ~= ADC * 0.4071
       *          2047 x 0.9
       */
      rv = sensor->value(ADC_SENSOR_TYPE_LIGHT);
      if(rv != -1) {
        sane = (float)(rv * 0.4071);
        dec = sane;
        frac = sane - dec;
        PRINTF(" Light=%d.%02ulux (%d)\n", dec, (unsigned int)(frac*100), rv);
      }
      /*
       * Power Supply Voltage.
       * Using 1.25V ref. voltage.
       * AD Conversion on VDD/3
       *
       * Thus, at 12bit resolution:
       *
       *          ADC x 1.25 x 3
       * Supply = -------------- V
       *               2047
       */
      rv = sensor->value(ADC_SENSOR_TYPE_VDD);
#if SEND_BATTERY_INFO
      sd.vdd = rv;
#endif
      if(rv != -1) {
        sane = rv * 3.75 / 2047;
        dec = sane;
        frac = sane - dec;
        PRINTF("Supply=%d.%02uV (%d)\n", dec, (unsigned int)(frac*100), rv);
        /* Store rv temporarily in dec so we can use it for the battery */
        dec = rv;
      }
      /*
       * Battery Voltage - Only 2/3 of the actual voltage reach the ADC input
       * Using 1.25V ref. voltage would result in 2047 AD conversions all the
       * time since ADC-in would be gt 1.25. We thus use AVDD_SOC as ref.
       *
       * Thus, at 12bit resolution (assuming VDD is 3.3V):
       *
       *           ADC x 3.3 x 3   ADC x 4.95
       * Battery = ------------- = ---------- V
       *             2047 x 2         2047
       *
       * Replacing the 3.3V with an ADC reading of the actual VDD would yield
       * better accuracy. See monitor-node.c for an example.
       *
       *           3 x ADC x VDD x 3.75   ADC x VDD x 11.25
       * Battery = -------------------- = ----------------- V
       *             2 x 2047 x 2047           0x7FE002
       *
       */
      rv = sensor->value(ADC_SENSOR_TYPE_BATTERY);
      if(rv != -1) {
        /* Instead of hard-coding 3.3 here, use the latest VDD (stored in dec)
         * (slightly inaccurate still, but better than crude 3.3) */
        sane = (11.25 * rv * dec) / (0x7FE002);
        dec = sane;
        frac = sane - dec;
        PRINTF(" Batt.=%d.%02uV (%d)\n", dec, (unsigned int)(frac*100), rv);
#if SEND_BATTERY_INFO
        sd.bat = rv;
        packetbuf_copyfrom(&sd, sizeof(sd));
        broadcast_send(&bc_con);
#endif
      }
      leds_off(LEDS_RED);
    }
    etimer_reset(&et);
  }
  PROCESS_END();
 }
/*---------------------------------------------------------------------------*/
