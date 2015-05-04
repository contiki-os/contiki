DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
all: decagon
APPS=servreg-hack
CONTIKI = ../../..

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
TARGET_LIBFILES += -lm
