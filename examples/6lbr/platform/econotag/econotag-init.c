#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "cetic-6lbr.h"
#include "sicslow-ethernet.h"
#include "nvm-config.h"
#include "mc1322x.h"

#define DEBUG 1                 //DEBUG_NONE
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr[0], lladdr[1], lladdr[2], lladdr[3],lladdr[4], lladdr[5], lladdr[6], lladdr[7])
#define PRINTETHADDR(addr) printf(" %02x:%02x:%02x:%02x:%02x:%02x ",(*addr)[0], (*addr)[1], (*addr)[2], (*addr)[3], (*addr)[4], (*addr)[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#define PRINTETHADDR(addr)
#endif

void
platform_init(void)
{
  rimeaddr_copy((rimeaddr_t *) & wsn_mac_addr, &rimeaddr_node_addr);
  mac_createEthernetAddr((uint8_t *) eth_mac_addr, &wsn_mac_addr);
  PRINTF("Eth MAC address : ");
  PRINTETHADDR(&eth_mac_addr);
  PRINTF("\n");
  eth_mac_addr_ready = 1;
  set_channel(nvm_data.channel - 11);
}
