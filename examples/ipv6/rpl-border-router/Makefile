all: border-router

CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1

APPS += webserver

CFLAGS += -DPROJECT_CONF_H=\"project-router-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c httpd-simple.c

include $(CONTIKI)/Makefile.include
