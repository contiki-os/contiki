#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/etimer.h>
#include <sys/autostart.h>
#include <sys/clock.h>
#include "contiki-net.h"
//#include <net/dhcpc.h>
#include "debug-uart.h"
//#include "emac-driver.h"

unsigned int idle_count = 0;

int
main()
{
  setup_debug_uart();
  printf("Initializing clocks\n");
  clock_init();
  printf("Initializing processes\n");
  process_init();
  printf("Starting etimers\n");
  process_start(&etimer_process, NULL );
//  printf("\rStarting EMAC service\r\n");
//  process_start(&emac_lpc1768, NULL);
//  printf("\rStarting TCP/IP service \r\n");
//  process_start(&tcpip_process, NULL);          // invokes uip_init();
  //printf("\rStarting DHCP service\r\n");
  //process_start(&dhcp_process, NULL);

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


void uip_log(char *m)
{
  printf("uIP: '%s'\n", m);
}
// required for #define LOG_CONF_ENABLED
void log_message(const char *part1, const char *part2) {
  printf("log: %s: %s\n", part1, part2);
}
