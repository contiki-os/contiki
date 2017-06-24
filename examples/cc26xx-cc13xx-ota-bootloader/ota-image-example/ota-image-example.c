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
#include "dev/leds.h"

#include "ext-flash.h"
#include "ota-download.h"

#if PLATFORM_HAS_LEDS
#define BLINKER_PIN BOARD_IOID_LED_1
#else
#define OTA_EXAMPLE_UNIQUE_ID   1
#endif

PROCESS(blinker_test_loop, "GPIO Blinker Lifecycle");
AUTOSTART_PROCESSES(&blinker_test_loop);

struct ctimer blink_timer;
bool blink_state = false;

void
blink_looper()
{
#if PLATFORM_HAS_LEDS
  if(blink_state){
    leds_off(BLINKER_PIN);
  }
  else {
    leds_on(BLINKER_PIN);
  }
#else
  printf("Platform does not support LED. Unique ID: %u", OTA_EXAMPLE_UNIQUE_ID);
#endif

  blink_state = !blink_state;
  ctimer_reset( &blink_timer );
}


PROCESS_THREAD(blinker_test_loop, ev, data)
{
  PROCESS_BEGIN();

  //	(1)	UART Output
  printf("OTA Image Example: Starting\n");
#if PLATFORM_HAS_LEDS
  //	(2)	Start blinking green LED
  leds_init();
  leds_on(BLINKER_PIN);
#else
  printf("Platform does not support LED. Unique ID: %u\n", OTA_EXAMPLE_UNIQUE_ID);
#endif

  ctimer_set( &blink_timer, (CLOCK_SECOND/2), blink_looper, NULL);

  //  (3) Get metadata about the current firmware version
  OTAMetadata_t current_firmware;
  get_current_metadata( &current_firmware );
  printf("\nCurrent Firmware\n");
  print_metadata( &current_firmware );

  ext_flash_init();

  int ota_slot;
  OTAMetadata_t ota_metadata;

  printf("\nNewest Firmware:\n");
  ota_slot = find_newest_ota_image();
  while( get_ota_slot_metadata( ota_slot, &ota_metadata ) );
  print_metadata( &ota_metadata );

  printf("\nOldest Firmware:\n");
  ota_slot = find_oldest_ota_image();
  while( get_ota_slot_metadata( ota_slot, &ota_metadata ) );
  print_metadata( &ota_metadata );

  int empty_slot = find_empty_ota_slot();
  printf("\nEmpty OTA slot: #%u\n", empty_slot);

  //  (4) OTA Download!
  process_start(ota_download_th_p, NULL);

  PROCESS_END();
}
