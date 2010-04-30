all: udp-client udp-server

CONTIKI=../../../../contiki-2.x

TARGET=sky

WITH_UIP6=1
UIP_CONF_IPV6=1

ifdef WITH_TCP
include test-tcp/Makefile.test-tcp
endif

include $(CONTIKI)/Makefile.include

upload-testbed:
	$(MAKE) testbed-log-stop
	$(MAKE) udp-client.testbed
	$(MAKE) udp-server.testbed NODE=1
	$(MAKE) testbed-reset testbed-log

download-logs: testbed-log-download

parse-mote-output:
	cat mote-output-* | grep -a "P "| ./scripts/parse-power-data > power-data
	cat mote-output-* | grep -a "P "| ./scripts/parse-node-power > node-power-data
	cat mote-output-* | ./scripts/parse-prr-data

parse-logs:
	cat testbed-log-dir/*.log | grep -a "P " | ./scripts/parse-power-data > power-data
	cat testbed-log-dir/*.log | grep -a "P " | ./scripts/parse-node-power | sort -nr > node-power-data
	cat testbed-log-dir/*.log | ./scripts/parse-prr-data

parse-some-logs:
	cat $(foreach L, 7 14 3 0, testbed-log-dir/$(L).log) | grep -a "P " | ./scripts/parse-power-data > power-data
	cat $(foreach L, 7 14 3 0, testbed-log-dir/$(L).log) | grep -a "P " | ./scripts/parse-node-power > node-power-data
	cat $(foreach L, 7 14 3 0, testbed-log-dir/$(L).log) | ./scripts/parse-prr-data

parse-simulation:
	cat roll-rpl-power-experiment-large.log | grep "P " | ./scripts/parse-power-data > power-data
	cat roll-rpl-power-experiment-large.log | grep "P " | ./scripts/parse-node-power | sort -nr > node-power-data
	cat roll-rpl-power-experiment-large.log | cut -f 3- -d \  | ./scripts/parse-prr-data

parse-ongoing-simulation:
	cat COOJA.testlog | grep "P " | ./scripts/parse-power-data > power-data
	cat COOJA.testlog | grep "P " | ./scripts/parse-node-power > node-power-data
	cat COOJA.testlog | cut -f 3- -d \  | ./scripts/parse-prr-data

plot:
	gnuplot scripts/plot-power

show:
	gv power.eps
	gv node-power.eps

graphs:
	make parse-logs plot
	cp power.eps experimental-power.eps
	cp node-power-narrow.eps experimental-node-power.eps
	make parse-simulation plot
	cp power.eps simulation-power.eps
	cp node-power.eps simulation-node-power.eps

copygraphs:
	cp experimental-*.eps simulation-*.eps ~/nes/papers/rpl-poster/figures/


download-show: download-logs parse-logs plot show


# ej 61, 106 17 67 39 90
# bara 36 20 8 59 13 62

# Good nodes: 3 7 11 14 0 9
# 7 (36), 11 (20), 14 (8), 3 (59), 0 (13), 9 (62)


LOGMOTES=1 2 3 4 5

logmote-%:
	make login MOTE=$* > mote-output-$*

logmotes:
	make -j 100 $(foreach M, $(LOGMOTES), logmote-$M)
