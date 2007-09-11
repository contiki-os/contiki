#
# Makefile for PC-6001 using z80/SDCC
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.1 2007/09/11 12:12:59 matsutsuka Exp $
#

CONTIKI = ../..
TARGET=pc-6001
APPS_DESKTOP = program-handler calc process-list shell about
APPS_NETWORK = netconf cmdd

#	editor netconf dhcp webbrowser ftp irc email telnet telnetd
#	cmdd codeprop webserver vnc
#	directory

PLATFORM_APPS_DESKTOP = multithread tinysample
PLATFORM_APPS_NETWORK = webserver-mini

#defualt values
#16K, 32K, or ROM
MEMORY=32K
#PC6001, PC6001A, or PC6001MK2
ARCH=PC6001

APPS = $(APPS_DESKTOP)
PLATFORM_APPS=$(PLATFORM_APPS_DESKTOP)

all: p6

p6:
	rm -f contiki.ihex
	make contiki
p62:
	rm -f contiki.ihex;
	make contiki ARCH=PC6001MK2
p6A:
	rm -f contiki.ihex;
	make contiki ARCH=PC6001A
minimal:
	rm -f contiki.ihex;
	make contiki MEMORY=16K CONTIKI_TARGET_MAIN=contiki-minimal-main.c

serial:
	rm -f contiki.ihex;
	make contiki MEMORY=16K CONTIKI_TARGET_MAIN=contiki-serial-main.c

rom:
	rm -f contiki.ihex;
	make contiki.rom MEMORY=ROM CONTIKI_TARGET_MAIN=contiki-rom-main.c
slip:
	rm -f contiki.ihex;
	make contiki APPS="$(APPS_NETWORK)" PLATFORM_APPS="$(PLATFORM_APPS_NETWORK)" CONTIKI_TARGET_MAIN=contiki-slip-main.c

include $(CONTIKI)/Makefile.include
