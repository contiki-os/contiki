CONTIKI_PROJECT = hello-world
TARGET=lpc17xx
all: $(CONTIKI_PROJECT)

#UIP_CONF_IPV6=1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

run: $(CONTIKI_PROJECT).$(TARGET)
	$(OBJCOPY) -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).ihex
	sudo lpc21isp -hex $(CONTIKI_PROJECT).ihex /dev/ttyUSB0 38400 12000
