EXE_MAKE=$(notdir $(shell which "make.exe"))
ifeq "$(EXE_MAKE)" "make.exe"
PLATFORM=windows
else
PLATFORM=linux
endif

include Rules.make

APP = nano_usb_programmer$(SUFFIX)
OBJS = $(SOURCES:.c=.o)

all: $(APP)

$(APP): $(OBJS) Makefile $(SOURCES)
	$(CC) -o $(APP) $(CFLAGS) $(OBJS) $(LDFLAGS)
	
platform-test:
	@echo $(PLATFORM)

clean:
	rm -f *.o ; rm -f $(APP)
	
.c.o:
	$(CC) -c -o $(<:.c=.o) $(CFLAGS) $<
	
ftd2xx.def:
	echo EXPORTS > ftd2xx.def
	nm ftd2xx.lib | grep ' T _' | sed 's/.* T _//' >> ftd2xx.def

ftd2xx.dll.a: ftd2xx.def
	dlltool --def ftd2xx.def --dllname ftd2xx.dll --output-lib ftd2xx.dll.a
