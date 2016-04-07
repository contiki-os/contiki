DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: fridge-node driver-node rfid-node

CONTIKI_TARGET_SOURCEFILES += sht25.c rgb-bl-lcd.c

CONTIKI_WITH_IPV6 = 1

APPS += mqtt

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
