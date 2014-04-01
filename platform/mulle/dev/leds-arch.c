#include "K60.h"

#include "contiki-conf.h"
#include "dev/leds.h"
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  SIM_SCGC5   |= SIM_SCGC5_PORTC_MASK;    /* Enable PORTC clock gate */
  PORTC_PCR13 |= 0x00100;                 /* Setup PTC13 as GPIO */
  PORTC_PCR14 |= 0x00100;                 /* Setup PTC14 as GPIO */
  PORTC_PCR15 |= 0x00100;                 /* Setup PTC15 as GPIO */
  GPIOC_PDDR  |= 0x0E000;                 /* Setup PTA14-PTA17 as outputs */

}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return ((GPIOC_PDIR & LEDS_CONF_RED) ? LEDS_RED : 0)
    | ((GPIOC_PDIR & LEDS_CONF_GREEN) ? LEDS_GREEN : 0)
    | ((GPIOC_PDIR & LEDS_CONF_YELLOW) ? LEDS_YELLOW : 0);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  GPIOC_PDOR = (GPIOC_PDOR & ~(LEDS_CONF_RED|LEDS_CONF_GREEN|LEDS_CONF_YELLOW))
    | ((leds & LEDS_RED) ? LEDS_CONF_RED : 0)
    | ((leds & LEDS_GREEN) ? LEDS_CONF_GREEN : 0)
    | ((leds & LEDS_YELLOW) ? LEDS_CONF_YELLOW : 0);
}
/*---------------------------------------------------------------------------*/
