CONTIKI_PROJECT = jcreate-shell
all: $(CONTIKI_PROJECT)

APPS = serial-shell
CONTIKI = ../..

jcreate-blink:
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "repeat 0 0 { acc | leds 1 } &"; sleep 4) | make login

%.shell-upload: %.ce
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "dec64 | write $*.ce | null"; sleep 4; \
	 ../../tools/base64-encode < $<; sleep 4; \
         echo "~K"; sleep 4; \
         echo "read $*.ce | size"; sleep 4) | make login

%.netsend: %.shell-upload
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "netcmd { recvnetfile | write $*.ce | size | send } "; sleep 4; \
         echo "netfile $*.ce"; sleep 4;\
         echo "collect | binprint"; sleep 40) | make login

include $(CONTIKI)/Makefile.include
