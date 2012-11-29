CONTIKI_PROJECT = hello-world
TARGET=dc-dc_converter
all: $(CONTIKI_PROJECT)

# variable for root Makefile.include
WITH_UIP6=1
# for some platforms
UIP_CONF_IPV6=1
UIP_CONF_IPV6_CHECKS=1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

run: $(CONTIKI_PROJECT).$(TARGET)
	$(OBJCOPY) -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).ihex
	sudo lpc21isp -hex $(CONTIKI_PROJECT).ihex /dev/ttyUSB0 115200 12000

nuke: clean
	rm -f -R $(CONTIKI_PROJECT).$(TARGET)
	rm -f -R $(CONTIKI_PROJECT)-nosyms.$(TARGET)
	rm -f -R src_$(TARGET)
