CONTIKI_PROJECT = testv6
all: $(CONTIKI_PROJECT)

APPS = webserver
CFLAGS = -DUIP_CONF_IPV6

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

testv6.$(TARGET): $(OBJECTDIR)/tapdev6.o #$(OBJECTDIR)/uip-keepalive.o
