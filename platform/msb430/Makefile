# $Id: Makefile,v 1.1 2007/06/28 12:49:57 nvt-se Exp $

all: core
upload: core core.u

core:
	$(MAKE) core.firmware && \
	$(MAKE) core.firmware && \
	$(MAKE) core.firmware

CONTIKI=../..
TARGET=msb430

blinker.cm: blinker.co blinker2.co

ifdef ENABLE_BACNET
APPS += bacnet
APPDIRS += apps/bacnet
endif

include $(CONTIKI)/Makefile.include
