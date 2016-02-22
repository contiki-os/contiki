
CC = gcc
CFLAGS = -c
SOURCES = wsbridge.c
OBJECTS = $(SOURCES:.c=.o)
EXE = wsbridge

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
