all: tcp-server

# Linker optimizations
SMALL = 1

# Use the examples project configuration
CFLAGS += -DPROJECT_CONF_H=\"../project-conf.h\"

CONTIKI = ../../../../..

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
