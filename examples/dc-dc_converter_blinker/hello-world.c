/**
 * DCDC-converter example for blinking the two LED's in different frequency
 */

#include "contiki.h"
#include "dev/leds.h"

/* Process for blinking GREEN led */
PROCESS(green_led_blink_process, "Process for blinking GREEN led");
PROCESS_THREAD(green_led_blink_process, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND/2);

  while (1)
    {
      PROCESS_WAIT_EVENT();
      if(etimer_expired(&et)){
          leds_toggle(LEDS_GREEN);
          etimer_reset(&et);
      }
    }

PROCESS_END();
}

/* Process for blinking RED led */
PROCESS(red_led_blink_process, "Process for blinking RED led");
PROCESS_THREAD(red_led_blink_process, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;

  leds_init();
  etimer_set(&et, CLOCK_SECOND);

  while (1)
    {
      PROCESS_WAIT_EVENT();
      if(etimer_expired(&et)){
          leds_toggle(LEDS_RED);
          etimer_reset(&et);
      }
    }

PROCESS_END();
}

AUTOSTART_PROCESSES(&red_led_blink_process, &green_led_blink_process);
