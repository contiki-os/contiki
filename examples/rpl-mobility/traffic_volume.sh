echo Control `tshark -r mob.pcap -2 -R icmpv6 -q -z io,stat,120000,BYTES -q  |grep '<>' | awk '{print $6}'` >> log_final.log
echo Data `tshark -r mob.pcap -2 -R udp -q -z io,stat,120000,BYTES -q  |grep '<>' | awk '{print $6}'` >> log_final.log
