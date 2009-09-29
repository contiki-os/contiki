CONTIKI_PROJECT = sky-shell
all: $(CONTIKI_PROJECT)

dumptest:
	(echo "~K"; \
	 echo "killall "; \
	 echo "time `date +%s` | null"; \
         echo 'netcmd { repeat 0 10 { randwait 10 echo 0123456789abcdef | blink | send } }'; \
	 sleep 2; \
	 echo "killall "; \
	 sleep 2; \
         echo 'collect | binprint &') | make login		

APPS = serial-shell
CONTIKI = ../..

%.shell-upload: %.ce
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "dec64 | write $*.ce | null"; sleep 4; \
	 ../../tools/base64-encode < $<; sleep 4; \
         echo "read $*.ce | size"; sleep 4) | make login

%.file-upload: %
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "dec64 | write $* | null"; sleep 4; \
	 ../../tools/base64-encode < $<; sleep 4; \
         echo "read $* | size"; sleep 4) | make login

%.netsend: %.shell-upload
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "netcmd { recvnetfile | write $*.ce | size | send } "; sleep 4; \
         echo "netfile $*.ce"; sleep 4;\
         echo "collect | binprint"; sleep 40) | make login

%.tgz: %.ihex
	mkdir $(basename $<) ; \
	mv $< $(basename $<) ; \
	echo $(basename $<)/$(basename $<).ihex 600 > $(basename $<)/runfile ; \
	tar czf $@ $(basename $<)
broadcast:
	echo "echo haha | broadcast" | make login

xprofile:
	(sleep 4; echo "sendtest 56.0 100 20"; sleep 4; echo "xmacprofile"; sleep 10) | make login

xprofile2:
	(sleep 4; sleep 4; echo "xmacprofile"; sleep 10) | make login MOTE=2

ping:
	echo ping 55.0 | make login

serrors:
	(echo "~K"; echo "var serial_byte_dropped") | make login

reboot:
	(echo "~K"; echo "reboot"; sleep 6) | make login

test64:
	(echo "~K"; echo "dec64 | write file"; sleep 2; \
	../../tools/base64-encode < symbols.c) | make login

distfile :
	(echo "~K"; echo "netcmd { recvnetfile | write file | size | blink }"; \
         sleep 1; echo "netfile file") | make login

test642:
	(echo "~K"; echo "dec64 | write file | null"; sleep 2; \
	../../tools/base64-encode < hello-world.c; echo ; echo "read file | size" ) | make login

test2:
	(echo "~K"; echo "write file"; sleep 2; \
	cat hello-world.c) | make login

upload64:
	(echo "~K"; echo "dec64 | write file"; sleep 2; \
	../../tools/base64-encode < symbols.c) | make login

netfile:
	(echo "~K"; echo "netfile file") | make login

sendtest:
	(echo "~K"; \
	 echo "killall "; \
	echo "repeat 10 2 sendtest $(NODE) 2000 64"; \
         sleep 24; \
	echo "repeat 10 2 sendtest $(NODE) 2000 32"; \
         sleep 24; \
	echo "repeat 10 2 sendtest $(NODE) 2000 16"; \
         sleep 24; \
         echo 'energy | powerconv' ) | make login

setup-sensor:
	(echo "~K"; \
	 echo "killall "; \
	 echo "time `date +%s` | null"; \
         echo 'netcmd { repeat 0 20 { randwait 20 sense | blink | send } }'; \
	 sleep 2; \
         echo 'collect | timestamp | blink | write file | binprint &') | make login

setup-sensor-alldata:
	(echo "~K"; \
	 echo "killall "; \
	 echo "time `date +%s` | null"; \
         echo 'netcmd { repeat 0 20 { randwait 20 sky-alldata | blink | send } }'; \
	 sleep 2; \
         echo 'collect | timestamp | blink | write file | binprint &') | make login

collect-sensor-data:
	make login | tee collect-sensor-data

collect-power:
	(echo ; \
         echo 'netcmd { repeat 1000 20 { randwait 20 power | send } }'; \
	 sleep 2; \
         echo 'collect | timestamp | hd &') | make login

collect-energy:
	(sleep 4; echo "~K"; \
	 echo "killall "; \
	 echo "time `date +%s` | null"; \
         echo 'netcmd { repeat 1000 40 { randwait 40 energy | send } }'; \
	 sleep 4; \
         echo 'collect | timestamp | write file | hd &') | make login

include $(CONTIKI)/Makefile.include
