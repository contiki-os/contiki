CONTIKI_PROJECT = dtls-server dtls-client ping_server
all: $(CONTIKI_PROJECT) 

CONTIKI = ../..

WITH_COAP=13
WITH_DTLS=1
WITH_CONTIKI=1
UIP_CONF_IPV6=1
WITH_JSON=1
WITH_PKI=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS+= -DUIP_CONF_IPV6_RPL=0 -DUIP_CONF_RPL=0

ifeq ($(WITH_COAP), 13)
${info INFO: compiling with CoAP-13}
CFLAGS += -DWITH_COAP=13
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-13
endif

CFLAGS += -DWITH_CONTIKI=1 -DDEBUG=1 -DWITH_DTLS=1 -DMACDEBUG=1
#-DWITH_DTLS_PSK_EXT=1

ifeq ($(WITH_DTLS), 1)
${info INFO: compiling with DTLS}
APPS += tinydtls/aes tinydtls/sha2 tinydtls/ecc tinydtls 
endif

APPS += erbium

include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -s /dev/ttyUSB3 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64