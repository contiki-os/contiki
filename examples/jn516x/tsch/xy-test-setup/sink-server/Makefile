all: sink-server
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

TARGET = jn516x
JN516x_WITH_DR1174 = 1


CONTIKI_WITH_IPV6 = 1

MODULES += core/net/mac/tsch

PROJECTDIRS += ..  ../tools 
PROJECT_SOURCEFILES += rich.c 


CONTIKI = ../../../../..
include $(CONTIKI)/Makefile.include
