all: udp-stream
APPS = servreg-hack
CONTIKI = ../..

ifndef TARGET
TARGET = sky
endif

WITH_UIP6 = 1
UIP_CONF_IPV6 = 1
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
SMALL = 1

include $(CONTIKI)/Makefile.include
