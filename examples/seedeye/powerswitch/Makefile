ifndef TARGET
TARGET=seedeye
endif

all: remotepowerswitch

CONTIKI=../../../
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

WITH_COAP=13

# REST framework, requires WITH_COAP
ifeq ($(WITH_COAP), 13)
${info INFO: compiling with CoAP-13}
CFLAGS += -DWITH_COAP=13
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-13
else ifeq ($(WITH_COAP), 12)
${info INFO: compiling with CoAP-12}
CFLAGS += -DWITH_COAP=12
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-12
else ifeq ($(WITH_COAP), 7)
${info INFO: compiling with CoAP-08}
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

APPS   += erbium

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
