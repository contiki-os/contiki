#
# Makefile for PC-6001 using z80/SDCC
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.7 2009/12/16 06:47:18 matsutsuka Exp $
#

CONTIKI = ../..
TARGET=pc-6001
APPS_DESKTOP = program-handler calc process-list shell about
APPS_NETWORK_CLIENT = $(APPS_DESKTOP) netconf webbrowser email telnet ftp irc
APPS_NETWORK_SERVER = program-handler cmdd telnetd shell
#	editor netconf dhcp webbrowser ftp irc email telnet telnetd
#	cmdd codeprop webserver vnc
#	directory

PLATFORM_APPS_DESKTOP = multithread tinysample
PLATFORM_APPS_NETWORK = webserver-mini
PLATFORM_APPS=$(PLATFORM_APPS_DESKTOP)

ifeq ($(MAKECMDGOALS),client)
  APPS = $(APPS_NETWORK_CLIENT)
else ifeq ($(MAKECMDGOALS),server)
  APPS = $(APPS_NETWORK_SERVER)
else
  APPS = $(APPS_DESKTOP)
endif

# Default values
MEMORY=32K
PLATFORM_APPS=$(PLATFORM_APPS_DESKTOP)
CONTIKI_TARGET_MAIN = contiki-desktop-main.c
CTKCONF = NORMAL
LOADER = 0

# Target dependent options
ifeq ($(MAKECMDGOALS),p62)
  ARCH=PC6001MK2
else ifeq ($(MAKECMDGOALS),p6A)
  ARCH=PC6001A
else ifeq ($(MAKECMDGOALS),minimal)
  MEMORY=16K
  CONTIKI_TARGET_MAIN=contiki-minimal-main.c
else ifeq ($(MAKECMDGOALS),serial)
  CONTIKI_TARGET_MAIN=contiki-serial-main.c
else ifeq ($(MAKECMDGOALS),rom)
  CTKCONF=NOICON
  MEMORY=ROM
  LOADER = 1
  CONTIKI_TARGET_MAIN=contiki-loader-main.c
else ifeq ($(MAKECMDGOALS),load)
  CTKCONF=NOICON
  LOADER = 1
  CONTIKI_TARGET_MAIN=contiki-loader-main.c
else ifeq ($(MAKECMDGOALS),server)
  CTKCONF=SERVER
  PLATFORM_APPS = $(PLATFORM_APPS_NETWORK)
  CONTIKI_TARGET_MAIN=contiki-server-main.c
else ifeq ($(MAKECMDGOALS),client)
  CTKCONF=CLIENT
  PLATFORM_APPS = $(PLATFORM_APPS_NETWORK)
  CONTIKI_TARGET_MAIN=contiki-client-main.c
endif

all	: p6

p6	: contiki
p62	: contiki
p6A	: contiki
minimal	: contiki
serial	: contiki
rom	: contiki.rom
load	: contiki
server	: contiki
client	: contiki

remove-ctk:
	rm -f obj_$(TARGET)/ctk*;
	rm -f contiki.ihex
remove-net:
	rm -f ${addprefix obj_$(TARGET)/,uip*.o hc.o psock.o rawpacket-udp.o resolv.o slip*.o tcp*.o uaod*.o rime*.o http*.o web*.o};
	rm -f contiki.ihex
remove-mt:
	rm -f obj_$(TARGET)/mt*.o
	rm -f contiki.ihex

include $(CONTIKI)/Makefile.include
