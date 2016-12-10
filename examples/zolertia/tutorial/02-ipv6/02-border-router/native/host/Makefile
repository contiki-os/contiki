CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT)
APPS = slip-cmd

CONTIKI = ../../../../../../..

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += border-router-cmds.c tun-bridge.c border-router-rdc.c \
slip-config.c slip-dev.c

# Enable webserver as default
WITH_WEBSERVER = 1
ifeq ($(WITH_WEBSERVER),1)
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c
else ifneq ($(WITH_WEBSERVER), 0)
APPS += $(WITH_WEBSERVER)
CFLAGS += -DWEBSERVER=2
endif

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

connect-router:	border-router.native
	sudo ./border-router.native aaaa::1/64
