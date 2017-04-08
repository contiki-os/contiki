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
clean: apple2-clean atari-clean c64-clean c128-clean

ifeq ($(shell echo),)
  NULLDEV = /dev/null
else
  NULLDEV = nul:
endif

ZIPCOMMENT := $(shell git rev-parse --short HEAD 2>$(NULLDEV))
ifeq ($(words $(ZIPCOMMENT)),1)
  ZIPCOMMENT := https://github.com/contiki-os/contiki/commits/$(ZIPCOMMENT)
else
  ZIPCOMMENT := N/A
endif

CC65 := $(shell cl65 --print-target-path)

ifdef SLIP
DEV = ser
else
DEV = eth
endif

define makes
.PHONY: $1-$2makes
$1-$2makes:
	$(MAKE) -C ../../cpu/6502/$(DEV)config     TARGET=$1             $2
	$(MAKE) -C ../../cpu/6502/ipconfig         TARGET=$1             $2
	$(MAKE) -C ../../examples/webbrowser       TARGET=$1             $2
	$(MAKE) -C ../../examples/webbrowser-80col TARGET=$1             $2
	$(MAKE) -C ../../examples/wget             TARGET=$1             $2
	$(MAKE) -C ../../examples/irc              TARGET=$1             $2
	$(MAKE) -C ../../examples/irc-80col        TARGET=$1             $2
	$(MAKE) -C ../../examples/webserver        TARGET=$1 HTTPD-CFS=1 $2
	$(MAKE) -C ../../examples/telnet-server    TARGET=$1             $2
endef

$(eval $(call makes,apple2enh))
$(eval $(call makes,apple2enh,clean))

%.zip:
	zip $@ $^
	echo $(ZIPCOMMENT) | zip -z $@

.PHONY: apple2 apple2-clean atari atari-clean c64 c64-clean c128 c128-clean

apple2: contiki-apple2.zip

apple2-clean: apple2enh-cleanmakes
	rm -f contiki-apple2.zip contiki-apple2-1.dsk contiki-apple2-2.dsk contiki-apple2-3.dsk contiki-apple2.po

contiki-apple2.zip: contiki-apple2-1.dsk contiki-apple2-2.dsk contiki-apple2-3.dsk contiki-apple2.po

contiki-apple2-1.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system        sys   < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ $(DEV)confi.system sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ $(DEV)confi        bin   < ../../cpu/6502/$(DEV)config/$(DEV)config.apple2enh
	java -jar $(AC) -p    $@ ipconfig.system    sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ ipconfig           bin   < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ webbrows.system    sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ webbrows           bin   < ../../examples/webbrowser-80col/webbrowser.apple2enh
	java -jar $(AC) -p    $@ wget.system        sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ wget               bin   < ../../examples/wget/wget.apple2enh
ifdef SLIP
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < default.cfg
else
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth        rel 0 < ../../cpu/6502/ethconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth       rel 0 < ../../cpu/6502/ethconfig/lan91c96.eth
	java -jar $(AC) -p    $@ w5100.eth          rel 0 < ../../cpu/6502/ethconfig/w5100.eth
endif

contiki-apple2-2.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system        sys   < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ $(DEV)confi.system sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ $(DEV)confi        bin   < ../../cpu/6502/$(DEV)config/$(DEV)config.apple2enh
	java -jar $(AC) -p    $@ ipconfig.system    sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ ipconfig           bin   < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ irc.system         sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ irc                bin   < ../../examples/irc-80col/irc-client.apple2enh
ifdef SLIP
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < default.cfg
else
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth        rel 0 < ../../cpu/6502/ethconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth       rel 0 < ../../cpu/6502/ethconfig/lan91c96.eth
	java -jar $(AC) -p    $@ w5100.eth          rel 0 < ../../cpu/6502/ethconfig/w5100.eth
endif

contiki-apple2-3.dsk: apple2enh-makes
	cp ../apple2enh/prodos.dsk $@
	java -jar $(AC) -p    $@ menu.system        sys   < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ $(DEV)confi.system sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ $(DEV)confi        bin   < ../../cpu/6502/$(DEV)config/$(DEV)config.apple2enh
	java -jar $(AC) -p    $@ ipconfig.system    sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ ipconfig           bin   < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ webserv.system     sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ webserv            bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    $@ telnetd.system     sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ telnetd            bin   < ../../examples/telnet-server/telnet-server.apple2enh
ifdef SLIP
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < default.cfg
else
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth        rel 0 < ../../cpu/6502/ethconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth       rel 0 < ../../cpu/6502/ethconfig/lan91c96.eth
	java -jar $(AC) -p    $@ w5100.eth          rel 0 < ../../cpu/6502/ethconfig/w5100.eth
endif
	java -jar $(AC) -p    $@ index.htm          bin 0 < ../../examples/webserver/httpd-cfs/index.htm
	java -jar $(AC) -p    $@ backgrnd.gif       bin 0 < ../../examples/webserver/httpd-cfs/backgrnd.gif
	java -jar $(AC) -p    $@ contiki.gif        bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    $@ notfound.htm       bin 0 < ../../examples/webserver/httpd-cfs/notfound.htm

contiki-apple2.po: apple2enh-makes
	cp ../apple2enh/prodos.po $@
	java -jar $(AC) -p    $@ menu.system        sys   < ../apple2enh/menu.system
	java -jar $(AC) -p    $@ $(DEV)confi.system sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ $(DEV)confi        bin   < ../../cpu/6502/$(DEV)config/$(DEV)config.apple2enh
	java -jar $(AC) -p    $@ ipconfig.system    sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ ipconfig           bin   < ../../cpu/6502/ipconfig/ipconfig.apple2enh
	java -jar $(AC) -p    $@ webbrows.system    sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ webbrows           bin   < ../../examples/webbrowser-80col/webbrowser.apple2enh
	java -jar $(AC) -p    $@ wget.system        sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ wget               bin   < ../../examples/wget/wget.apple2enh
	java -jar $(AC) -p    $@ irc.system         sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ irc                bin   < ../../examples/irc-80col/irc-client.apple2enh
	java -jar $(AC) -p    $@ webserv.system     sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ webserv            bin   < ../../examples/webserver/webserver-example.apple2enh
	java -jar $(AC) -p    $@ telnetd.system     sys   < $(CC65)/apple2enh/util/loader.system
	java -jar $(AC) -cc65 $@ telnetd            bin   < ../../examples/telnet-server/telnet-server.apple2enh
ifdef SLIP
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < default.cfg
else
	java -jar $(AC) -p    $@ contiki.cfg        bin 0 < ../apple2enh/default.cfg
	java -jar $(AC) -p    $@ cs8900a.eth        rel 0 < ../../cpu/6502/ethconfig/cs8900a.eth
	java -jar $(AC) -p    $@ lan91c96.eth       rel 0 < ../../cpu/6502/ethconfig/lan91c96.eth
	java -jar $(AC) -p    $@ w5100.eth          rel 0 < ../../cpu/6502/ethconfig/w5100.eth
endif
	java -jar $(AC) -p    $@ contiki.mou        rel 0 < $(CC65)/apple2enh/drv/mou/a2e.stdmou.mou
	java -jar $(AC) -p    $@ index.htm          bin 0 < ../../examples/webserver/httpd-cfs/index.htm
	java -jar $(AC) -p    $@ backgrnd.gif       bin 0 < ../../examples/webserver/httpd-cfs/backgrnd.gif
	java -jar $(AC) -p    $@ contiki.gif        bin 0 < ../../examples/webserver/httpd-cfs/contiki.gif
	java -jar $(AC) -p    $@ notfound.htm       bin 0 < ../../examples/webserver/httpd-cfs/notfound.htm

$(eval $(call makes,atarixl))
$(eval $(call makes,atarixl,clean))

atari: contiki-atari.zip

atari-clean: atarixl-cleanmakes
	rm -f contiki-atari.zip contiki-atari-1.atr contiki-atari-2.atr contiki-atari-3.atr contiki-atari.atr

contiki-atari.zip: contiki-atari-1.atr contiki-atari-2.atr contiki-atari-3.atr contiki-atari.atr

contiki-atari-1.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                     atr/dos.sys
	cp ../atarixl/dos25/dup.sys                     atr/dup.sys
ifdef SLIP
	cp ../../cpu/6502/serconfig/serconfig.atarixl   atr/serconfi.com
endif
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl     atr/ipconfig.com
	cp ../../examples/webbrowser/webbrowser.atarixl atr/webbrows.com
	cp ../../examples/wget/wget.atarixl             atr/wget.com
ifdef SLIP
	cp default.cfg                                  atr/contiki.cfg
else
	cp ../atarixl/default.cfg                       atr/contiki.cfg
endif
	cp $(CC65)/atarixl/drv/mou/atrxst.mou           atr/contiki.mou
	cp $(CC65)/atarixl/drv/mou/atrxami.mou          atr/ami.mou
	cp $(CC65)/atarixl/drv/mou/atrxjoy.mou          atr/joy.mou
	cp $(CC65)/atarixl/drv/mou/atrxtrk.mou          atr/trk.mou
	cp $(CC65)/atarixl/drv/mou/atrxtt.mou           atr/tt.mou
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari-2.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                   atr/dos.sys
	cp ../atarixl/dos25/dup.sys                   atr/dup.sys
ifdef SLIP
	cp ../../cpu/6502/serconfig/serconfig.atarixl atr/serconfi.com
endif
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl   atr/ipconfig.com
	cp ../../examples/irc/irc-client.atarixl      atr/irc.com
ifdef SLIP
	cp default.cfg                                atr/contiki.cfg
else
	cp ../atarixl/default.cfg                     atr/contiki.cfg
endif
	cp $(CC65)/atarixl/drv/mou/atrxst.mou         atr/contiki.mou
	cp $(CC65)/atarixl/drv/mou/atrxami.mou        atr/ami.mou
	cp $(CC65)/atarixl/drv/mou/atrxjoy.mou        atr/joy.mou
	cp $(CC65)/atarixl/drv/mou/atrxtrk.mou        atr/trk.mou
	cp $(CC65)/atarixl/drv/mou/atrxtt.mou         atr/tt.mou
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari-3.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/dos25/dos.sys                           atr/dos.sys
	cp ../atarixl/dos25/dup.sys                           atr/dup.sys
ifdef SLIP
	cp ../../cpu/6502/serconfig/serconfig.atarixl         atr/serconfi.com
endif
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl           atr/ipconfig.com
	cp ../../examples/webserver/webserver-example.atarixl atr/webserv.com
	cp ../../examples/telnet-server/telnet-server.atarixl atr/telnetd.com
ifdef SLIP
	cp default.cfg                                        atr/contiki.cfg
else
	cp ../atarixl/default.cfg                             atr/contiki.cfg
endif
	cp $(CC65)/atarixl/drv/mou/atrxst.mou                 atr/contiki.mou
	cp $(CC65)/atarixl/drv/mou/atrxami.mou                atr/ami.mou
	cp $(CC65)/atarixl/drv/mou/atrxjoy.mou                atr/joy.mou
	cp $(CC65)/atarixl/drv/mou/atrxtrk.mou                atr/trk.mou
	cp $(CC65)/atarixl/drv/mou/atrxtt.mou                 atr/tt.mou
	cp ../../examples/webserver/httpd-cfs/index.htm       atr/index.htm
	cp ../../examples/webserver/httpd-cfs/backgrnd.gif    atr/backgrnd.gif
	cp ../../examples/webserver/httpd-cfs/contiki.gif     atr/contiki.gif
	cp ../../examples/webserver/httpd-cfs/notfound.htm    atr/notfound.htm
	$(DIR2ATR) -b Dos25 1040 $@ atr
	rm -r atr

contiki-atari.atr: atarixl-makes
	mkdir atr
	cp ../atarixl/mydos4534/dos.sys                       atr/dos.sys
	cp ../atarixl/mydos4534/dup.sys                       atr/dup.sys
ifdef SLIP
	cp ../../cpu/6502/serconfig/serconfig.atarixl         atr/serconfi.com
endif
	cp ../../cpu/6502/ipconfig/ipconfig.atarixl           atr/ipconfig.com
	cp ../../examples/webbrowser/webbrowser.atarixl       atr/webbrows.com
	cp ../../examples/wget/wget.atarixl                   atr/wget.com
	cp ../../examples/irc/irc-client.atarixl              atr/irc.com
	cp ../../examples/webserver/webserver-example.atarixl atr/webserv.com
	cp ../../examples/telnet-server/telnet-server.atarixl atr/telnetd.com
ifdef SLIP
	cp default.cfg                                        atr/contiki.cfg
else
	cp ../atarixl/default.cfg                             atr/contiki.cfg
endif
	cp $(CC65)/atarixl/drv/mou/atrxst.mou                 atr/contiki.mou
	cp $(CC65)/atarixl/drv/mou/atrxami.mou                atr/ami.mou
	cp $(CC65)/atarixl/drv/mou/atrxjoy.mou                atr/joy.mou
	cp $(CC65)/atarixl/drv/mou/atrxtrk.mou                atr/trk.mou
	cp $(CC65)/atarixl/drv/mou/atrxtt.mou                 atr/tt.mou
	cp ../../examples/webserver/httpd-cfs/index.htm       atr/index.htm
	cp ../../examples/webserver/httpd-cfs/backgrnd.gif    atr/backgrnd.gif
	cp ../../examples/webserver/httpd-cfs/contiki.gif     atr/contiki.gif
	cp ../../examples/webserver/httpd-cfs/notfound.htm    atr/notfound.htm
	$(DIR2ATR) -d -b MyDos4534 3200 $@ atr
	rm -r atr

$(eval $(call makes,c64))
$(eval $(call makes,c64,clean))

c64: contiki-c64.zip

c64-clean: c64-cleanmakes
	rm -f contiki-c64.zip contiki-c64-1.d64 contiki-c64-2.d64 contiki-c64-3.d64 contiki-c64.d71 contiki-c64.d81

contiki-c64.zip: contiki-c64-1.d64 contiki-c64-2.d64 contiki-c64-3.d64 contiki-c64.d71 contiki-c64.d81

contiki-c64-1.d64: c64-makes
	$(C1541) -format contiki-1,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c64   $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64           ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c64       webbrowser,p   >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser-80col/webbrowser.c64 webbrowser80,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/wget/wget.c64                   wget,p         >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                    contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c64/default.cfg                             contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth           cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth          lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-1351.mou               contiki.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-inkwell.mou            inkwell.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-joy.mou                joy.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-pot.mou                pot.mou,s      >$(NULLDEV)

contiki-c64-2.d64: c64-makes
	$(C1541) -format contiki-2,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c64 $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64         ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c64            irc,p          >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc-80col/irc-client.c64      irc80,p        >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                  contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c64/default.cfg                           contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth         cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth        lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-1351.mou             contiki.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-inkwell.mou          inkwell.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-joy.mou              joy.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-pot.mou              pot.mou,s      >$(NULLDEV)

contiki-c64-3.d64: c64-makes
	$(C1541) -format contiki-3,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c64    $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64            ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c64  webserver,p    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c64  telnetd,p      >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c64/default.cfg                              contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-1351.mou                contiki.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-inkwell.mou             inkwell.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-joy.mou                 joy.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-pot.mou                 pot.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,s    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,s >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,s >$(NULLDEV)

contiki-c64.d71: c64-makes
	$(C1541) -format contiki,00 d71 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c64    $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64            ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c64        webbrowser,p   >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser-80col/webbrowser.c64  webbrowser80,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/wget/wget.c64                    wget,p         >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c64               irc,p          >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc-80col/irc-client.c64         irc80,p        >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c64  webserver,p    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c64  telnetd,p      >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c64/default.cfg                              contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-1351.mou                contiki.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-inkwell.mou             inkwell.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-joy.mou                 joy.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-pot.mou                 pot.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,s    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,s >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,s >$(NULLDEV)

contiki-c64.d81: c64-makes
	$(C1541) -format contiki,00 d81 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c64    $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c64            ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser/webbrowser.c64        webbrowser,p   >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser-80col/webbrowser.c64  webbrowser80,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/wget/wget.c64                    wget,p         >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc/irc-client.c64               irc,p          >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc-80col/irc-client.c64         irc80,p        >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c64  webserver,p    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c64  telnetd,p      >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c64/default.cfg                              contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-1351.mou                contiki.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-inkwell.mou             inkwell.mou,s  >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-joy.mou                 joy.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write $(CC65)/c64/drv/mou/c64-pot.mou                 pot.mou,s      >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,s    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,s >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,s >$(NULLDEV)

$(eval $(call makes,c128))
$(eval $(call makes,c128,clean))

c128: contiki-c128.zip

c128-clean: c128-cleanmakes
	rm -f contiki-c128.zip contiki-c128-1.d64 contiki-c128-2.d64 contiki-c128.d71 contiki-c128.d81

contiki-c128.zip: contiki-c128-1.d64 contiki-c128-2.d64 contiki-c128.d71 contiki-c128.d81

contiki-c128-1.d64: c128-makes
	$(C1541) -format contiki-1,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c128   $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128           ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser-80col/webbrowser.c128 webbrowser,p   >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/wget/wget.c128                   wget,p         >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc-80col/irc-client.c128        irc,p          >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c128/default.cfg                             contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif

contiki-c128-2.d64: c128-makes
	$(C1541) -format contiki-3,00 d64 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c128   $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128           ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c128 webserver,p    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c128 telnetd,p      >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c128/default.cfg                             contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,s    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,s >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,s >$(NULLDEV)

contiki-c128.d71: c128-makes
	$(C1541) -format contiki,00 d71 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c128   $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128           ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser-80col/webbrowser.c128 webbrowser,p   >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/wget/wget.c128                   wget,p         >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc-80col/irc-client.c128        irc,p          >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c128 webserver,p    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c128 telnetd,p      >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c128/default.cfg                             contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,s    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,s >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,s >$(NULLDEV)

contiki-c128.d81: c128-makes
	$(C1541) -format contiki,00 d81 $@
	$(C1541) -attach $@ -write ../../cpu/6502/$(DEV)config/$(DEV)config.c128   $(DEV)config,p >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ipconfig/ipconfig.c128           ipconfig,p     >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webbrowser-80col/webbrowser.c128 webbrowser,p   >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/wget/wget.c128                   wget,p         >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/irc-80col/irc-client.c128        irc,p          >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/webserver-example.c128 webserver,p    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/telnet-server/telnet-server.c128 telnetd,p      >$(NULLDEV)
ifdef SLIP
	$(C1541) -attach $@ -write default.cfg                                     contiki.cfg,s  >$(NULLDEV)
else
	$(C1541) -attach $@ -write ../c128/default.cfg                             contiki.cfg,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/cs8900a.eth            cs8900a.eth,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../cpu/6502/ethconfig/lan91c96.eth           lan91c96.eth,s >$(NULLDEV)
endif
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/index.htm    index.htm,s    >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/backgrnd.gif backgrnd.gif,s >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/contiki.gif  contiki.gif,s  >$(NULLDEV)
	$(C1541) -attach $@ -write ../../examples/webserver/httpd-cfs/notfound.htm notfound.htm,s >$(NULLDEV)
