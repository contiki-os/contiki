all: ipsec-example border-router

ifndef TARGET
TARGET=sky
endif

CONTIKI = ../..

UIP_CONF_IPV6=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DLOCAL_PLATFORM_CONF_H=1
PROJECT_SOURCEFILES += slip-bridge.c

include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja: $(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64

realclean: clean
	rm -rf obj_* symbols.c symbols.h *.sky *.log
