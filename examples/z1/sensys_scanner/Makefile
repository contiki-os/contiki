DEFINES=PROJECT_CONF_H=\"project-conf.h\"

# Define the target platform
ifndef TARGET
TARGET=z1
endif

# Give a name to your project 
CONTIKI = ../../../../contiki-2.x/
CONTIKI_PROJECT = rssi-scanner

# Compile project typing "make"
all: $(CONTIKI_PROJECT)

# Upload project typing "make upload"
upload: $(CONTIKI_PROJECT).upload

%.class: %.java
	javac $(basename $<).java
	
viewrssi: ViewRSSI.class
	make login | java ViewRSSI

# ContikiProjects: including the makefile
#include ../../../Makefile.projects
include $(CONTIKI)/Makefile.include
