#!/bin/bash
# Do not return an error
RUNALL=$1
shift
# The simulation to run
CSC=$1
shift
#Contiki directory
CONTIKI=$1
shift
#The basename of the experiment
BASENAME=$1
shift
# The test will end on the first successfull run

#set -x

while (( "$#" )); do
	RANDOMSEED=$1
	echo -n "Running test $BASENAME with random Seed $RANDOMSEED: "

	java -Xshare:on -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CSC -contiki=$CONTIKI -random-seed=$RANDOMSEED > $BASENAME.log &
	JPID=$!

	# Copy the log and only print "." if it changed
	touch $BASENAME.log.prog
	while kill -0 $JPID 2> /dev/null
	do
		sleep 1
		diff $BASENAME.log $BASENAME.log.prog > /dev/null
		if [ $? -ne 0 ] 
		then
		  echo -n "."
		  cp $BASENAME.log $BASENAME.log.prog
		fi
	done
	rm $BASENAME.log.prog


	wait $JPID
	JRV=$?

	if [ $JRV -eq 0 ] ; then
		touch COOJA.testlog; 
		mv COOJA.testlog $BASENAME.testlog 
		echo " OK"
		exit 0
	fi



	# In case of failure



	#Verbose output when using CI
	if [ "$CI" = "true" ]; then
		echo "==== $BASENAME.log ====" ; cat $BASENAME.log;
		echo "==== COOJA.testlog ====" ; cat COOJA.testlog;
		echo "==== Files used for simulation (sha1sum) ===="
		grep "Loading firmware from:" COOJA.log | cut -d " " -f 10 | uniq  | xargs -r sha1sum
		grep "Creating core communicator between Java class" COOJA.log | cut -d " " -f 17 | uniq  | xargs -r sha1sum
	else
		tail -50 $BASENAME.log ;
	fi;

	mv COOJA.testlog $BASENAME.$RANDOMSEED.faillog

	shift
done

#All seeds failed
	echo " FAIL ಠ_ಠ" | tee -a $BASENAME.$RANDOMSEED.faillog;

# We do not want Make to stop -> Return 0
if [ "$RUNALL" = "true" ] ; then
	touch COOJA.testlog; 
	mv COOJA.testlog $BASENAME.testlog; 
	exit 0
fi


exit 1




