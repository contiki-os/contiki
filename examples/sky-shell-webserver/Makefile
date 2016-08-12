CONTIKI_PROJECT = sky-shell-webserver
all: $(CONTIKI_PROJECT)
PROJECT_SOURCEFILES = webserver-nogui.c
HTTPD_CFS=1
CFLAGS = -DRESOLV_CONF_SUPPORTS_MDNS=0
DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver 
SMALL=1

CONTIKI = ../..
APPS = webserver serial-shell
CONTIKI_WITH_IPV4 = 1
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include

