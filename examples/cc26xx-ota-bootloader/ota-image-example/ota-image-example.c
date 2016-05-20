/**
 *    Example Contiki 3.0 Firmware
 *    Blinks the LEDs @ 1Hz.

 *    If we see the LED blinking, we know
 *    the bootloader loaded our code!
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "ti-lib.h"
#include "gpio.h"
#include "sys/ctimer.h"
#include "sys/timer.h"

#define BLINKER_PIN	GPIO_PIN_10

struct ctimer blink_timer;
bool blink_state = false;

void
blink_looper()
{
  GPIOPinWrite( BLINKER_PIN, blink_state );
  blink_state = !blink_state;
  ctimer_reset( &blink_timer );
}

PROCESS(blinker_test_loop, "GPIO Blinker Lifecycle");
AUTOSTART_PROCESSES(&blinker_test_loop);

PROCESS_THREAD(blinker_test_loop, ev, data)
{
  PROCESS_BEGIN();

  //	(1)	UART Output
  printf("OTA Image Example: Starting\n");

  //	(2)	Start blinking green LED
	GPIODirModeSet( BLINKER_PIN, GPIO_DIR_MODE_OUT);
  ctimer_set( &blink_timer, (CLOCK_SECOND/2), blink_looper, NULL);

  PROCESS_END();
}
