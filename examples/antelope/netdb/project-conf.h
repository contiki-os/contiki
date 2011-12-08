#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    4

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC                    nullrdc_driver

#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 4

#undef DCOSYNC_CONF_ENABLED
#define DCOSYNC_CONF_ENABLED                 0

#undef DB_FEATURE_JOIN
#define DB_FEATURE_JOIN                      0

#undef RF_CHANNEL
#define RF_CHANNEL                           16

#undef ROUTE_CONF_DEFAULT_LIFETIME
#define ROUTE_CONF_DEFAULT_LIFETIME          300
