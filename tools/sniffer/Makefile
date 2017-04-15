CONTIKI = ../..
CONTIKI_PROJECT = sniffer
all: $(CONTIKI_PROJECT)

UIP_CONF_RPL=0
UIP_CONF_IPV6=1

CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\" 

ifeq ($(TARGET),mbxxx)
CFLAGS+= -DSNIFFER_FRAMES_HAVE_FCS
endif

PROJECT_SOURCEFILES+= sniffer-fake-rdc.c
include $(CONTIKI)/Makefile.include

slip2pcap: slip2pcap.c
	@gcc -o slip2pcap slip2pcap.c

sniff: slip2pcap
	@sudo echo # ask for the password first (make login may require it)
	@make login | ./slip2pcap | wireshark -k -i -



