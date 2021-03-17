#!/bin/bash


insert_msg() {
    echo "Sending message: $1"
    echo "$1" > /proc/msg_queue
}

get_msg() {
    cat /proc/msg_queue
    echo "<MSG DELIM>"
}

cd src
make

sudo insmod queue.ko

insert_msg "hello 1000000"
insert_msg "hello 2"
insert_msg "hello 3"

echo "Should obtain back three messages"

get_msg
get_msg
get_msg

echo "Now queue should be empty (no data after this message)"

get_msg

sudo rmmod queue.ko