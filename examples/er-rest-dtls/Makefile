all: er-example-server er-example-client

CONTIKI=../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

WITH_DTLS_COAP ?= 1

# automatically build RESTful resources
ER_EXAMPLE = $(CONTIKI)/examples/er-rest-example
REST_RESOURCES_DIR = $(ER_EXAMPLE)/resources
REST_RESOURCES_FILES += res-push.c
REST_RESOURCES_FILES += res-hello.c
PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

# linker optimizations
SMALL=1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
