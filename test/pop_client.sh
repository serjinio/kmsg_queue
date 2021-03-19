#!/bin/bash -e

get_msg() {
    cat /proc/msg_queue
}

# Introduce 0.1 second delay before we start inserting messages
# so that parent scripts starts all clients before they start to
# insert messages
# sleep 0.1

CLIENT_NAME=$1
MSG_NUM=$2
echo "$CLIENT_NAME: Will pop $MSG_NUM messages from the queue..."
for i in $(seq $MSG_NUM)
do
    get_msg
done
