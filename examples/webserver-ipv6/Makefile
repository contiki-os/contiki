CONTIKI_PROJECT=webserver6
all: $(CONTIKI_PROJECT) $(WITH_WEBSERVER)

ifeq ($(WITH_WEBSERVER),)
#
#Set default webserver for different platforms
#Although Makefile.include sets the default target, we need to know it now.
#
ifeq ($(TARGET),)
  -include Makefile.target
endif
ifeq ($(TARGET),sky)
SMALL=1
WITH_WEBSERVER=webserver-nano
else ifeq ($(TARGET),avr-raven)
WITH_WEBSERVER=raven-webserver
endif
#
endif

ifneq ($(WITH_WEBSERVER),)
APPS = $(WITH_WEBSERVER)
ifeq ($(WITH_WEBSERVER),raven-webserver)
APPS+=raven-lcd-interface
endif
#copy output to e.g. webserver-nano.sky, raven-webserver.avr-raven
$(WITH_WEBSERVER) : $(CONTIKI_PROJECT)
	cp $(CONTIKI_PROJECT).$(TARGET) $(WITH_WEBSERVER).$(TARGET)
	@if (test -n "$(ELF_SIZE)");then $(ELF_SIZE) $(WITH_WEBSERVER).$(TARGET);fi
else
APPS=webserver

all : $(CONTIKI_PROJECT)
	@if (test -n "$(ELF_SIZE)");then $(ELF_SIZE) $(CONTIKI_PROJECT).$(TARGET);fi	
endif

UIP_CONF_IPV6=1
DEFINES=WITH_UIP6

# Make no RPL the default for minimal-net builds
ifeq ($(TARGET),minimal-net)
ifndef UIP_CONF_RPL
UIP_CONF_RPL=0
endif
endif

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
