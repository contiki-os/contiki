all: 03-udp-client

# Linker optimizations
SMALL = 1

# Includes the project-conf configuration file
CFLAGS += -DPROJECT_CONF_H=\"../project-conf.h\"

CONTIKI = ../../../../..

# This flag includes the IPv6 libraries
CONTIKI_WITH_IPV6 = 1

include $(CONTIKI)/Makefile.include
