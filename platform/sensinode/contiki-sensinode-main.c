#include <stdio.h>

#include "contiki.h"
#include "sys/clock.h"
#include "sys/autostart.h"

#include "dev/serial-line.h"
#include "dev/bus.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/models.h"
#include "dev/cc2430_rf.h"
#include "net/mac/sicslowmac.h"
#include "net/mac/frame802154.h"
#include "net/rime.h"

volatile int i, a;
unsigned short node_id = 0;			/* Manually sets MAC address when > 0 */

/*---------------------------------------------------------------------------*/
static void
print_processes(struct process * const processes[])
{
  printf("Starting");
  while(*processes != NULL) {
    printf(" '%s'", (*processes)->name);
    processes++;
  }
  printf("\n");
}
/*---------------------------------------------------------------------------*/
void
putchar(char c)
{
  /* UART1 used for debugging on Sensinode products. */
  uart1_writeb(c);
}
/*---------------------------------------------------------------------------*/
static void
fade(int l)
{
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
set_rime_addr(void)
{
  rimeaddr_t addr;
  uint8_t ft_buffer[8];
  uint8_t *addr_long = NULL;
  uint16_t addr_short = 0;
  int i;

  /* TODO: This flash_read routine currently gets a different address
   * than nano_programmer -m... something broken or misconfigured...
   */

  flash_read(&ft_buffer[0], 0x1FFF8, 8);

  printf("Read MAC from flash: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	 ft_buffer[0], ft_buffer[1], ft_buffer[2], ft_buffer[3],
	 ft_buffer[4], ft_buffer[5], ft_buffer[6], ft_buffer[7]);

  memset(&addr, 0, sizeof(rimeaddr_t));

#if UIP_CONF_IPV6
  memcpy(addr.u8, ft_buffer, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(rimeaddr_t); ++i) {
      addr.u8[i] = ft_buffer[7 - i];
    }
  } else {
    printf("Setting manual address from node_id\n");
    addr.u8[1] = node_id >> 8;
    addr.u8[0] = node_id & 0xff;
  }
#endif

  rimeaddr_set_node_addr(&addr);
  printf("Rime configured with address ");
  for(i = (sizeof(addr.u8)) - 1; i > 0; i--) {
    printf("%02x:", addr.u8[i]);
  }
  printf("%02x\n", addr.u8[i]);

  /* Set the cc2430 RF addresses */
  if (sizeof(addr.u8) == 6)
	  addr_long = (uint8_t *) addr.u8;
  else
	  addr_short = (addr.u8[1] * 256) + addr.u8[0];

  cc2430_rf_set_addr(0xffff, addr_short, addr_long);
}
/*---------------------------------------------------------------------------*/
int
main(void)
{

  /* Hardware initialization */
  bus_init();

  leds_init();
  fade(LEDS_GREEN);

  uart1_init(115200);
  uart1_set_input(serial_line_input_byte);

  /* initialize process manager. */
  process_init();

  serial_line_init();

  printf("\n" CONTIKI_VERSION_STRING " started\n");
  printf("model: " SENSINODE_MODEL "\n\n");

  /* initialize the radio driver */

  cc2430_rf_init();
  rime_init(sicslowmac_init(&cc2430_rf_driver));
  set_rime_addr();

  /* start services */
  process_start(&etimer_process, NULL);

  fade(LEDS_RED);

  autostart_start(autostart_processes);

  while(1) {
    process_run();
    etimer_request_poll();
  }
}
/*---------------------------------------------------------------------------*/
