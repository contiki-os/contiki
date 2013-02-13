#include "contiki.h"
#include "contiki-lib.h"

#include <stdio.h>              /* For printf() */
#include <string.h>
#include <ctype.h>

#include "rpl-private.h"

#include "cetic-bridge.h"
#include "nvm-config.h"
#include "nvm-itf.h"

nvm_data_t nvm_data;

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",(*lladdr)[0], (*lladdr)[1], (*lladdr)[2], (*lladdr)[3], (*lladdr)[4], (*lladdr)[5], (*lladdr)[6], (*lladdr)[7])
#define PRINTETHADDR(addr) printf(" %02x:%02x:%02x:%02x:%02x:%02x ",(*addr)[0], (*addr)[1], (*addr)[2], (*addr)[3], (*addr)[4], (*addr)[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#define PRINTETHADDR(addr)
#endif

/*---------------------------------------------------------------------------*/

void
check_nvm(volatile nvm_data_t * nvm_data)
{
  uint8_t flash = 0;
  uip_ipaddr_t loc_fipaddr;

  if(nvm_data->magic != CETIC_6LBR_NVM_MAGIC
     || nvm_data->version > CETIC_6LBR_NVM_VERSION) {
    //NVM is invalid or we are rollbacking from another version
    //Set all data to default values
    printf
      ("Invalid NVM magic number or unsupported NVM version, reseting it...\n");
    nvm_data->magic = CETIC_6LBR_NVM_MAGIC;
    nvm_data->version = CETIC_6LBR_NVM_VERSION;

    uip_ip6addr(&loc_fipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x0);
    memcpy(&nvm_data->eth_net_prefix, &loc_fipaddr.u8, 16);

    uip_ip6addr(&loc_fipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x100);
    memcpy(&nvm_data->eth_ip_addr, &loc_fipaddr.u8, 16);

    uip_ip6addr(&loc_fipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x0);
    memcpy(&nvm_data->wsn_net_prefix, &loc_fipaddr.u8, 16);

    uip_ip6addr(&loc_fipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x100);
    memcpy(&nvm_data->wsn_ip_addr, &loc_fipaddr.u8, 16);

    uip_ip6addr(&loc_fipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
    memcpy(&nvm_data->eth_dft_router, &loc_fipaddr.u8, 16);

    nvm_data->rpl_version_id = RPL_LOLLIPOP_INIT;

    nvm_data->mode =
      CETIC_MODE_WSN_AUTOCONF | CETIC_MODE_WAIT_RA_MASK |
      CETIC_MODE_ROUTER_SEND_CONFIG | CETIC_MODE_REWRITE_ADDR_MASK |
      CETIC_MODE_FILTER_RPL_MASK | CETIC_MODE_FILTER_NDP_MASK;

    nvm_data->channel = 26;

    flash = 1;
  }
  //Migration paths should be done here

  if(flash) {
    nvm_data_write();
  }
}

void
load_nvm_config(void)
{
  nvm_data_read();

  PRINTF("NVM Magic : %x\n", nvm_data.magic);
  PRINTF("NVM Version : %x\n", nvm_data.version);

  PRINTF("WSN Prefix :");
  PRINT6ADDR(&nvm_data.wsn_net_prefix);
  PRINTF("\n");

  PRINTF("WSN IP address :");
  PRINT6ADDR(&nvm_data.wsn_ip_addr);
  PRINTF("\n");

  PRINTF("Eth Prefix :");
  PRINT6ADDR(&nvm_data.eth_net_prefix);
  PRINTF("\n");

  PRINTF("Eth IP address :");
  PRINT6ADDR(&nvm_data.eth_ip_addr);
  PRINTF("\n");

  PRINTF("Mode : %x\n", nvm_data.mode);

  PRINTF("Channel : %d\n", nvm_data.channel);

  check_nvm(&nvm_data);
}

void
store_nvm_config(void)
{
  nvm_data_write();
}
