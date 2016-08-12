all: udp-server udp-client

CONTIKI = ../..
CONTIKI_WITH_IPV6 = 1
CFLAGS += -DUIP_CONF_ND6_SEND_NA=1
include $(CONTIKI)/Makefile.include
