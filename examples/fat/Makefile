DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI = ../..

all: example-fat

CONTIKI_WITH_RIME = 1

ifeq ($(TARGET), zoul)
  BOARD ?= remote-revb
endif

include $(CONTIKI)/Makefile.include
