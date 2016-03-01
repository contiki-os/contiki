#!/bin/bash
#mrpl-v     0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 1 0 1 1 0 0 0 
#rpl-static 0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 0 0 0 0 0 0 0
#mmrpl      0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 0 0 0 0 1 0 0
#me-rpl     0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 0 0 0 0 0 1 1
#co-rpl     0.7 20 40 130 40 60 0.5 10 0 123456789 3600000 1 1 0 0 0 0 0
i="0"
max=${20-20}
sh script.conf "$@"
cd $1
increment=${11}

echo "Partial:" > ../results/$1.log

while [ $i -lt $max ]
do
sed -e s/SCRIPT_SEED/"$increment"/g < ${1}-preseed.csc > ${1}.csc
cooja $1.csc
sh ../traffic_volume.sh
sh ../used_time.sh
increment=$(($increment+10))
eval `echo rename "'s/log_([A-Za-z0-9]*).log/log_"'$1'"_$i.log/'" *.log`
eval `echo rename "'s/([A-Za-z]*).pcap/"'$1'"_$i.pcap/'" *.pcap`
i=$((i+1))
awk -f ../final.awk final.log >> ../results/$1.log
done
awk -f ../final.awk final.log > ../results/$1.log
