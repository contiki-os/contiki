// #undef TSCH_CONF_DEFAULT_HOPPING_SEQUENCE
// #define TSCH_HOPPING_SEQUENCE_MY_SEQUENCE (uint8_t[]){17, 23, 15, 25, 19, 11, 13, 21}
// #define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_MY_SEQUENCE

// #undef TSCH_CONF_JOIN_MY_PANID_ONLY
// #define TSCH_CONF_JOIN_MY_PANID_ONLY 1

/* RPL Trickle timer tuning */
// #undef RPL_CONF_DIO_INTERVAL_MIN
// #define RPL_CONF_DIO_INTERVAL_MIN 12 /* 4.096 s */

// #undef RPL_CONF_DIO_INTERVAL_DOUBLINGS
// #define RPL_CONF_DIO_INTERVAL_DOUBLINGS 2 /* Max factor: x4. 4.096 s * 4 = 16.384 s */

// #define TSCH_CONF_EB_PERIOD (4 * CLOCK_SECOND)
// #define TSCH_CONF_KEEPALIVE_TIMEOUT (24 * CLOCK_SECOND)



/* RPL probing */
// #define RPL_CONF_PROBING_INTERVAL (5 * CLOCK_SECOND)
// #define RPL_CONF_PROBING_EXPIRATION_TIME (2 * 60 * CLOCK_SECOND)

/* CoAP */

// #undef COAP_SERVER_PORT
// #define COAP_SERVER_PORT 5684

// #undef COAP_OBSERVE_RETURNS_REPRESENTATION
// #define COAP_OBSERVE_RETURNS_REPRESENTATION 1

/* RPL */
// #undef UIP_CONF_ROUTER
// #define UIP_CONF_ROUTER                 1

// /* RPL storing mode */
// #undef RPL_CONF_MOP
// #define RPL_CONF_MOP RPL_MOP_NON_STORING

// /* Default link metric */
// #undef RPL_CONF_INIT_LINK_METRIC
// #define RPL_CONF_INIT_LINK_METRIC 2  /*default 5*/ 

// #define RPL_CONF_MAX_INSTANCES    1 /* default 1 */
// #define RPL_CONF_MAX_DAG_PER_INSTANCE 1 /* default 2 */

// /* No RA, No NA */
// #undef UIP_CONF_ND6_SEND_NA
// #define UIP_CONF_ND6_SEND_NA 0

// #undef UIP_CONF_ND6_SEND_RA
// #define UIP_CONF_ND6_SEND_RA 0


////////////// A LITTLE TEST ////////////////////
// #undef TSCH_MAX_INCOMING_PACKETS
// #define TSCH_MAX_INCOMING_PACKETS 64
// #undef UIP_DS6_ROUTE_NB
// #define UIP_DS6_ROUTE_NB	16

// /* RPL storing mode */
// #undef RPL_CONF_MOP
// #define RPL_CONF_MOP RPL_MOP_NON_STORING

// #define NBR_TABLE_CONF_MAX_NEIGHBORS 16
//////////////////////////////////////////////////

/* CoAP */

// #undef COAP_SERVER_PORT
// #define COAP_SERVER_PORT 5684

// #undef COAP_OBSERVE_RETURNS_REPRESENTATION
// #define COAP_OBSERVE_RETURNS_REPRESENTATION 1