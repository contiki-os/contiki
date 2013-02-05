all: sky-websense

CONTIKI=../..

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

PING_SERVER ?= 0

ifeq ($(PING_SERVER),1)
APPS += ping6
endif
CFLAGS += -DPING_SERVER=$(PING_SERVER)

APPS += webbrowser
SMALL=1
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_SOURCEFILES += udp-client.c wget.c httpd-simple.c

include $(CONTIKI)/Makefile.include
