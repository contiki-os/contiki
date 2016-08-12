all: udp-stream
APPS = servreg-hack
CONTIKI = ../..

ifndef TARGET
TARGET = sky
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
SMALL = 1

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
