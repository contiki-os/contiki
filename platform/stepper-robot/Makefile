TARGET=stepper-robot

all: core

core: 
	$(MAKE) core.$(TARGET)

CONTIKI=../..

CORE=core-nosyms.$(TARGET)

CONTIKI_TARGET_MAIN=contiki-main.c

codeprop: $(CONTIKI)/tools/codeprop.c
	cc -g -Wall $< -o $@

clean: stepper_robot_clean

.PHONY: stepper_robot_clean

stepper_robot_clean:
	-rm codeprop

include $(CONTIKI)/Makefile.include

