CONTIKI_PROJECT = node

TARGET ?= jn516x
JN516x_WITH_DONGLE = 1

CONTIKI=../../../../..

CONTIKI_WITH_IPV6 = 1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECTDIRS += .. ../../tools
PROJECT_SOURCEFILES += rpl-tools.c
CFLAGS += -DWITH_COAP
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += orchestra
APPS += json
APPS += er-coap
APPS += rest-engine

MODULES += core/net/mac/tsch

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
