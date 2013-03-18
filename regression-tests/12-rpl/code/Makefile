all: sender-node receiver-node root-node
CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

CFLAGS+=-DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
