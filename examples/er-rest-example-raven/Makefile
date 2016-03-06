TARGET=avr-raven

APPS += raven-lcd-interface

export

CONTIKI=../..

ER_REST_EXAMPLE=$(CONTIKI)/examples/er-rest-example

all %:
	@(cd $(ER_REST_EXAMPLE) && $(MAKE) $@)
	@echo 
	@echo "*** Binaries can be found in $(ER_REST_EXAMPLE) ***"

