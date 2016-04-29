DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-demo

CONTIKI_TARGET_SOURCEFILES += sht25.c

# Enable if using IPV6 only
CONTIKI_WITH_IPV6 = 1

# Enable the following if using NAT64 address translation
# WITH_IP64 = 1

APPS += mqtt

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
