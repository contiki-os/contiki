CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

TARGET ?= jn516x
JN516x_WITH_DR1174 = 1

CONTIKI=../../../..

CONTIKI_WITH_IPV6 = 1

PROJECTDIRS += .. ../tools
PROJECT_SOURCEFILES += rpl-tools.c
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
