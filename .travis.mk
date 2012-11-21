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

COOJA_TESTS_DIR             = tools/cooja/contiki_tests
COOJA_TESTS_HELLO_WORLD     = $(COOJA_TESTS_DIR)/*_helloworld.csc
COOJA_TESTS_BASE            = $(COOJA_TESTS_DIR)/multithreading.csc \
                              $(COOJA_TESTS_DIR)/sky_coffee.csc
#                              $(COOJA_TESTS_DIR)/crosslevel.csc
# The crosslevel test is unstable and therefore not included yet
COOJA_TESTS_RIME            = $(COOJA_TESTS_DIR)/rime_abc.csc \
                              $(COOJA_TESTS_DIR)/rime_rucb.csc \
                              $(COOJA_TESTS_DIR)/rime_runicast.csc \
                              $(COOJA_TESTS_DIR)/sky_deluge.csc \
                              $(COOJA_TESTS_DIR)/sky_collect.csc
#                              $(COOJA_TESTS_DIR)/rime_trickle.csc
# The rime_trickle test sometimes fails so it isn't included here
COOJA_TESTS_NETPERF         = $(COOJA_TESTS_DIR)/netperf-*.csc
COOJA_TESTS_SKY_SHELL       = $(COOJA_TESTS_DIR)/sky_shell_basic_commands.csc \
                              $(COOJA_TESTS_DIR)/sky_shell_compliation_test.csc \
                              $(COOJA_TESTS_DIR)/sky_shell_download.csc \
                              $(COOJA_TESTS_DIR)/sky_checkpointing.csc \
                              $(COOJA_TESTS_DIR)/sky_shell_sendcmd.csc
COOJA_TESTS_ELFLOADER       = $(COOJA_TESTS_DIR)/sky_shell_exec_serial.csc
COOJA_TESTS_COLLECT         = $(COOJA_TESTS_DIR)/rime_collect.csc \
                              $(COOJA_TESTS_DIR)/rime_collect_sky.csc
COOJA_TESTS_COLLECT_SKY     = $(COOJA_TESTS_DIR)/sky_shell_collect.csc
COOJA_TESTS_COLLECT_LOSSY   = $(COOJA_TESTS_DIR)/sky_shell_collect_lossy.csc
COOJA_TESTS_IPV6            = $(COOJA_TESTS_DIR)/sky-ipv6-rpl-collect.csc \
                              $(COOJA_TESTS_DIR)/sky_ipv6_udp.csc \
                              $(COOJA_TESTS_DIR)/sky_ipv6_udp_fragmentation.csc \
                              $(COOJA_TESTS_DIR)/cooja_ipv6_udp.csc
COOJA_TESTS_IPV6_APPS       = $(COOJA_TESTS_DIR)/servreg-hack.csc \
                              $(COOJA_TESTS_DIR)/rest_rpl_coap.csc

# The IPv4 test are not stable yet:
#COOJA_TESTS_IPV4            = $(COOJA_TESTS_DIR)/ip_cooja_telnet_ping.csc \
#                              $(COOJA_TESTS_DIR)/ip_sky_telnet_ping.csc \
#                              $(COOJA_TESTS_DIR)/ip_sky_webserver_wget.csc

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


# This is the definition of the basic compilation test
ifeq ($(BUILD_TYPE),compile)
THIS = $(MAKE) -C examples/$$e TARGET=$@ > $(LOG) 2>&1
MINE = $(filter-out $(EXCLUDE_$(subst -,_,$@)), $(EXAMPLES_ALL) $(EXAMPLES_$(subst -,_,$@)))
SKIP_LIST = $(MARK_EXAMPLES_$(subst -,_,$@))
endif

# Below are the Contiki functional tests, which all use Cooja so we
# need to define JAVA and the COOJA_COMMAND
JAVA = java -mx512m
COOJA_COMMAND = $(SHELL) -x -c "cd `dirname $$e` && $(JAVA) -jar ../dist/cooja.jar -nogui=`basename $$e`" > $(LOG) #2>&1

ifeq ($(BUILD_TYPE),helloworld)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_HELLO_WORLD)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),base)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_BASE)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),rime)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_RIME)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),netperf)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_NETPERF)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),sky-shell)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_SKY_SHELL)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),elfloader)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_ELFLOADER)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),collect)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_COLLECT)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),collect-sky)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_COLLECT_SKY)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),collect-lossy)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_COLLECT_LOSSY)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),ipv6)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_IPV6)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

ifeq ($(BUILD_TYPE),ipv6-apps)
THIS = $(COOJA_COMMAND)
MINE = $(filter-out $(addprefix $(COOJA_TESTS_DIR)/, $(EXCLUDE_COOJA_TESTS)), \
       $(wildcard $(COOJA_TESTS_IPV6_APPS)))
SKIP_LIST = $(addprefix $(COOJA_TESTS_DIR)/, $(MARK_COOJA_TESTS))
endif

LOG  = /tmp/$@_`echo $$e | sed 's:/:_:g'`.log

%:
	@echo "\033[1;37m ➠ ❨$@❩ $(CT)"
	@for e in $(MINE); do $(THIS) && $(PASS) || $(SKIP) || $(FAIL); done;
