all: border-router

CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1

CFLAGS += -DUIP_FALLBACK_INTERFACE=rpl_interface
PROJECT_SOURCEFILES += slip-bridge.c

include $(CONTIKI)/Makefile.include
