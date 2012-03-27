ifndef TARGET
TARGET=sensinode
endif

# Make absolutely certain that you specify your device here
DEFINES+=MODEL_N740,PROJECT_CONF_H

PROJECT_SOURCEFILES += stub-rdc.c

CONTIKI_PROJECT = energy-scan

all: $(CONTIKI_PROJECT)

CONTIKI = ../../..

include $(CONTIKI)/Makefile.include
