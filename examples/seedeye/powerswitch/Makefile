ifndef TARGET
TARGET=seedeye
endif

all: remotepowerswitch

CONTIKI=../../../
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

APPS += er-coap
APPS += rest-engine

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
