DEFINES=PROJECT_CONF_H=\"project-conf.h\"

ifndef TARGET
	TARGET = openmote
endif

CONTIKI = ../..

all: example-abc example-mesh example-collect example-trickle example-polite \
     example-rudolph1 example-rudolph2 example-rucb \
     example-runicast example-unicast example-neighbors

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
