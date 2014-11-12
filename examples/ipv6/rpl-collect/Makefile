CONTIKI = ../../..
APPS = powertrace collect-view
CONTIKI_PROJECT = udp-sender udp-sink
PROJECT_SOURCEFILES += collect-common.c

ifdef PERIOD
CFLAGS=-DPERIOD=$(PERIOD)
endif

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
