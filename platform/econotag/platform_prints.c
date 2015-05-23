/* Temp place to put informational printing that happens a lot in platform code */
/* XXX TODO Factor this out to some place, almost all of the platforms use it */

#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/uip-ds6.h"

void
print_processes(struct process * const processes[])
{
  /*  const struct process * const * p = processes;*/
  printf("Starting");
  while(*processes != NULL) {
    printf(" '%s'", (*processes)->name);
    processes++;
  }
  printf("\n");
}

void
print_netstack(void) {
	printf("%s %s, channel check rate %lu Hz, radio channel %u\n",
	       NETSTACK_MAC.name, NETSTACK_RDC.name,
	       CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
			       NETSTACK_RDC.channel_check_interval()),
	       RF_CHANNEL);
}

void print_lladdrs(void) {	
	int i, a;
	printf("Tentative link-local IPv6 address ");
	
	for(a = 0; a < UIP_DS6_ADDR_NB; a++) {
		if (uip_ds6_if.addr_list[a].isused) {
			for(i = 0; i < 7; ++i) {
				printf("%02x%02x:",
				       uip_ds6_if.addr_list[a].ipaddr.u8[i * 2],
				       uip_ds6_if.addr_list[a].ipaddr.u8[i * 2 + 1]);
			}
			printf("%02x%02x\n",
			       uip_ds6_if.addr_list[a].ipaddr.u8[14],
			       uip_ds6_if.addr_list[a].ipaddr.u8[15]);
		}
	}
}
