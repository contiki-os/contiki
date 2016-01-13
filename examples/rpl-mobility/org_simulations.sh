#!/bin/bash
#mrpl-v 0.7 20 1 0 1 1 0 40 130 40 60 0.5 10 0 123456789 3600000
#rpl-static 0.7 20 0 0 0 0 0 40 130 40 60 0.5 10 0 123456789 3600000
#mmrpl 0.7 20 0 0 0 0 1 40 130 40 60 0.5 10 0 123456789 3600000
#
i="0"
max=${18-20}
sh script.conf "$@"
cd $1

while [ $i -lt $max ]
do
cooja $1.csc
sh ../traffic_volume.sh
sh ../used_time.sh
eval `echo rename "'s/log_([A-Za-z0-9]*).log/log_"'$1'"_$i.log/'" *.log`
eval `echo rename "'s/([A-Za-z]*).pcap/"'$1'"_$i.pcap/'" *.pcap`
sed -e "s/<generation_seed>.*<\/generation_seed>/<generation_seed>$((${15}+10))<\/generation_seed>/g" -i ${1}.csc
i=$((i+1))
done
awk -f ../final.awk final.log > ../results/$1.log
