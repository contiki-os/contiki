#!/bin/bash                                                                     
 
export ALARMTIME=$1
shift
 
export PARENTPID=$$
 
exit_timeout() {
    echo Timeout
    kill $CHILDPID
    exit 1
}

tcpdump $* &
CHILDPID=$!

#Prepare to catch SIGALRM, call exit_timeout
trap exit_timeout SIGALRM
 
#Sleep in a subprocess, then signal parent with ALRM
(sleep $ALARMTIME; kill -ALRM $PARENTPID) &
#Record PID of subprocess                                                       
ALARMPID=$!
 
#Wait for child processes to complete normally
wait $CHILDPID
 
echo "Alarm never reached, children exited cleanly."
#Tidy up the Alarm subprocess                    
kill -9 $ALARMPID
exit 0
