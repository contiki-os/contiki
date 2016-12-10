CONTIKI_PROJECT = slip-radio
all: $(CONTIKI_PROJECT)
APPS = slip-cmd

ifeq ($(TARGET),)
  -include Makefile.target
endif

CONTIKI = ../../../../../../..

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-net.c no-framer.c

CONTIKI_WITH_RPL = 0
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
