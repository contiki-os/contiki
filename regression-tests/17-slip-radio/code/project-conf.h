#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#define NETSTACK_CONF_MAC     		csma_driver

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     		nullrdc_driver
#define NULLRDC_CONF_802154_AUTOACK 	1

#define RPL_CONF_DAO_ACK		1

#endif /* __PROJECT_CONF_H__ */
