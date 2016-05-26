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

#include "contiki-net.h"
#include "http-socket.h"
#include "ip64-addr.h"

#include "ota.h"

#define BLINKER_PIN	BOARD_LED_1

PROCESS(blinker_test_loop, "GPIO Blinker Lifecycle");
AUTOSTART_PROCESSES(&blinker_test_loop);

struct ctimer blink_timer;
bool blink_state = false;

void
blink_looper()
{
  GPIOPinWrite( BLINKER_PIN, blink_state );
  blink_state = !blink_state;
  ctimer_reset( &blink_timer );
}

OTAMetadata_t current_firmware;

static struct http_socket s;
static int bytes_received = 0;
static void
callback(struct http_socket *s, void *ptr,
         http_socket_event_t e,
         const uint8_t *data, uint16_t datalen)
{
  if(e == HTTP_SOCKET_ERR) {
    printf("HTTP socket error\n");
  } else if(e == HTTP_SOCKET_TIMEDOUT) {
    printf("HTTP socket error: timed out\n");
  } else if(e == HTTP_SOCKET_ABORTED) {
    printf("HTTP socket error: aborted\n");
  } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
    printf("HTTP socket error: hostname not found\n");
  } else if(e == HTTP_SOCKET_CLOSED) {
    printf("HTTP socket closed, %d bytes received\n", bytes_received);
  } else if(e == HTTP_SOCKET_DATA) {
    bytes_received += datalen;
    printf("HTTP socket received %d bytes of data\n", datalen);
  }
}

struct ctimer http_timer;

void
http_looper() {
  http_socket_get(&s, "http://[bbbb::1]:3003/", 0, 0, callback, NULL);
  ctimer_reset( &http_timer );
}

PROCESS_THREAD(blinker_test_loop, ev, data)
{
  PROCESS_BEGIN();

  //	(1)	UART Output
  printf("OTA Image Example: Starting\n");

  //	(2)	Start blinking green LED
	GPIODirModeSet( BLINKER_PIN, GPIO_DIR_MODE_OUT);
  ctimer_set( &blink_timer, (CLOCK_SECOND/2), blink_looper, NULL);

  //  (3) HTTP testing!
  uip_ip6addr_t ip6addr;
  uip_ip6addr(&ip6addr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
  http_socket_init(&s);
  ctimer_set(&http_timer, (CLOCK_SECOND * 5), http_looper, NULL);



  //  (1) Get metadata about the current firmware version
  FlashRead( (uint8_t *)&current_firmware, (CURRENT_FIRMWARE<<12), OTA_METADATA_LENGTH );
  printf("\nCurrent Firmware\n");
  print_metadata( &current_firmware );

  ext_flash_init();



  //generate_fake_metadata();

  //  (2) Get metadata about OTA firmwares stored in the external flash.
  for (int img=0; img<sizeof(ota_images); img++) {
    OTAMetadata_t ota_firmare;

    /* Make sure the external flash is in the lower power mode */
    int eeprom_access = ext_flash_open();

    if(!eeprom_access) {
      printf("[external-flash]:\tError - Could not access EEPROM.\n");
      ext_flash_close();
      return false;
    }

    eeprom_access = ext_flash_read((ota_images[img] << 12), OTA_METADATA_LENGTH, (uint8_t *)&ota_firmare);

    ext_flash_close();

    if(!eeprom_access) {
      printf("[external-flash]:\tError - Could not read EEPROM.\n");
      ext_flash_close();
    }

    printf("\nOTA Firmware Slot %u\n", img);
    print_metadata( &ota_firmare );
  }

  PROCESS_END();
}
