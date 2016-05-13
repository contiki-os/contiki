TARGET ?= sky

#CONTIKI_PROJECT = pdr
CONTIKI_PROJECT = pdr

CONTIKI_TARGET_SOURCEFILES += sampling_process.c

ifeq ($(TARGET),sky)
CONTIKI_TARGET_SOURCEFILES +=  temperature-sensor.c
endif

all: $(CONTIKI_PROJECT)

# use RIME, not IP
CONTIKI_WITH_RIME=1

ifeq ($(MANY_NODES),1)
CFLAGS += -DMANY_NODES=1
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DCOSYNCH_CONF_ENABLED

# to build a hex file
hex: all
ifeq ($(TARGET),avr-rss2)
	$(OBJCOPY) pdr.$(TARGET) -j .text -j .data -O ihex pdr.$(TARGET).hex
else
	$(OBJCOPY) pdr.$(TARGET) -O ihex pdr.$(TARGET).hex
	$(OBJCOPY) stringtest.$(TARGET) -O ihex stringtest.$(TARGET).hex
endif

scp:
	scp pdr.$(TARGET).hex atis@194.17.12.253:/home/atis/images

# the default for profun is zero
CONST_SERIAL_LINE_CONF_BUFSIZE=128
CFLAGS += -DSERIAL_LINE_CONF_BUFSIZE=128

# required for Zolertia Z1
CFLAGS += -DUSE_FAST_UART=1

# we do not want to use extended memory (in case the old bootloader fails to handle it)
TARGET_MEMORY_MODEL=small

ifeq ($(TARGET),avr-rss2)
CONTIKI_TARGET_SOURCEFILES += ds18b20.c
CONTIKI = ../../../../
else
CONTIKI = ../../../../
endif
include $(CONTIKI)/Makefile.include

