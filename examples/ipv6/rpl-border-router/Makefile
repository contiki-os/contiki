all: border-router

CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1

#Override inclusion of internal webserver with make WITH_WEBSERVER=0
WITH_WEBSERVER=1

ifeq ($(WITH_WEBSERVER), 0)
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c
else
APPS += webserver
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DWEBSERVER
PROJECT_SOURCEFILES += slip-bridge.c httpd-simple.c
endif

include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64
