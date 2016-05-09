#!/bin/bash

make TARGET=avr-rss2

DEV=/dev/ttyUSB0
[ "$1" ] && DEV=$1

FIRMWARE=pdr.avr-rss2

#S_PROG=avr109
S_PROG=stk500v2

# Firmware upgrade of the avr-rss2 boards. 2016-05-07 -ro
# 
# The procedure is simple:
#   1. tty_talk passes the upgr command which kicks the bootloader
#   2  we run avrdude programmming

# tty_talk is availble via:
#  https://github.com/herjulf/tty_talk

echo Using device=$DEV baud=$BAUD firmware=$FIRMWARE

stty sane
tty_talk -38400 $DEV "upgr
"
#avrdude -p m128rfa1 -c $S_PROG -P $DEV -b 38400 -e -U flash:w:$FIRMWARE
avrdude -p m256rfr2 -c $S_PROG -P $DEV -b 38400 -e -U flash:w:$FIRMWARE
