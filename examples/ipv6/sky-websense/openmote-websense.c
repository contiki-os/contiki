/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

/**
 * \file
 *         Light and temperatur sensor web demo
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 */

#include "contiki.h"
#include "httpd-simple.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/sht21.h"
#include "dev/max44009.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include <stdio.h> 
PROCESS(web_sense_process, "Sense Web Demo");
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
AUTOSTART_PROCESSES(&web_sense_process,&webserver_nogui_process);

#define HISTORY 16
static int temperature[HISTORY];
static int humidity[HISTORY];
static int light[HISTORY];
static int sensors_pos;
static int  temperature1, humidity1, light1;
static uint16_t  sht21_present, max44009_present;
/*---------------------------------------------------------------------------*/

static int
get_temp(void)
{sht21_present = SENSORS_ACTIVATE(sht21);
  temperature1 = sht21.value(SHT21_READ_TEMP)/100;
  return (temperature1);
}
static int
get_hum(void)
{sht21_present = SENSORS_ACTIVATE(sht21);
   humidity1 = (sht21.value(SHT21_READ_RHUM)/100)%100;
  return (humidity1);
}
static int
get_light(void)
{  max44009_present = SENSORS_ACTIVATE(max44009);
 light1 = (max44009.value(MAX44009_READ_LIGHT)/100)%100;
  return (light1);
}
/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>openmote-cc2538 sensors </title> <head><body> \n";
static const char *TOP1 = "<h4> <font color =\"green\"> Mesures en temps reel </font> </h4>  \n";


 static const char *BOTTOM = " </body>  </html>\n";
/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)
static void
generate_chart(const char *title, const char *unit, int min, int max, int *values)
{
  int i;
  blen = 0;
  ADD("<h1>%s</h1>\n"
      "<img src=\"http://chart.apis.google.com/chart?"
      "cht=lc&chs=400x300&chxt=x,x,y,y&chxp=1,50|3,50&"
      "chxr=2,%d,%d|0,0,30&chds=%d,%d&chxl=1:|Time|3:|%s&chd=t:",
      title, min, max, min, max, unit);
  for(i = 0; i < HISTORY; i++) {
    ADD("%s%d", i > 0 ? "," : "", values[(sensors_pos + i) % HISTORY]);
  }
  ADD("\">");
}
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);
SEND_STRING(&s->sout, TOP1);
//SEND_STRING(&s->sout, TOP2);

  if(strncmp(s->filename, "/index", 6) == 0 ||
     s->filename[1] == '\0') {
    /* Default page: show latest sensor values as text (does not
       require Internet connection to Google for charts). */
    blen = 0;
    ADD(
        " capteur SHT21:  <br> \n"
        " Temperature: %u&deg; C  <br> \n\
          Humidite: %u&#37    <br> \n "
        " capteur MAX44009: <br> \n\
          lumiere: %u </br> \n  ",
         get_temp(), get_hum(), get_light());
    SEND_STRING(&s->sout, buf);

  } else if(s->filename[1] == '0') {
    /* Turn off leds */
    leds_off(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned off leds!");

  } else if(s->filename[1] == '1') {
    /* Turn on leds */
    leds_on(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned on leds!");

  } else {
    
    if(s->filename[1] != 'h') {
      generate_chart("Temperature", "Celsius", 15, 50, temperature);
      SEND_STRING(&s->sout, buf);
    }
 if(s->filename[1] != 'l') {
      generate_chart("humidity", "humidity", 15, 50, humidity);
      SEND_STRING(&s->sout, buf);
    }  
if(s->filename[1] != 't') {
      generate_chart("Light", "Light", 0, 500, light);
      SEND_STRING(&s->sout, buf);
    }
}
 
  SEND_STRING(&s->sout, BOTTOM);


  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return send_values;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  sensors_pos = 0;

  etimer_set(&timer, CLOCK_SECOND * 2);
  sht21_present = SENSORS_ACTIVATE(sht21);
  max44009_present = SENSORS_ACTIVATE(max44009);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

 
    temperature[sensors_pos] = get_temp();
    humidity[sensors_pos] =get_hum();
    light[sensors_pos] = get_light();
    sensors_pos = (sensors_pos + 1) % HISTORY;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
