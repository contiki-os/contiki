CONTIKI = ../../../..
APPS = bloom-filter dinas
CONTIKI_PROJECT = server-sink client

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

ifdef PERIOD
CFLAGS=-DPERIOD=$(PERIOD)
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include

