#
# Makefile for PC-6001 using z80/SDCC
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.4 2007/11/28 09:43:01 matsutsuka Exp $
#

CONTIKI = ../..
TARGET=pc-6001
APPS_DESKTOP = program-handler calc process-list shell about
APPS_NETWORK_CLIENT = $(APPS_DESKTOP) netconf webbrowser email telnet ftp irc
APPS_NETWORK_SERVER = cmdd telnetd shell www
#	editor netconf dhcp webbrowser ftp irc email telnet telnetd
#	cmdd codeprop webserver vnc
#	directory

PLATFORM_APPS_DESKTOP = multithread tinysample
PLATFORM_APPS_NETWORK = webserver-mini

APPS = $(APPS_DESKTOP)
PLATFORM_APPS=$(PLATFORM_APPS_DESKTOP)

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

include $(CONTIKI)/Makefile.include
