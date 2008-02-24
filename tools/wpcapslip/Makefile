CONTIKI=../..

CFLAGS=-Wall -Werror -I$(CONTIKI)/core -I.

all: wpcapslip

vpath %.c $(CONTIKI)/core/net

wpcapslip: wpcapslip.o wpcap.o tcpdump.o

%: %.o
	$(CC) $(LDFLAGS) $^ /lib/w32api/libws2_32.a /lib/w32api/libiphlpapi.a -o $@
