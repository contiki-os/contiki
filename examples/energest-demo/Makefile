
all: burn-nodeids javaapp energest-demo.ihex

burn-nodeids:
	(cd nodeid; $(MAKE))

javaapp:
	ant compile

ifndef CONTIKI
CONTIKI = ../..
endif

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


%.upload: %.ihex
	cp $< ../testbed-server/image.ihex
	(cd ../testbed-server; make upload)


include $(CONTIKI)/Makefile.include
