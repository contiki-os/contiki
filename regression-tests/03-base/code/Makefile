all: test-uip-ds6-route test-uip-ds6-nbr

CFLAGS  += -D PROJECT_CONF_H=\"project-conf.h\"
APPS    += unit-test

PROJECT_SOURCEFILES += common.c

ifeq ($(WITH_MULTI_IPV6_ADDRS), 1)
CFLAGS  += -D UIP_DS6_NBR_CONF_MULTI_IPV6_ADDRS=1
endif

CONTIKI = ../../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
