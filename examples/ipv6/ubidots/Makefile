DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = ubidots-demo

APPS = ubidots

ifdef UBIDOTS_WITH_AUTH_TOKEN
  DEFINES+=UBIDOTS_CONF_AUTH_TOKEN=\"$(UBIDOTS_WITH_AUTH_TOKEN)\"
endif

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 1

CONTIKI = ../../..

include $(CONTIKI)/Makefile.include
