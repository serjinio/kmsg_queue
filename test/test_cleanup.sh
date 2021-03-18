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

sudo insmod bin/queue.ko

insert_msg "hello 1"
insert_msg "hello 2"
insert_msg "hello 3"
insert_msg "hello 1"
insert_msg "hello 2"
insert_msg "hello 3"

echo "Should obtain back three messages"

get_msg
get_msg
get_msg

echo "Now queue should still contain 3 messages - should be cleaned on rmmod"

sudo rmmod queue.ko

echo "Last five lines from kern.log:"
tail -n 5 /var/log/kern.log
