all: broadcast-example unicast-sender unicast-receiver

CONTIKI=../../..
APPS=servreg-hack
APPS+=powertrace collect-view

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_WITH_IPV6 = 1

include $(CONTIKI)/Makefile.include
