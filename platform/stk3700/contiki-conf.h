#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include "platform-conf.h"

#define CC_CONF_INLINE inline
#define CC_CONF_DOUBLE_HASH 1
#define CCIF
#define CLIF

/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;

#define KHZ 1000uL
#define MHZ 1000000uL

#define sleep(sec)   clock_wait(sec*1000)
#define msleep(msec) clock_wait(msec)
#define usleep(usec) clock_delay_usec(usec)

void clock_delay_msec(uint16_t dt);



#define SERIAL_LINE_CONF_LOCALECHO
#define SHELL_CONF_PROMPT
#define SERIAL_SHELL_CONF_PROMPT(str) \
  printf("%02X%02X: %s",rimeaddr_node_addr.u8[1],rimeaddr_node_addr.u8[0],str)



// Manchester encoded packets can use 128*2 sized packets
// Took into account in RF driver (use PACKETBUF_SIZE * 2)
//#define PACKETBUF_CONF_SIZE 256

#ifndef NETSTACK_CONF_MAC
//#define NETSTACK_CONF_MAC     sicslowmac
//#define NETSTACK_CONF_MAC     nullmac_driver
#endif /* NETSTACK_CONF_MAC */

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_noframer_driver
#endif /* NETSTACK_CONF_RDC */

#ifndef NETSTACK_CONF_FRAMER
//#define NETSTACK_CONF_FRAMER  framer_802154
#define NETSTACK_CONF_FRAMER  framer_nullmac
#endif /* NETSTACK_CONF_FRAMER */

#ifndef NETSTACK_CONF_RADIO
//#define NETSTACK_CONF_RADIO   cc112x_driver
#endif /* NETSTACK_CONF_RADIO */


#endif /* CONTIKI_CONF_H */
