CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

TARGET ?= jn516x

CONTIKI=../../../..

CONTIKI_WITH_IPV6 = 1

MODULES += core/net/mac/tsch
PROJECTDIRS += .. ../tools
PROJECT_SOURCEFILES += rpl-tools.c
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
