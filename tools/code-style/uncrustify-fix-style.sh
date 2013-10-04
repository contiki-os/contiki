#!/bin/sh
uncrustify --no-backup --replace -c `dirname $0`/uncrustify.cfg $*
