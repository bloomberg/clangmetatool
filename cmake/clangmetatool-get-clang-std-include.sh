#!/bin/bash

VERSION=$($1/../bin/clang --version | head -n 1 | cut -d '(' -f 1 | cut -d ' ' -f 3 | cut -d '-' -f 1)

DIRS=$(realpath $1/../lib*/clang/$VERSION/include)

# pick directory which has a sample std header
for dir in $DIRS
do
  if [ -f "$dir/stddef.h" ]
  then
    echo $dir
    break
  fi
done
