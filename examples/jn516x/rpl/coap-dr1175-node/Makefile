CONTIKI_PROJECT = dr1175-node
all: $(CONTIKI_PROJECT)

TARGET ?= jn516x
JN516x_WITH_DR1175 = 1

CONTIKI=../../../..

CONTIKI_WITH_IPV6 = 1

PROJECTDIRS += .. ../tools
PROJECT_SOURCEFILES += rpl-tools.c
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CFLAGS += -DWITH_COAP
CFLAGS += -DUIP_CONF_TCP=0
APPS = json
APPS += er-coap
APPS += rest-engine

include $(CONTIKI)/Makefile.include
