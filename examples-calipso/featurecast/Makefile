all:  node sink 
APPS=servreg-hack
CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1
PROJECT_SOURCEFILES += labels.c
CFLAGS+= -DUIP_CONF_IPV6_RPL
#CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"


include $(CONTIKI)/Makefile.include
