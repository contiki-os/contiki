CONTIKI_PROJECT=rpl-border-router

TARGET ?= jn516x
JN516x_WITH_DONGLE = 1

CONTIKI=../../../../..

CONTIKI_WITH_IPV6 = 1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c slip.c

PROJECTDIRS += .. ../../tools
PROJECT_SOURCEFILES += rpl-tools.c 

CFLAGS += -DWITH_COAP
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += orchestra
APPS += json
APPS += er-coap
APPS += rest-engine

MODULES += core/net/mac/tsch

all: $(CONTIKI_PROJECT)
include $(CONTIKI)/Makefile.include

ifeq ($(PREFIX),)
 PREFIX = aaaa::1/64
endif

#no flow control
connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -v1 -B 1000000 $(PREFIX)

#using XON/XOFF flow control
connect-router-sw:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -v1 -X -B 1000000 $(PREFIX)

#using hw flow control	
connect-router-hw:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -v1 -H -B 1000000 $(PREFIX)

#using no flow control	
connect-router-no:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -v1 -B 1000000 $(PREFIX)

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 $(PREFIX)
