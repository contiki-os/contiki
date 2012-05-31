ifndef TARGET
TARGET=sensinode
endif

# Make absolutely certain that you specify your device here
DEFINES+=MODEL_N740,PROJECT_CONF_H

# This example won't fit in flash without banking so we turn it on
HAVE_BANKING=1
UIP_CONF_IPV6=1
UIP_CONF_RPL=1

CONTIKI_SOURCEFILES += ping6.c

CONTIKI_PROJECT = client server
all: $(CONTIKI_PROJECT) 

CONTIKI = ../../..

include $(CONTIKI)/Makefile.include
