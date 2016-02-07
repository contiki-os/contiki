CONTIKI_PROJECT = dongle-node
all: $(CONTIKI_PROJECT)

TARGET ?= jn516x
JN516x_WITH_DONGLE = 1

CONTIKI=../../../..

CONTIKI_WITH_IPV6 = 1

MODULES += core/net/mac/tsch
PROJECTDIRS += .. ../tools
PROJECT_SOURCEFILES += rpl-tools.c
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CFLAGS += -DWITH_COAP
CFLAGS += -DUIP_CONF_TCP=0
APPS = json
APPS += er-coap
APPS += rest-engine

include $(CONTIKI)/Makefile.include
