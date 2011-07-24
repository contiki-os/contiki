all: webserver6 $(WITH_WEBSERVER)

ifeq ($(WITH_WEBSERVER),)
#
#Set default webserver for different platforms
#Although Makefile.include sets the default target, we need to know it now.
#
ifeq ($(TARGET),)
  -include Makefile.target
endif
ifeq ($(TARGET),sky)
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
$(WITH_WEBSERVER) : webserver6
	cp webserver6.$(TARGET) $(WITH_WEBSERVER).$(TARGET)
else
APPS=webserver
endif

UIP_CONF_IPV6=1
DEFINES=WITH_UIP6

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
