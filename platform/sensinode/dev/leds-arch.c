#include "contiki-conf.h"
#include "dev/models.h"
#include "dev/leds.h"

#include "cc2430_sfr.h"

/*
 * Sensinode v1.0 HW products have 2 red LEDs, LED1 is mapped to the Contiki
 * LEDS_GREEN and LED2 is mapped to LEDS_RED.
 */

/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
#ifdef MODEL_N740
  /*
   * We don't need explicit led initialisation for N740s. They are controlled
   * by the ser/par chip which is initalised already
   */
  return;
#else
  P0DIR |= 0x30;
#endif
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  unsigned char l = 0;

#ifdef MODEL_N740
  /* Read the current ser-par chip status */
  uint8_t ser_par;
  ser_par = n740_ser_par_get();
  /* Check bits 7 & 8, ignore the rest */
  if(ser_par & N740_SER_PAR_LED_GREEN) {
    l |= LEDS_GREEN;
  }
  if(ser_par & N740_SER_PAR_LED_RED) {
    l |= LEDS_RED;
  }
#else
  if(LED1_PIN) {
    l |= LEDS_GREEN;
  }
  if(LED2_PIN) {
    l |= LEDS_RED;
  }
#endif
  return l;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
#ifdef MODEL_N740
  /* Read the current ser-par chip status - we want to change bits 7 & 8 but
   * the remaining bit values should be retained */
  uint8_t ser_par;
  ser_par = n740_ser_par_get();
  if(leds & LEDS_GREEN) {
    ser_par |= N740_SER_PAR_LED_GREEN; /* Set bit 7 */
  } else {
    ser_par &= ~N740_SER_PAR_LED_GREEN; /* Unset bit 7 */
  }

  if(leds & LEDS_RED) {
    ser_par |= N740_SER_PAR_LED_RED; /* Set bit 8 */
  } else {
    ser_par &= ~N740_SER_PAR_LED_RED; /* Unset bit 8 */
  }

  /* Write the new status back to the chip */
  n740_ser_par_set(ser_par);
#else
  if(leds & LEDS_GREEN) {
    LED1_PIN = 1;
  } else {
    LED1_PIN = 0;
  }

  if(leds & LEDS_RED) {
    LED2_PIN = 1;
  } else {
    LED2_PIN = 0;
  }
#endif
}
/*---------------------------------------------------------------------------*/
