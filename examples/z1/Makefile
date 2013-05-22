ifndef TARGET
TARGET=z1
endif

CONTIKI_PROJECT = test-phidgets blink test-adxl345 test-tmp102 test-light-ziglet test-battery test-sht11 test-relay-phidget test-tlc59116 #test-potent
CONTIKI_SOURCEFILES += sht11.c# potentiometer-sensor.c
APPS=serial-shell


all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
