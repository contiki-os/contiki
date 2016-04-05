DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-demo

CONTIKI_TARGET_SOURCEFILES += sht25.c

CONTIKI_WITH_IPV6 = 1

APPS += mqtt

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
