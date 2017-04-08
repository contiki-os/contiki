all: 

CFLAGS  += -D PROJECT_CONF_H=\"project-conf.h\"
APPS    += unit-test
MODULES += core/net/mac/tsch core/net/mac/tsch/sixtop

PROJECT_SOURCEFILES += common.c

CONTIKI = ../../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
