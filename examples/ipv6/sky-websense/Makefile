all: sky-websense

CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1

APPS += webserver

CFLAGS += -DPROJECT_CONF_H=\"project-rpl-web-conf.h\"
PROJECTDIRS += ../rpl-border-router
PROJECT_SOURCEFILES += httpd-simple.c

include $(CONTIKI)/Makefile.include
