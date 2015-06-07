TIME="3s" 
TESTS=(big_puts big_gets) # filenames of tests to run, important that puts precedes gets
TESTDIR="./test/" # directory containing tests
NUMTHREADS="4"
STORES="dragon_store-*"

echo "Testing "$TESTIN
for THREAD in `seq 1 $NUMTHREADS`; do # perform each test
  for TEST in ${TESTS[*]}; do # perform each test
    TESTIN=$TESTDIR$TEST".tst"
    TESTOUT=$TESTDIR$TEST".out"

    start=$(date +%s%N) #current time in nanoseconds
    "./dragonDB $THREAD $TESTIN" > $TEST$THREAD"_res.out"
    PID=$!
    now=$(date +%s%N)
    time_elapsed=$((now-start))
    echo $TEST" "$THREAD" threads took "$time_elapsed" nanoseconds to complete"
    sleep $TIME
    echo "----"
  done
  
  rm $STORES
done
