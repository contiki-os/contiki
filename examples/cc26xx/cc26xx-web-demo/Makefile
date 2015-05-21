DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: cc26xx-web-demo

REST_RESOURCES_DIR = ./resources

REST_RESOURCES_FILES += res-leds.c res-toggle-leds.c res-device.c
REST_RESOURCES_FILES += res-sensors.c res-ble-advd.c

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

PROJECT_SOURCEFILES += cetic-6lbr-client.c coap-server.c net-uart.c mqtt-client.c
PROJECT_SOURCEFILES += httpd-simple.c

CONTIKI_WITH_IPV6 = 1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine
APPS += mqtt

CONTIKI=../../..
include $(CONTIKI)/Makefile.include
