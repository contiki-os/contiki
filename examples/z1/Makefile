ifndef TARGET
TARGET=z1
endif

CONTIKI_PROJECT = test-phidgets blink test-adxl345
CONTIKI_SOURCEFILES += cc2420-arch.c
PROJECT_SOURCEFILES = i2cmaster.c adxl345.c
APPS=serial-shell


all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
