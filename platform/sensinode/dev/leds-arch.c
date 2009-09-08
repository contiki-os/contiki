#include "contiki-conf.h"
#include "dev/models.h"
#include "dev/leds.h"

#include "cc2430_sfr.h"

/*
 * Sensinode v1.0 HW products have 2 red LEDs, LED1 is mapped to the Contiki
 * LEDS_RED and LED2 is mapped to LEDS_GREEN.
 */

/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  P0DIR |= 0x30;
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  unsigned char l = 0;

  if(LED1_PIN) {
    l |= LEDS_RED;
  }
  if(LED2_PIN) {
    l |= LEDS_GREEN;
  }
  return l;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_RED) {
    LED1_PIN = 1;
  } else {
    LED1_PIN = 0;
  }

  if(leds & LEDS_GREEN) {
    LED2_PIN = 1;
  } else {
    LED2_PIN = 0;
  }

}
/*---------------------------------------------------------------------------*/
