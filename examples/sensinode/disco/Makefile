ifndef TARGET
TARGET=sensinode
endif

# Make absolutely certain that you specify your device here
DEFINES+=MODEL_N740

HAVE_BANKING=1
UIP_CONF_IPV6=1
UIP_CONF_RPL=1
OFFSET_FIRMWARE=1

CONTIKI_PROJECT = disco-example

all: $(CONTIKI_PROJECT) 

CONTIKI = ../../..

include $(CONTIKI)/Makefile.include
