CONTIKI = ../..
LIBMC1322X = ../../../libmc1322x

CONTIKI_PROJECT = econotag-flash-test
TARGET = econotag
CLEAN = *.d $(CONTIKI_PROJECT)_e_$(TARGET).bin $(CONTIKI_PROJECT)_e_$(TARGET).txt $(CONTIKI_PROJECT)_e_$(TARGET).pbm

all: $(CONTIKI_PROJECT) blast

CFLAGS += -DFLASH_CONF_B1=30
CFLAGS += -DFLASH_CONF_B2=10

APPS += flash

flash:
	$(LIBMC1322X)/tools/mc1322x-load \
	-f $(LIBMC1322X)/tests/flasher_$(TARGET).bin \
	-s $(CONTIKI_PROJECT)_e_$(TARGET).bin \
	-c 'sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset' \
	-t /dev/ttyUSB1 -l

clear:
	$(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

blast: $(CONTIKI)/tools/blaster/blaster
	$(CONTIKI)/tools/blaster/blaster $(CONTIKI_PROJECT).cfg

$(CONTIKI)/tools/blaster/blaster:	$(CONTIKI)/tools/blaster/blaster.c
	(cd $(CONTIKI)/tools/blaster && $(MAKE))

include $(CONTIKI)/Makefile.include
