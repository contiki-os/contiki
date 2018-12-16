CONTIKI_PROJECT = coord
all: $(CONTIKI_PROJECT)

MODULES += core/net/mac/tsch

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI=../../../..
include $(CONTIKI)/Makefile.include
