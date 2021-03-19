
#!/bin/bash -e

echo "*** Test for parallel insert ***"

sudo insmod bin/queue.ko

INSERT_CLIENTS_NUM=1
INSERT_CLIENT_MSG_NUM=2000

echo "Will start $INSERT_CLIENTS_NUM clients to insert messages into the queue..."
for i in $(seq $INSERT_CLIENTS_NUM)
do
    test/insert_client.sh "client #$i" $INSERT_CLIENT_MSG_NUM &
done

# Let the clients finish insertion
sleep 3

echo "Will obtain back some messages..."

POP_CLIENTS_NUM=100
POP_CLIENT_POP_MSG_NUM=20

echo "Will start $POP_CLIENTS_NUM clients to pop messages from the queue..."
for i in $(seq $POP_CLIENTS_NUM)
do
    test/pop_client.sh "client #$i" $POP_CLIENT_POP_MSG_NUM &
done

# Let the clients finish reads from the queue
sleep 15

echo "Finished reading from the queue. Will do rmmod"

sudo rmmod queue.ko

echo "Last 20 lines from kern.log:"
tail -n 20 /var/log/kern.log

echo "*** END: Test for parallel insert ***"
