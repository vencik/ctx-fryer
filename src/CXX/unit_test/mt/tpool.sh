#!/bin/sh

cat <<HERE | ./tpool -f 2>/dev/null
job Sleep for 5 seconds
    print 1: going to sleep for 5 seconds
    sleep 5
    print 1: slept well, goodbye
endjob

job Sleep for 3 + 3 seconds
    sleep 3
    print 2: slept for 3 seconds, shall sleep 3 more
    sleep 3
    print 2: slept fine, bye
endjob
HERE
