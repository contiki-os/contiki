CONTIKI_PROJECT = er-example-server
TARGET=dc-dc_converter
all: $(CONTIKI_PROJECT)

CONTIKI=../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# variable for root Makefile.include
WITH_UIP6=1
# for some platforms
UIP_CONF_IPV6=1
UIP_CONF_IPV6_CHECKS=1

# variable for this Makefile
# configure CoAP implementation (3|7) (er-coap-07 also supports CoAP draft 08)
WITH_COAP=7

# REST framework, requires WITH_COAP
ifeq ($(WITH_COAP), 7)
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

APPS += erbium

include $(CONTIKI)/Makefile.include

run: $(CONTIKI_PROJECT).$(TARGET)
	$(OBJCOPY) -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).ihex
	sudo lpc21isp -hex $(CONTIKI_PROJECT).ihex /dev/ttyUSB0 115200 12000

nuke: clean
	rm -f -R $(CONTIKI_PROJECT).$(TARGET)
	rm -f -R $(CONTIKI_PROJECT)-nosyms.$(TARGET)
	rm -f -R src_$(TARGET)
