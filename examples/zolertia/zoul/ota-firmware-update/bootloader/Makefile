DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: bootloader

PROJECTDIRS += ./res
PROJECT_SOURCEFILES += res-firmware.c

WITH_IP64 = 1

SMALL=1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

CONTIKI = ../../../../..
include $(CONTIKI)/Makefile.include
