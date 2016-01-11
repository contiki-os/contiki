DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = zoul-demo test-tsl2563 test-sht25 test-pwm test-power-mgmt
CONTIKI_PROJECT += test-bmp085 test-motion
CONTIKI_TARGET_SOURCEFILES += tsl2563.c sht25.c bmp085.c motion-sensor.c

all: $(CONTIKI_PROJECT)

CONTIKI = ../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
