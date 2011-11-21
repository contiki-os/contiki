CONTIKI_PROJECT = sky-webserver
all: sky-webserver sky-telnet-server telnet-tweet 
PLATFORM_BUILD=1 # This is needed to avoid the shell to include the httpd-cfs version of the webserver
APPS = webserver twitter telnetd
CFLAGS = -DWITH_UIP=1 -I.
SMALL=1
DEFINES=NETSTACK_CONF_RDC=cxmac_driver,NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE=8

# The webserver application normally contains a built-in file system and support
# for server-side includes.
#
# This webserver example supports building the alternative webserver application
# which serves files from an cfs file system. To build the alternative webserver
# run make with the parameter HTTPD-CFS=1.

# ifeq ($(HTTPD-CFS),1)
# override webserver_src = webserver-nogui.c http-strings.c psock.c memb.c \
#                          httpd-cfs.c
# endif

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

sky-webserver.$(TARGET): $(OBJECTDIR)/ajax-cgi.o

# Intentionally httpd.c and httpd-cfs.c implement the same interface. When
# switching from one webserver alternative to the other with an existent
# Contiki library then both files end up in the library making the linker
# use whichever it finds first :-(
#
# The most straightforward way to make sure this doesn't happen is to delete
# the Contiki library. But it would be undesirable to do that on every build
# so the existence of the "wrong" object file is used to detect a switch and
# trigger deletion of the Contiki library - and the trigger file of course.

# ifeq ($(HTTPD-CFS),1)
#   ifneq (${wildcard $(OBJECTDIR)/httpd.o},)
#   DUMMY := ${shell rm -f contiki-$(TARGET).a $(OBJECTDIR)/httpd.o}
#   endif
# else
#   ifneq (${wildcard $(OBJECTDIR)/httpd-cfs.o},)
#   DUMMY := ${shell rm -f contiki-$(TARGET).a $(OBJECTDIR)/httpd-cfs.o}
#   endif
# endif

slip:
ifdef INTERFACE
	$(CONTIKI)/tools/wpcapslip/wpcapslip -s /dev/com$(firstword $(CMOTES)) $(INTERFACE) 172.16.0.0 255.255.0.0
else
	@echo "Usage: \"$(MAKE) $@ INTERFACE=<the IP address of a local network interface>\""
	@echo "Use the \"ipconfig\" command to find out the IP addresses of the local interfaces"
endif
