CONTIKI=../..

TARGET=stepper-robot

CONTIKI_TARGET_DIRS=stepper

KERNELS= sys-tst.elf robot-main.elf

# Master clock frequency
MCK=23961600

ARCH=debug-uart.o clock.o sys-interrupt.o interrupt-utils.o newlib-syscalls.o \
leds-arch.o sam7s-spi.o

SYSTEM=process.o procinit.o service.o clock.o etimer.o timer.o leds.o uip-log.o

UIP=uip.o uiplib.o tcpip.o uip-fw.o uip-fw-service.o uipbuf.o \
    tcpdump.o psock.o uaodv.o uaodv-rt.o uip-udp-packet.o

UIPDRIVERS= cc2420.o cc2420_send_ip.o cc2420_send_uaodv.o cc2420-interrupt.o \
cc2420-spi.o

SYSLIB=memb.o list.o

STEPPER=stepper-interrupt.o stepper-move.o
CFLAGS+= -I stepper

all: $(KERNELS)

sys-tst.elf: sys-tst.o $(ARCH) $(SYSTEM)

robot-main.elf:  robot-main.o stepper-process.o $(ARCH) $(SYSTEM) $(SYSLIB) $(UIP) $(UIPDRIVERS) $(STEPPER)

include $(CONTIKI)/cpu/at91sam7s/Makefile.at91sam7s
