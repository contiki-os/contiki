
#ifndef CONTIKI_PWRPIN_H_
#define CONTIKI_PWRPIN_H_

void pwr_pin_init(void); /* Initialize the LEDs driver. */
void pwr_pin_on(void); /* Turn on a set of LEDs. */
void pwr_pin_off(void); /* Turn off a set of LEDs. */

#endif /* CONTIKI_LED_H_ */
