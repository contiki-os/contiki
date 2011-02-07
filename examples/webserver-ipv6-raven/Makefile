#Commit with LF endings. Don't want to push this change, it's a test
ifndef WEBDIR
 OUTFILE=webserver6
else
 OUTFILE=webserver6-$(WEBDIR)
endif

all:
	${MAKE} -f Makefile.webserver TARGET=avr-raven NOAVRSIZE=1 webserver6.elf
#can't just rename, webserver6 may be open in debugger
	@if [ $(OUTFILE) != "webserver6" ]; then cp webserver6.elf $(OUTFILE).elf;fi
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE).elf $(OUTFILE).hex
	avr-size -C --mcu=atmega1284p $(OUTFILE).elf

clean:
	${MAKE} -f Makefile.webserver TARGET=avr-raven clean
# Delete httpd-fsdata.c only if your toolchain includes perl
#	rm ../../platform/avr-raven/apps/raven-webserver/httpd-fsdata.c
	rm symbols.c symbols.h webserver6.elf webserver6.hex
	rm -rf obj_avr-raven
