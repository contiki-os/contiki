ifndef TARGET
  TARGET=avr-raven
  MCU=atmega1284p
endif
ifndef WEBDIR
 OUTFILE=webserver6-$(TARGET)
else
 OUTFILE=webserver6-$(WEBDIR)-$(TARGET)
endif

ifndef MCU
#TODO:extract MCU from platform
  MCU=atmega128rfa1
endif

all:
	${MAKE} -f Makefile.webserver TARGET=$(TARGET) NOAVRSIZE=1 webserver6.elf   
#can't just rename, webserver6 may be open in debugger
	@if [ $(OUTFILE) != "webserver6" ]; then cp webserver6.elf $(OUTFILE).elf;fi
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE).elf $(OUTFILE).hex
#	avr-size -C --mcu=atmega1284p $(OUTFILE).elf
	avr-size -C --mcu=$(MCU) $(OUTFILE).elf

clean:
	${MAKE} -f Makefile.webserver TARGET=$(TARGET) clean
# Delete httpd-fsdata.c only if your toolchain includes perl
#	rm ../../platform/avr-raven/apps/raven-webserver/httpd-fsdata.c
	rm symbols.c symbols.h webserver6.elf $(OUTFILE).elf  $(OUTFILE).hex
	rm -rf obj_avr-raven
