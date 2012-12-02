#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/etimer.h>
#include <sys/autostart.h>
#include <sys/clock.h>
#include <contiki-net.h>
#include <net/dhcpc.h>
#include "debug-uart.h"
#include "emac-driver.h"
#include "contiki-conf.h"

unsigned int idle_count = 0;

int
main()
{
  uip_ipaddr_t addr;

  setup_debug_uart();
  printf("Initializing clocks\n");
  clock_init();
  printf("Initializing processes\n");
  process_init();

  #if defined (PLATFORM_HAS_LEDS)
  printf("Initializing leds\n");
  leds_init();
  #endif

  printf("Starting etimers\n");
  process_start(&etimer_process, NULL);
  printf("Starting EMAC service\n");
  process_start(&emac_lpc1768, NULL);
  printf("Starting TCP/IP service\n");
  process_start(&tcpip_process, NULL);          // invokes uip_init();

#if UIP_CONF_IPV6
  // init MAC address
  uip_lladdr.addr[0] = EMAC_ADDR0;
  uip_lladdr.addr[1] = EMAC_ADDR1;
  uip_lladdr.addr[2] = EMAC_ADDR2;
  uip_lladdr.addr[3] = EMAC_ADDR3;
  uip_lladdr.addr[4] = EMAC_ADDR4;
  uip_lladdr.addr[5] = EMAC_ADDR5;

  printf("Tentative link-local IPv6 address ");
    {
      uip_ds6_addr_t *lladdr;
      int i;
      lladdr = uip_ds6_get_link_local(-1);
      for (i = 0; i < 7; ++i)
        {
          printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
              lladdr->ipaddr.u8[i * 2 + 1]);
        }
      printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
    }

  uip_ipaddr_t ipaddr;
  int i;
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_PREFERRED);
  printf("Tentative global IPv6 address ");
  for (i = 0; i < 7; ++i)
    {
      printf("%02x%02x:", ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
    }
  printf("%02x%02x\n", ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
#else
  // init MAC address
  uip_ethaddr.addr[0] = EMAC_ADDR0;
  uip_ethaddr.addr[1] = EMAC_ADDR1;
  uip_ethaddr.addr[2] = EMAC_ADDR2;
  uip_ethaddr.addr[3] = EMAC_ADDR3;
  uip_ethaddr.addr[4] = EMAC_ADDR4;
  uip_ethaddr.addr[5] = EMAC_ADDR5;
  uip_setethaddr(uip_ethaddr);

  uip_ipaddr(&addr, 192, 168, 1, 5);
  printf("IP Address:  %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_sethostaddr(&addr);

  uip_ipaddr(&addr, 255, 255, 255, 0);
  printf("Subnet Mask: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_setnetmask(&addr);

  uip_ipaddr(&addr, 192, 168, 1, 1);
  printf("Def. Router: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_setdraddr(&addr);
#endif

  autostart_start(autostart_processes);

  printf("Processes running\n");
  while (1)
    {
      do
        {
        }
      while (process_run() > 0);
      idle_count++;
      /* Idle! */
      /* Stop processor clock */
      /* asm("wfi"::); */
    }
  return 0;
}

void
uip_log(char *m)
{
  printf("uIP: '%s'\n", m);
}
// required for #define LOG_CONF_ENABLED
void
log_message(const char *part1, const char *part2)
{
  printf("log: %s: %s\n", part1, part2);
}
