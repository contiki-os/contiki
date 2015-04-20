CONTIKI_PROJECT = node
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

TARGET ?= jn5168

APPS = json

CONTIKI_WITH_IPV6 = 1

PROJECTDIRS += .. ../tools
PROJECT_SOURCEFILES += rich.c #rich-scheduler-interface.c 

CFLAGS += -DWITH_COAP
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap
APPS += rest-engine

all: $(CONTIKI_PROJECT)

CONTIKI=../../..
include $(CONTIKI)/Makefile.include
