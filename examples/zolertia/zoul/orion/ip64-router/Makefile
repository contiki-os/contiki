DEFINES += PROJECT_CONF_H=\"project-conf.h\"

all: ip64-router

BOARD = orion

CONTIKI = ../../../../..

WITH_IP64 = 1
CONTIKI_WITH_IPV6 = 1

DEFINES += UIP_CONF_TCP=1
PROJECT_SOURCEFILES += httpd-simple.c

include $(CONTIKI)/Makefile.include

