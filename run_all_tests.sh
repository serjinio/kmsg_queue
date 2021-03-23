#!/bin/bash

FILES=test/test_*.sh
for f in $FILES
do
    echo "Running $f file..."
    $f
done
