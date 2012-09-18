#include "contiki.h"
#include "soc.h"
#include "stack.h"
#include "sys/clock.h"
#include "sys/autostart.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "dev/io-arch.h"
#include "dev/dma.h"
#include "dev/cc2530-rf.h"
#include "dev/watchdog.h"
#include "dev/clock-isr.h"
#include "dev/port2.h"
#include "dev/lpm.h"
#include "dev/button-sensor.h"
#include "dev/adc-sensor.h"
#include "dev/leds-arch.h"
#include "net/rime.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "debug.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "contiki-lib.h"
#include "contiki-net.h"
/*---------------------------------------------------------------------------*/
#if VIZTOOL_CONF_ON
PROCESS_NAME(viztool_process);
#endif
/*---------------------------------------------------------------------------*/
#if STARTUP_CONF_VERBOSE
#define PUTSTRING(...) putstring(__VA_ARGS__)
#define PUTHEX(...) puthex(__VA_ARGS__)
#define PUTBIN(...) putbin(__VA_ARGS__)
#define PUTCHAR(...) putchar(__VA_ARGS__)
#else
#define PUTSTRING(...)
#define PUTHEX(...)
#define PUTBIN(...)
#define PUTCHAR(...)
#endif
/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
extern volatile uint8_t sleep_flag;
#endif
/*---------------------------------------------------------------------------*/
extern rimeaddr_t rimeaddr_node_addr;
static CC_AT_DATA uint16_t len;
/*---------------------------------------------------------------------------*/
#if ENERGEST_CONF_ON
static unsigned long irq_energest = 0;
#define ENERGEST_IRQ_SAVE(a) do { \
    a = energest_type_time(ENERGEST_TYPE_IRQ); } while(0)
#define ENERGEST_IRQ_RESTORE(a) do { \
    energest_type_set(ENERGEST_TYPE_IRQ, a); } while(0)
#else
#define ENERGEST_IRQ_SAVE(a) do {} while(0)
#define ENERGEST_IRQ_RESTORE(a) do {} while(0)
#endif
/*---------------------------------------------------------------------------*/
static void
fade(int l) CC_NON_BANKED
{
  volatile int i, a;
  int k, j;
  for(k = 0; k < 400; ++k) {
    j = k > 200? 400 - k: k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      a = i;
    }
    leds_off(l);
    for(i = 0; i < 200 - j; ++i) {
      a = i;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void) CC_NON_BANKED
{
  char i;

#if CC2530_CONF_MAC_FROM_PRIMARY
  __xdata unsigned char * macp = &X_IEEE_ADDR;
#else
  __code unsigned char * macp = (__code unsigned char *) 0xFFE8;
#endif

  PUTSTRING("Rime is 0x");
  PUTHEX(sizeof(rimeaddr_t));
  PUTSTRING(" bytes long\n");

#if CC2530_CONF_MAC_FROM_PRIMARY
  PUTSTRING("Reading MAC from Info Page\n");
#else
  PUTSTRING("Reading MAC from flash\n");

  /*
   * The MAC is always stored in 0xFFE8 of the highest BANK of our flash. This
   * maps to address 0xFFF8 of our CODE segment, when this BANK is selected.
   * Load the bank, read 8 bytes starting at 0xFFE8 and restore last BANK.
   * Since we are called from main(), this MUST be BANK1 or something is very
   * wrong. This code can be used even without a bankable firmware.
   */

  /* Don't interrupt us to make sure no BANK switching happens while working */
  DISABLE_INTERRUPTS();

  /* Switch to the BANKn,
   * map CODE: 0x8000 - 0xFFFF to FLASH: 0xn8000 - 0xnFFFF */
  FMAP = CC2530_LAST_FLASH_BANK;
#endif

  for(i = (RIMEADDR_SIZE - 1); i >= 0; --i) {
    rimeaddr_node_addr.u8[i] = *macp;
    macp++;
  }

#if !CC2530_CONF_MAC_FROM_PRIMARY
  /* Remap 0x8000 - 0xFFFF to BANK1 */
  FMAP = 1;
  ENABLE_INTERRUPTS();
#endif

  /* Now the address is stored MSB first */
#if STARTUP_CONF_VERBOSE
  PUTSTRING("Rime configured with address ");
  for(i = 0; i < RIMEADDR_SIZE - 1; i++) {
    PUTHEX(rimeaddr_node_addr.u8[i]);
    PUTCHAR(':');
  }
  PUTHEX(rimeaddr_node_addr.u8[i]);
  PUTCHAR('\n');
#endif

  cc2530_rf_set_addr(IEEE802154_PANID);
  return;
}
/*---------------------------------------------------------------------------*/
int
main(void) CC_NON_BANKED
{
  /* Hardware initialization */
  clock_init();
  soc_init();
  rtimer_init();

  stack_poison();

  /* Init LEDs here */
  leds_init();
  leds_off(LEDS_ALL);
  fade(LEDS_GREEN);

  /* initialize process manager. */
  process_init();

#if DMA_ON
  dma_init();
#endif

  io_arch_init();

#if SLIP_ARCH_CONF_ENABLE
  slip_arch_init(0);
#else
  io_arch_set_input(serial_line_input_byte);
  serial_line_init();
#endif
  fade(LEDS_RED);

  PUTSTRING("##########################################\n");
  putstring(CONTIKI_VERSION_STRING "\n");
  putstring(MODEL_STRING);
  switch(CHIPID) {
  case 0xA5:
    putstring("cc2530");
    break;
  case 0xB5:
    putstring("cc2531");
    break;
  case 0x95:
    putstring("cc2533");
    break;
  case 0x8D:
    putstring("cc2540");
    break;
  }

  putstring("-" CC2530_FLAVOR_STRING ", ");
  puthex(CHIPINFO1 + 1);
  putstring("KB SRAM\n");

  PUTSTRING("\nSDCC Build:\n");
#if STARTUP_CONF_VERBOSE
#ifdef HAVE_SDCC_BANKING
  PUTSTRING("  With Banking.\n");
#endif /* HAVE_SDCC_BANKING */
#ifdef SDCC_MODEL_LARGE
  PUTSTRING("  --model-large\n");
#endif /* SDCC_MODEL_LARGE */
#ifdef SDCC_MODEL_HUGE
  PUTSTRING("  --model-huge\n");
#endif /* SDCC_MODEL_HUGE */
#ifdef SDCC_STACK_AUTO
  PUTSTRING("  --stack-auto\n");
#endif /* SDCC_STACK_AUTO */

  PUTCHAR('\n');

  PUTSTRING(" Net: ");
  PUTSTRING(NETSTACK_NETWORK.name);
  PUTCHAR('\n');
  PUTSTRING(" MAC: ");
  PUTSTRING(NETSTACK_MAC.name);
  PUTCHAR('\n');
  PUTSTRING(" RDC: ");
  PUTSTRING(NETSTACK_RDC.name);
  PUTCHAR('\n');

  PUTSTRING("##########################################\n");
#endif

  watchdog_init();

  /* Initialise the H/W RNG engine. */
  random_init(0);

  /* start services */
  process_start(&etimer_process, NULL);
  ctimer_init();

  /* initialize the netstack */
  netstack_init();
  set_rime_addr();

#if BUTTON_SENSOR_ON || ADC_SENSOR_ON
  process_start(&sensors_process, NULL);
  BUTTON_SENSOR_ACTIVATE();
  ADC_SENSOR_ACTIVATE();
#endif

#if UIP_CONF_IPV6
  memcpy(&uip_lladdr.addr, &rimeaddr_node_addr, sizeof(uip_lladdr.addr));
  queuebuf_init();
  process_start(&tcpip_process, NULL);
#endif /* UIP_CONF_IPV6 */

#if VIZTOOL_CONF_ON
  process_start(&viztool_process, NULL);
#endif

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  autostart_start(autostart_processes);

  watchdog_start();

  fade(LEDS_YELLOW);

  while(1) {
    uint8_t r;
    do {
      /* Reset watchdog and handle polls and events */
      watchdog_periodic();

#if CLOCK_CONF_STACK_FRIENDLY
      if(sleep_flag) {
        if(etimer_pending() &&
            (etimer_next_expiration_time() - clock_time() - 1) > MAX_TICKS) {
          etimer_request_poll();
        }
        sleep_flag = 0;
      }
#endif
      r = process_run();
    } while(r > 0);
    len = NETSTACK_RADIO.pending_packet();
    if(len) {
      packetbuf_clear();
      len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_RDC.input();
      }
    }

#if LPM_MODE
#if (LPM_MODE==LPM_MODE_PM2)
    SLEEP &= ~OSC_PD;            /* Make sure both HS OSCs are on */
    while(!(SLEEP & HFRC_STB));  /* Wait for RCOSC to be stable */
    CLKCON |= OSC;               /* Switch to the RCOSC */
    while(!(CLKCON & OSC));      /* Wait till it's happened */
    SLEEP |= OSC_PD;             /* Turn the other one off */
#endif /* LPM_MODE==LPM_MODE_PM2 */

    /*
     * Set MCU IDLE or Drop to PM1. Any interrupt will take us out of LPM
     * Sleep Timer will wake us up in no more than 7.8ms (max idle interval)
     */
    SLEEPCMD = (SLEEPCMD & 0xFC) | (LPM_MODE - 1);

#if (LPM_MODE==LPM_MODE_PM2)
    /*
     * Wait 3 NOPs. Either an interrupt occurred and SLEEP.MODE was cleared or
     * no interrupt occurred and we can safely power down
     */
    __asm
      nop
      nop
      nop
    __endasm;

    if(SLEEPCMD & SLEEP_MODE0) {
#endif /* LPM_MODE==LPM_MODE_PM2 */

      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);

      /* We are only interested in IRQ energest while idle or in LPM */
      ENERGEST_IRQ_RESTORE(irq_energest);

      /* Go IDLE or Enter PM1 */
      PCON |= PCON_IDLE;

      /* First instruction upon exiting PM1 must be a NOP */
      __asm
        nop
      __endasm;

      /* Remember energest IRQ for next pass */
      ENERGEST_IRQ_SAVE(irq_energest);

      ENERGEST_ON(ENERGEST_TYPE_CPU);
      ENERGEST_OFF(ENERGEST_TYPE_LPM);

#if (LPM_MODE==LPM_MODE_PM2)
      SLEEPCMD &= ~SLEEP_OSC_PD;            /* Make sure both HS OSCs are on */
      while(!(SLEEPCMD & SLEEP_XOSC_STB));  /* Wait for XOSC to be stable */
      CLKCONCMD &= ~CLKCONCMD_OSC;              /* Switch to the XOSC */
      /*
       * On occasion the XOSC is reported stable when in reality it's not.
       * We need to wait for a safeguard of 64us or more before selecting it
       */
      clock_delay_usec(65);
      while(CLKCONCMD & CLKCONCMD_OSC);         /* Wait till it's happened */
    }
#endif /* LPM_MODE==LPM_MODE_PM2 */
#endif /* LPM_MODE */
  }
}
/*---------------------------------------------------------------------------*/
