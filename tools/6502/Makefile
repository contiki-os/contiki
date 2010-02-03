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
# $Id: Makefile,v 1.1 2010/02/03 23:52:31 oliverschmidt Exp $
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

disks: apple2enh-server-disk apple2enh-client-disk c64-disk c128-disk

apple2enh-server-disk:
	cp ../apple2enh/prodos.dsk contiki-server.dsk
	java -jar $(AC) -p    contiki-server.dsk contiki.cfg     bin 0 < dummy.cfg
	java -jar $(AC) -p    contiki-server.dsk webserve.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-server.dsk webserve        bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    contiki-server.dsk cs8900a.eth     rel 0 < ../../examples/webserver/cs8900a.eth
	java -jar $(AC) -p    contiki-server.dsk lan91c96.eth    rel 0 < ../../examples/webserver/lan91c96.eth
	java -jar $(AC) -p    contiki-server.dsk index.html      bin 0 < ../../examples/webserver/httpd-cfs/index.html
	java -jar $(AC) -p    contiki-server.dsk background.gif  bin 0 < ../../examples/webserver/httpd-cfs/background.gif
	java -jar $(AC) -p    contiki-server.dsk contiki.gif     bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    contiki-server.dsk notfound.html   bin 0 < ../../examples/webserver/httpd-cfs/notfound.html

apple2enh-client-disk:
	cp ../apple2enh/prodos.dsk contiki-client.dsk
	java -jar $(AC) -p    contiki-client.dsk contiki.cfg     bin 0 < dummy.cfg
	java -jar $(AC) -p    contiki-client.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-client.dsk webbrows.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-client.dsk webbrows        bin   < ../../examples/webbrowser/webbrowser.apple2enh
	java -jar $(AC) -p    contiki-client.dsk irc.system      sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-client.dsk irc             bin   < ../../examples/irc/irc-client.apple2enh
	java -jar $(AC) -p    contiki-client.dsk email.system    sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-client.dsk email           bin   < ../../examples/email/email-client.apple2enh
	java -jar $(AC) -p    contiki-client.dsk cs8900a.eth     rel 0 < ../../examples/webserver/cs8900a.eth
	java -jar $(AC) -p    contiki-client.dsk lan91c96.eth    rel 0 < ../../examples/webserver/lan91c96.eth
	java -jar $(AC) -p    contiki-client.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

c64-disk:
	$(C1541) -format contiki,00 d64 contiki.d64
	$(C1541) -attach contiki.d64 -write dummy.cfg                                         contiki.cfg
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/webserver-example.c64    webserver
	$(C1541) -attach contiki.d64 -write ../../examples/webbrowser/webbrowser.c64          webbrowser
	$(C1541) -attach contiki.d64 -write ../../examples/irc/irc-client.c64                 irc
	$(C1541) -attach contiki.d64 -write ../../examples/email/email-client.c64             email
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/cs8900a.eth              cs8900a.eth
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/lan91c96.eth             lan91c96.eth
	$(C1541) -attach contiki.d64 -write $(CC65_HOME)/mou/c64-1351.mou                     c64-1351.mou
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/httpd-cfs/index.html     index.html
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/httpd-cfs/background.gif background.gif
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/httpd-cfs/contiki.gif    contiki.gif
	$(C1541) -attach contiki.d64 -write ../../examples/webserver/httpd-cfs/notfound.html  notfound.html

c128-disk:
	$(C1541) -format contiki,00 d71 contiki.d71
	$(C1541) -attach contiki.d71 -write dummy.cfg                                         contiki.cfg
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/webserver-example.c128   webserver
	$(C1541) -attach contiki.d71 -write ../../examples/webbrowser/webbrowser.c128         webbrowser
	$(C1541) -attach contiki.d71 -write ../../examples/irc/irc-client.c128                irc
	$(C1541) -attach contiki.d71 -write ../../examples/email/email-client.c128            email
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/cs8900a.eth              cs8900a.eth
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/lan91c96.eth             lan91c96.eth
	$(C1541) -attach contiki.d71 -write $(CC65_HOME)/mou/c128-1351.mou                    c128-1351.mou
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/index.html     index.html
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/background.gif background.gif
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/contiki.gif    contiki.gif
	$(C1541) -attach contiki.d71 -write ../../examples/webserver/httpd-cfs/notfound.html  notfound.html
