ifdef CONTIKI_HOME
CONTIKI = $(CONTIKI_HOME)
else
ifndef CONTIKI
CONTIKI=../../..
endif
endif

DEFINES=PROJECT_CONF_H=\"bridge-conf.h\"
UIP_CONF_IPV6=1

ifndef TARGET
TARGET=mb851
endif

PROJECTDIRS = dev
PROJECT_SOURCEFILES = fakeuip.c sicslow_ethernet.c slip.c

all:	uip6-bridge-tap

include $(CONTIKI)/Makefile.include
