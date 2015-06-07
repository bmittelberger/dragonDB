#!/bin/bash

MAX=100000
OUTPUT="../tests/big_puts.tst"
rm $OUTPUT
echo "open dragon_store" > $OUTPUT
for i in `seq 1 $MAX`
do 
	echo "puts $i $i" >> $OUTPUT  #puts key-val pairs where key=val
done
#echo "close" >> $OUTPUT
