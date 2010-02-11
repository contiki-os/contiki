#
# Copyright (c) 2010, Adam Dunkels.
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
# $Id: Makefile,v 1.3 2010/02/11 10:13:18 oliverschmidt Exp $
#

ifndef CC65_HOME
  ${error CC65_HOME not defined! You must specify where cc65 resides}
endif

ifndef AC
  ${error AC not defined! You must specify where the AppleCommander jar resides}
endif

ifndef C1541
  ${error C1541 not defined! You must specify where VICE c1541 resides}
endif

disks: apple2enh-1-disk apple2enh-2-disk c64-1-disk c64-2-disk c128-disk

apple2enh-1-disk:
	cp ../apple2enh/prodos.dsk contiki-1.dsk
	java -jar $(AC) -p    contiki-1.dsk contiki.cfg     bin 0 < dummy.cfg
	java -jar $(AC) -p    contiki-1.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-1.dsk dhcp.system     sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-1.dsk dhcp            bin 0 < ../../cpu/6502/dhcp/dhcp-client.apple2enh
	java -jar $(AC) -p    contiki-1.dsk webserve.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-1.dsk webserve        bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    contiki-1.dsk webbrows.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-1.dsk webbrows        bin   < ../../examples/webbrowser/webbrowser.apple2enh
	java -jar $(AC) -p    contiki-1.dsk cs8900a.eth     rel 0 < ../../cpu/6502/dhcp/cs8900a.eth
	java -jar $(AC) -p    contiki-1.dsk lan91c96.eth    rel 0 < ../../cpu/6502/dhcp/lan91c96.eth
	java -jar $(AC) -p    contiki-1.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou
	java -jar $(AC) -p    contiki-1.dsk index.html      bin 0 < ../../examples/webserver/httpd-cfs/index.html
	java -jar $(AC) -p    contiki-1.dsk background.gif  bin 0 < ../../examples/webserver/httpd-cfs/background.gif
	java -jar $(AC) -p    contiki-1.dsk contiki.gif     bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    contiki-1.dsk notfound.html   bin 0 < ../../examples/webserver/httpd-cfs/notfound.html

apple2enh-2-disk:
	cp ../apple2enh/prodos.dsk contiki-2.dsk
	java -jar $(AC) -p    contiki-2.dsk contiki.cfg     bin 0 < dummy.cfg
	java -jar $(AC) -p    contiki-2.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-2.dsk wget.system     sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-2.dsk wget            bin   < ../../examples/wget/wget.apple2enh
	java -jar $(AC) -p    contiki-2.dsk irc.system      sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-2.dsk irc             bin   < ../../examples/irc/irc-client.apple2enh
	java -jar $(AC) -p    contiki-2.dsk email.system    sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-2.dsk email           bin   < ../../examples/email/email-client.apple2enh
	java -jar $(AC) -p    contiki-2.dsk cs8900a.eth     rel 0 < ../../cpu/6502/dhcp/cs8900a.eth
	java -jar $(AC) -p    contiki-2.dsk lan91c96.eth    rel 0 < ../../cpu/6502/dhcp/lan91c96.eth
	java -jar $(AC) -p    contiki-2.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

c64-1-disk:
	$(C1541) -format contiki,00 d64 contiki-1.d64
	$(C1541) -attach contiki-1.d64 -write dummy.cfg                                         contiki.cfg
	$(C1541) -attach contiki-1.d64 -write ../../cpu/6502/dhcp/dhcp-client.c64               dhcp
	$(C1541) -attach contiki-1.d64 -write ../../examples/webserver/webserver-example.c64    webserver
	$(C1541) -attach contiki-1.d64 -write ../../examples/webbrowser/webbrowser.c64          webbrowser
	$(C1541) -attach contiki-1.d64 -write ../../examples/wget/wget.c64                      wget
	$(C1541) -attach contiki-1.d64 -write ../../cpu/6502/dhcp/cs8900a.eth                   cs8900a.eth
	$(C1541) -attach contiki-1.d64 -write ../../cpu/6502/dhcp/lan91c96.eth                  lan91c96.eth
	$(C1541) -attach contiki-1.d64 -write $(CC65_HOME)/mou/c64-1351.mou                     c64-1351.mou
	$(C1541) -attach contiki-1.d64 -write ../../examples/webserver/httpd-cfs/index.html     index.html
	$(C1541) -attach contiki-1.d64 -write ../../examples/webserver/httpd-cfs/background.gif background.gif
	$(C1541) -attach contiki-1.d64 -write ../../examples/webserver/httpd-cfs/contiki.gif    contiki.gif
	$(C1541) -attach contiki-1.d64 -write ../../examples/webserver/httpd-cfs/notfound.html  notfound.html

c64-2-disk:
	$(C1541) -format contiki,00 d64 contiki-2.d64
	$(C1541) -attach contiki-2.d64 -write dummy.cfg                             contiki.cfg
	$(C1541) -attach contiki-2.d64 -write ../../cpu/6502/dhcp/dhcp-client.c64   dhcp
	$(C1541) -attach contiki-2.d64 -write ../../examples/irc/irc-client.c64     irc
	$(C1541) -attach contiki-2.d64 -write ../../examples/email/email-client.c64 email
	$(C1541) -attach contiki-2.d64 -write ../../cpu/6502/dhcp/cs8900a.eth       cs8900a.eth
	$(C1541) -attach contiki-2.d64 -write ../../cpu/6502/dhcp/lan91c96.eth      lan91c96.eth
	$(C1541) -attach contiki-2.d64 -write $(CC65_HOME)/mou/c64-1351.mou         c64-1351.mou

c128-disk:
	$(C1541) -format contiki,00 d71 contiki.d71
	$(C1541) -attach contiki.d71 -write dummy.cfg                                         contiki.cfg
	$(C1541) -attach contiki.d71 -write ../../cpu/6502/dhcp/dhcp-client.c128              dhcp
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/webserver-example.c128   webserver
	$(C1541) -attach contiki.d71 -write ../../examples/webbrowser/webbrowser.c128         webbrowser
	$(C1541) -attach contiki.d71 -write ../../examples/wget/wget.c128                     wget
	$(C1541) -attach contiki.d71 -write ../../examples/irc/irc-client.c128                irc
	$(C1541) -attach contiki.d71 -write ../../examples/email/email-client.c128            email
	$(C1541) -attach contiki.d71 -write ../../cpu/6502/dhcp/cs8900a.eth                   cs8900a.eth
	$(C1541) -attach contiki.d71 -write ../../cpu/6502/dhcp/lan91c96.eth                  lan91c96.eth
	$(C1541) -attach contiki.d71 -write $(CC65_HOME)/mou/c128-1351.mou                    c128-1351.mou
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/index.html     index.html
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/background.gif background.gif
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/contiki.gif    contiki.gif
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/notfound.html  notfound.html
