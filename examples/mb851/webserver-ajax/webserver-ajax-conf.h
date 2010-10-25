

#ifndef __WEBSERVER_AJAX_CONF_H__
#define __WEBSERVER_AJAX_CONF_H__

#undef WITH_RIME
#define WITH_RIME 1

#undef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON    1

#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER	          0

#undef WITH_SERIAL_LINE_INPUT
#define WITH_SERIAL_LINE_INPUT    0

/* Needed for communicating with other nodes outside the LoWPAN,
 as UDP checksum is not optional in IPv6. */
#undef UIP_CONF_UDP_CHECKSUMS
#define UIP_CONF_UDP_CHECKSUMS    1


#endif /* __WEBSERVER_AJAX_CONF_H__ */
