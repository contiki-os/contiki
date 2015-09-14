CC = gcc
CFLAGS = -Wall -g
HEADERS = 

all: blaster

depend:
	sudo apt-get install libconfig8-dev uuid-dev libqrencode-dev

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

blaster: blaster.o
	$(CC) $(CFLAGS) -o $@ $^ -lconfig -luuid -lqrencode

clean:
	rm *.o blaster

.PHONY: all depend clean
