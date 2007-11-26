
ifndef CONTIKI
CONTIKI = ../..
endif

zip: all jar
	rm -rf contiki-energest-demo
	mkdir contiki-energest-demo
	cp nodeid/*.ihex contiki-energest-demo
	cp energest-demo.ihex contiki-energest-demo
	cp README contiki-energest-demo
	cp -r handouts contiki-energest-demo/
	mkdir contiki-energest-demo/tools
	cp -r $(CONTIKI)/tools/sky contiki-energest-demo/tools
	cp dist/*.jar contiki-energest-demo
	cp Makefile.contiki-energest-demo contiki-energest-demo/Makefile
	zip -r contiki-energest-demo.zip contiki-energest-demo

all: burn-nodeids javaapp energest-demo.ihex

upload:	energest-demo.upload

burn-nodeids:
	(cd nodeid; $(MAKE))

rundemo: zip
	cd contiki-energest-demo; make rundemo

jar:
	ant jar

javaapp:
	ant compile

# Ensure that ./contiki-conf.h is included
CFLAGS+=-I.

ifndef TARGET
TARGET=sky
endif

ifdef UPDATE
CFLAGS += -DUPDATE_TICKS=CLOCK_SECOND*$(UPDATE)
else
CFLAGS += -DUPDATE_TICKS=CLOCK_SECOND
endif

ifdef CHANNEL
CFLAGS += -DRF_CHANNEL=$(CHANNEL)
else
CFLAGS += -DRF_CHANNEL=20
endif


include $(CONTIKI)/Makefile.include
