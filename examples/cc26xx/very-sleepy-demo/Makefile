DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = very-sleepy-demo

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 1

APPS += er-coap
APPS += rest-engine

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
