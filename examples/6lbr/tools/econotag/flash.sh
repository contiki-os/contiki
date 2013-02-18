#!/bin/sh

MC1322X_LOAD=../../../../cpu/mc1322x/tools/mc1322x-load.pl

NVM_FILE=$1
shift
DEV=$1
shift

if [ ! -e "$NVM_FILE" ]; then
	echo "NVM file $NVM_FILE not found"
	exit 1
fi

if [ ! -e "$DEV" ]; then
	echo "Device $DEV not found"
	exit 1
fi

if [ ! -x $MC1322X_LOAD ]; then
	echo "$MC1322X_LOAD not found"
	exit 1
fi

$MC1322X_LOAD -t $DEV -f 6lbr_nvm_flasher_redbee-econotag.bin -z $* 0x100,`hexdump -v -e '"0x" 1/4 "%08x" ","' $NVM_FILE`