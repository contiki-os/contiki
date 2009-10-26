ifdef CONTIKI_HOME
CONTIKI = $(CONTIKI_HOME)
else
ifndef CONTIKI
CONTIKI=../../../../contiki-2.x
endif
endif

DEFINES=WITH_UIP6=1,WITH_SLIP=1,PROJECT_CONF_H=\"bridge-conf.h\"
UIP_CONF_IPV6=1

ifndef TARGET
TARGET=sky
endif

PROJECTDIRS = dev
PROJECT_SOURCEFILES = fakeuip.c sicslow_ethernet.c

all:	uip6-bridge-tap.sky

upload: uip6-bridge-tap.ihex
	cp $< $(IHEXFILE)
	$(MAKE) sky-u.$(subst /,-,$(word 1, $(MOTES)))

include $(CONTIKI)/Makefile.include

../../tapslip6: ../../tapslip6.c
	(cd ../../; $(MAKE) tapslip6)

ifndef MOTE
MOTE=1
endif

connect: ../../tapslip6
	sudo ../../tapslip6 -t tap0 -s $(USBDEVPREFIX)$(word $(MOTE), $(CMOTES)) 127.0.0.1 255.0.0.0

bridge:
	sudo sysctl net.ipv6.conf.all.forwarding=1
	sudo ifconfig tap0 down
	sudo ifconfig tap0 hw ether $(MAC)
	sudo ifconfig tap0 up
	sudo service radvd restart
	sudo route add -6 aaaa::/64 tap0
	sudo ip -6 address add aaaa::1/64 dev tap0
