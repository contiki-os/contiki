#include <dev/leds.h>
#include <AT91SAM7S64.h>

#define GREEN_PIN AT91C_PIO_PA3
#define YELLOW_PIN AT91C_PIO_PA4
#define RED_PIN AT91C_PIO_PA8

#define ALL_PINS (GREEN_PIN | YELLOW_PIN | RED_PIN)

void
leds_arch_init(void)
{
  *AT91C_PIOA_PER = ALL_PINS;
  *AT91C_PIOA_OER = ALL_PINS;
  *AT91C_PIOA_MDER = ALL_PINS;
  *AT91C_PIOA_SODR = ALL_PINS;
}

unsigned char
leds_arch_get(void)
{
  unsigned char on = 0;
  if (*AT91C_PIOA_ODSR & GREEN_PIN) on |= LEDS_GREEN;
  if (*AT91C_PIOA_ODSR & YELLOW_PIN) on |= LEDS_YELLOW;
  if (*AT91C_PIOA_ODSR & RED_PIN) on |= LEDS_RED;
  return on;
}

void
leds_arch_set(unsigned char leds)
{
  unsigned int set = 0;
  if (leds & LEDS_GREEN) set |= GREEN_PIN;
  if (leds & LEDS_YELLOW) set |= YELLOW_PIN;
  if (leds & LEDS_RED) set |= RED_PIN;
  *AT91C_PIOA_CODR = set;
  *AT91C_PIOA_SODR = (~set) & ALL_PINS;
}
