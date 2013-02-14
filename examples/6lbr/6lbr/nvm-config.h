#ifndef NVM_CONFIG_H_
#define NVM_CONFIG_H_

typedef struct {
  uint16_t magic;
  uint16_t version;
  uint8_t eth_ip_addr[16];
  uint8_t mode;
  uint8_t rpl_version_id;
  uint8_t wsn_net_prefix[16];
  uint8_t wsn_ip_addr[16];
  uint8_t eth_net_prefix[16];
  uint8_t eth_dft_router[16];
  uint8_t channel;
} nvm_data_t;

#define CETIC_6LBR_NVM_MAGIC	0x2009
#define CETIC_6LBR_NVM_VERSION	0

#define CETIC_MODE_REWRITE_ADDR_MASK	0x01
#define CETIC_MODE_FILTER_RPL_MASK		0x02
#define CETIC_MODE_WAIT_RA_MASK			0x04
#define CETIC_MODE_ROUTER_SEND_CONFIG	0x08
#define CETIC_MODE_WSN_AUTOCONF         0x10
#define CETIC_MODE_ETH_AUTOCONF         0x20
#define CETIC_MODE_FILTER_NDP_MASK		0x40

extern nvm_data_t nvm_data;

void load_nvm_config(void);
void store_nvm_config(void);

#endif
