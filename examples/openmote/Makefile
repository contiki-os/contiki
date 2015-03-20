DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = openmote-demo test-timer test-adxl346 test-max44009 test-sht21

all: $(CONTIKI_PROJECT)	
	
sht: test-sht21

CONTIKI_WITH_RIME = 1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
