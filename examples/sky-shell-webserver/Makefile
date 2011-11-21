CONTIKI_PROJECT = sky-shell-webserver
all: $(CONTIKI_PROJECT)
PROJECT_SOURCEFILES = webserver-nogui.c
HTTPD_CFS=1
CFLAGS = -DWITH_UIP=1
DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver 
SMALL=1

CONTIKI = ../..
APPS = webserver serial-shell
include $(CONTIKI)/Makefile.include
 
