#!/bin/bash
TESTIN="tests/small_puts.tst"
STORES="*.drg"

./dragonDB 4 $TESTIN
rm $STORES
./dragonDB 3 $TESTIN
rm $STORES
./dragonDB 2 $TESTIN
rm $STORES
./dragonDB 1 $TESTIN
rm $STORES
