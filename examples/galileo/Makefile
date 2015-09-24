KNOWN_EXAMPLES = gpio-output

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
