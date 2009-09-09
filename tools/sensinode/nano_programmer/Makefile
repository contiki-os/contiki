EXE_MAKE=$(notdir $(shell which "make.exe" 2>/dev/null))
ifeq "$(EXE_MAKE)" "make.exe"
PLATFORM=windows
else
PLATFORM=linux
endif

OBJECTS = ihex.o $(PLATFORM)/port.o programmer.o cdi_program.o
SUBDIRS = 

ifeq "$(PLATFORM)" "linux"
CFLAGS	= -Wall -D_REENTRANT -I.
LDFLAGS = -L. -D_REENTRANT -lpthread
SUFFIX=
else
CFLAGS=-I. -I../nano_usb_programmer/ftdi_win32
CFLAGS+= -L. -L../nano_usb_programmer/ftdi_win32 -DPLATFORM_WINDOWS
CFLAGS+= -mwin32 
LDFLAGS=../nano_usb_programmer/ftdi_win32/ftd2xx.lib -lkernel32
SUFFIX=.exe
endif

TARGET  = nano_programmer$(SUFFIX)

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
