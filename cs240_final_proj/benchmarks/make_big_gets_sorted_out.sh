#!/bin/bash

MAX=10000
OUTPUT="../tests/big_gets.out"
rm $OUTPUT
for i in `seq 1 $MAX`
do 
	echo "$i : $i" >> $OUTPUT  #puts key-val pairs where key=val
done
