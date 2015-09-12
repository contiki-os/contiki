CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

CONTIKI=../../..

CONTIKI_WITH_IPV6 = 1
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

APPS += orchestra
MODULES += tsch

include $(CONTIKI)/Makefile.include
