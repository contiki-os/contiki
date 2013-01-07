# To build for the cc2531 USB stick:
DEFINES+=MODEL_CC2531=1,

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

# We need uIPv6, therefore we also need banking
HAVE_BANKING=1
UIP_CONF_IPV6=1
UIP_CONF_RPL=1

PROJECT_SOURCEFILES += slip-bridge.c

CONTIKI_PROJECT = border-router

all: $(CONTIKI_PROJECT) 

CONTIKI = ../../..

include $(CONTIKI)/Makefile.include
