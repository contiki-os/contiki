
#include "contiki.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/uip-nd6.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "net/rpl/rpl.h"

#include <stdio.h> /* For printf() */
#include <string.h>
#include <ctype.h>

#if CONTIKI_TARGET_NATIVE
#include "native_nvm.h"
#else
#include "mc1322x.h"
#endif

#include "cetic_bridge.h"
#include "nvm_config.h"


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

#define RIMEADDR_NVM 0x1E000
#define RIMEADDR_NBYTES 8

extern const rimeaddr_t addr_ff;
extern const rimeaddr_t rimeaddr_null;

void iab_to_eui48(ethaddr_t *eui48, uint32_t oui, uint16_t iab, uint16_t ext) {
	/* OUI for IABs */
	(*eui48)[0] =  0x00;
	(*eui48)[1] =  0x50;
	(*eui48)[2] =  0xc2;

	/* IAB */
	(*eui48)[3] = (iab >> 4)  & 0xff;
	(*eui48)[4] = (iab << 4)  & 0xf0;

	/* EXT */

	(*eui48)[4] |= (ext >> 8)  & 0xf;
	(*eui48)[5] =   ext        & 0xff;
}

void oui_to_eui48(ethaddr_t *eui48, uint32_t oui, uint32_t ext) {
	/* OUI */
	*eui48[0] = (oui >> 16) & 0xff;
	*eui48[1] = (oui >>  8) & 0xff;
	*eui48[2] =  oui        & 0xff;

	/* EXT */
	*eui48[3] = (ext >> 16) & 0xff;
	*eui48[4] = (ext >>  8) & 0xff;
	*eui48[5] =  ext        & 0xff;
}

void set_eth_addr( volatile nvm_data_t *nvm_data )
{
	  nvmType_t type=0;
	  nvmErr_t err;

	if (memcmp(&nvm_data->eth_mac_addr, &addr_ff, 6)==0) {

		//set addr to EUI48
#ifdef IAB
   #ifdef ETH_EXT_ID
		PRINTF("eth address in flash blank, setting to defined IAB and extension.\n\r");
	  	iab_to_eui48(&eth_mac_addr, OUI, IAB, ETH_EXT_ID);
   #else  /* ifdef EXT_ID */
		PRINTF("eth address in flash blank, setting to defined IAB with a random extension.\n\r");
		iab_to_eui48(&eth_mac_addr, OUI, IAB, *MACA_RANDOM);
   #endif /* ifdef EXT_ID */

#else  /* ifdef IAB */

   #ifdef ETH_EXT_ID
		PRINTF("eth address in flash blank, setting to defined OUI and extension.\n\r");
		oui_to_eui48(&eth_mac_addr, OUI, ETH_EXT_ID);
   #else  /*ifdef EXT_ID */
		PRINTF("eth address in flash blank, setting to defined OUI with a random extension.\n\r");
		oui_to_eui48(&eth_mac_addr, OUI, *MACA_RANDOM);
   #endif /*endif EXTID */

#endif /* ifdef IAB */
		PRINTF("New MAC address");
		PRINTETHADDR(&eth_mac_addr);
		PRINTF("\n");
#ifdef FLASH_BLANK_ADDR
		PRINTF("flashing blank eth address\n\r");
		memcpy(&nvm_data->eth_mac_addr, &eth_mac_addr, 6);
		err = nvm_detect(gNvmInternalInterface_c, &type);
		err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)nvm_data, RIMEADDR_NVM, sizeof(nvm_data_t));
		PRINTF("err : %d\n", err);
#endif /* ifdef FLASH_BLANK_ADDR */
	} else {
		PRINTF("loading eth address from flash.\n\r");
		memcpy(eth_mac_addr, &nvm_data->eth_mac_addr, 6);
	}
}

void set_eth_ip_addr( volatile nvm_data_t *nvm_data )
{
	nvmType_t type=0;
	nvmErr_t err;
	uint8_t flash = 0;
	uip_ipaddr_t loc_fipaddr;

	//TODO: addr_ff and sizeof are wrong of ipv6 addr, should be corrected !
	if (memcmp(&nvm_data->eth_net_prefix, &addr_ff, sizeof(addr_ff))==0) {
		uip_ip6addr(&loc_fipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x0);
		memcpy(&nvm_data->eth_net_prefix, &loc_fipaddr.u8, 16);
		flash = 1;
	}
	if (memcmp(&nvm_data->eth_ip_addr, &addr_ff, sizeof(addr_ff))==0) {
	    uip_ip6addr(&loc_fipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x100);
		memcpy(&nvm_data->eth_ip_addr, &loc_fipaddr.u8, 16);
	    flash = 1;
	}
	if (memcmp(&nvm_data->wsn_net_prefix, &addr_ff, sizeof(addr_ff))==0) {
	    uip_ip6addr(&loc_fipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x0);
		memcpy(&nvm_data->wsn_net_prefix, &loc_fipaddr.u8, 16);
	    flash = 1;
	}
	if (memcmp(&nvm_data->wsn_ip_addr, &addr_ff, sizeof(addr_ff))==0) {
	    uip_ip6addr(&loc_fipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x100);
		memcpy(&nvm_data->wsn_ip_addr, &loc_fipaddr.u8, 16);
	    flash = 1;
	}
	if (memcmp(&nvm_data->eth_dft_router, &addr_ff, sizeof(addr_ff))==0) {
		uip_ip6addr(&loc_fipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
		memcpy(&nvm_data->eth_dft_router, &loc_fipaddr.u8, 16);
		flash = 1;
	}
	if (nvm_data->channel == 0xff) {
		nvm_data->channel = 26;
		flash = 1;
	}

#ifdef FLASH_BLANK_ADDR
	if ( flash ) {
		PRINTF("flashing blank eth ip address\n\r");
		err = nvm_detect(gNvmInternalInterface_c, &type);
		err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)nvm_data, RIMEADDR_NVM, sizeof(nvm_data_t));
		PRINTF("err : %d\n", err);
	}
#endif /* ifdef FLASH_BLANK_ADDR */
}

void load_nvm_config(void)
{
	nvmType_t type=0;
	nvmErr_t err;

	err = nvm_detect(gNvmInternalInterface_c, &type);
	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)&nvm_data, RIMEADDR_NVM, sizeof(nvm_data_t));

	PRINTF( "WSN MAC address :");
	PRINTLLADDR(&nvm_data.rime_addr);
	PRINTF("\n");

	PRINTF( "WSN Prefix :");
	PRINT6ADDR(&nvm_data.wsn_net_prefix);
	PRINTF("\n");

	PRINTF( "WSN IP address :");
	PRINT6ADDR(&nvm_data.wsn_ip_addr);
	PRINTF("\n");

	PRINTF( "Eth MAC address :");
	PRINTETHADDR(&nvm_data.eth_mac_addr);
	PRINTF("\n");

	PRINTF( "Eth Prefix :");
	PRINT6ADDR(&nvm_data.eth_net_prefix);
	PRINTF("\n");

	PRINTF( "Eth IP address :");
	PRINT6ADDR(&nvm_data.eth_ip_addr);
	PRINTF("\n");

	PRINTF("Mode : %x\n", nvm_data.mode);

	PRINTF("Channel : %d\n", nvm_data.channel);

	rimeaddr_copy( (rimeaddr_t *)&wsn_mac_addr, &rimeaddr_node_addr);
	set_eth_addr(&nvm_data);
	set_eth_ip_addr(&nvm_data);
}

void store_nvm_config(void)
{
	nvmType_t type=0;
	nvmErr_t err;
	volatile uint32_t buf[8];

	err = nvm_detect(gNvmInternalInterface_c, &type);

	err = nvm_erase(gNvmInternalInterface_c, type, 0x40000000);
	PRINTF("nvm_erase err : %d\n", err);
	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)&nvm_data, RIMEADDR_NVM, sizeof(nvm_data_t));
	PRINTF("nvm_write err : %d\n", err);
}
