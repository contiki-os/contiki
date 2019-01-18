CONTIKI = ../..
ifndef TARGET
TARGET=sky
endif

#APPS=rudolph

all: blink sky-collect #rt-leds test-button test-cfs tcprudolph0

serialdump:
	$(SERIALDUMP) -b115200 /dev/com$(firstword $(CMOTES)) | $(CONTIKI)/tools/timestamp | tee serialdump-`date +%Y%m%d-%H%M`

include $(CONTIKI)/Makefile.include
