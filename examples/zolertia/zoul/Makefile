DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = zoul-demo test-tsl2563 test-sht25 test-pwm test-power-mgmt
CONTIKI_PROJECT += test-bmp085 test-motion test-rotation-sensor
CONTIKI_PROJECT += test-grove-light-sensor test-grove-loudness-sensor
CONTIKI_TARGET_SOURCEFILES += tsl2563.c sht25.c bmp085.c motion-sensor.c
CONTIKI_TARGET_SOURCEFILES += adc-wrapper.c

all: $(CONTIKI_PROJECT)

CONTIKI = ../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
