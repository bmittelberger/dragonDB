#!/bin/bash
FILENAMES="no_file-*"

./dragonDB 1 -STRONG
rm $FILENAMES
./dragonDB 1 
rm $FILENAMES

./dragonDB 2 -STRONG
rm -rf $FILENAMES
./dragonDB 2
rm $FILENAMES

./dragonDB 3 -STRONG
rm $FILENAMES
./dragonDB 3
rm $FILENAMES

./dragonDB 4 -STRONG
rm $FILENAMES
./dragonDB 4
rm $FILENAMES