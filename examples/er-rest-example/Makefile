all: rest-server-example coap-client-example plugtest-server

CONTIKI=../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# for static routing, if enabled
ifneq ($(TARGET), minimal-net)
PROJECT_SOURCEFILES += static-routing.c
endif

# variable for root Makefile.include
WITH_UIP6=1
# for some platforms
UIP_CONF_IPV6=1

# variable for this Makefile
# configure CoAP implementation (3|7)
WITH_COAP=7

# new variable since slip-radio
ifneq ($(TARGET), minimal-net)
UIP_CONF_RPL=1
else
# minimal-net does not support RPL under Linux and is mostly used to test CoAP only
${info INFO: compiling without RPL}
UIP_CONF_RPL=0
CFLAGS += -DUIP_CONF_ND6_DEF_MAXDADNS=0
CFLAGS += -DHARD_CODED_ADDRESS=\"2001:620:8:1010::10\"
CFLAGS += -DUIP_CONF_BUFFER_SIZE=1280
endif

# linker optimizations
SMALL=1

# REST framework, requires WITH_COAP
ifeq ($(WITH_COAP), 7)
${info INFO: compiling with CoAP-07}
CFLAGS += -DWITH_COAP=7
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-07
else ifeq ($(WITH_COAP), 3)
${info INFO: compiling with CoAP-03}
CFLAGS += -DWITH_COAP=3
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-03
else
${info INFO: compiling with HTTP}
CFLAGS += -DWITH_HTTP
CFLAGS += -DREST=http_rest_implementation
CFLAGS += -DUIP_CONF_TCP=1
APPS += er-http-engine
endif

APPS += erbium


#CUSTOM_RULE_C_TO_OBJECTDIR_O = 1
#CUSTOM_RULE_S_TO_OBJECTDIR_O = 1

include $(CONTIKI)/Makefile.include

#$(OBJECTDIR)/%.o: asmdir/%.S
#	$(CC) $(CFLAGS) -MMD -c $< -o $@
#	@$(FINALIZE_DEPENDENCY)
#
#asmdir/%.S: %.c
#	$(CC) $(CFLAGS) -MMD -S $< -o $@
 
 



$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64

tap0up:
	sudo ip address add fdfd::1/64 dev tap0
