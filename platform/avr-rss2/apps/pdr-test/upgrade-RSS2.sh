#!/bin/bash

# Flash Upgrade Script. Copyright, GPL by Robert Olsson <robert@herjulf.se>
# Script to upgrade mote firmware on unix 
# needs tty_talk avrdude, stty 
# 
# tty_talk source code:
# https://github.com/herjulf/tty_talk

DEV=/dev/ttyUSB0
BAUD=38400
#SPROTO=avr109
SPROTO=stk500v2
#MCU=atmega128rfa1
MCU=atmega256rfr2
BLDR=stk500boot_v2_mega256rfr2.hex
FIRMWARE=control.avr-rss2

[ "$1" ] && FIRMWARE=$1
FPATH=.

echo Using device=$DEV baud=$BAUD firmware=$FIRMWARE

function usb_flash
{
        echo usb_flash 1
   avrdude -F -p $MCU -c $SPROTO -P $DEV -b $BAUD -e -U flash:w:$FPATH/$FIRMWARE 
}

function usb_flash_init
{
    echo usb_flash_init 1
    RES=$( { tty_talk -$BAUD $DEV upgr; } 2>&1 )
    OK=$(echo $RES | cut -d\  -f2)
    [ "$OK" = OK ]
    echo usb_flash_init 2
}

function upgrade_flash
{
    for (( c=0; c<=10; c++ ))
    do
	if usb_flash_init; then
	    usb_flash
	    break
	fi
    done
}

stty -F  $DEV sane
upgrade_flash
