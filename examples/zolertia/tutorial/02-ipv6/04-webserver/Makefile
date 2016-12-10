all: zolertia-websense

CONTIKI = ../../../../..

# Linker size optimizations
SMALL = 1

# Include the webserver and webbrowser applications
APPS += webserver webbrowser

# Use the examples project configuration
CFLAGS += -DPROJECT_CONF_H=\"../project-conf.h\"

# This includes the httpd code from the Border Router
PROJECTDIRS += ../02-border-router
PROJECT_SOURCEFILES += httpd-simple.c

CONTIKI_WITH_IPV6 = 1

include $(CONTIKI)/Makefile.include
