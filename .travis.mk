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

EXAMPLES_redbee_econotag = $(EXAMPLES_most_non_native)

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


SKIP  = (echo "\033[1;36m  $(SKIP_SIGN) ➝ ❨ $$e ⊈ $@ ❩$(CT)"; echo skip >> results)
FAIL  = (echo "\033[1;35m  $(FAIL_SIGN) ➝ ❨ $$e ∉ $@ ❩$(CT)"; $(TAIL) $(LOG); echo fail >> results)
PASS  = (echo "\033[1;32m  $(PASS_SIGN) ➝ ❨ $$e ∈ $@ ❩$(CT)"; echo pass >> results)

ifeq ($(BUILD_TYPE),multi)
THIS = $(MAKE) -C examples/$$e TARGET=$@ > $(LOG) 2>&1
MINE = $(EXAMPLES_ALL) $(EXAMPLES_$(subst -,_,$@))
endif

LOG  = /tmp/$@_`echo $$e | sed 's:/:_:g'`.log

%:
	@echo "\033[1;37m ➠ ❨$@❩ $(CT)"
	@for e in $(MINE); do $(THIS) && $(PASS) || $(FAIL); done;
