ifdef CONTIKI_HOME
CONTIKI = $(CONTIKI_HOME)
else
ifndef CONTIKI
CONTIKI=../../..
endif
endif

DEFINES=PROJECT_CONF_H=\"bridge-conf.h\"

ifndef TARGET
TARGET=mbxxx
endif

PROJECTDIRS = dev
PROJECT_SOURCEFILES = fakeuip.c sicslow_ethernet.c slip.c

all:	uip6-bridge-tap

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
