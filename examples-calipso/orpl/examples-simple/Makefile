CONTIKI=../../..

CONTIKI_PROJECT = app-collect-only
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
TARGET=sky

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
