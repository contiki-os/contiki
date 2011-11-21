all:
	@rm -f *.output
	@echo Compiling on:
	@uname -a
	@$(MAKE) -s -k compile || cat *.output

compile: rime-examples shell hello-world \
         ipso-ipv6-raven jcreate multi-threading sky sky-ip sky-shell \
         webserver-ipv6 webserver-ipv6-raven tools

tools:
	@(cd ../../../tools ; $(MAKE))
	@(cd ../../../tools/sky/uip6-bridge ; $(MAKE))

shell: example-example-shell.native
hello-world: example-hello-world.sky example-hello-world.native \
             example-hello-world.minimal-net example-hello-world.msb430 \
             example-hello-world.micaz example-hello-world.avr-raven \
             example-hello-world.redbee-econotag example-hello-world.z1
rime-examples: example-rime.sky example-rime.z1 example-rime.esb example-rime.redbee-econotag
ipso-ipv6-raven: example-ipso-ipv6-raven.avr-raven
jcreate: example-jcreate.jcreate
multi-threading: example-multi-threading.sky \
                 example-multi-threading.native example-multi-threading.msb430 \
		 example-multi-threading.z1
sky: example-sky.sky
sky-ip: example-sky-ip.sky
sky-shell: example-sky-shell.sky
webserver-ipv6: example-webserver-ipv6.sky
webserver-ipv6-raven: example-webserver-ipv6-raven.avr-raven

example-%:
	@((echo; echo ------------------- $* -------------------; \
           cd  ../../../examples/${basename $*} ; $(MAKE) TARGET=${subst .,,${suffix $*}} clean;\
                  ($(MAKE) TARGET=${subst .,,${suffix $*}} > /dev/null 2>&1 ) ; \
                   $(MAKE) TARGET=${subst .,,${suffix $*}}) > $*.output 2>&1 && echo "$* succeeded" && rm $*.output) || \
           (echo; echo "$* failed"; exit 1)
#	(cd ../../../examples/${basename $*}; make TARGET=${subst .,,${suffix $*}})
