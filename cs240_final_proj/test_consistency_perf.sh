#!/bin/bash
FILENAMES="no_file-*"

./dragonDB 1 -STRONG
rm -rf $FILENAMES
./dragonDB 1 
rm -rf $FILENAMES

./dragonDB 2 -STRONG
rm -rf $FILENAMES
./dragonDB 2
rm -rf $FILENAMES

./dragonDB 3 -STRONG
rm -rf $FILENAMES
./dragonDB 3
rm -rf $FILENAMES

./dragonDB 4 -STRONG
rm -rf $FILENAMES
./dragonDB 4
rm -rf $FILENAMES