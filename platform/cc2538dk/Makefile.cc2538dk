# cc2538dk platform makefile

ifndef CONTIKI
  $(error CONTIKI not defined! You must specify where CONTIKI resides!)
endif

CONTIKI_TARGET_DIRS = . dev

CONTIKI_TARGET_SOURCEFILES += leds.c leds-arch.c
CONTIKI_TARGET_SOURCEFILES += contiki-main.c
CONTIKI_TARGET_SOURCEFILES += sensors.c smartrf-sensors.c
CONTIKI_TARGET_SOURCEFILES += button-sensor.c als-sensor.c

CONTIKI_SOURCEFILES += $(CONTIKI_TARGET_SOURCEFILES)

CLEAN += *.cc2538dk

### Unless the example dictates otherwise, build with code size optimisations
ifndef SMALL
  SMALL = 1
endif

### Define the CPU directory
CONTIKI_CPU=$(CONTIKI)/cpu/cc2538
include $(CONTIKI_CPU)/Makefile.cc2538

MODULES += core/net core/net/mac \
           core/net/mac/contikimac \
           core/net/llsec core/net/llsec/noncoresec

PYTHON = python
BSL_FLAGS += -e -w -v

ifdef PORT
  BSL_FLAGS += -p $(PORT)
endif

BSL = $(CONTIKI)/tools/cc2538-bsl/cc2538-bsl.py

%.upload: %.bin %.elf
ifeq ($(wildcard $(BSL)), )
	@echo "ERROR: Could not find the cc2538-bsl script. Did you run 'git submodule update --init' ?"
else
	$(eval BSL_ADDRESS_ARG := -a $(shell $(OBJDUMP) -h $*.elf | grep -B1 LOAD | \
	                               grep -Ev 'LOAD|\-\-' | awk '{print "0x" $$5}' | \
	                               sort -g | head -1))
	$(PYTHON) $(BSL) $(BSL_FLAGS) $(BSL_ADDRESS_ARG) $<
endif
