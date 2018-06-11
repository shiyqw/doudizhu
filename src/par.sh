#!/bin/bash

count=0
for ; do
    ./autorun
    (( count ++ ))        
    if (( count = 10 )); then
        wait
        count=0
    fi
done
