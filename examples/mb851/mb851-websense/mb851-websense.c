/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * Copyright (c) 2011, STMicroelectronics.
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
 *         Light and temperature sensor web demo
 */

#include "contiki.h"
#include "httpd-simple.h"
#include "webserver-nogui.h"
#include "dev/temperature-sensor.h"
#include "dev/acc-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

PROCESS(web_sense_process, "Sense Web Demo");

AUTOSTART_PROCESSES(&web_sense_process);

#define HISTORY 11
#define INTERV  5
static int temperature[HISTORY];
static int acc_x[HISTORY], acc_y[HISTORY], acc_z[HISTORY];
static int sensors_pos;

/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Contiki Web Sense</title></head><body>\n";
static const char *BOTTOM_HOME = "<br><br><a href=\"0\">Turn off leds</a><br><a href=\"1\">Turn on leds</a></body></html>\n";
static const char *BOTTOM = "<br><br><a href=\"index\">Home</a></body></html>\n";

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
      "chxr=2,%d,%d|0,0,%u&chds=%d,%d&chxl=1:|Time|3:|%s&chd=t:",
      title, min, max, (HISTORY-1)*INTERV, min, max, unit);
  for(i = 0; i < HISTORY; i++) {
    ADD("%s%d", i > 0 ? "," : "", values[(sensors_pos + i) % HISTORY]);
  }
  ADD("\">");
}
static void
start_acc_chart(const char *title, const char *unit, int min, int max)
{
  blen = 0;
  ADD("<h1>%s</h1>\n"
      "<img src=\"http://chart.apis.google.com/chart?"
      "cht=lc&chs=400x300&chxt=x,x,y,y&chxp=1,50|3,50&"
	  "chco=FF0000,008000,224499&"
      "chdl=X|Y|Z&"
      "chxr=2,%d,%d|0,0,%u&chds=%d,%d&chxl=1:|Time|3:|%s&chd=t:",
      title, min, max,(HISTORY-1)*INTERV, min, max, unit);
}

static void
add_chart_values(int *values)
{
  int i;
  for(i = 0; i < HISTORY; i++) {
      ADD("%s%d", i > 0 ? "," : "", values[(sensors_pos + i) % HISTORY]);
    }
}

static void
end_acc_chart()
{
  ADD("\">");
}

static
PT_THREAD(send_values(struct httpd_state *s))
{

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  if(strncmp(s->filename, "/index", 6) == 0 ||
     s->filename[1] == '\0') {
    /* Default page: show latest sensor values as text (does not
       require Internet connection to Google for charts). */
    blen = 0;
    ADD("<h1>Current readings</h1>\n"
        "<a href=\"a\">Acceleration</a>: (%d, %d, %d) mg<br>"
        "<a href=\"t\">Temperature</a>: %u&deg; C",
        acc_sensor.value(ACC_X_AXIS),
        acc_sensor.value(ACC_Y_AXIS),
        acc_sensor.value(ACC_Z_AXIS),
        temperature_sensor.value(0)/10);
    SEND_STRING(&s->sout, buf);

    SEND_STRING(&s->sout, BOTTOM_HOME);


  } else if(s->filename[1] == '0') {
    /* Turn off leds */
    leds_off(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned off leds!");

    SEND_STRING(&s->sout, BOTTOM);

  } else if(s->filename[1] == '1') {
    /* Turn on leds */
    leds_on(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned on leds!");

    SEND_STRING(&s->sout, BOTTOM);

  } else {
    if(s->filename[1] != 't') {
      start_acc_chart("Acceleration", "mg", -1100, 1100);
      SEND_STRING(&s->sout, buf);
      blen=0;
      add_chart_values(acc_x);
      ADD("|");
      SEND_STRING(&s->sout, buf);
      blen=0;
      add_chart_values(acc_y);
      ADD("|");
      SEND_STRING(&s->sout, buf);
      blen=0;
      add_chart_values(acc_z);
      end_acc_chart();
      SEND_STRING(&s->sout, buf);

    }
    if(s->filename[1] != 'a') {
      generate_chart("Temperature", "Celsius", 10, 50, temperature);
      SEND_STRING(&s->sout, buf);
    }

    SEND_STRING(&s->sout, BOTTOM);
  }

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
  process_start(&webserver_nogui_process, NULL);

  etimer_set(&timer, CLOCK_SECOND * INTERV);
  SENSORS_ACTIVATE(acc_sensor);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    acc_x[sensors_pos] = acc_sensor.value(ACC_X_AXIS);
    acc_y[sensors_pos] = acc_sensor.value(ACC_Y_AXIS);
    acc_z[sensors_pos] = acc_sensor.value(ACC_Z_AXIS);
    temperature[sensors_pos] = temperature_sensor.value(0)/10;
    sensors_pos = (sensors_pos + 1) % HISTORY;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
