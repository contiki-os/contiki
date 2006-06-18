# $Id: Makefile,v 1.1 2006/06/18 05:54:28 adamdunkels Exp $

all: core
upload: core core.u

core:
	$(MAKE) core.firmware && \
	$(MAKE) core.firmware && \
	$(MAKE) core.firmware

CONTIKI=../..
TARGET=esb


ifdef ENABLE_BACNET
APPS += bacnet
APPDIRS += apps/bacnet
endif

include $(CONTIKI)/Makefile.include
