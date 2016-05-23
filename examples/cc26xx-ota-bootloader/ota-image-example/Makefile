TARGET=srf06-cc26xx
#BOARD=sensortag/cc2650
BOARD=srf06/cc26xx

CONTIKI_PROJECT = ota-image-example

CONTIKI = ../../..
MODULES += core/net/http-socket
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

OTA=1
CFLAGS += -g -DDEBUG=1

PROJECT_SOURCEFILES += ota.c ota-download.c
CONTIKI_WITH_IPV6 = 1


clean:
	-rm -f *~ *core core *.srec \
	*.lst *.map \
	*.cprg *.bin *.elf *.hex *.data contiki*.a *.firmware core-labels.S *.ihex *.ini \
	*.ce *.co
	rm -rf $(CLEAN)
	-rm -rf $(OBJECTDIR)

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
