all: node

APPS = serial-shell

CONTIKI = ../../../..
CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL = 0

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

ifeq ($(TSCH_NODE), 1)
MODULES += core/net/mac/tsch
CFLAGS += -DWITH_TSCH=1 -DWITH_IEEE802154E_2012=1
endif

ifeq ($(IEEE802154E_2012_NODE), 1)
CFLAGS += -DWITH_IEEE802154E_2012=1
endif

ifeq ($(RPL), 1)
CONTIKI_WITH_RPL = 1
endif

include $(CONTIKI)/Makefile.include
