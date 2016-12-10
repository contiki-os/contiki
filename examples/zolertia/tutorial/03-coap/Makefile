all: er-example-server

CONTIKI = ../../../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# Automatically build RESTful resources
REST_RESOURCES_DIR = ./resources

REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

CONTIKI_TARGET_SOURCEFILES += sht25.c

# linker optimizations
SMALL = 1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

