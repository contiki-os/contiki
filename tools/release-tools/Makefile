ifndef TAG
  ifndef RELEASE
    ${error You must specify the RELEASE variable (e.g. make RELEASE=2.5)}
  endif
  RELEASETAG=$(subst .,-,$(RELEASE))
  TAG=contiki-$(RELEASETAG)
else
  RELEASE=$(TAG)
endif

GITROOT=git://contiki.git.sourceforge.net/gitroot/contiki/contiki

package: all

test: export-package compile almost-clean

export-package: clean checkout copy tgz zip

almost-clean:
	rm -rf contiki contiki-$(RELEASE) contiki-collect-$(RELEASE)

clean:
	rm -rf contiki contiki-$(RELEASE) contiki-collect-$(RELEASE) *.zip *.tar.gz

copy:
	cp -r contiki contiki-$(RELEASE)

doc:
	(cd contiki/doc; make dox pdf)

copydoc:
	cp contiki/doc/latex/refman.pdf contiki-$(RELEASE)/doc/
	cp -r contiki/doc/html contiki-$(RELEASE)/doc/

tgz:
	tar czf contiki-$(RELEASE).tar.gz contiki-$(RELEASE)
	chmod 644 contiki-$(RELEASE).tar.gz

zip:
	zip -r contiki-$(RELEASE).zip contiki-$(RELEASE)
	chmod 644 contiki-$(RELEASE).zip

compile:
	(cd contiki/examples/compile-platforms; make)

checkout:
	git clone $(GITROOT)
	git checkout $(TAG)
	rm -rf contiki/.git

-include Makefile.sky
-include Makefile.raven

all: export-package $(BINARIES) almost-clean
