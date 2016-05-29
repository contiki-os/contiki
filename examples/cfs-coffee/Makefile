DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI = ../..

all: test-cfs test-coffee example-coffee

CONTIKI_WITH_RIME = 1

ifeq ($(TARGET),avr-raven)
  COFFEE_FILES = 4
endif

include $(CONTIKI)/Makefile.include
