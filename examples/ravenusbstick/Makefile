all:
	make TARGET=avr-ravenusb -f Makefile.ravenusbstick ravenusbstick.elf
	avr-objcopy -O ihex -R .eeprom ravenusbstick.elf ravenusbstick.hex
	avr-size -C ravenusbstick.elf

clean:
	make -f Makefile.ravenusbstick clean
	rm -rf obj_avr-ravenusb
	rm symbols.c symbols.h ravenusbstick.elf ravenusbstick.hex
