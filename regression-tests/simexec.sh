#!/bin/bash
RUNALL=$1
CSC=$2
CONTIKI=$3
BASENAME=$4
RANDOMSEED=$5

#set -x

echo -n "Running test $BASENAME "

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

echo " FAIL ಠ_ಠ" | tee -a COOJA.testlog; 

#Verbose output when using CI
if [ "$CI" = "true" ];  then 
  echo "==== COOJA.log ====" ; cat COOJA.log; 
  echo "==== COOJA.testlog ====" ; cat COOJA.testlog; 
else  
  tail -50 COOJA.log ; 
fi; 

mv COOJA.testlog $BASENAME.faillog

# We do not want Make to stop -> Return 0
if [ "$RUNALL" = "true" ] ; then
  touch COOJA.testlog; 
  mv COOJA.testlog $BASENAME.testlog; 
  exit 0
fi

#This is a failure
exit 1




