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

#define GPIO_CONFIG(PIN, CFG, GPIO_DIR_MODE)  ti_lib_ioc_port_configure_set(PIN, IOC_PORT_GPIO, CFG); \
																							ti_lib_gpio_dir_mode_set((1 << PIN), GPIO_DIR_MODE)


#define BLINKER_CFG         (IOC_CURRENT_2MA  | IOC_STRENGTH_AUTO | \
                            IOC_IOPULL_DOWN    | IOC_SLEW_ENABLE  | \
                            IOC_HYST_DISABLE | IOC_FALLING_EDGE    | \
                            IOC_INT_DISABLE   | IOC_IOMODE_NORMAL | \
                            IOC_NO_WAKE_UP   | IOC_INPUT_DISABLE)

#define BLINKER_PIN	IOID_25

struct ctimer blink_timer;
bool blink_state = false;

void
blink_looper()
{
  GPIOPinWrite( (1<<BLINKER_PIN), blink_state );
  blink_state = !blink_state;
  ctimer_reset( &blink_timer );
}

PROCESS(blinker_test_loop, "GPIO Blinker Lifecycle");
AUTOSTART_PROCESSES(&blinker_test_loop);

PROCESS_THREAD(blinker_test_loop, ev, data)
{
  PROCESS_BEGIN();

	printf("GPIO Blinker Test: Starting\n");

	GPIODirModeSet(GPIO_PIN_27, GPIO_DIR_MODE_OUT);
  GPIOPinWrite(GPIO_PIN_27, 1);

	GPIO_CONFIG( BLINKER_PIN, BLINKER_CFG, GPIO_DIR_MODE_OUT );
  ctimer_set( &blink_timer, (CLOCK_SECOND/2), blink_looper, NULL);

  PROCESS_END();
}
