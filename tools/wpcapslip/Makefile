CFLAGS=-Wall -Werror -I../../core -I.

all: wpcapslip

wpcapslip: wpcapslip.o wpcap.o

%: %.o
	$(CC) $(LDFLAGS) $^ /lib/w32api/libws2_32.a /lib/w32api/libiphlpapi.a -o $@
