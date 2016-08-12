/* This is a very simple hello_world program.
 * It aims to demonstrate the co-existence of two processes:
 * One of them prints a hello world message and the other blinks the LEDs
 *
 * It is largely based on hello_world in $(CONTIKI)/examples/sensinode
 *
 * Author: George Oikonomou - <oikonomou@users.sourceforge.net>
 */

/**
 * \addtogroup nrf52dk nRF52 Development Kit
 * @{
 *
 * \addtogroup nrf52dk-examples Demo projects for nRF52 DK
 * @{
 *
 * \defgroup nrf52dk-blink-hello Basic sensors and LEDs demo
 * @{
 *
 * This demo demonstrates use of Contiki processes, sensors, and LEDs
 * on nRF52 DK. Pressing a button will start a timer that blinks a
 * respective LED (e.g., button 1 controls LED 1). Each time the button
 * is pressed blinking frequency is doubled. On 4th press the LED is
 * switched off and the sequence can be started from the beginning.
 *
 * \file Main file for Basic sensors and LEDs demo.
 */
#include <stdio.h> /* For printf() */
#include <inttypes.h>
#include "contiki.h"
#include "dev/leds.h"
#include "dev/temperature-sensor.h"
#include "lib/sensors.h"
#include "button-sensor.h"

/*---------------------------------------------------------------------------*/
PROCESS(blink_process_1, "LED1 blink process");
PROCESS(blink_process_2, "LED2 blink process");
PROCESS(blink_process_3, "LED3 blink process");
PROCESS(blink_process_4, "LED4 blink process");
PROCESS(temp, "Temperautre");

AUTOSTART_PROCESSES(
    &blink_process_1,
    &blink_process_2,
    &blink_process_3,
    &blink_process_4,
    &temp
);

struct blink_process_ctx {
  struct etimer et_blink;
  unsigned char c;
  const struct sensors_sensor *button;
  unsigned char led;
};

static void handle_event(process_event_t ev, process_data_t data, struct blink_process_ctx *ctx)
{
  if (ev == PROCESS_EVENT_TIMER && etimer_expired(&ctx->et_blink)) {
    leds_toggle(ctx->led);
    etimer_set(&ctx->et_blink, CLOCK_SECOND / ctx->c);
    printf("Blink %d\n", ctx->led);
  } else if (ev == sensors_event && data == ctx->button) {
    if (ctx->button->value(BUTTON_SENSOR_VALUE_STATE) == 0) {
      if (ctx->c == 0) {
        ctx->c = 1;
      } else if (ctx->c < 8){
        ctx->c <<= 1;
      } else {
        ctx->c = 0;
        leds_off(ctx->led);
      }
      if (ctx->c) {
        etimer_set(&ctx->et_blink, CLOCK_SECOND / ctx->c);
      } else {
        etimer_stop(&ctx->et_blink);
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process_1, ev, data)
{
  static struct blink_process_ctx ctx;

  PROCESS_BEGIN();

  ctx.button = &button_1;
  ctx.c = 0;
  ctx.led = LEDS_1;
  ctx.button->configure(SENSORS_ACTIVE, 1);

  while (1) {
    PROCESS_WAIT_EVENT();
    handle_event(ev, data, &ctx);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process_2, ev, data)
{
  static struct blink_process_ctx ctx;

  PROCESS_BEGIN();

  ctx.button = &button_2;
  ctx.c = 0;
  ctx.led = LEDS_2;
  ctx.button->configure(SENSORS_ACTIVE, 1);

  while (1) {
    PROCESS_WAIT_EVENT();
    handle_event(ev, data, &ctx);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process_3, ev, data)
{
  static struct blink_process_ctx ctx;

  PROCESS_BEGIN();

  ctx.button = &button_3;
  ctx.c = 0;
  ctx.led = LEDS_3;
  ctx.button->configure(SENSORS_ACTIVE, 1);

  while (1) {
    PROCESS_WAIT_EVENT();
    handle_event(ev, data, &ctx);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process_4, ev, data)
{
  static struct blink_process_ctx ctx;

  PROCESS_BEGIN();

  ctx.button = &button_4;
  ctx.c = 0;
  ctx.led = LEDS_4;
  ctx.button->configure(SENSORS_ACTIVE, 1);

  while (1) {
    PROCESS_WAIT_EVENT();
    handle_event(ev, data, &ctx);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(temp, ev, data)
{
  static struct etimer tick;
  PROCESS_BEGIN();

  etimer_set(&tick, CLOCK_SECOND);

  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER && etimer_expired(&tick)) {
      int32_t temp = temperature_sensor.value(0);
      printf("temp: %"PRId32".%02"PRId32"\n", temp >> 2, (temp & 0x03)*25);
      etimer_reset(&tick);
    }
  }

  PROCESS_END();
}
/**
 * @}
 * @}
 * @}
 */
