
#ifndef PROJECT_ROUTER_CONF_H_
#define PROJECT_ROUTER_CONF_H_

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#undef UIP_CONF_MAX_ROUTES

/* Set to run orchestra */
#ifndef WITH_ORCHESTRA
#define WITH_ORCHESTRA 0
#endif /* WITH_ORCHESTRA */

/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

#ifndef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
#endif

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4 ///////////default 4
#endif

////////////// A LITTLE TEST ////////////////////
// #undef UIP_DS6_ROUTE_NB
// #define UIP_DS6_ROUTE_NB	4


// #define NBR_TABLE_CONF_MAX_NEIGHBORS 4
// #undef TSCH_MAX_INCOMING_PACKETS
// #define TSCH_MAX_INCOMING_PACKETS 64

/* Turn of DAO ACK to make code smaller */
// #undef RPL_CONF_WITH_DAO_ACK
// #define RPL_CONF_WITH_DAO_ACK          0
/////////////////////////////////////////////////////



#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    240 //140
#endif

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  60
#endif

// #ifndef WEBSERVER_CONF_CFS_CONNS
// #define WEBSERVER_CONF_CFS_CONNS 2
// #endif

// /* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012
// /* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154


/* TSCH and RPL callbacks */
#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network

//Comment on 20160519: TSCH_HOPPING_SEQUENCE below to have the same default HOPPING_SEQUENCE with the leaf node
// #undef TSCH_CONF_DEFAULT_HOPPING_SEQUENCE
// #define TSCH_HOPPING_SEQUENCE_MY_SEQUENCE (uint8_t[]){17, 23, 15, 25, 19, 11, 13, 21}
// #define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_MY_SEQUENCE

/* RPL probing */
#define RPL_CONF_PROBING_INTERVAL (5 * CLOCK_SECOND)
#define RPL_CONF_PROBING_EXPIRATION_TIME (2 * 60 * CLOCK_SECOND)

/* Needed for cc2420 platforms only */
/* Disable DCO calibration (uses timerB) */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED            0
/* Enable SFD timestamps (uses timerB) */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS       1

/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

// /* TSCH logging. 0: disabled. 1: basic log. 2: with delayed
//  * log messages from interrupt */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 0

/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xabcd

// Do not start TSCH at init, wait for NETSTACK_MAC.on() 
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 0

// /* RPL storing mode */
// #undef RPL_CONF_MOP
// #define RPL_CONF_MOP RPL_MOP_NON_STORING

/* 6TiSCH minimal schedule length.
/ * Larger values result in less frequent active slots: reduces capacity and saves energy. */
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3 //default 3, test change for RPL routing table

#if CONTIKI_TARGET_Z1
/* Save some space to fit the limited RAM of the z1 */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 4 //default 4

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    140

#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES  16 //original value: 8
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 16   //original value: 8
#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA 0
#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG 0

/* Define as minutes */
#define RPL_CONF_DEFAULT_LIFETIME_UNIT   60

/* 10 minutes lifetime of routes */
#define RPL_CONF_DEFAULT_LIFETIME        10

#define RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME 1



#endif /* CONTIKI_TARGET_Z1 */



#endif /* PROJECT_ROUTER_CONF_H_ */
