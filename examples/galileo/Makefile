TARGET=galileo

KNOWN_EXAMPLES = gpio-input gpio-output gpio-interrupt i2c-LSM9DS0 i2c-callbacks print-imr prot-domain-switch-latency

ifeq ($(filter $(EXAMPLE),$(KNOWN_EXAMPLES)),)
    $(info Set the variable EXAMPLE to one of the following Galileo-specific examples:)
    $(foreach EXAMPLE,$(KNOWN_EXAMPLES),$(info - $(EXAMPLE)))
    $(error Unable to proceed)
endif

ifeq ($(EXAMPLE),print-imr)
    CFLAGS += -DDBG_IMRS
endif

ifeq ($(EXAMPLE),prot-domain-switch-latency)
ifeq ($(SAMPLE_METADATA),1)
CFLAGS += -DSAMPLE_METADATA=1
endif
endif

CONTIKI_PROJECT = $(EXAMPLE)

all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
