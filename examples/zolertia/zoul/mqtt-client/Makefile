DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-client

# User configuration parameters
MQTT_PLATFORM = relayr
MQTT_SENSORS  = fridge

# Don't touch anything below if you don't know what you are doing :-)
PROJECT_SOURCEFILES += httpd-simple.c

ifeq ($(MQTT_PLATFORM), relayr)
  PROJECT_SOURCEFILES += relayr.c
  CFLAGS += -DMQTT_PLATFORM="relayr"
endif

ifeq ($(MQTT_SENSORS), fridge)
  PROJECT_SOURCEFILES += fridge.c
  CONTIKI_TARGET_SOURCEFILES += sht25.c
  CFLAGS += -DMQTT_SENSORS="fridge"
endif

WITH_IP64 = 1

APPS += mqtt

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
