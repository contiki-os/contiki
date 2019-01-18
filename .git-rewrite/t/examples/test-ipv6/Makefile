all: testv6
APPS=webserver
DEFAULT_TARGET=minimal-net
CFLAGS=-DUIP_CONF_IPV6

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

testv6.$(TARGET): $(OBJECTDIR)/tapdev6.o #$(OBJECTDIR)/uip-keepalive.o
