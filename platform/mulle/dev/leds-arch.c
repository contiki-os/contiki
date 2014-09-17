#include "K60.h"

#include "contiki-conf.h"
#include "dev/leds.h"
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;      /* Enable PORTC clock gate */
  PORTC->PCR[13] |= 0x00100;                 /* Setup PTC13 as GPIO */
  PORTC->PCR[14] |= 0x00100;                 /* Setup PTC14 as GPIO */
  PORTC->PCR[15] |= 0x00100;                 /* Setup PTC15 as GPIO */
  PTC->PDDR |= 0x0E000;                  /* Setup PTA14-PTA17 as outputs */
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return ((PTC->PDIR & LEDS_CONF_RED) ? LEDS_RED : 0)
         | ((PTC->PDIR & LEDS_CONF_GREEN) ? LEDS_GREEN : 0)
         | ((PTC->PDIR & LEDS_CONF_YELLOW) ? LEDS_YELLOW : 0);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  PTC->PDOR = (PTC->PDOR & ~(LEDS_CONF_RED | LEDS_CONF_GREEN | LEDS_CONF_YELLOW))
    | ((leds & LEDS_RED) ? LEDS_CONF_RED : 0)
    | ((leds & LEDS_GREEN) ? LEDS_CONF_GREEN : 0)
    | ((leds & LEDS_YELLOW) ? LEDS_CONF_YELLOW : 0);
}
/*---------------------------------------------------------------------------*/
