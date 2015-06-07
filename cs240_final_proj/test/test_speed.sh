#!/bin/bash
TIME="3s" 
TESTS=(big_puts big_gets) # filenames of tests to run, important that puts precedes gets
TESTDIR="." # directory containing tests
DBFILE="dragon_store" # path to database file
NUMTHREADS="3"

for TEST in ${TESTS[*]}; do # perform each test
  TESTIN=$TESTDIR$TEST".tst"
  TESTOUT=$TESTDIR$TEST".out"
  echo "Testing "$TESTIN
  for THREAD in `seq 1 $NUMTHREADS`; do # perform each test
    start=$(date +%s%N) #current time in nanoseconds
    COMMAND="../dragonDB "$THREAD" "$TESTIN
    $COMMAND > $TEST"_"THREAD"_res.out" &
    PID=$!
    now=$(date +%s%N)
    time_elapsed=$((now-start))
    echo $THREAD" threads took "$time_elapsed" nanoseconds to complete"
    sleep $TIME
    disown $PID # don't get output for the server dying
    kill -9 $PID 2> /dev/null # kill the server
    echo "----"
  done
done
rm -rf $DBFILE # remove leftover database file