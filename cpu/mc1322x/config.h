/* MC1322x flash config system */

#ifndef MC1322X_CONFIG_H
#define MC1322X_CONFIG_H

#define MC1322X_CONFIG_PAGE 0x1E000 /* nvm page where conf will be stored */
#define MC1322X_CONFIG_VERSION 1
#define MC1322X_CONFIG_MAGIC 0x1322

/* bitfield for various config flags */
struct FLAGS {
	uint32_t demod:1;   /* radio demodulation mode */
	uint32_t autoack:1; /* radio autoack vs. promiscuous mode */
	uint32_t nvmtype:4; /* stores the result of nvm_detect */
        uint32_t : 26;
};

typedef struct {
	uint16_t magic; /* mc1322x magic number 0x1322 */
	uint16_t version; /* mc1322x config version number */
	uint64_t eui;
	uint8_t channel; /* value to pass to set_channel */
	uint8_t power;   /* value to pass to set_power */
	struct FLAGS flags;
} mc1322xConfig;

extern mc1322xConfig mc1322x_config;

void mc1322x_config_set_default(mc1322xConfig *c);
void mc1322x_config_save(mc1322xConfig *c);
void mc1322x_config_restore(mc1322xConfig *c);
int mc1322x_config_valid(mc1322xConfig *c);
void mc1322x_config_print(void);

#endif
