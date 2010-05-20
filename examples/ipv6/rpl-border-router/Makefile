all: border-router

CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1

APPS += webserver

CFLAGS += -DPROJECT_CONF_H=\"project-router-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c httpd-simple.c

include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64
