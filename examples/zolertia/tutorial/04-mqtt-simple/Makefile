DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-example

APPS += mqtt

# Linker size optimization
SMALL = 1

# Use IPv6 only
CONTIKI_WITH_IPV6 = 1

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
