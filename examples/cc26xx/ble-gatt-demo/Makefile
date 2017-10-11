DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = ble-gatt-demo
all: $(CONTIKI_PROJECT)

CONTIKI_WITH_RPL = 0
CONTIKI_WITH_IPV6 = 0
MODULES += core/net/ble core/net/ble/gatt-services

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
