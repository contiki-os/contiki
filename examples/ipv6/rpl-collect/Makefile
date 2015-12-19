CONTIKI = ../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

APPS = powertrace collect-view
CONTIKI_PROJECT = udp-sender udp-sink
PROJECT_SOURCEFILES += collect-common.c

ifdef PERIOD
CFLAGS=-DPERIOD=$(PERIOD)
endif

APPS += orchestra

ifeq ($(MAKE_WITH_ORCHESTRA),1)
CFLAGS += -DWITH_ORCHESTRA=1
endif

ifeq ($(MAKE_WITH_SECURITY),1)
CFLAGS += -DWITH_SECURITY=1
endif

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
