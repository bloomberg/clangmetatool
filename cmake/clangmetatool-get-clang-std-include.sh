#!/bin/bash

VERSION=$($1/../bin/clang --version | head -n 1 | cut -d '(' -f 1 | cut -d ' ' -f 3 | cut -d '-' -f 1)

echo $(realpath $1/../lib*/clang/$VERSION/include)
