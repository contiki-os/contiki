CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT) 

### Specify where CONTIKI resides
CONTIKI = ../../..

### Build with code size optimisations
SMALL = 1

### Include project specific configuration file
DEFINES += PROJECT_CONF_H=\"project-conf.h\"

### Choose whether or not to use SLIP. If not, assume IP64 via ethernet.
WITH_SLIP = 0
ifeq ($(WITH_SLIP),1)
PROJECT_SOURCEFILES += slip-bridge.c
else 
WITH_IP64 = 1
endif

### 
# Built-in webserver configuration
# Set to 0 for no webserver, 1 for the simple builtin, or webserver-name
# WITH_WEBSERVER=webserver-name will use /apps/webserver-name if it can be
# found in the /apps, /platform/$(TARGET)/apps/, or current directory 
# (in that order).
# WITH_WEBSERVER=webserver for /apps/webserver
# Please make sure to make clean after changing webservers.
WITH_WEBSERVER = 1

ifeq ($(WITH_WEBSERVER),1)
  DEFINES += UIP_CONF_TCP=1
  DEFINES += WEBSERVER=1
  PROJECT_SOURCEFILES += httpd-simple.c httpd-simple-handler.c
else ifneq ($(WITH_WEBSERVER),0)
  APPS += $(WITH_WEBSERVER)
  DEFINES += UIP_CONF_TCP=1
  DEFINES += WEBSERVER=2
endif

### Use IPv6 stack
CONTIKI_WITH_IPV6 = 1

### Include Contiki's global makefile
include $(CONTIKI)/Makefile.include

ifeq ($(PREFIX),)
 PREFIX = fd00::1/64
endif

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -v $(PREFIX)

