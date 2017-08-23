
#ifndef CONTIKI_LED_H_
#define CONTIKI_LED_H_

#define LEDS_GREEN                    1
#define LEDS_YELLOW                   2
#define LEDS_RED                      4
#define LEDS_ALL                      7

void leds_init(void); /* Initialize the LEDs driver. */
unsigned char leds_get(void); /* Get the status of a LED. */
void leds_on(unsigned char ledv); /* Turn on a set of LEDs. */
void leds_off(unsigned char ledv); /* Turn off a set of LEDs. */
void leds_toggle(unsigned char ledv); /* Toggle a set of LEDs. */
void leds_invert(unsigned char ledv); /* Toggle a set of LEDs. */

#endif /* CONTIKI_LED_H_ */
