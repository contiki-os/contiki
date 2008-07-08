# $Id: Makefile,v 1.1 2008/07/08 11:21:53 nifi Exp $

all: core
upload: core core.u

core:
	$(MAKE) core.firmware && \
	$(MAKE) core.firmware && \
	$(MAKE) core.firmware

CONTIKI=../..
TARGET=esb

blinker.cm: blinker.co blinker2.co

ifdef ENABLE_BACNET
APPS += bacnet
APPDIRS += apps/bacnet
endif

include $(CONTIKI)/Makefile.include
