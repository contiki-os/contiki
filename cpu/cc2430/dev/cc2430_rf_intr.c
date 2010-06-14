/**
 * \file
 *         CC2430 RF driver
 * \author
 *         Zach Shelby <zach@sensinode.com>
 *
 *  Non-bankable code for cc2430 rf driver.  
 *  Interrupt routine and code called through function pointers
 *  must be placed into the HOME bank.
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


/*---------------------------------------------------------------------------*/
PROCESS(cc2430_rf_process, "CC2430 RF driver");

/*---------------------------------------------------------------------------*/
/**
 * RF interrupt service routine.
 *
 */
void
cc2430_rf_ISR( void ) __interrupt (RF_VECTOR)
{
  EA = 0;
  if(RFIF & IRQ_TXDONE) {
    RF_TX_LED_OFF();
    RFIF &= ~IRQ_TXDONE;
    cc2430_rf_command(ISFLUSHTX);
  }
  if(RFIF & IRQ_FIFOP) {
    if(RFSTATUS & FIFO) {
      RF_RX_LED_ON();
      /* Poll the RF process which calls cc2430_rf_read() */
      process_poll(&cc2430_rf_process);
    } else {
      cc2430_rf_command(ISFLUSHRX);
      cc2430_rf_command(ISFLUSHRX);
      RFIF &= ~IRQ_FIFOP;
    }
  }
  S1CON &= ~(RFIF_0 | RFIF_1);
  EA = 1;
}
/*---------------------------------------------------------------------------*/
/**
 * RF error interrupt service routine.
 *
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

void (* receiver_callback)(const struct radio_driver *);

void
cc2430_rf_set_receiver(void (* recv)(const struct radio_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
/* 
 * non-banked functions called through function pointers then call banked code
 */
int
cc2430_rf_off(void)
{
  return cc2430_rf_rx_disable();
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_on(void)
{
  return cc2430_rf_rx_enable();
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_send(void *payload, unsigned short payload_len)
{
  return cc2430_rf_send_b(payload, payload_len);
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_read(void *buf, unsigned short bufsize) 
{
  return cc2430_rf_read_banked(buf, bufsize);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2430_rf_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    if(receiver_callback != NULL) {
      PRINTF("cc2430_rf_process: calling receiver callback\n");
      receiver_callback(&cc2430_rf_driver);
    } else {
      PRINTF("cc2430_rf_process: no receiver callback\n");
      cc2430_rf_command(ISFLUSHRX);
    }
  }

  PROCESS_END();
}
