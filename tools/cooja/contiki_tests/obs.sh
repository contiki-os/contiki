n=0
lines=0
while true
do
	wget "http://[aaaa::200:0:0:1]" -O nodes.txt
	n=$(wc -l < nodes.txt)
	lines=$(($2*2 -2))
	echo $n
	
	if [ $n -gt $lines ]
	then
		java -jar observe_v8.jar $2 3600000 | tee -a $1
		exit 0
	fi

	sleep 60
done
