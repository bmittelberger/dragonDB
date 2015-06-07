TIME="3s" # amount of time to wait for server before communicating with it
TESTS=(gets getsputs puts) # filenames of tests to run
TESTDIR="." # directory containing tests
DBFILE="dragon_store" # path to database file
NUMTHREADS="3"

for TEST in ${TESTS[*]}; do # perform each test
  TESTIN=$TESTDIR$TEST".tst"
  TESTOUT=$TESTDIR$TEST".out"
  echo "Testing "$TESTIN

  rm -rf $DBFILE # remove extant database file
  COMMAND="./dragonDB "$NUMTHREADS" "$TESTIN
  $COMMAND > /dev/null &
  PID=$!
  sleep $TIME
  disown $PID # don't get output for the server dying
  kill -9 $PID 2> /dev/null # kill the server
  echo
done
rm -rf $DBFILE # remove leftover database file