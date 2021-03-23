#!/bin/bash


echo "*** Running C test driver ***"

sudo insmod bin/queue.ko

bin/test_driver

sudo rmmod queue.ko

echo "*** END: Running C test driver ***"
