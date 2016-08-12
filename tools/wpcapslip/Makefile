CONTIKI=../..

CC=gcc
CFLAGS=-Wall -I$(CONTIKI)/core -I.

all: wpcapslip wpcapstdio Connect.class ConnectSocket.class

%.class: %.java
	javac $*.java
	
vpath %.c $(CONTIKI)/core/net

wpcapslip: wpcapslip.o wpcap.o tcpdump.o

wpcapstdio: wpcapstdio.o wpcap.o tcpdump.o

%: %.o
	$(CC) $(LDFLAGS) $^ /lib/w32api/libws2_32.a /lib/w32api/libiphlpapi.a -o $@

clean:
	rm -f *.class *.exe *.o