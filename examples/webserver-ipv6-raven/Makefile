ifndef WEBDIR
 OUTFILE=webserver6-$(TARGET)
else
 OUTFILE=webserver6-$(WEBDIR)-$(TARGET)
endif

all:
	${MAKE} -f Makefile.webserver NOAVRSIZE=1 webserver6.elf $@
#can't just rename, webserver6 may be open in debugger
	@if [ $(OUTFILE) != "webserver6" ]; then cp webserver6.elf $(OUTFILE).elf;fi
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE).elf $(OUTFILE).hex


clean:
	${MAKE} -f Makefile.webserver clean $@
# Delete httpd-fsdata.c only if your toolchain includes perl
#	rm ../../platform/avr-raven/apps/raven-webserver/httpd-fsdata.c
	rm symbols.c symbols.h httpd-fsdata.c webserver6.elf $(OUTFILE).elf  $(OUTFILE).hex

connect:
#Connect to serial debug port for timestamp log (requires PERL).
#Change the baud rate and device to agree with your serial connection.
	../../tools/serial-log.pl -b 57600 -t /dev/com1 -l
	
%:
	$(MAKE) -f Makefile.webserver $@