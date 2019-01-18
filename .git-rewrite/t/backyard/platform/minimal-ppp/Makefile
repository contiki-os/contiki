# Copyright (c) 2002-2004, Adam Dunkels.
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
# This file is part of the Contiki desktop environment
#
# $Id: Makefile,v 1.1 2007/05/26 07:21:01 oliverschmidt Exp $
#

all: contiki

CONTIKI=../..
SDIRS = . net
APPDIRS = ${wildcard ${addprefix $(CONTIKI)/,apps apps/*}}
CSDIRS = ${addprefix $(CONTIKI)/core/,. dev lib net sys cfs lib/ctk ctk loader net/ppp }

CC=gcc
STRIP=strip
CFLAGS=-Wall -Wstrict-prototypes -Wmissing-prototypes -g \
        -I$(CONTIKI) \
	${addprefix -I,$(CONTIKI)/core} \
	${addprefix -I,$(SDIRS)} \
	${addprefix -I,$(APPDIRS)}
LDFLAGS=-Wl,-Map=contiki-main.map,-export-dynamic,-lutil


vpath %.c ${SDIRS} $(CSDIRS) $(APPDIRS)

-include $(CONTIKI)/arch/Makefile.common

SYSTEM=process.o procinit.o service.o arg.o
UIP=uip.o uiplib.o tcpip.o \
    uip-fw.o uip-fw-service.o tcpdump.o \
    psock.o uipbuf.o uip_arp.o resolv.o \
    ppp/ahdlc.o ppp/ipcp.o ppp/lcp.o ppp/pap.o ppp/ppp.o ppp/ppp_process.o
LIBS=memb.o timer.o list.o etimer.o

CFS=cfs.o

WWW=www.o webclient.o http-strings.o htmlparser.o html-strings.o www-dsc.o http-user-agent-string.o
EMAIL=email.o smtp-socket.o smtp-strings.o ctk-textedit.o
WEBSERVER=webserver.o webserver-dsc.o httpd.o httpd-fs.o http-strings.o httpd-cgi.o webserver-nogui.o
TELNET=simpletelnet.o telnet.o
VNCVIEWER=vnc.o vnc-viewer.o vnc-draw.o vnc-dsc.o

CTK=ctk.o
CTKGTK=ctk-gtksim.o ctk-draw.o ctk-gtksim-service.o libconio.o ctk-gtksim-draw.o
CTKVNC=ctk-vncserver.o libconio.o vnc-server.o vnc-out.o ctk-vncfont.o
CTKTERM=libconio.o ctk-term.o ctk-term-in.o ctk-term-out.o ctk-termtelnet.o

POP=pop.o popc.o popc-strings.o

TELNETD=telnetd.o shell.o

APPS=$(WWW) $(EMAIL) $(WEBSERVER) $(TELNET) $(TELNETD) program-handler.o

contiki.a:  $(SYSTEM) $(UIP) uip-split.o $(LIBS) $(CFS) \
 ppp-arch.o $(APPS) \
 $(CTK) $(CTKVNC) \
 dlloader.o process.o clock.o
	$(AR) rcf $@ $^

webserver.co: webserver-nogui.co httpd.o httpd-fs.o http-strings.o httpd-cgi.o psock.o uipbuf.o
	$(LD) -i -o $@ $^

%.co: %.o
	$(LD) -i -o $@ $^

%.ce: %.co
	$(LD) -shared -o $@ $^

%.co: %.c
	$(CC) $(CFLAGS) -DPROCESS_LOADABLE -c $< -o $@
	$(STRIP) --strip-unneeded -g -x $@


contiki: contiki-main.o contiki.a
	gcc $(LDFLAGS) -o $@ $^

test: test-main.o contiki.a
	gcc $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o *~ *core contiki *.s *.ce *.co *.a *.map Makefile.depend ppp/*.o

Makefile.depend depend:
	gcc -MM \
	${addprefix -I,$(CSDIRS)} \
	${addprefix -I,$(SDIRS)} \
	${addsuffix /*.c,$(CSDIRS)} \
	${addsuffix /*.c,$(SDIRS)} \
	> Makefile.depend 2> /dev/null || true

include Makefile.depend

