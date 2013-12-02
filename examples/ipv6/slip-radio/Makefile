CONTIKI_PROJECT=slip-radio
all: $(CONTIKI_PROJECT)
APPS = slip-cmd

ifeq ($(TARGET),)
  -include Makefile.target
endif

CONTIKI=../../..

UIP_CONF_IPV6=1
UIP_CONF_RPL=0

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-net.c no-framer.c
ifeq ($(TARGET),sky)
  PROJECT_SOURCEFILES += slip-radio-cc2420.c slip-radio-sky-sensors.c
endif
ifeq ($(TARGET),nooliberry)
  PROJECT_SOURCEFILES += slip-radio-rf230.c
endif
ifeq ($(TARGET),econotag)
  PROJECT_SOURCEFILES += slip-radio-mc1322x.c
endif

include $(CONTIKI)/Makefile.include
