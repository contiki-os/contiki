ifdef CONTIKI_HOME
CONTIKI = $(CONTIKI_HOME)
else
ifndef CONTIKI
CONTIKI=../../../../contiki-2.x
endif
endif

DEFINES=WITH_UIP6=1,WITH_SLIP=1,PROJECT_CONF_H=\"bridge-conf.h\"
UIP_CONF_IPV6=1

ifndef TARGET
TARGET=sky
endif

PROJECTDIRS = dev
PROJECT_SOURCEFILES = fakeuip.c sicslow_ethernet.c

all:	uip6-bridge-tap.sky

include $(CONTIKI)/Makefile.include
