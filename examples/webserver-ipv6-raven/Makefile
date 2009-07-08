all: 
	make -f Makefile.webserver TARGET=avr-raven webserver6.elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature webserver6.elf webserver6.hex
	avr-size -C --mcu=atmega1284p webserver6.elf

clean:
	make -f Makefile.webserver TARGET=avr-raven clean
	rm symbols.c symbols.h webserver6.elf webserver6.hex
	rm -rf obj_avr-raven
