This is an implementation of loadng, derived from the AODV of contiki. It adds the construction of a collection tree by means of sending OPT messages that play a similar role to the DIO messages of RPL. 

- loadng is enabled when the macro WITH_IPV6_LOADNG==1. (Obviously, one needs to set UIP_CONF_IPV6_RPL=0)

- loadng is the process "loadng_process". It needs to be started! Eg. add in the contiki-main.c file: 
process_start(&loadng_process, NULL); 

- USE_OPT: this macro is for the code that is specific to cases where we use OPT messages (the collection tree).

- To set the behavior of each node, there are a few macros:
  - LOADNG_IS_COORDINATOR: If set to 1, this node will periodically re-send OPT messages -- set to 0 for a leaf node that will not relay traffic;
  - LOADNG_CONF_IS_SINK: this node will send OPT to advertise itself as a sink -- the LBR should do this;
  - LOADNG_RREQ_RETRIES, LOADNG_RREQ_RATELIMIT, LOADNG_R_HOLD_TIME.
  
- Set UIP_ND6_SEND_NA to 1 if you want to use NS/NA, set it to 0 and loadng determines the link layer address from the link local address (and vice-versa). Obviously, the objective is to get rid of NS/NA, unless you use nullmac, for instance.



So a typical Makefile could look like this: 

CFLAGS += -D'LOADNG_CONF_IS_COORDINATOR()=1'

CFLAGS += -DWITH_IPV6_LOADNG=1
CFLAGS += -DUSE_OPT=1
CFLAGS += -DUIP_CONF_IPV6_RPL=0
#CFLAGS += -DUIP_CONF_ND6_SEND_NA=0
CFLAGS += -DRESOLV_CONF_SUPPORTS_MDNS=0
CFLAGS += -DUIP_CONF_ROUTER=1
CFLAGS += -DLOADNG_CONF_IS_SINK=0 # Set to 1 at the sink!




