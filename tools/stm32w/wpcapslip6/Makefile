CONTIKI=../../..


CFLAGS=-Wall -Werror -I$(CONTIKI)/core -I.

ifdef WITH_STDIN
	CFLAGS += -DWITH_STDIN
endif

all: clean wpcapslip6
ifdef WITH_STDIN
	mv wpcapslip6.exe wpcapslip6-stdin.exe
endif 

clean:
	rm -f wpcapslip6.o wpcap6.o ip-process.o fakeuip.o


vpath %.c $(CONTIKI)/core/net

wpcapslip6: wpcapslip6.o wpcap6.o ip-process.o fakeuip.o

%: %.o
	$(CC) $(LDFLAGS) $^ /lib/w32api/libws2_32.a /lib/w32api/libiphlpapi.a -o $@
