#include <avr/pgmspace.h>
#include "rss2.h"

void pwr_pin_off();

void
pwr_pin_init(void)
{
	DDRE |= (1 << PWR_1); //init power pin (uplink switch)
  /* Off */
pwr_pin_off();
}
void
pwr_pin_on()
{
  PORTE &= ~ (1 << PWR_1); //high
}
void
pwr_pin_off()
{
	PORTE |= (1 << PWR_1); //and make sure its off -- pulled high
}