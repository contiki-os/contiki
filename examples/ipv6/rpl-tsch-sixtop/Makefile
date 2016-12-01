CONTIKI_PROJECT = node-sixtop
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += sf-simple.c
CONTIKI=../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI_WITH_IPV6 = 1
MAKE_WITH_SECURITY ?= 0 # force Security from command line

MODULES += core/net/mac/tsch
MODULES += core/net/mac/tsch/sixtop

ifeq ($(MAKE_WITH_SECURITY),1)
CFLAGS += -DWITH_SECURITY=1
endif

include $(CONTIKI)/Makefile.include
