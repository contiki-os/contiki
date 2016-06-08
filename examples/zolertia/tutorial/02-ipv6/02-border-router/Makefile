CONTIKI_PROJECT=border-router
all: $(CONTIKI_PROJECT) 

CONTIKI = ../../../../..

SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"../project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c

CFLAGS += -DUIP_FALLBACK_INTERFACE=rpl_interface

WITH_WEBSERVER=1
CFLAGS += -DUIP_CONF_TCP=1
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c

ifeq ($(PREFIX),)
 PREFIX = fd00::1/64
endif

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 $(PREFIX)

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 $(PREFIX)
