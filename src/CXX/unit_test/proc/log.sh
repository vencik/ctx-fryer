#!/bin/sh

thread_cnt=100
message_cnt=100
do_echo=off

./log $thread_cnt $message_cnt 2>&1 | awk '
BEGIN {
    thread_cnt  = '$thread_cnt';
    message_cnt = '$message_cnt';
    do_echo     = '$do_echo';
    error_cnt   = 0;
}

/message [0-9]+$/ {
    if ("on" == do_echo) print $0;

    thread_id  = $11;
    message_no = $13;

    prev_message_no = message[thread_id];
    if (0 != length(prev_message_no)) {
        if (prev_message_no + 1 != message_no) {
            print "Unexpected message from worker " thread_id \
                  ": got #" message_no " after #" prev_message_no;

            ++error_cnt;
        }
    }
    else {
        if (0 != message_no) {
            print "Unexpected 1st message from worker " thread_id \
                  ": got #" message_no;

            ++error_cnt;
        }
    }

    message[thread_id] = message_no;
}

END {
    for (thread_id in message) {
        message_no = message[thread_id];

        if (message_cnt - 1 != message_no) {
            print "Unexpected last message from worker " thread_id \
                  ": got #" message_no;

            ++error_cnt;
        }
    }

    if (0 != error_cnt) {
        print "Unit test FAILED";

        exit 1;
    }
}'
