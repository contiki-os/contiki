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
# $Id: Makefile,v 1.11 2010/10/23 20:39:47 oliverschmidt Exp $
#

ifndef CONTIKI
  ${error CONTIKI not defined! You must specify where CONTIKI resides}
endif

ifndef CC65_HOME
  ${error CC65_HOME not defined! You must specify where cc65 resides}
endif

ifndef AC
  ${error AC not defined! You must specify where the AppleCommander jar resides}
endif

ifndef C1541
  ${error C1541 not defined! You must specify where VICE c1541 resides}
endif

ifndef DIR2ATR
  ${error DIR2ATR not defined! You must specify where dir2atr resides}
endif

disks: apple2-dsk-1 apple2-dsk-2 apple2-dsk-3 apple2-dsk-4 \
          c64-d64-1    c64-d64-2    c64-d71      c64-d81 \
         c128-d64-1   c128-d64-2   c128-d71     c128-d81 \
        atari-atr-1  atari-atr-2  atari-atr-3  atari-atr-4

define makes
$1-makes:
	$(MAKE) -C ../../cpu/6502/ipconfig   TARGET=$1
	$(MAKE) -C ../../examples/webserver  TARGET=$1 HTTPD-CFS=1
	$(MAKE) -C ../../examples/webbrowser TARGET=$1
	$(MAKE) -C ../../examples/wget       TARGET=$1
	$(MAKE) -C ../../examples/irc        TARGET=$1
	$(MAKE) -C ../../examples/email      TARGET=$1
	$(MAKE) -C ../../examples/ftp        TARGET=$1
	$(MAKE) -C ../../../contikiprojects/vandenbrande.com/twitter/platform/$1
endef

$(eval $(call makes,apple2enh))

apple2-dsk-1: apple2enh-makes
	cp ../apple2enh/prodos.dsk contiki-apple2-1.dsk
	java -jar $(AC) -p    contiki-apple2-1.dsk contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    contiki-apple2-1.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-apple2-1.dsk ipconfig.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-1.dsk ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    contiki-apple2-1.dsk webserve.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-1.dsk webserve        bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    contiki-apple2-1.dsk webbrows.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-1.dsk webbrows        bin   < ../../examples/webbrowser/webbrowser.apple2enh
	java -jar $(AC) -p    contiki-apple2-1.dsk cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    contiki-apple2-1.dsk lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    contiki-apple2-1.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou
	java -jar $(AC) -p    contiki-apple2-1.dsk index.htm       bin 0 < ../../examples/webserver/httpd-cfs/index.htm
	java -jar $(AC) -p    contiki-apple2-1.dsk backgrnd.gif    bin 0 < ../../examples/webserver/httpd-cfs/backgrnd.gif
	java -jar $(AC) -p    contiki-apple2-1.dsk contiki.gif     bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    contiki-apple2-1.dsk notfound.htm    bin 0 < ../../examples/webserver/httpd-cfs/notfound.htm

apple2-dsk-2: apple2enh-makes
	cp ../apple2enh/prodos.dsk contiki-apple2-2.dsk
	java -jar $(AC) -p    contiki-apple2-2.dsk contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    contiki-apple2-2.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-apple2-2.dsk ipconfig.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-2.dsk ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    contiki-apple2-2.dsk wget.system     sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-2.dsk wget            bin   < ../../examples/wget/wget.apple2enh
	java -jar $(AC) -p    contiki-apple2-2.dsk irc.system      sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-2.dsk irc             bin   < ../../examples/irc/irc-client.apple2enh
	java -jar $(AC) -p    contiki-apple2-2.dsk cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    contiki-apple2-2.dsk lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    contiki-apple2-2.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

apple2-dsk-3: apple2enh-makes
	cp ../apple2enh/prodos.dsk contiki-apple2-3.dsk
	java -jar $(AC) -p    contiki-apple2-3.dsk contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    contiki-apple2-3.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-apple2-3.dsk ipconfig.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-3.dsk ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    contiki-apple2-3.dsk email.system    sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-3.dsk email           bin   < ../../examples/email/email-client.apple2enh
	java -jar $(AC) -p    contiki-apple2-3.dsk ftp.system      sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-3.dsk ftp             bin   < ../../examples/ftp/ftp-client.apple2enh
	java -jar $(AC) -p    contiki-apple2-3.dsk cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    contiki-apple2-3.dsk lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    contiki-apple2-3.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

apple2-dsk-4: apple2enh-makes
	cp ../apple2enh/prodos.dsk contiki-apple2-4.dsk
	java -jar $(AC) -p    contiki-apple2-4.dsk contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    contiki-apple2-4.dsk menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    contiki-apple2-4.dsk ipconfig.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-4.dsk ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    contiki-apple2-4.dsk breadbox.system sys 0 < ../apple2enh/loader.system
	java -jar $(AC) -cc65 contiki-apple2-4.dsk breadbox        bin   < ../../../contikiprojects/vandenbrande.com/twitter/platform/apple2enh/breadbox64.apple2enh
	java -jar $(AC) -p    contiki-apple2-4.dsk cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    contiki-apple2-4.dsk lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    contiki-apple2-4.dsk a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

$(eval $(call makes,c64))

c64-d64-1: c64-makes
	$(C1541) -format contiki-1,00 d64 contiki-c64-1.d64
	$(C1541) -attach contiki-c64-1.d64 -write ../c64/default.cfg                              contiki.cfg,u
	$(C1541) -attach contiki-c64-1.d64 -write ../../cpu/6502/ipconfig/ipconfig.c64            ipconfig,p
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/webserver/webserver-example.c64  webserver,p
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/webbrowser/webbrowser.c64        webbrowser,p
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/wget/wget.c64                    wget,p
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/irc/irc-client.c64               irc,p
	$(C1541) -attach contiki-c64-1.d64 -write ../../cpu/6502/ipconfig/cs8900a.eth             cs8900a.eth,u
	$(C1541) -attach contiki-c64-1.d64 -write ../../cpu/6502/ipconfig/lan91c96.eth            lan91c96.eth,u
	$(C1541) -attach contiki-c64-1.d64 -write $(CC65_HOME)/mou/c64-1351.mou                   c64-1351.mou,u
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,u
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,u
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,u
	$(C1541) -attach contiki-c64-1.d64 -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,u

c64-d64-2: c64-makes
	$(C1541) -format contiki-2,00 d64 contiki-c64-2.d64
	$(C1541) -attach contiki-c64-2.d64 -write ../c64/default.cfg                                                            contiki.cfg,u
	$(C1541) -attach contiki-c64-2.d64 -write ../../cpu/6502/ipconfig/ipconfig.c64                                          ipconfig,p
	$(C1541) -attach contiki-c64-2.d64 -write ../../examples/email/email-client.c64                                         email,p
	$(C1541) -attach contiki-c64-2.d64 -write ../../examples/ftp/ftp-client.c64                                             ftp,p
	$(C1541) -attach contiki-c64-2.d64 -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c64/breadbox64.c64 breadbox64,p
	$(C1541) -attach contiki-c64-2.d64 -write ../../cpu/6502/ipconfig/cs8900a.eth                                           cs8900a.eth,u
	$(C1541) -attach contiki-c64-2.d64 -write ../../cpu/6502/ipconfig/lan91c96.eth                                          lan91c96.eth,u
	$(C1541) -attach contiki-c64-2.d64 -write $(CC65_HOME)/mou/c64-1351.mou                                                 c64-1351.mou,u

c64-d71: c64-makes
	$(C1541) -format contiki,00 d71 contiki-c64.d71
	$(C1541) -attach contiki-c64.d71 -write ../c64/default.cfg                                                            contiki.cfg,u
	$(C1541) -attach contiki-c64.d71 -write ../../cpu/6502/ipconfig/ipconfig.c64                                          ipconfig,p
	$(C1541) -attach contiki-c64.d71 -write ../../examples/webserver/webserver-example.c64                                webserver,p
	$(C1541) -attach contiki-c64.d71 -write ../../examples/webbrowser/webbrowser.c64                                      webbrowser,p
	$(C1541) -attach contiki-c64.d71 -write ../../examples/wget/wget.c64                                                  wget,p
	$(C1541) -attach contiki-c64.d71 -write ../../examples/irc/irc-client.c64                                             irc,p
	$(C1541) -attach contiki-c64.d71 -write ../../examples/email/email-client.c64                                         email,p
	$(C1541) -attach contiki-c64.d71 -write ../../examples/ftp/ftp-client.c64                                             ftp,p
	$(C1541) -attach contiki-c64.d71 -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c64/breadbox64.c64 breadbox64,p
	$(C1541) -attach contiki-c64.d71 -write ../../cpu/6502/ipconfig/cs8900a.eth                                           cs8900a.eth,u
	$(C1541) -attach contiki-c64.d71 -write ../../cpu/6502/ipconfig/lan91c96.eth                                          lan91c96.eth,u
	$(C1541) -attach contiki-c64.d71 -write ../../examples/webserver/httpd-cfs/index.htm                                  index.htm,u
	$(C1541) -attach contiki-c64.d71 -write ../../examples/webserver/httpd-cfs/backgrnd.gif                               backgrnd.gif,u
	$(C1541) -attach contiki-c64.d71 -write ../../examples/webserver/httpd-cfs/contiki.gif                                contiki.gif,u
	$(C1541) -attach contiki-c64.d71 -write ../../examples/webserver/httpd-cfs/notfound.htm                               notfound.htm,u

c64-d81: c64-makes
	$(C1541) -format contiki,00 d81 contiki-c64.d81
	$(C1541) -attach contiki-c64.d81 -write ../c64/default.cfg                                                            contiki.cfg,u
	$(C1541) -attach contiki-c64.d81 -write ../../cpu/6502/ipconfig/ipconfig.c64                                          ipconfig,p
	$(C1541) -attach contiki-c64.d81 -write ../../examples/webserver/webserver-example.c64                                webserver,p
	$(C1541) -attach contiki-c64.d81 -write ../../examples/webbrowser/webbrowser.c64                                      webbrowser,p
	$(C1541) -attach contiki-c64.d81 -write ../../examples/wget/wget.c64                                                  wget,p
	$(C1541) -attach contiki-c64.d81 -write ../../examples/irc/irc-client.c64                                             irc,p
	$(C1541) -attach contiki-c64.d81 -write ../../examples/email/email-client.c64                                         email,p
	$(C1541) -attach contiki-c64.d81 -write ../../examples/ftp/ftp-client.c64                                             ftp,p
	$(C1541) -attach contiki-c64.d81 -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c64/breadbox64.c64 breadbox64,p
	$(C1541) -attach contiki-c64.d81 -write ../../cpu/6502/ipconfig/cs8900a.eth                                           cs8900a.eth,u
	$(C1541) -attach contiki-c64.d81 -write ../../cpu/6502/ipconfig/lan91c96.eth                                          lan91c96.eth,u
	$(C1541) -attach contiki-c64.d81 -write ../../examples/webserver/httpd-cfs/index.htm                                  index.htm,u
	$(C1541) -attach contiki-c64.d81 -write ../../examples/webserver/httpd-cfs/backgrnd.gif                               backgrnd.gif,u
	$(C1541) -attach contiki-c64.d81 -write ../../examples/webserver/httpd-cfs/contiki.gif                                contiki.gif,u
	$(C1541) -attach contiki-c64.d81 -write ../../examples/webserver/httpd-cfs/notfound.htm                               notfound.htm,u

$(eval $(call makes,c128))

c128-d64-1: c128-makes
	$(C1541) -format contiki-1,00 d64 contiki-c128-1.d64
	$(C1541) -attach contiki-c128-1.d64 -write ../c128/default.cfg                             contiki.cfg,u
	$(C1541) -attach contiki-c128-1.d64 -write ../../cpu/6502/ipconfig/ipconfig.c128           ipconfig,p
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/webserver/webserver-example.c128 webserver,p
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/webbrowser/webbrowser.c128       webbrowser,p
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/wget/wget.c128                   wget,p
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/irc/irc-client.c128              irc,p
	$(C1541) -attach contiki-c128-1.d64 -write ../../cpu/6502/ipconfig/cs8900a.eth             cs8900a.eth,u
	$(C1541) -attach contiki-c128-1.d64 -write ../../cpu/6502/ipconfig/lan91c96.eth            lan91c96.eth,u
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,u
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,u
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,u
	$(C1541) -attach contiki-c128-1.d64 -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,u

c128-d64-2: c128-makes
	$(C1541) -format contiki-2,00 d64 contiki-c128-2.d64
	$(C1541) -attach contiki-c128-2.d64 -write ../c128/default.cfg                                                             contiki.cfg,u
	$(C1541) -attach contiki-c128-2.d64 -write ../../cpu/6502/ipconfig/ipconfig.c128                                           ipconfig,p
	$(C1541) -attach contiki-c128-2.d64 -write ../../examples/email/email-client.c128                                          email,p
	$(C1541) -attach contiki-c128-2.d64 -write ../../examples/ftp/ftp-client.c128                                              ftp,p
	$(C1541) -attach contiki-c128-2.d64 -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c128/breadbox64.c128 breadbox64,p
	$(C1541) -attach contiki-c128-2.d64 -write ../../cpu/6502/ipconfig/cs8900a.eth                                             cs8900a.eth,u
	$(C1541) -attach contiki-c128-2.d64 -write ../../cpu/6502/ipconfig/lan91c96.eth                                            lan91c96.eth,u

c128-d71: c128-makes
	$(C1541) -format contiki,00 d71 contiki-c128.d71
	$(C1541) -attach contiki-c128.d71 -write ../c128/default.cfg                                                             contiki.cfg,u
	$(C1541) -attach contiki-c128.d71 -write ../../cpu/6502/ipconfig/ipconfig.c128                                           ipconfig,p
	$(C1541) -attach contiki-c128.d71 -write ../../examples/webserver/webserver-example.c128                                 webserver,p
	$(C1541) -attach contiki-c128.d71 -write ../../examples/webbrowser/webbrowser.c128                                       webbrowser,p
	$(C1541) -attach contiki-c128.d71 -write ../../examples/wget/wget.c128                                                   wget,p
	$(C1541) -attach contiki-c128.d71 -write ../../examples/irc/irc-client.c128                                              irc,p
	$(C1541) -attach contiki-c128.d71 -write ../../examples/email/email-client.c128                                          email,p
	$(C1541) -attach contiki-c128.d71 -write ../../examples/ftp/ftp-client.c128                                              ftp,p
	$(C1541) -attach contiki-c128.d71 -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c128/breadbox64.c128 breadbox64,p
	$(C1541) -attach contiki-c128.d71 -write ../../cpu/6502/ipconfig/cs8900a.eth                                             cs8900a.eth,u
	$(C1541) -attach contiki-c128.d71 -write ../../cpu/6502/ipconfig/lan91c96.eth                                            lan91c96.eth,u
	$(C1541) -attach contiki-c128.d71 -write ../../examples/webserver/httpd-cfs/index.htm                                    index.htm,u
	$(C1541) -attach contiki-c128.d71 -write ../../examples/webserver/httpd-cfs/backgrnd.gif                                 backgrnd.gif,u
	$(C1541) -attach contiki-c128.d71 -write ../../examples/webserver/httpd-cfs/contiki.gif                                  contiki.gif,u
	$(C1541) -attach contiki-c128.d71 -write ../../examples/webserver/httpd-cfs/notfound.htm                                 notfound.htm,u

c128-d81: c128-makes
	$(C1541) -format contiki,00 d81 contiki-c128.d81
	$(C1541) -attach contiki-c128.d81 -write ../c128/default.cfg                                                             contiki.cfg,u
	$(C1541) -attach contiki-c128.d81 -write ../../cpu/6502/ipconfig/ipconfig.c128                                           ipconfig,p
	$(C1541) -attach contiki-c128.d81 -write ../../examples/webserver/webserver-example.c128                                 webserver,p
	$(C1541) -attach contiki-c128.d81 -write ../../examples/webbrowser/webbrowser.c128                                       webbrowser,p
	$(C1541) -attach contiki-c128.d81 -write ../../examples/wget/wget.c128                                                   wget,p
	$(C1541) -attach contiki-c128.d81 -write ../../examples/irc/irc-client.c128                                              irc,p
	$(C1541) -attach contiki-c128.d81 -write ../../examples/email/email-client.c128                                          email,p
	$(C1541) -attach contiki-c128.d81 -write ../../examples/ftp/ftp-client.c128                                              ftp,p
	$(C1541) -attach contiki-c128.d81 -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c128/breadbox64.c128 breadbox64,p
	$(C1541) -attach contiki-c128.d81 -write ../../cpu/6502/ipconfig/cs8900a.eth                                             cs8900a.eth,u
	$(C1541) -attach contiki-c128.d81 -write ../../cpu/6502/ipconfig/lan91c96.eth                                            lan91c96.eth,u
	$(C1541) -attach contiki-c128.d81 -write ../../examples/webserver/httpd-cfs/index.htm                                    index.htm,u
	$(C1541) -attach contiki-c128.d81 -write ../../examples/webserver/httpd-cfs/backgrnd.gif                                 backgrnd.gif,u
	$(C1541) -attach contiki-c128.d81 -write ../../examples/webserver/httpd-cfs/contiki.gif                                  contiki.gif,u
	$(C1541) -attach contiki-c128.d81 -write ../../examples/webserver/httpd-cfs/notfound.htm                                 notfound.htm,u

$(eval $(call makes,atari))

atari-atr-1: atari-makes
	mkdir atr
	cp ../atari/dos.sys                                 atr/dos.sys
	cp ../atari/dup.sys                                 atr/dup.sys
	cp ../atari/default.cfg                             atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/ipconfig.atari           atr/ipconfig.com
	cp ../../examples/webserver/webserver-example.atari atr/webserve.com
	cp ../../examples/webbrowser/webbrowser.atari       atr/webbrows.com
	cp ../../examples/webserver/httpd-cfs/index.htm     atr/index.htm
	cp ../../examples/webserver/httpd-cfs/backgrnd.gif  atr/backgrnd.gif
	cp ../../examples/webserver/httpd-cfs/contiki.gif   atr/contiki.gif
	cp ../../examples/webserver/httpd-cfs/notfound.htm  atr/notfound.htm
	$(DIR2ATR) -b Dos25 1040 contiki-atari-1.atr atr
	rm -r atr

atari-atr-2: atari-makes
	mkdir atr
	cp ../atari/dos.sys                       atr/dos.sys
	cp ../atari/dup.sys                       atr/dup.sys
	cp ../atari/default.cfg                   atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/ipconfig.atari atr/ipconfig.com
	cp ../../examples/wget/wget.atari         atr/wget.com
	cp ../../examples/irc/irc-client.atari    atr/irc.com
	$(DIR2ATR) -b Dos25 1040 contiki-atari-2.atr atr
	rm -r atr

atari-atr-3: atari-makes
	mkdir atr
	cp ../atari/dos.sys                        atr/dos.sys
	cp ../atari/dup.sys                        atr/dup.sys
	cp ../atari/default.cfg                    atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/ipconfig.atari  atr/ipconfig.com
	cp ../../examples/email/email-client.atari atr/email.com
	cp ../../examples/ftp/ftp-client.atari     atr/ftp.com
	$(DIR2ATR) -b Dos25 1040 contiki-atari-3.atr atr
	rm -r atr

atari-atr-4: atari-makes
	mkdir atr
	cp ../atari/dos.sys                                                                  atr/dos.sys
	cp ../atari/dup.sys                                                                  atr/dup.sys
	cp ../atari/default.cfg                                                              atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/ipconfig.atari                                            atr/ipconfig.com
	cp ../../../contikiprojects/vandenbrande.com/twitter/platform/atari/breadbox64.atari atr/breadbox.com
	$(DIR2ATR) -b Dos25 1040 contiki-atari-4.atr atr
	rm -r atr
