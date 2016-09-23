DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI_PROJECT = test-power-mgmt

BOARD ?= remote-revb

# Works in Linux and probably on OSX too (RTCC example)
CFLAGS = -DDATE="\"`date +"%02u %02d %02m %02y %02H %02M %02S"`\""

all: $(CONTIKI_PROJECT)

CONTIKI = ../../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
