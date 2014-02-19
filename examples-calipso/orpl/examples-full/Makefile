CONTIKI=../../..

CONTIKI_PROJECT = app-collect-only app-down-only app-any-to-any
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
TARGET=sky

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL
PROJECTDIRS += tools
PROJECT_SOURCEFILES += deployment.c simple-energest.c orpl-log.c

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include

