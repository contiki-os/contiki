all: 

CFLAGS  += -D PROJECT_CONF_H=\"project-conf.h\"
APPS    += unit-test
MODULES += core/net/mac/tsch core/net/mac/tsch/sixtop

PROJECT_SOURCEFILES += common.c

ifeq ($(TEST_04),1)
CFLAGS  += -DSIXP_MSG_API_TEST=1
endif

CONTIKI = ../../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
