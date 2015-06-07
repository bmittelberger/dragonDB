#!/bin/bash

MAX=100000
OUTPUT="../test/big_gets.tst"
rm $OUTPUT
echo "open dragon_store" > $OUTPUT
for i in `seq 1 $MAX`
do 
	echo "get $i" >> $OUTPUT  #puts key-val pairs where key=val
done
