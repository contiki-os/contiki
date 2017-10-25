TARGET=srf06-cc26xx
ifndef BOARD
BOARD=sensortag/cc2650
#BOARD=srf06/cc26xx
#BOARD=launchpad/cc1310
endif

CONTIKI = ../../..
CONTIKI_PROJECT = ota-image-example
PROJECTDIR?=.
APPDIRS += $(PROJECTDIR)/../apps
APPS += er-coap rest-engine ota
OTA=1

CONTIKI_WITH_IPV6 = 1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -g -DDEBUG=1

clean:
	-rm -f *~ *core core *.srec \
	*.lst *.map \
	*.cprg *.bin *.elf *.hex *.data contiki*.a *.firmware core-labels.S *.ihex *.ini \
	*.ce *.co
	rm -rf $(CLEAN)
	-rm -rf $(OBJECTDIR)

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
