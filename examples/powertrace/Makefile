DEFINES=PROJECT_CONF_H=\"project-conf.h\"

ifndef TARGET
TARGET = openmote
endif

CONTIKI_PROJECT = example-powertrace
APPS+=powertrace
all: $(CONTIKI_PROJECT)

CONTIKI = ../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
