CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

all: coap-client coap-server
APPS=servreg-hack
CONTIKI=../../../..

ifdef WITH_COMPOWER
APPS+=powertrace
CFLAGS+= -DCONTIKIMAC_CONF_COMPOWER=1 -DWITH_COMPOWER=1 -DQUEUEBUF_CONF_NUM=4
endif

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif

### -------------------------------------------------------- ###

#CONTIKI_TARGET_SOURCEFILES += control.c vdc.c

# automatically build RESTful resources
REST_RESOURCES_DIR = ./resources
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))

DC_SENSOR_DIR = ./dev
DC_SENSOR_FILES = $(notdir $(shell find $(DC_SENSOR_DIR) -name '*.c'))

PROJECTDIRS += $(REST_RESOURCES_DIR) $(DC_SENSOR_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES) $(DC_SENSOR_FILES)

# linker optimizations
SMALL=1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

#CFLAGS += -DUIP_CONF_BUFFER_SIZE=384
CFLAGS += -DREST_MAX_CHUNK_SIZE=128
CFLAGS += -DCOAP_MAX_HEADER_SIZE=64

CFLAGS += -DUIP_CONF_TCP=0

### -------------------------------------------------------- ###

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
