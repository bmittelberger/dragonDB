TIME="3s" 
TESTS=(big_puts) # filenames of tests to run, important that puts precedes gets
TESTDIR="./tests/" # directory containing tests
NUMTHREADS="3"
STORES="dragon_store-*"

echo "Testing "$TESTIN
for THREAD in `seq 1 $NUMTHREADS`; do # perform each test
  for TEST in ${TESTS[*]}; do # perform each test
    TESTIN=$TESTDIR$TEST".tst"
    TESTOUT=$TESTDIR$TEST".out"

    start=$(date +%s%N) #current time in nanoseconds
    ./dragonDB $THREAD $TESTIN
    PID=$!
    now=$(date +%s%N)
    time_elapsed=$((now-start))
    echo $TEST" "$THREAD" threads took "$time_elapsed" nanoseconds to complete"
    #kill -9 $PID
    sleep $TIME
    echo "----"
  done
  rm $STORES
done
