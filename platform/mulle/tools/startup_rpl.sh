#!/bin/bash
if [ $# -ne 1 ]
then
  echo "Usage: sudo `basename $0` <serial interface>"
  exit 65
fi

sysctl -w net.ipv6.conf.all.forwarding=1
./../../../tools/tunslip6 -t tun1 -B 115200 -s $1 fdfd::ff/64
