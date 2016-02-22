#!/bin/bash
#mrpl-v     0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 1 0 1 1 0 0 0 
#rpl-static 0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 0 0 0 0 0 0 0
#mmrpl      0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 0 0 0 0 1 0 0
#me-rpl     0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 0 0 0 0 0 1 1
i="0"
max=${20-20}
sh script.conf "$@"
cd $1
increment=${16}

echo "Partial:" > ../results/$1.log

while [ $i -lt $max ]
do
cooja $1.csc
sh ../traffic_volume.sh
sh ../used_time.sh
increment=$(($increment+10))
eval `echo rename "'s/log_([A-Za-z0-9]*).log/log_"'$1'"_$i.log/'" *.log`
eval `echo rename "'s/([A-Za-z]*).pcap/"'$1'"_$i.pcap/'" *.pcap`
sed -e "s/<generation_seed>.*<\/generation_seed>/<generation_seed>$increment<\/generation_seed>/g" -i ${1}.csc
i=$((i+1))
awk -f ../final.awk final.log >> ../results/$1.log
done
awk -f ../final.awk final.log > ../results/$1.log
