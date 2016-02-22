/* Copyright Robert Olsson */

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <dev/watchdog.h>
#include "params.h"
#include "rss2.h"
#include "contiki.h"
#include "adc.h"
#include "rss2.h"

uint16_t
adc_read(uint8_t pin)
{
  uint16_t volt = 0;
  int i;

  ADMUX = pin; /* ADC pin PA0-PA7 ->  0-7 */
  ADCSRB &= ~(1 << MUX5); /* Needs to write before ADMUX pp 418 */
  ADMUX |= (1 << REFS1) | (1 << REFS0); /* 1.6 */

  /* Maximal Track and Hold time */

  ADCSRA = (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);  /* Prescaler /32 */
  ADCSRA |= (1 << ADEN);  /* Enable the ADC */

  while(!(ADCSRB & (1 << AVDDOK))) ;  /* Wait for AVDD ok */
  while(!(ADCSRB & (1 << REFOK))) ;  /* Wait for ref ok  */

  /* Ignore first result */

  ADCSRA |= (1 << ADSC);
  while(ADCSRA & _BV(ADSC)) {
  }

  ADCW = 0;
#define RES 4

  for(i = 0; i < RES; i++) {

    /* Start the conversion */
    ADCSRA |= (1 << ADSC);

    /* Wait for conversion */
    while(ADCSRA & (1 << ADSC)) {
    }

    volt += ADCW;
  }
  ADMUX = 0;                /* turn off internal vref */
  volt = volt / RES;

  /* Disable the ADC to save power */
  ADCSRA &= ~_BV(ADEN);

  return volt;
}
double
adc_read_v_in(void)
{
  /* Special treatment for v_in. Schottky voltage drop add */
  return ((double)adc_read(AV_IN)) * V_IN_FACTOR_SCHOTTKY + SCHOTTKY_DROP;
}
double
adc_read_a1(void)
{
  return ((double)adc_read(A1)) * V_IN_FACTOR;
}
double
adc_read_a2(void)
{
  return ((double)adc_read(A2)) * V_IN_FACTOR;
}
