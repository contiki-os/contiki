
# Zolertia Z1 MODULE and NXP MFRC522 Contactless RFID Reader

## RFID Tag UID Reader implemented on Z1 platform

This is a implementation on Z1 platform to detect the RFID tag nearby and printout the UID of the tag. It is a much refined version, compared to the full RFID library on Arduino and more similar to the one on Actel. Thus, it is adapted from:


- [ Actel boards implementation] (github.com/knksmith57/eecs373-actel-mfrc522-spi)  


- [ Arduino boards implementation ] (github.com/miguelbalboa/rfid)  


This implementation is only tested for MIFARE 1KB tags so far. Other tag types may need modifications in the code.


The mfrc522.c driver file shall be under contiki/examples/platform/z1/dev directory
The main and make files are located under contiki/platform/z1/apps/RFID 


The JP1B East port of Z1 is used to connect to the RFID module with jumper wires. No additional circuitry is required. It is highly recommended to solder a female header connector onto JP1B port of Z1 module and a male pin header connector onto RFID module to avoid any conduction related problems.


In contrast to Arduino and Actel implementations, RESET pin of the RFID reader is not set into different modes in the code. RESET Low is active for the RFID module and in order to avoid this hard reset on device, RESET pin of the module should be connected to a 3V (High) pin or such. IRQ pin of the RFID module is not used and not connected to Z1 module at all.


The pin layout of the Z1 module and RFID reader is given below:

| Signal              	| MFRC522 Pin 	| Zolertia Z1 pin  	|
|---------------------	|-------------	|------------------	|
| RESET               	| RST         	| Pin22 (D+3V)     	|
| CHIP/SLAVE SELECT   	| SDA         	| Pin28 (I2C.SDA)  	|
| MASTER OUT SLAVE IN 	| MOSI        	| Pin36 (SPI.SIMO) 	|
| MASTER IN SLAVE OUT 	| MISO        	| Pin38 (SPI.SOMI) 	|
| CLOCK               	| SCK         	| Pin34 (SPI.CLK)  	|
| GROUND              	| GND         	| Pin18 (CPDGND)   	|
| 3.3V                	| 3.3V        	| Pin20 (CPD+3.3V) 	|