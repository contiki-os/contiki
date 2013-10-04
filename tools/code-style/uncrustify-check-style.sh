#!/bin/sh
uncrustify -q -c `dirname $0`/uncrustify.cfg -f $1 | diff -u $1 --to-file=/dev/stdin
