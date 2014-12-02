CONTIKI_PROJECT = tests
all: $(CONTIKI_PROJECT)

CONTIKI = ../../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

#linker optimizations
SMALL=1

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
