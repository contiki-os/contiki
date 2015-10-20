DEFINES=PROJECT_CONF_H=\"project-conf.h\"

ifndef TARGET
TARGET = openmote
endif

CONTIKI_PROJECT = collect-view-shell
all: $(CONTIKI_PROJECT)

APPS = serial-shell powertrace collect-view
CONTIKI = ../..

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
