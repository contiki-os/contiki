#Note to AVR Raven users: Output goes to UART1, not the LCD. To see it,
#make TARGET=avr-raven hello-world.elf
#Load the .elf in AVR Studio and connect a hapsim terminal to the 1284p simulation.

CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)

#UIP_CONF_IPV6=1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
