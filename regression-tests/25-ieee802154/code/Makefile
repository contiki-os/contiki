all: test-panid-handling test-tcsh-create-packet

APPS    += unit-test
MODULES += core/net/mac/tsch
CFLAGS  += -D PROJECT_CONF_H=\"project-conf.h\"

WITH_TSCH        ?= 0
TEST_CONFIG_TYPE ?= DEFAULT

CFLAGS += -D WITH_TSCH=1

ifeq ($(TEST_CONFIG_TYPE), SECURITY_ON)
CFLAGS += -D WITH_SECURITY_ON=1
endif

ifeq ($(TEST_CONFIG_TYPE), ALL_ENABLED)
CFLAGS += -D WITH_ALL_ENABLED=1
endif

CONTIKI = ../../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
