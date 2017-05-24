CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

CONTIKI=../../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

#automatically build RESTful resources
REST_RESOURCES_DIR = ./resources
ifndef TARGET
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
ifeq ($(TARGET), native)
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c' ! -name 'res-plugtest*'))
endif
endif

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

# linker optimizations
SMALL=1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

CONTIKI_WITH_IPV6 = 1


#CFLAGS += -DWITH_COAP
#CFLAGS += -DREST=coap_rest_implementation
#CFLAGS += -DUIP_CONF_TCP=0
#APPS += orchestra
#APPS += json
#APPS += er-coap
#APPS += rest-engine

MODULES += core/net/mac/tsch


include $(CONTIKI)/Makefile.include
