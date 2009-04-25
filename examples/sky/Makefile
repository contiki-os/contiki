CONTIKI = ../..
ifndef TARGET
TARGET=sky
endif

all: blink sky-collect #rt-leds test-button test-cfs tcprudolph0

%.tgz: %.ihex
	mkdir $(basename $<) ; \
	mv $< $(basename $<) ; \
	echo $(basename $<)/$(basename $<).ihex 600 > $(basename $<)/runfile ; \
	tar czf $@ $(basename $<)

%.class: %.java
	javac $(basename $<).java

viewrssi: ViewRSSI.class
	make login | java ViewRSSI

include $(CONTIKI)/Makefile.include
