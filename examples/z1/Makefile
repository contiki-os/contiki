ifndef TARGET
TARGET=z1
endif

CONTIKI_PROJECT = test-phidgets blink test-adxl345 tmp102-test test-battery test-sht11 #test-potent
CONTIKI_SOURCEFILES += sht11.c potentiometer-sensor.c
APPS=serial-shell


all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
