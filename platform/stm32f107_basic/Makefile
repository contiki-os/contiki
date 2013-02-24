TARGET=stm32f107_basic

all: stm32f107_basic


CONTIKI=../..

CONTIKI_TARGET_MAIN=contiki-main.c

PROJECT_SOURCEFILES = parity.c 


randgen: randgen.c
	gcc -DNDEBUG -I $(CONTIKI)/cpu/arm/stm32f107/ -I . -I $(CONTIKI)/core  randgen.c -o randgen

randcheck: randcheck.c
	gcc -DNDEBUG -I $(CONTIKI)/cpu/arm/stm32f107/ -I . -I $(CONTIKI)/core  randcheck.c -o randcheck

clean: stm32test_clean

.PHONY: stm32test_clean

stm32test_clean:

include $(CONTIKI)/Makefile.include

