
#!/bin/bash -e


insert_msg() {
    echo "$1" > /proc/msg_queue
}

get_msg() {
    cat /proc/msg_queue
    echo "<MSG DELIM>"
}

# ./build.sh

echo "*** Test for cleanup of left-over resources using many messages ***"

sudo insmod bin/queue.ko

MSG_NUM=1000
echo "Will put $MSG_NUM messages to the queue..."
for i in $(seq $MSG_NUM)
do
    insert_msg "Welcome number $i"
done

echo "Will obtain back three messages"

get_msg
get_msg
get_msg

echo "Now queue should still contain $(expr $MSG_NUM - 3) messages - should be cleaned on rmmod"

sudo rmmod queue.ko

echo "Last fifty lines from kern.log:"
tail -n 50 /var/log/kern.log

echo "*** END: Test for cleanup of left-over resources on rmmod call ***"
