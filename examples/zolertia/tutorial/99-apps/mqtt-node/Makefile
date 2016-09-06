DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-client

# Platform options: relayr, thingsio, bluemix
MQTT_PLATFORM ?= bluemix

# Sensor options: fridge, agriculture, remote, irrigation
MQTT_SENSORS  ?= remote

# Don't touch anything below if you don't know what you are doing
PROJECT_SOURCEFILES += httpd-simple.c mqtt-sensors.c

# Platform checks
ifeq ($(MQTT_PLATFORM), bluemix)
  PROJECT_SOURCEFILES += bluemix.c
endif

ifeq ($(MQTT_PLATFORM), relayr)
  PROJECT_SOURCEFILES += relayr.c
endif

ifeq ($(MQTT_PLATFORM), thingsio)
  PROJECT_SOURCEFILES += thingsio.c
endif

# Sensor checks
ifeq ($(MQTT_SENSORS), fridge)
  PROJECT_SOURCEFILES += fridge.c
  CONTIKI_TARGET_SOURCEFILES += sht25.c
  CFLAGS += -DMQTT_SENSORS="fridge"
  CFLAGS += -DDEFAULT_SENSORS_NUM=2
endif

ifeq ($(MQTT_SENSORS), agriculture)
  PROJECT_SOURCEFILES += agriculture.c
  CONTIKI_TARGET_SOURCEFILES += sht25.c tsl2563.c bmpx8x.c
  CFLAGS += -DMQTT_SENSORS="agriculture"
  CFLAGS += -DDEFAULT_SENSORS_NUM=4
endif

ifeq ($(MQTT_SENSORS), irrigation)
  PROJECT_SOURCEFILES += irrigation.c
  CONTIKI_TARGET_SOURCEFILES += soil-humidity.c grove-relay.c
  CFLAGS += -DMQTT_SENSORS="irrigation"
  CFLAGS += -DDEFAULT_SENSORS_NUM=1
  CFLAGS += -DDEFAULT_COMMANDS_NUM=1
endif

ifeq ($(MQTT_SENSORS), remote)
  PROJECT_SOURCEFILES += remote.c
  CONTIKI_TARGET_SOURCEFILES += adc-sensors.c
  CFLAGS += -DMQTT_SENSORS="remote"
  CFLAGS += -DDEFAULT_SENSORS_NUM=5
endif

WITH_LLSEC ?= 0
CFLAGS += -DWITH_LLSEC_ENABLED=$(WITH_LLSEC)

WITH_IP64 = 1

CFLAGS += -DMQTT_PLATFORM=$(MQTT_PLATFORM)

APPS += mqtt

CONTIKI = ../../../../..
include $(CONTIKI)/Makefile.include
