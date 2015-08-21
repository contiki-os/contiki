CONTIKI_PROJECT = dongle-node
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

TARGET = jn516x
JN516x_WITH_DONGLE = 1

CONTIKI_WITH_IPV6 = 1

PROJECTDIRS += .. ../tools 
PROJECT_SOURCEFILES += rich.c 

CFLAGS += -DWITH_COAP
CFLAGS += -DUIP_CONF_TCP=0
APPS = orchestra
APPS += json
APPS += er-coap
APPS += rest-engine

MODULES += core/net/mac/tsch

all: $(CONTIKI_PROJECT)

CONTIKI=../../..
include $(CONTIKI)/Makefile.include
