#include "contiki.h"
#include "sys/clock.h"
#include "sys/autostart.h"

#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/bus.h"
#include "dev/leds.h"
#include "dev/uart1.h"
#include "dev/dma.h"
#include "dev/models.h"
#include "dev/cc2430_rf.h"
#include "dev/watchdog.h"
#include "dev/lpm.h"
#include "net/rime.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "debug.h"
#include "stack.h"
#include "dev/watchdog-cc2430.h"
#include "dev/sensinode-sensors.h"
#include "disco.h"
#include "contiki-lib.h"
#include "contiki-net.h"

unsigned short node_id = 0;			/* Manually sets MAC address when > 0 */

#if VIZTOOL_CONF_ON
PROCESS_NAME(viztool_process);
#endif

#if BATMON_CONF_ON
PROCESS_NAME(batmon_process);
#endif

#if NETSTACK_CONF_SHORTCUTS
static CC_AT_DATA uint16_t len;
#endif

#ifdef STARTUP_CONF_VERBOSE
#define STARTUP_VERBOSE STARTUP_CONF_VERBOSE
#else
#define STARTUP_VERBOSE 0
#endif

#if STARTUP_VERBOSE
#define PUTSTRING(...) putstring(__VA_ARGS__)
#define PUTHEX(...) puthex(__VA_ARGS__)
#define PUTBIN(...) putbin(__VA_ARGS__)
#define PUTCHAR(...) putchar(__VA_ARGS__)
#else
#define PUTSTRING(...) do {} while(0)
#define PUTHEX(...) do {} while(0)
#define PUTBIN(...) do {} while(0)
#define PUTCHAR(...) do {} while(0)
#endif

#if CLOCK_CONF_STACK_FRIENDLY
extern volatile uint8_t sleep_flag;
#endif

extern rimeaddr_t rimeaddr_node_addr;
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
  uint8_t *addr_long = NULL;
  uint16_t addr_short = 0;
  char i;
  __code unsigned char * macp;

  PUTSTRING("Rime is 0x");
  PUTHEX(sizeof(rimeaddr_t));
  PUTSTRING(" bytes long\n");

  if(node_id == 0) {
    PUTSTRING("Reading MAC from flash\n");
    /*
     * The MAC is always stored in 0x1FFF8 of our flash. This maps to address
     * 0xFFF8 of our CODE segment, when BANK3 is selected.
     * Switch to BANK3, read 8 bytes starting at 0xFFF8 and restore last BANK
     * Since we are called from main(), this MUST be BANK1 or something is very
     * wrong. This code can be used even without banking
     */

    /* Don't interrupt us to make sure no BANK switching happens while working */
    DISABLE_INTERRUPTS();

    /* Switch to BANK3, map CODE: 0x8000 - 0xFFFF to FLASH: 0x18000 - 0x1FFFF */
    FMAP = 3;

    /* Set our pointer to the correct address and fetch 8 bytes of MAC */
    macp = (__code unsigned char *) 0xFFF8;

    for(i = (RIMEADDR_SIZE - 1); i >= 0; --i) {
      rimeaddr_node_addr.u8[i] = *macp;
      macp++;
    }

    /* Remap 0x8000 - 0xFFFF to BANK1 */
    FMAP = 1;
    ENABLE_INTERRUPTS();

  } else {
    PUTSTRING("Setting manual address from node_id\n");
    rimeaddr_node_addr.u8[RIMEADDR_SIZE - 1] = node_id >> 8;
    rimeaddr_node_addr.u8[RIMEADDR_SIZE - 2] = node_id & 0xff;
  }

  /* Now the address is stored MSB first */
#if STARTUP_VERBOSE
  PUTSTRING("Rime configured with address ");
  for(i = 0; i < RIMEADDR_SIZE - 1; i++) {
    PUTHEX(rimeaddr_node_addr.u8[i]);
    PUTCHAR(':');
  }
  PUTHEX(rimeaddr_node_addr.u8[i]);
  PUTCHAR('\n');
#endif

  /* Set the cc2430 RF addresses */
#if (RIMEADDR_SIZE==8)
  addr_short = (rimeaddr_node_addr.u8[6] * 256) + rimeaddr_node_addr.u8[7];
  addr_long = (uint8_t *) &rimeaddr_node_addr;
#else
  addr_short = (rimeaddr_node_addr.u8[0] * 256) + rimeaddr_node_addr.u8[1];
#endif
  cc2430_rf_set_addr(IEEE802154_PANID, addr_short, addr_long);
}
/*---------------------------------------------------------------------------*/
int
main(void)
{

  /* Hardware initialization */
  bus_init();
  rtimer_init();

  stack_poison();

  /* model-specific h/w init. */
  model_init();

  /* Init LEDs here */
  leds_init();
  fade(LEDS_GREEN);

  /* initialize process manager. */
  process_init();

  /* Init UART1 */
  uart1_init();

#if DMA_ON
  dma_init();
#endif

#if SLIP_ARCH_CONF_ENABLE
  /* On cc2430, the argument is not used */
  slip_arch_init(0);
#else
  uart1_set_input(serial_line_input_byte);
  serial_line_init();
#endif

  PUTSTRING("##########################################\n");
  putstring(CONTIKI_VERSION_STRING "\n");
  putstring(SENSINODE_MODEL " (CC24");
  puthex(((CHIPID >> 3) | 0x20));
  putstring("-" FLASH_SIZE ")\n");

#if STARTUP_VERBOSE
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

  /* Initialise the cc2430 RNG engine. */
  random_init(0);

  /* start services */
  process_start(&etimer_process, NULL);
  ctimer_init();

  /* initialize the netstack */
  netstack_init();
  set_rime_addr();

#if BUTTON_SENSOR_ON || ADC_SENSOR_ON
  process_start(&sensors_process, NULL);
  sensinode_sensors_activate();
#endif

#if UIP_CONF_IPV6
  memcpy(&uip_lladdr.addr, &rimeaddr_node_addr, sizeof(uip_lladdr.addr));
  queuebuf_init();
  process_start(&tcpip_process, NULL);

#if DISCO_ENABLED
  process_start(&disco_process, NULL);
#endif /* DISCO_ENABLED */

#if VIZTOOL_CONF_ON
  process_start(&viztool_process, NULL);
#endif

#if (!UIP_CONF_IPV6_RPL)
  {
    uip_ipaddr_t ipaddr;

    uip_ip6addr(&ipaddr, 0x2001, 0x630, 0x301, 0x6453, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
  }
#endif /* UIP_CONF_IPV6_RPL */
#endif /* UIP_CONF_IPV6 */

  /*
   * Acknowledge the UART1 RX interrupt
   * now that we're sure we are ready to process it
   */
  model_uart_intr_en();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  fade(LEDS_RED);

#if BATMON_CONF_ON
  process_start(&batmon_process, NULL);
#endif

  autostart_start(autostart_processes);

  watchdog_start();

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
#if NETSTACK_CONF_SHORTCUTS
    len = NETSTACK_RADIO.pending_packet();
    if(len) {
      packetbuf_clear();
      len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_RDC.input();
      }
    }
#endif

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
    SLEEP = (SLEEP & 0xFC) | (LPM_MODE - 1);

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

    if (SLEEP & SLEEP_MODE0) {
#endif /* LPM_MODE==LPM_MODE_PM2 */

      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);

      /* We are only interested in IRQ energest while idle or in LPM */
      ENERGEST_IRQ_RESTORE(irq_energest);

      /* Go IDLE or Enter PM1 */
      PCON |= IDLE;

      /* First instruction upon exiting PM1 must be a NOP */
      __asm
        nop
      __endasm;

      /* Remember energest IRQ for next pass */
      ENERGEST_IRQ_SAVE(irq_energest);

      ENERGEST_ON(ENERGEST_TYPE_CPU);
      ENERGEST_OFF(ENERGEST_TYPE_LPM);

#if (LPM_MODE==LPM_MODE_PM2)
      SLEEP &= ~OSC_PD;            /* Make sure both HS OSCs are on */
      while(!(SLEEP & XOSC_STB));  /* Wait for XOSC to be stable */
      CLKCON &= ~OSC;              /* Switch to the XOSC */
      /*
       * On occasion the XOSC is reported stable when in reality it's not.
       * We need to wait for a safeguard of 64us or more before selecting it
       */
      clock_delay_usec(65);
      while(CLKCON & OSC);         /* Wait till it's happened */
    }
#endif /* LPM_MODE==LPM_MODE_PM2 */
#endif /* LPM_MODE */
  }
}
/*---------------------------------------------------------------------------*/
