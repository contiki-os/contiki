EXE_MAKE=$(notdir $(shell which "make.exe" 2>/dev/null))
ifeq "$(EXE_MAKE)" "make.exe"
PLATFORM=windows
else
PLATFORM=linux
endif

OBJECTS = ihex.o converter.o
SUBDIRS = 

CFLAGS	= -Wall -D_REENTRANT -I.
LDFLAGS = -L. -D_REENTRANT -lpthread
ifeq "$(PLATFORM)" "linux"
SUFFIX=
else
SUFFIX=.exe
endif

TARGET  = converter$(SUFFIX)

all: binary

binary: $(TARGET)
	strip $(TARGET)

$(TARGET): $(OBJECTS)
	gcc -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	
.c.o:
	gcc -c -o $(<:.c=.o) -O2 -Wall $(CFLAGS) $<

platform-test:
	@echo $(PLATFORM)
	
old-strip:
	if [ -x $(TARGET).exe ]; then $(PLATFORM)strip $(TARGET).exe; else $(PLATFORM)strip $(TARGET); fi
	
clean:
	rm -f $(TARGET) $(OBJECTS)
