CONTIKI_PROJECT = mbxxx-webserver
all: $(CONTIKI_PROJECT)

DEFINES=PROJECT_CONF_H=\"webserver-ajax-conf.h\"

#APPS = webserver

PROJECTDIRS = . $(CONTIKI)/apps/webserver
PROJECT_SOURCEFILES = ajax-cgi.c httpd-fs.c http-strings.c \
				httpd.c webserver-dsc.c webserver-nogui.c

CONTIKI = ../../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
