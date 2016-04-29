DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-relayr

CONTIKI_TARGET_SOURCEFILES += sht25.c

WITH_IP64 = 1

APPS += mqtt

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
