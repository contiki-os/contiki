ifndef TARGET
TARGET=z1
endif

CONTIKI_PROJECT = test-phidgets blink test-adxl345 tmp102-test test-battery
CONTIKI_SOURCEFILES += cc2420-arch.c
PROJECT_SOURCEFILES = i2cmaster.c tmp102.c adxl345.c battery-sensor.c sky-sensors.c
APPS=serial-shell


all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
