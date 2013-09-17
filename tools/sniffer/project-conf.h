#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

// set the RDC layer to sniffer_fake_rdc_driver
#ifdef NETSTACK_CONF_RDC
#undef NETSTACK_CONF_RDC
#endif /* NETSTACK_CONF_RDC */
#define NETSTACK_CONF_RDC           sniffer_fake_rdc_driver

// disable RPL
#ifdef UIP_CONF_IPV6_RPL
#undef UIP_CONF_IPV6_RPL
#endif /* UIP_CONF_IPV6_RPL */
#define UIP_CONF_IPV6_RPL                        0

// nullmac
#ifdef NETSTACK_CONF_MAC
#undef NETSTACK_CONF_MAC
#endif /* NETSTACK_CONF_MAC */
#define NETSTACK_CONF_MAC           nullmac_driver

// sisclowpan
#ifdef NETSTACK_CONF_NETWORK
#undef NETSTACK_CONF_NETWORK
#endif /* NETSTACK_CONF_NETWORK */
#define NETSTACK_CONF_NETWORK		sicslowpan_driver

// some radio features must be disabled 
// in order to sniff properly

// needed for CC2420-based sensors (sky...)
#ifdef CC2420_CONF_CHECKSUM
#undef CC2420_CONF_CHECKSUM
#endif
#define CC2420_CONF_CHECKSUM		0

#ifdef CC2420_CONF_AUTOACK
#undef CC2420_CONF_AUTOACK
#endif
#define CC2420_CONF_AUTOACK		0

#endif /* __PROJECT_CONF_H__ */

