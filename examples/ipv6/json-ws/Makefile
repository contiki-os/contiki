CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1
SMALL=1

PROJECT_SOURCEFILES += json-ws.c

ifdef WITH_COSM
 CFLAGS += -DWITH_COSM=1
endif

ifdef WITH_UDP
 CFLAGS += -DWITH_UDP=1
 PROJECT_SOURCEFILES += json-ws-udp.c
endif

APPS += httpd-ws json

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

ifeq ($(TARGET),)
 -include Makefile.target
endif

ifneq ($(TARGET),)
all: websense-$(TARGET)
endif

include $(CONTIKI)/Makefile.include
