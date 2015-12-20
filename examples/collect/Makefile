DEFINES=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI_PROJECT = collect-view-shell
all: $(CONTIKI_PROJECT)

MAKE_WITH_ORCHESTRA ?= 0 # force Orchestra from command line
MAKE_WITH_SECURITY ?= 0 # force Security from command line

APPS += orchestra

ifeq ($(MAKE_WITH_ORCHESTRA),1)
CFLAGS += -DWITH_ORCHESTRA=1
endif

ifeq ($(MAKE_WITH_SECURITY),1)
CFLAGS += -DWITH_SECURITY=1
endif

APPS = serial-shell powertrace collect-view
CONTIKI = ../..

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
