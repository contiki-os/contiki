# $Id: Makefile,v 1.3 2007/08/17 13:10:16 nvt-se Exp $

all: test-abc
upload: test-abc test-abc.u

test-abc:
	$(MAKE) test-abc.firmware && \
	$(MAKE) test-abc.firmware && \
	$(MAKE) test-abc.firmware

CONTIKI=../..
TARGET=msb430

include $(CONTIKI)/Makefile.include
