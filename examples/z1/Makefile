ifndef TARGET
TARGET=z1
endif

# Enable to pull-in Z1SP specific test/source files
ZOLERTIA_Z1SP=0

CONTIKI_PROJECT = test-phidgets blink test-adxl345 test-tmp102 test-light-ziglet test-battery test-sht11 test-relay-phidget test-tlc59116
CONTIKI_SOURCEFILES += sht11.c
APPS=serial-shell

ifeq ($(ZOLERTIA_Z1SP),1)
CONTIKI_PROJECT += test-potent
endif

all: $(CONTIKI_PROJECT)
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
