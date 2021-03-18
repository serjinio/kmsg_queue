#!/bin/bash -e


insert_msg() {
    echo "Sending message: $1"
    echo "$1" > /proc/msg_queue
}

get_msg() {
    cat /proc/msg_queue
    echo "<MSG DELIM>"
}

# ./build.sh

echo "*** Test for simple insert & pop ***"

sudo insmod bin/queue.ko

insert_msg "hello 1"
insert_msg "hello 2"
insert_msg "hello 3"

echo "Should obtain back three messages"

get_msg
get_msg
get_msg

echo "Now queue should be empty (no data after this message)"

get_msg

sudo rmmod queue.ko

echo "*** END: Test for simple insert & pop ***"
