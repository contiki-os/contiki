/**
 * \file
 *         CC2430 RF driver
 * \author
 *         Zach Shelby <zach@sensinode.com> (Original)
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *           (recent updates for the contiki cc2430 port)
 *
 *  Non-bankable code for cc2430 rf driver.  
 *  Interrupt routines must be placed into the HOME bank.
 *
 */

#include <stdio.h>

#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc2430_rf.h"
#include "cc2430_sfr.h"
#ifdef RF_LED_ENABLE
#include "dev/leds.h"
#endif
#include "sys/clock.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#ifdef RF_LED_ENABLE
#define RF_RX_LED_ON()		leds_on(LEDS_RED);
#define RF_RX_LED_OFF()		leds_off(LEDS_RED);
#define RF_TX_LED_ON()		leds_on(LEDS_GREEN);
#define RF_TX_LED_OFF()		leds_off(LEDS_GREEN);
#else
#define RF_RX_LED_ON()
#define RF_RX_LED_OFF()
#define RF_TX_LED_ON()
#define RF_TX_LED_OFF()
#endif

#ifdef HAVE_RF_ERROR
uint8_t rf_error = 0;
#endif

PROCESS_NAME(cc2430_rf_process);

#if !NETSTACK_CONF_SHORTCUTS
/*---------------------------------------------------------------------------*/
/**
 * RF interrupt service routine.
 *
 */
void
cc2430_rf_ISR( void ) __interrupt (RF_VECTOR)
{
  EA = 0;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /*
   * We only vector here if RFSTATUS.FIFOP goes high.
   * Just double check the flag.
   */
  if(RFIF & IRQ_FIFOP) {
      RF_RX_LED_ON();
      /* Poll the RF process which calls cc2430_rf_read() */
      process_poll(&cc2430_rf_process);
  }
  S1CON &= ~(RFIF_0 | RFIF_1);

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  EA = 1;
}
#endif
/*---------------------------------------------------------------------------*/
#if CC2430_RFERR_INTERRUPT
/**
 * RF error interrupt service routine.
 * Turned off by default, can be turned on in contiki-conf.h
 */
void
cc2430_rf_error_ISR( void ) __interrupt (RFERR_VECTOR)
{
  EA = 0;
  TCON_RFERRIF = 0;
#ifdef HAVE_RF_ERROR
  rf_error = 254;
#endif
  cc2430_rf_command(ISRFOFF);
  cc2430_rf_command(ISFLUSHRX);
  cc2430_rf_command(ISFLUSHRX);
  cc2430_rf_command(ISRXON);
  RF_RX_LED_OFF();
  RF_TX_LED_OFF();
  EA = 1;
}
#endif
/*---------------------------------------------------------------------------*/
