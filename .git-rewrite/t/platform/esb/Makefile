# $Id: Makefile,v 1.2 2006/06/20 21:23:10 adamdunkels Exp $

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
