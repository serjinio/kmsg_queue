#!/bin/bash -e


# run build
pushd src
make
popd

# copy binary to separate folder
pwd
mkdir -p bin
cp src/queue.ko bin/queue.ko
