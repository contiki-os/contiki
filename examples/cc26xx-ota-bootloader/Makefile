%.o: %.c
	gcc -std=c99 -c $< -o $@

generate-metadata: generate-metadata.o
	gcc -o $@ generate-metadata.o
	rm -f *.o

.PHONY: bootloader ota-image-example master-hex

bootloader:
	printf "Making OTA Bootloader\n";
	cd ./bootloader; 				\
		make clean;						\
		make bootloader.hex;	\

ota-image-example: generate-metadata
	rm -f firmware*.hex firmware*.bin;
	printf "\nMaking OTA Example Image\n"
	cd ./ota-image-example;		\
		make clean;								\
		make ota-image-example;		\
	printf "\nAdding metadata to OTA Example Image\n";
	./generate-metadata ota-image-example/ota-image-example.bin 0x1 0xabcd
	srec_cat firmware-metadata.bin -binary ota-image-example/ota-image-example.bin -binary -offset 0x100 -o firmware-with-metadata.bin -binary

master-hex: bootloader ota-image-example
	printf "\nMerging OTA Example Image with Bootloader\n"
	srec_cat bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 firmware-with-metadata.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o firmware.hex -intel
