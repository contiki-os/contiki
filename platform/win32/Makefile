#
# Copyright (c) 2007, Adam Dunkels.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# This file is part of the Contiki operating system.
#
# Author: Oliver Schmidt <ol.sc@web.de>
#
# $Id: Makefile,v 1.9 2008/11/10 22:10:33 oliverschmidt Exp $
#

PRGS = about calc dhcp directory email ftp irc netconf processes shell \
       telnet telnetd webserver www
DSCS = about calc dhcp email ftp irc netconf processes shell telnet \
       telnetd webserver
all: contiki-main $(PRGS:=.prg) $(DSCS:=.dsc)

appname = ${subst processes,process-list,${subst www,webbrowser,$(1)}}

# Omit all the target and network stack specific shell commands
SHELL_SOURCEFILES = shell.c shell-file.c shell-ps.c shell-run.c \
		    shell-text.c shell-time.c shell-wget.c \
		    http-strings.c http-user-agent-string.c \
		    webclient.c list.c random.c
		    
# Use gui-shell.c instead of serial-shell.c to get CTK support
override shell_src = gui-shell.c ctk-textentry-cmdline.c \
		     $(SHELL_SOURCEFILES)

# Add telnetd-gui.c to get CTK support
override telnetd_src = telnetd.c telnetd-gui.c memb.c \
		       $(SHELL_SOURCEFILES)

# Use webserver.c instead of webserver-nogui.c to get CTK support
override webserver_src = webserver.c httpd.c http-strings.c psock.c \
			 memb.c httpd-fs.c httpd-cgi.c

PLATFORM_BUILD = 1

CONTIKI = ../..
APPS = program-handler ${call appname,$(PRGS)}
TARGET = win32
-include $(CONTIKI)/Makefile.include
