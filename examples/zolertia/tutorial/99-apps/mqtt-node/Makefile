DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-client

# Platform options: relayr, thingsio, bluemix, ubidots
MQTT_PLATFORM ?= bluemix

# Sensor options: fridge, agriculture, remote, watering, coldchain, coinbox, flowerpot
MQTT_SENSORS  ?= remote

# -----------------------------------------------------------------------------#
# Don't touch anything below if you don't know what you are doing
# -----------------------------------------------------------------------------#
CLOUD_RESOURCES_DIR = ./cloud
DEVICE_RESOURCES_DIR = ./device
INTERNALS_RESOURCES_DIR = ./internals
PROJECTDIRS += $(CLOUD_RESOURCES_DIR) $(DEVICE_RESOURCES_DIR) $(INTERNALS_RESOURCES_DIR)

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

ifeq ($(MQTT_PLATFORM), ubidots)
  PROJECT_SOURCEFILES += ubidots.c
endif

# Sensor checks
ifeq ($(MQTT_SENSORS), fridge)
  PROJECT_SOURCEFILES += fridge.c
  CONTIKI_TARGET_SOURCEFILES += sht25.c
  CFLAGS += -DMQTT_SENSORS="fridge"
  CFLAGS += -DDEFAULT_SENSORS_NUM=2
endif

ifeq ($(MQTT_SENSORS), coldchain)
  PROJECT_SOURCEFILES += coldchain.c
  CONTIKI_TARGET_SOURCEFILES += dht22.c tsl256x.c
  CFLAGS += -DMQTT_SENSORS="coldchain"
  CFLAGS += -DDEFAULT_SENSORS_NUM=3
endif

ifeq ($(MQTT_SENSORS), agriculture)
  PROJECT_SOURCEFILES += agriculture.c
  CONTIKI_TARGET_SOURCEFILES += sht25.c tsl256x.c bmpx8x.c
  CFLAGS += -DMQTT_SENSORS="agriculture"
  CFLAGS += -DDEFAULT_SENSORS_NUM=4
endif

ifeq ($(MQTT_SENSORS), watering)
  PROJECT_SOURCEFILES += watering.c
  CONTIKI_TARGET_SOURCEFILES += relay.c adc-sensors.c
  CFLAGS += -DMQTT_SENSORS="watering"
  CFLAGS += -DDEFAULT_SENSORS_NUM=1
  CFLAGS += -DDEFAULT_COMMANDS_NUM=1
endif

ifeq ($(MQTT_SENSORS), remote)
  PROJECT_SOURCEFILES += remote.c
  CONTIKI_TARGET_SOURCEFILES += adc-sensors.c
  CFLAGS += -DMQTT_SENSORS="remote"
  CFLAGS += -DDEFAULT_SENSORS_NUM=5
endif

ifeq ($(MQTT_SENSORS), coinbox)
  PROJECT_SOURCEFILES += coinbox.c
  CONTIKI_TARGET_SOURCEFILES += mp3-wtv020sd.c led-strip.c
  CFLAGS += -DMQTT_SENSORS="coinbox"
  CFLAGS += -DDEFAULT_COMMANDS_NUM=1
endif

ifeq ($(MQTT_SENSORS), flowerpot)
  PROJECT_SOURCEFILES += flowerpot.c
  CONTIKI_TARGET_SOURCEFILES += sht25.c adc-sensors.c
  CFLAGS += -DMQTT_SENSORS="flowerpot"
  CFLAGS += -DDEFAULT_SENSORS_NUM=3
endif


WITH_LLSEC ?= 1
CFLAGS += -DWITH_LLSEC_ENABLED=$(WITH_LLSEC)

WITH_IP64 = 1

CFLAGS += -DMQTT_PLATFORM=$(MQTT_PLATFORM)

APPS += mqtt

# MQTT internals
PROJECT_SOURCEFILES += mqtt-sensors.c mqtt-res.c

# Alternatively includes the webserver to configure credentials
WITH_WEBSERVER ?= 1
ifeq ($(WITH_WEBSERVER), 1)
  PROJECT_SOURCEFILES += httpd-simple.c
endif

CFLAGS += -DWITH_WEBSERVER=$(WITH_WEBSERVER)

CONTIKI = ../../../../..
include $(CONTIKI)/Makefile.include
