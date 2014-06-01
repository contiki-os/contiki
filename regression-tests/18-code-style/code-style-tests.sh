#!/bin/bash
set -evx

PULL_DATA=/tmp/pull.json
CONTIKI=$2
UNCRUSTIFY_DIR=$CONTIKI/tools/code-style

if [ "$1" = "false" ]; then
    echo "TEST OK"
    exit 0
else
    echo "checking pull $1"
fi

curl "https://api.github.com/repos/contiki-os/contiki/pulls/$1" > $PULL_DATA

FROM=`jq -r .base.sha $PULL_DATA`
TO=`jq -r .head.sha $PULL_DATA`

for file in `git diff --name-only $FROM..$TO | grep "\.[ch]$"`
do
    full_path=$CONTIKI/$file
    uncrustify -q -c $UNCRUSTIFY_DIR/uncrustify.cfg -f $full_path | diff -u $full_path --to-file=/dev/stdin
done

echo "TEST OK"
exit 0
