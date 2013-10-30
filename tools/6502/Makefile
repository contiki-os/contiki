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

ifndef CONTIKI
  ${error CONTIKI not defined! You must specify where Contiki resides}
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

all: apple2 atari c64 c128

define makes
$1-makes:
	$(MAKE) -C ../../cpu/6502/ipconfig      TARGET=$1
	$(MAKE) -C ../../examples/webbrowser    TARGET=$1
	$(MAKE) -C ../../examples/wget          TARGET=$1
	$(MAKE) -C ../../examples/irc           TARGET=$1
	$(MAKE) -C ../../../contikiprojects/vandenbrande.com/twitter/platform/$1
	$(MAKE) -C ../../examples/email         TARGET=$1
	$(MAKE) -C ../../examples/ftp           TARGET=$1
	$(MAKE) -C ../../examples/webserver     TARGET=$1 HTTPD-CFS=1
	$(MAKE) -C ../../examples/telnet-server TARGET=$1
endef

$(eval $(call makes,apple2enh))

apple2: contiki-apple2-1.dsk contiki-apple2-2.dsk contiki-apple2-3.dsk contiki-apple2-4.dsk contiki-apple2.2mg

contiki-apple2-1.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ ipconfig.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ webbrows.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ webbrows        bin   < ../../examples/webbrowser/webbrowser.apple2enh
	java -jar $(AC) -p    $@ wget.system     sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ wget            bin   < ../../examples/wget/wget.apple2enh
	java -jar $(AC) -p    $@ contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    $@ a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

contiki-apple2-2.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ ipconfig.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ irc.system      sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ irc             bin   < ../../examples/irc/irc-client.apple2enh
	java -jar $(AC) -p    $@ breadbox.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ breadbox        bin   < ../../../contikiprojects/vandenbrande.com/twitter/platform/apple2enh/breadbox64.apple2enh
	java -jar $(AC) -p    $@ contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    $@ a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

contiki-apple2-3.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ ipconfig.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ email.system    sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ email           bin   < ../../examples/email/email-client.apple2enh
	java -jar $(AC) -p    $@ ftp.system      sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ftp             bin   < ../../examples/ftp/ftp-client.apple2enh
	java -jar $(AC) -p    $@ contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    $@ a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou

contiki-apple2-4.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ ipconfig.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ webserv.system  sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ webserv         bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    $@ telnetd.system  sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ telnetd         bin   < ../../examples/telnet-server/telnet-server.apple2enh
	java -jar $(AC) -p    $@ contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    $@ a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou
	java -jar $(AC) -p    $@ index.htm       bin 0 < ../../examples/webserver/httpd-cfs/index.htm
	java -jar $(AC) -p    $@ backgrnd.gif    bin 0 < ../../examples/webserver/httpd-cfs/backgrnd.gif
	java -jar $(AC) -p    $@ contiki.gif     bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    $@ notfound.htm    bin 0 < ../../examples/webserver/httpd-cfs/notfound.htm

contiki-apple2.2mg: apple2enh-makes
	cp ../apple2enh/prodos.2mg $@
	java -jar $(AC) -p    $@ menu.system     sys 0 < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ ipconfig.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ipconfig        bin 0 < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ webbrows.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ webbrows        bin   < ../../examples/webbrowser/webbrowser.apple2enh
	java -jar $(AC) -p    $@ wget.system     sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ wget            bin   < ../../examples/wget/wget.apple2enh
	java -jar $(AC) -p    $@ irc.system      sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ irc             bin   < ../../examples/irc/irc-client.apple2enh
	java -jar $(AC) -p    $@ breadbox.system sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ breadbox        bin   < ../../../contikiprojects/vandenbrande.com/twitter/platform/apple2enh/breadbox64.apple2enh
	java -jar $(AC) -p    $@ email.system    sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ email           bin   < ../../examples/email/email-client.apple2enh
	java -jar $(AC) -p    $@ ftp.system      sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ ftp             bin   < ../../examples/ftp/ftp-client.apple2enh
	java -jar $(AC) -p    $@ webserv.system  sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ webserv         bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    $@ telnetd.system  sys 0 < $(CC65_HOME)/targetutil/loader.system
	java -jar $(AC) -cc65 $@ telnetd         bin   < ../../examples/telnet-server/telnet-server.apple2enh
	java -jar $(AC) -p    $@ contiki.cfg     bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth     rel 0 < ../../cpu/6502/ipconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth    rel 0 < ../../cpu/6502/ipconfig/lan91c96.eth
	java -jar $(AC) -p    $@ a2e.stdmou.mou  rel 0 < $(CC65_HOME)/mou/a2e.stdmou.mou
	java -jar $(AC) -p    $@ index.htm       bin 0 < ../../examples/webserver/httpd-cfs/index.htm
	java -jar $(AC) -p    $@ backgrnd.gif    bin 0 < ../../examples/webserver/httpd-cfs/backgrnd.gif
	java -jar $(AC) -p    $@ contiki.gif     bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    $@ notfound.htm    bin 0 < ../../examples/webserver/httpd-cfs/notfound.htm

$(eval $(call makes,atarixl))

atari: contiki-atari-1.atr contiki-atari-2.atr contiki-atari-3.atr contiki-atari-4.atr contiki-atari.atr

contiki-atari-1.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                     atr/dos.sys
	cp ../atarixl/dos25/dup.sys                     atr/dup.sys
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl     atr/ipconfig.com
	cp ../../examples/webbrowser/webbrowser.atarixl atr/webbrows.com
	cp ../../examples/wget/wget.atarixl             atr/wget.com
	cp ../atarixl/default.cfg                       atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/cs8900a.eth          atr/cs8900a.eth
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari-2.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                                                              atr/dos.sys
	cp ../atarixl/dos25/dup.sys                                                              atr/dup.sys
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl                                              atr/ipconfig.com
	cp ../../examples/irc/irc-client.atarixl                                                 atr/irc.com
	cp ../../../contikiprojects/vandenbrande.com/twitter/platform/atarixl/breadbox64.atarixl atr/breadbox.com
	cp ../atarixl/default.cfg                                                                atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/cs8900a.eth                                                   atr/cs8900a.eth
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari-3.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                  atr/dos.sys
	cp ../atarixl/dos25/dup.sys                  atr/dup.sys
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl  atr/ipconfig.com
	cp ../../examples/email/email-client.atarixl atr/email.com
	cp ../../examples/ftp/ftp-client.atarixl     atr/ftp.com
	cp ../atarixl/default.cfg                    atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/cs8900a.eth       atr/cs8900a.eth
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari-4.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                           atr/dos.sys
	cp ../atarixl/dos25/dup.sys                           atr/dup.sys
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl           atr/ipconfig.com
	cp ../../examples/webserver/webserver-example.atarixl atr/webserv.com
	cp ../../examples/telnet-server/telnet-server.atarixl atr/telnetd.com
	cp ../atarixl/default.cfg                             atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/cs8900a.eth                atr/cs8900a.eth
	cp ../../examples/webserver/httpd-cfs/index.htm       atr/index.htm
	cp ../../examples/webserver/httpd-cfs/backgrnd.gif    atr/backgrnd.gif
	cp ../../examples/webserver/httpd-cfs/contiki.gif     atr/contiki.gif
	cp ../../examples/webserver/httpd-cfs/notfound.htm    atr/notfound.htm
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/mydos4534/dos.sys                                                          atr/dos.sys
	cp ../atarixl/mydos4534/dup.sys                                                          atr/dup.sys
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl                                              atr/ipconfig.com
	cp ../../examples/webbrowser/webbrowser.atarixl                                          atr/webbrows.com
	cp ../../examples/wget/wget.atarixl                                                      atr/wget.com
	cp ../../examples/irc/irc-client.atarixl                                                 atr/irc.com
	cp ../../../contikiprojects/vandenbrande.com/twitter/platform/atarixl/breadbox64.atarixl atr/breadbox.com
	cp ../../examples/email/email-client.atarixl                                             atr/email.com
	cp ../../examples/ftp/ftp-client.atarixl                                                 atr/ftp.com
	cp ../../examples/webserver/webserver-example.atarixl                                    atr/webserv.com
	cp ../../examples/telnet-server/telnet-server.atarixl                                    atr/telnetd.com
	cp ../atarixl/default.cfg                                                                atr/contiki.cfg
	cp ../../cpu/6502/ipconfig/cs8900a.eth                                                   atr/cs8900a.eth
	cp ../../examples/webserver/httpd-cfs/index.htm                                          atr/index.htm
	cp ../../examples/webserver/httpd-cfs/backgrnd.gif                                       atr/backgrnd.gif
	cp ../../examples/webserver/httpd-cfs/contiki.gif                                        atr/contiki.gif
	cp ../../examples/webserver/httpd-cfs/notfound.htm                                       atr/notfound.htm
	$(DIR2ATR) -d -b MyDos4534 3200 $@ atr
	rm -r atr

$(eval $(call makes,c64))

c64: contiki-c64-1.d64 contiki-c64-2.d64 contiki-c64.d71 contiki-c64.d81

contiki-c64-1.d64: c64-makes
	$(C1541) -format contiki-1,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64                                          ipconfig,p
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c64                                      webbrowser,p
	$(C1541) -attach $@ -write ../../examples/wget/wget.c64                                                  wget,p
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c64                                             irc,p
	$(C1541) -attach $@ -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c64/breadbox64.c64 breadbox64,p
	$(C1541) -attach $@ -write ../c64/default.cfg                                                            contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth                                           cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth                                          lan91c96.eth,u
	$(C1541) -attach $@ -write $(CC65_HOME)/mou/c64-1351.mou                                                 c64-1351.mou,u

contiki-c64-2.d64: c64-makes
	$(C1541) -format contiki-2,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64            ipconfig,p
	$(C1541) -attach $@ -write ../../examples/email/email-client.c64           email,p
	$(C1541) -attach $@ -write ../../examples/ftp/ftp-client.c64               ftp,p
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c64  webserver,p
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c64  telnetd,p
	$(C1541) -attach $@ -write ../c64/default.cfg                              contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth             cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth            lan91c96.eth,u
	$(C1541) -attach $@ -write $(CC65_HOME)/mou/c64-1351.mou                   c64-1351.mou,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,u

contiki-c64.d71: c64-makes
	$(C1541) -format contiki,00 d71 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64                                          ipconfig,p
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c64                                      webbrowser,p
	$(C1541) -attach $@ -write ../../examples/wget/wget.c64                                                  wget,p
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c64                                             irc,p
	$(C1541) -attach $@ -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c64/breadbox64.c64 breadbox64,p
	$(C1541) -attach $@ -write ../../examples/email/email-client.c64                                         email,p
	$(C1541) -attach $@ -write ../../examples/ftp/ftp-client.c64                                             ftp,p
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c64                                webserver,p
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c64                                telnetd,p
	$(C1541) -attach $@ -write ../c64/default.cfg                                                            contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth                                           cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth                                          lan91c96.eth,u
	$(C1541) -attach $@ -write $(CC65_HOME)/mou/c64-1351.mou                                                 c64-1351.mou,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm                                  index.htm,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif                               backgrnd.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif                                contiki.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm                               notfound.htm,u

contiki-c64.d81: c64-makes
	$(C1541) -format contiki,00 d81 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64                                          ipconfig,p
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c64                                      webbrowser,p
	$(C1541) -attach $@ -write ../../examples/wget/wget.c64                                                  wget,p
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c64                                             irc,p
	$(C1541) -attach $@ -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c64/breadbox64.c64 breadbox64,p
	$(C1541) -attach $@ -write ../../examples/email/email-client.c64                                         email,p
	$(C1541) -attach $@ -write ../../examples/ftp/ftp-client.c64                                             ftp,p
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c64                                webserver,p
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c64                                telnetd,p
	$(C1541) -attach $@ -write ../c64/default.cfg                                                            contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth                                           cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth                                          lan91c96.eth,u
	$(C1541) -attach $@ -write $(CC65_HOME)/mou/c64-1351.mou                                                 c64-1351.mou,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm                                  index.htm,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif                               backgrnd.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif                                contiki.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm                               notfound.htm,u

$(eval $(call makes,c128))

c128: contiki-c128-1.d64 contiki-c128-2.d64 contiki-c128.d71 contiki-c128.d81

contiki-c128-1.d64: c128-makes
	$(C1541) -format contiki-1,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128                                           ipconfig,p
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c128                                       webbrowser,p
	$(C1541) -attach $@ -write ../../examples/wget/wget.c128                                                   wget,p
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c128                                              irc,p
	$(C1541) -attach $@ -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c128/breadbox64.c128 breadbox64,p
	$(C1541) -attach $@ -write ../c128/default.cfg                                                             contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth                                             cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth                                            lan91c96.eth,u

contiki-c128-2.d64: c128-makes
	$(C1541) -format contiki-2,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128           ipconfig,p
	$(C1541) -attach $@ -write ../../examples/email/email-client.c128          email,p
	$(C1541) -attach $@ -write ../../examples/ftp/ftp-client.c128              ftp,p
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c128 webserver,p
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c128 telnetd,p
	$(C1541) -attach $@ -write ../c128/default.cfg                             contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth             cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth            lan91c96.eth,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,u

contiki-c128.d71: c128-makes
	$(C1541) -format contiki,00 d71 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128                                           ipconfig,p
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c128                                       webbrowser,p
	$(C1541) -attach $@ -write ../../examples/wget/wget.c128                                                   wget,p
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c128                                              irc,p
	$(C1541) -attach $@ -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c128/breadbox64.c128 breadbox64,p
	$(C1541) -attach $@ -write ../../examples/email/email-client.c128                                          email,p
	$(C1541) -attach $@ -write ../../examples/ftp/ftp-client.c128                                              ftp,p
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c128                                 webserver,p
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c128                                 telnetd,p
	$(C1541) -attach $@ -write ../c128/default.cfg                                                             contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth                                             cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth                                            lan91c96.eth,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm                                    index.htm,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif                                 backgrnd.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif                                  contiki.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm                                 notfound.htm,u

contiki-c128.d81: c128-makes
	$(C1541) -format contiki,00 d81 $@
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128                                           ipconfig,p
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c128                                       webbrowser,p
	$(C1541) -attach $@ -write ../../examples/wget/wget.c128                                                   wget,p
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c128                                              irc,p
	$(C1541) -attach $@ -write ../../../contikiprojects/vandenbrande.com/twitter/platform/c128/breadbox64.c128 breadbox64,p
	$(C1541) -attach $@ -write ../../examples/email/email-client.c128                                          email,p
	$(C1541) -attach $@ -write ../../examples/ftp/ftp-client.c128                                              ftp,p
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c128                                 webserver,p
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c128                                 telnetd,p
	$(C1541) -attach $@ -write ../c128/default.cfg                                                             contiki.cfg,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/cs8900a.eth                                             cs8900a.eth,u
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/lan91c96.eth                                            lan91c96.eth,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm                                    index.htm,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif                                 backgrnd.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif                                  contiki.gif,u
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm                                 notfound.htm,u
