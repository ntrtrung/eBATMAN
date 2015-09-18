#!/bin/bash

# very simple script for running several instances (10) 
# of "do_chanel" for testing purposes

i="0"

while [ $i -lt 10 ]
do
    echo CHANEL EXPERIMENT RUN \#$i
    ./do_chanel > /dev/null
    i=$[$i+1]
done
