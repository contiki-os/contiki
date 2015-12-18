all: udp-client udp-server
APPS+=servreg-hack
CONTIKI=../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

APPS+= powertrace
ifdef WITH_COMPOWER
CFLAGS+= -DCONTIKIMAC_CONF_COMPOWER=1 -DWITH_COMPOWER=1 -DQUEUEBUF_CONF_NUM=4
endif

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
