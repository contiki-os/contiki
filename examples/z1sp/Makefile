ifndef TARGET
TARGET=z1sp
endif

CONTIKI_PROJECT = test-potentiometer
CONTIKI_SOURCEFILES += cc2420-arch.c
PROJECT_SOURCEFILES = sky-sensors.c potentiometer-sensor.c
APPS=serial-shell


all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
