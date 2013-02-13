#ifndef __PROJECT_6LBR_CONF_H__
#define __PROJECT_6LBR_CONF_H__

/*------------------------------------------------------------------*/
/* Mode selection                                                   */
/*------------------------------------------------------------------*/

#define CETIC_6LBR							1

#ifndef CETIC_6LBR_SMARTBRIDGE
#define CETIC_6LBR_SMARTBRIDGE				0
#endif

#ifndef CETIC_6LBR_TRANSPARENTBRIDGE
#define CETIC_6LBR_TRANSPARENTBRIDGE		0
#endif

#ifndef CETIC_6LBR_ROUTER
#define CETIC_6LBR_ROUTER					0
#endif

#ifndef CETIC_NODE_INFO
#define CETIC_NODE_INFO						0
#endif

/*------------------------------------------------------------------*/
/* Common configuration                                             */
/*------------------------------------------------------------------*/

#undef NEIGHBOR_CONF_ATTR_MAX_NEIGHBORS
#define NEIGHBOR_CONF_ATTR_MAX_NEIGHBORS	8

#undef UIP_CONF_DS6_DEFRT_NB
#define UIP_CONF_DS6_DEFRT_NB				2

#undef RPL_CONF_INIT_LINK_METRIC
#define RPL_CONF_INIT_LINK_METRIC			3

#define RPL_CONF_DEFAULT_LIFETIME_UNIT  0x0100
#define RPL_CONF_DEFAULT_LIFETIME       0x1E

#undef UIP_FALLBACK_INTERFACE

#if CETIC_6LBR_SMARTBRIDGE
/*------------------------------------------------------------------*/
/* SMART BRIDGE MODE                                                */
/*------------------------------------------------------------------*/

#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER             0

//Local link address is already reserved
#define UIP_CONF_DS6_ADDR_NBU 1

#define UIP_CONF_DS6_ROUTE_INFORMATION	0

/*------------------------------------------------------------------*/
#endif

#if CETIC_6LBR_TRANSPARENTBRIDGE
/*------------------------------------------------------------------*/
/* TRANSPARENT BRIDGE MODE                                          */
/*------------------------------------------------------------------*/

#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER             0

//Local link address is already reserved
#define UIP_CONF_DS6_ADDR_NBU 1

//Setup 802.15.4 interface in promiscuous mode
#define NULLRDC_CONF_ADDRESS_FILTER	0
#undef MACA_AUTOACK
#define MACA_AUTOACK				0

#define UIP_CONF_DS6_ROUTE_INFORMATION	0

/*------------------------------------------------------------------*/
#endif

#if CETIC_6LBR_ROUTER
/*------------------------------------------------------------------*/
/* ROUTER MODE                                                      */
/*------------------------------------------------------------------*/

#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER             1

#undef UIP_CONF_ND6_SEND_RA
#define UIP_CONF_ND6_SEND_RA		1

#undef UIP_CONF_ROUTER_LIFETIME
#define UIP_CONF_ROUTER_LIFETIME	0

#undef UIP_CONF_ND6_DEF_MAXDADNS
#define UIP_CONF_ND6_DEF_MAXDADNS	1

//Local link address is already reserved
#define UIP_CONF_DS6_ADDR_NBU 3

#define UIP_CONF_DS6_ROUTE_INFORMATION	1

/*------------------------------------------------------------------*/
#endif

/*------------------------------------------------------------------*/
/* Platform related configuration                                   */
/*------------------------------------------------------------------*/

#if CONTIKI_TARGET_NATIVE
/*------------------------------------------------------------------*/
/* NATIVE 6LBR                                                      */
/*------------------------------------------------------------------*/

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM         4

#undef UIP_CONF_DS6_NBR_NBU
#define UIP_CONF_DS6_NBR_NBU     200

#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   200

#define WEBSERVER_EDITABLE_CONFIG 1

#define WEBSERVER_CONF_CFS_PATHLEN 1000

#define WEBSERVER_CONF_CFS_URLCONV 1

/* Do not change lines below */
#define SLIP_DEV_CONF_SEND_DELAY (CLOCK_SECOND / 32)

#define SERIALIZE_ATTRIBUTES 1

#define CMD_CONF_OUTPUT border_router_cmd_output

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC border_router_rdc_driver

#define SELECT_CALLBACK 1

#endif

#if CONTIKI_TARGET_ECONOTAG
/*------------------------------------------------------------------*/
/* ECONOTAG 6LBR                                                    */
/*------------------------------------------------------------------*/

#if WEBSERVER
#undef UIP_CONF_DS6_NBR_NBU
#define UIP_CONF_DS6_NBR_NBU     35

#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   35
#else
#undef UIP_CONF_DS6_NBR_NBU
#define UIP_CONF_DS6_NBR_NBU     100

#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   100
#endif

#define WEBSERVER_CONF_CFS_URLCONV	0

#define WEBSERVER_CONF_CFS_CONNS	1

/* Do not change lines below */

#endif

/* Do not change lines below */
#define CETIC_6LBR_NVM_MAGIC	0x2009
#define CETIC_6LBR_NVM_VERSION	0

#define CETIC_6LBR_VERSION "1.0.0"

//Sanity checks
#if ( (CETIC_6LBR_SMARTBRIDGE && (CETIC_6LBR_TRANSPARENTBRIDGE || CETIC_6LBR_ROUTER)) || ( CETIC_6LBR_TRANSPARENTBRIDGE && CETIC_6LBR_ROUTER ) )
#error Only one mode can be selected at a time
#endif
#if !CETIC_6LBR_SMARTBRIDGE && !CETIC_6LBR_TRANSPARENTBRIDGE && !CETIC_6LBR_ROUTER
#error A mode must be selected
#endif

#endif /* __PROJECT_6LBR_CONF_H__ */
