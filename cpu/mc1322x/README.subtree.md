MC1322x subtree
===============

The Contiki MC1322x port includes libmc1322x as a subtree. This makes pulling
updates to libmc1322x easy, but pushing changes from contiki to libmc1322x is
not so easy. However, this should not stop you from implementing core features
in contiki first, (especially if you are in a bind). The way to do this is to
make files prefixed with contiki- in cpu/mc1322x and add them to
Makefile.mc1322x.

For instance, if you need a routine called sleep, but libmc1322x doesn't have
that yet, you could implement sleep in contiki-crm.c. Feel free to use as many
contiki specific things in here as you want. We can pull these changes directly
into the contiki tree. There is nothing stopping you from making changes to the
lib files --- in fact you should if that is the right thing to do (and then
push your changes upstream). The subtree merge should make it easy to still
pull updates.

You'll also notice that the libmc1322x build system is still present. This
allows you to cd ./tests and make all of the libmc1322x unit tests as normal.
This is a handy way to perform a sainity check on all of the mc1322x specific
code.

The subtree was set up as follows:

From:
[http://www.kernel.org/pub/software/scm/git/docs/howto/using-merge-subtree.html](http://www.kernel.org/pub/software/scm/git/docs/howto/using-merge-subtree.html)

Moved mc1322x.lds to mc1322x.lds.contiki

Setup with:

    git remote add -f libmc1322x git://git.devl.org/git/malvira/libmc1322x.git
    git read-tree --prefix=cpu/mc1322x -u libmc1322x/master
    git commit -m "Merge libmc1322x as a subdirectory"

And to do subsequent merges from libmc1322x do:

    git pull -s subtree libmc1322x master
