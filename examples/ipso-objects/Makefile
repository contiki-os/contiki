CONTIKI_PROJECT = example-ipso-objects

CONTIKI_SOURCEFILES += serial-protocol.c example-ipso-temperature.c

all: $(CONTIKI_PROJECT)

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

APPS += rest-engine
APPS += er-coap
APPS += oma-lwm2m
APPS += ipso-objects

CONTIKI=../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

# border router rules
$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 -p 60001 aaaa::1/64

connect-router-native:	$(CONTIKI)/examples/ipv6/native-border-router/border-router.native
	sudo $(CONTIKI)/exmples/ipv6/native-border-router/border-router.native -a 127.0.0.1 -p 60001 aaaa::1/64
