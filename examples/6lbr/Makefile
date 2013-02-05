CONTIKI_PROJECT = cetic_bridge

all: $(CONTIKI_PROJECT)

UIP_CONF_IPV6=1

CONTIKI = ../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
LDFLAGS += -Wl,--gc-sections

CETIC_6LBR_SMARTBRIDGE=0
CETIC_6LBR_TRANSPARENTBRIDGE=0
CETIC_6LBR_ROUTER=0

#TODO: add check on flags

CFLAGS +=-DCETIC_6LBR_SMARTBRIDGE=$(CETIC_6LBR_SMARTBRIDGE) -DCETIC_6LBR_TRANSPARENTBRIDGE=$(CETIC_6LBR_TRANSPARENTBRIDGE) -DCETIC_6LBR_ROUTER=$(CETIC_6LBR_ROUTER)

WITH_WEBSERVER=1
ifneq ($(WITH_WEBSERVER),0)
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += webserver.c httpd-simple.c 
endif

WITH_UDPSERVER=1
ifneq ($(WITH_UDPSERVER),0)
CFLAGS += -DUDPSERVER=1
PROJECT_SOURCEFILES += udp-server.c 
endif

CETIC_NODE_INFO=1
ifneq ($(CETIC_NODE_INFO),0)
CFLAGS += -DCETIC_NODE_INFO=1
PROJECT_SOURCEFILES += node-info.c 
endif

PROJECT_SOURCEFILES += rio.c packet-filter.c sicslow_ethernet.c nvm_config.c node-info.c
ifeq ($(TARGET),native)
APPS = slip-cmd
PROJECT_SOURCEFILES += native_nvm.c raw-tap-eth-drv.c raw-tap-dev.c native-rdc.c slip-cmds.c slip-config.c slip-dev.c
else
PROJECT_SOURCEFILES += enc28j60.c enc28j60-drv.c
endif

all_native:
	mkdir -p bin
	$(MAKE) TARGET=native clean
	$(MAKE) TARGET=native CETIC_6LBR_SMARTBRIDGE=1 WITH_WEBSERVER=$(WITH_WEBSERVER) WITH_UDPSERVER=$(WITH_UDPSERVER) CETIC_NODE_INFO=$(CETIC_NODE_INFO)
	mv cetic_bridge.native bin/cetic_6lbr_smart_bridge 
	$(MAKE) TARGET=native clean
	$(MAKE) TARGET=native CETIC_6LBR_TRANSPARENTBRIDGE=1 WITH_WEBSERVER=$(WITH_WEBSERVER) WITH_UDPSERVER=$(WITH_UDPSERVER) CETIC_NODE_INFO=$(CETIC_NODE_INFO)
	mv cetic_bridge.native bin/cetic_6lbr_transparent_bridge 
	$(MAKE) TARGET=native clean
	$(MAKE) TARGET=native CETIC_6LBR_ROUTER=1 WITH_WEBSERVER=$(WITH_WEBSERVER) WITH_UDPSERVER=$(WITH_UDPSERVER) CETIC_NODE_INFO=$(CETIC_NODE_INFO)
	mv cetic_bridge.native bin/cetic_6lbr_router 

VERSION=$(word 2,$(shell awk '/Version:/' package/DEBIAN/control))
PKG=6lbr-$(VERSION)
IPK=6lbr-$(VERSION)-ipk

dist:
	rm -rf $(PKG)
	cp -R package $(PKG)
	mkdir -p $(PKG)/usr/lib/6lbr/bin
	cp bin/* $(PKG)/usr/lib/6lbr/bin

deb: dist
	dpkg --build $(PKG) .

ipk: dist
	rm -rf $(IPK)
	rm -f $(IPK).ipk
	mkdir $(IPK)
	cd $(PKG) && tar zcvf ../$(IPK)/data.tar.gz --exclude=DEBIAN --exclude='*~' *
	cd control-ipk && tar zcvfh ../$(IPK)/control.tar.gz --exclude='*~' *
	echo 2.0 > $(IPK)/debian-binary
	cd $(IPK) && ar rc ../$(IPK).ipk debian-binary data.tar.gz control.tar.gz

include $(CONTIKI)/Makefile.include
