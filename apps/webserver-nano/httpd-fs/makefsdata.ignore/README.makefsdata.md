Move the desired pages into httpd-fs and regenerate httpd-fsdata.c using the
PERL script makefsdata.  Stage unused pages in this directory; anything here
will be ignored by makefsdata.  When using non-ram storage it must be invoked
with the HTTPD_STRING_ATTR!

    cd ~/contiki/apps/webserver (or webserver-nano, -micro, -mini, ...)
    ../../tools/makefsdata -A HTTPD_STRING_ATTR
