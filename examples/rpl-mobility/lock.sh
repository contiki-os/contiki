lockdir=.lock

while true; do
    if mkdir "$lockdir" 2> /dev/null
    then
        break
    else
        sleep 1
    fi
done
