#!/bin/bash
if [ $# -ne 2 ]
then
  echo "Usage: sudo `basename $0` <serial interface> <ipv6 prefix>"
  echo "Example: sudo `basename $0` /dev/ttyUSB0 fdfd:1:2:3"
  exit 65
fi

sysctl -w net.ipv6.conf.all.forwarding=1
./../../../tools/tunslip6 -t tun1 -B 115200 -s $1 $2::ff/64
