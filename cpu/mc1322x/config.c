/* MC1322x flash config system */

#include <mc1322x.h>
#include "config.h"

/* debug */
#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

mc1322xConfig mc1322x_config;

void dump_bytes(uint32_t addr, uint16_t num);

/* takes an mc1322xConf and initializes to default values */
void mc1322x_config_set_default(mc1322xConfig *c) {
	nvmType_t type;
	c->magic = MC1322X_CONFIG_MAGIC;
	c->version = MC1322X_CONFIG_VERSION;
	c->eui = 0;
	c->channel = RF_CHANNEL - 11;
	c->power = 0x11;
	c->flags.demod = DEMOD_DCD;
	c->flags.autoack = AUTOACK;
	nvm_detect(gNvmInternalInterface_c, &type);
	c->flags.nvmtype = type;
}

/* write out config to flash */
void mc1322x_config_save(mc1322xConfig *c) {
	nvmErr_t err;
	err = nvm_erase(gNvmInternalInterface_c, c->flags.nvmtype, 1 << MC1322X_CONFIG_PAGE/4096);
	err = nvm_write(gNvmInternalInterface_c, c->flags.nvmtype, (uint8_t *)c, MC1322X_CONFIG_PAGE, sizeof(mc1322xConfig));
}

/* load the config from flash to the pass conf structure */
void mc1322x_config_restore(mc1322xConfig *c) {
	nvmErr_t err;
	nvmType_t type;
	if (c->flags.nvmtype == 0) { nvm_detect(gNvmInternalInterface_c, &type); }
	c->flags.nvmtype = type;
	err = nvm_read(gNvmInternalInterface_c, c->flags.nvmtype, c, MC1322X_CONFIG_PAGE, sizeof(mc1322xConfig));
}

/* check the flash for magic number and proper version */
int mc1322x_config_valid(mc1322xConfig *c) {
	if (c->magic == MC1322X_CONFIG_MAGIC &&
	    c->version == MC1322X_CONFIG_VERSION) {
		return 1;
	} else {
#if DEBUG
		if (c->magic != MC1322X_CONFIG_MAGIC) { PRINTF("config bad magic %04x\n\r", c->magic); }
		if (c->version != MC1322X_CONFIG_MAGIC) { PRINTF("config bad version %04x\n\r", c->version); }
#endif
		return -1;
	}
}

void mc1322x_config_print(void) {
	uint64_t eui64;
	PRINTF("mc1322x config:\n\r");
	PRINTF("  magic:    %04x\n\r", mc1322x_config.magic);
	PRINTF("  version:  %d\n\r",   mc1322x_config.version);
	PRINTF("  eui:      %08x%08x\n\r", (uint32_t)(mc1322x_config.eui>>32), (uint32_t)(mc1322x_config.eui & 0xffffffff));
	PRINTF("  channel:  %d\n\r",   mc1322x_config.channel);
	PRINTF("  power:    %d\n\r",   mc1322x_config.power);
	PRINTF("  flags:    %08x\n\r",   mc1322x_config.flags);
	PRINTF("     demod:    %d\n\r",   mc1322x_config.flags.demod);
	PRINTF("     autoack:  %d\n\r",   mc1322x_config.flags.autoack);
	PRINTF("     nvm type: %d\n\r",   mc1322x_config.flags.nvmtype);
}

void dump_bytes(uint32_t addr, uint16_t num) {
	uint32_t buf[num/4];
	nvmErr_t err;
	uint16_t i;

	err = nvm_read(gNvmInternalInterface_c, mc1322x_config.flags.nvmtype, (uint8_t *)buf, addr, num);
	PRINTF("nvm_read returned: 0x%02x\r\n", err);

	for(i=0; i < num/4; i++) {
		printf("0x%08x\r\n", (unsigned int)buf[i]);
	}
}
