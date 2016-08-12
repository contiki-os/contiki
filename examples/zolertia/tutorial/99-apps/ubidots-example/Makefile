DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = ubidots-client

APPS = ubidots

CONTIKI_TARGET_SOURCEFILES += sht25.c

all: $(CONTIKI_PROJECT)

# If using an IPv6 connection
CONTIKI_WITH_IPV6 = 1

# If using a NAT64 address
# WITH_IP64 = 1

CONTIKI = ../../../../..

include $(CONTIKI)/Makefile.include
