DEFINES=PROJECT_CONF_H=\"project-conf.h\"

# Define the target platform
ifndef TARGET
TARGET=z1
endif

CONTIKI_SOURCEFILES += cc2420-arch.c sensors.c sht11.c
PROJECT_SOURCEFILES = i2cmaster.c tmp102.c adxl345.c battery-sensor.c sky-sensors.c #potentiometer-sensor.c

# Give a name to your project 
CONTIKI = ../../../../contiki-2.x/
CONTIKI_PROJECT = rssi-scanner

# Compile project typing "make"
all: $(CONTIKI_PROJECT)

# Upload project typing "make upload"
upload: $(CONTIKI_PROJECT).upload

%.class: %.java
	javac $(basename $<).java
	
viewrssi3d: ViewRSSI3D.class
	make login | java ViewRSSI3D

viewrssi: ViewRSSI.class
	make login | java ViewRSSI

# ContikiProjects: including the makefile
#include ../../../Makefile.projects
include $(CONTIKI)/Makefile.include

