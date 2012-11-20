## Makefile.CI
#
# This is for use on [Travis CI](http://travis-ci.org), which is
# a free distributed continuous integration service with unique
# set of features:
# * uses clean ephemeral virtual machines for every build
# * integrates with GitHub - triggers builds on push and pull-request
# * it is open source and free for public repositories
# * features great web UI and bunch of other stuff

JOBS ?= 2
TAIL ?= tail -n 15


EXAMPLES_ALL = hello-world

EXAMPLES_native = ipv6/native-border-router

EXAMPLES_most_non_native = \
	er-rest-example \
	ipv6/rpl-border-router \
	ipv6/rpl-collect \
	ipv6/rpl-udp \
	ipv6/simple-udp-rpl \
	ipv6/slip-radio \
	webserver-ipv6

EXAMPLES_econotag = $(EXAMPLES_most_non_native)

EXAMPLES_most_avr  = $(EXAMPLES_most_non_native)

EXAMPLES_micaz     = $(EXAMPLES_most_avr)
EXAMPLES_avr_raven = $(EXAMPLES_most_avr)

## MARK_EXAMPLES_$(TARGET) provides the mechanism to
## build the examples which are known to fail but
## shouldn't be counted as failing for some reason
MARK_EXAMPLES_micaz = \
	er-rest-example \
	ipv6/rpl-border-router \
	ipv6/rpl-collect \
	ipv6/rpl-udp \
	ipv6/simple-udp-rpl \
	ipv6/slip-radio \
	webserver-ipv6

## EXCLUDE_$(TARGETS) provides the mechanism to
## not build the examples which are known to fail
EXCLUDE_avr_raven = ipv6/rpl-collect

EXAMPLES_sky = $(EXAMPLES_most_non_native) sky-shell

COOJA_TESTS_DIR     = tools/cooja/contiki_tests
COOJA_TESTS_ALL     = $(COOJA_TESTS_DIR)/*_helloworld.csc
## Some Cooja tests can also be excluded:
#EXCLUDE_COOJA_TESTS = servreg-hack.csc sky_coffee.csc rime_collect.csc rime_trickle.csc 
## And some can also be marked:
#MARK_COOJA_TESTS    = crosslevel.csc ip_sky_telnet_ping.csc

CT := \033[0;0m

ifdef BOXED_SIGNS
  SKIP_SIGN := ☐
  FAIL_SIGN := ☒
  PASS_SIGN := ☑
else
  SKIP_SIGN := ✈
  FAIL_SIGN := ✘
  PASS_SIGN := ✔
endif


## FIXME: this will misbehave when we intend to skip `examples1` but there is also `examples123` that fails also
SKIP  = (echo $(SKIP_LIST) | grep -q $$e && (echo "\033[1;36m  $(SKIP_SIGN) ➝ ❨ $$e ⊈ $@ ❩$(CT)"; $(TAIL) $(LOG); echo skip >> results) || false)
FAIL  = (echo "\033[1;35m  $(FAIL_SIGN) ➝ ❨ $$e ∉ $@ ❩$(CT)"; $(TAIL) $(LOG); echo fail >> results)
PASS  = (echo "\033[1;32m  $(PASS_SIGN) ➝ ❨ $$e ∈ $@ ❩$(CT)"; echo pass >> results)

ifeq ($(BUILD_TYPE),multi)
THIS = $(MAKE) -C examples/$$e TARGET=$@ > $(LOG) 2>&1
MINE = $(filter-out $(EXCLUDE_$(subst -,_,$@)), $(EXAMPLES_ALL) $(EXAMPLES_$(subst -,_,$@)))
SKIP_LIST = $(MARK_EXAMPLES_$(subst -,_,$@))
endif

ifeq ($(BUILD_TYPE),cooja)
JAVA = java -mx512m
THIS = $(SHELL) -x -c "cd `dirname $$e` && $(JAVA) -jar ../dist/cooja.jar -nogui=`basename $$e`" > $(LOG) #2>&1
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), $(wildcard $(COOJA_TESTS_ALL)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

LOG  = /tmp/$@_`echo $$e | sed 's:/:_:g'`.log

%:
	@echo "\033[1;37m ➠ ❨$@❩ $(CT)"
	@for e in $(MINE); do $(THIS) && $(PASS) || $(SKIP) || $(FAIL); done;
