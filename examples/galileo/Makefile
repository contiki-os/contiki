TARGET=galileo

KNOWN_EXAMPLES = gpio-input gpio-output gpio-interrupt i2c-LSM9DS0

ifneq ($(filter $(EXAMPLE),$(KNOWN_EXAMPLES)),)
  CONTIKI_PROJECT = $(EXAMPLE)
else
  CONTIKI_PROJECT = help
endif

all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

help:
	@echo -e "\nSet the variable EXAMPLE to one of the following Galileo-specific examples:"
	@for EXAMPLE in $(KNOWN_EXAMPLES); do echo $$EXAMPLE; done
