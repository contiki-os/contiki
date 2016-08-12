DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI_PROJECT = zoul-demo test-tsl2563 test-sht25 test-power-mgmt
CONTIKI_PROJECT += test-bmp085-bmp180 test-motion test-rotation-sensor
CONTIKI_PROJECT += test-grove-light-sensor test-grove-loudness-sensor
CONTIKI_PROJECT += test-weather-meter test-grove-gyro test-lcd test-iaq
CONTIKI_PROJECT += test-pm10-sensor test-vac-sensor test-aac-sensor
CONTIKI_PROJECT += test-zonik test-dht22.c

CONTIKI_TARGET_SOURCEFILES += tsl2563.c sht25.c bmpx8x.c motion-sensor.c
CONTIKI_TARGET_SOURCEFILES += adc-sensors.c weather-meter.c grove-gyro.c
CONTIKI_TARGET_SOURCEFILES += rgb-bl-lcd.c pm10-sensor.c iaq.c zonik.c relay.c
CONTIKI_TARGET_SOURCEFILES += dht22.c

all: $(CONTIKI_PROJECT)

CONTIKI = ../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
