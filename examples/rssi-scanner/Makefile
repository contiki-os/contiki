DEFINES=PROJECT_CONF_H=\"project-conf.h\"

ifndef TARGET
TARGET = z1
endif

CONTIKI = ../..

CONTIKI_PROJECT = rssi-scanner-cc2420

all: $(CONTIKI_PROJECT)

%.class: %.java
	javac $(basename $<).java
	
viewrssi3d: ViewRSSI3D.class
	make login | java ViewRSSI3D

viewrssi: ViewRSSI.class
	make login | java ViewRSSI

include $(CONTIKI)/Makefile.include

