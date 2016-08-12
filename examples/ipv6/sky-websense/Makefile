all: sky-websense

CONTIKI=../../..

APPS += webbrowser

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_SOURCEFILES += wget.c
PROJECTDIRS += ../rpl-border-router
PROJECT_SOURCEFILES += httpd-simple.c

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 fd00::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 fd00::1/64
