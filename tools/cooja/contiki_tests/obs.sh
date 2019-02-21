routes.py=0
while true
do
	wget "http://[aaaa::200:0:0:1]" -O nodes.txt
  routes=$(python routes.py 2>&1)
	echo $routes
	if [ $routes -ge $2 ]
	then
		java -jar empada2.jar $2 1800000 $3 | tee -a $1
		exit 0
	fi

	sleep 20
done
