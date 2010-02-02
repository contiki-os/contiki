CONTIKI_PROJECT = sky-shell-webserver
all: $(CONTIKI_PROJECT)
PLATFORM_BUILD = 1
PROJECT_SOURCEFILES = webserver-nogui.c

CFLAGS = -DWITH_UIP=1 -DWITH_NULLMAC=1

CONTIKI = ../..
APPS = webserver serial-shell
include $(CONTIKI)/Makefile.include
 
