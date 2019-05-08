#!/bin/sh
# Arguments to this script are:
#  1. the path to the tool as built by CMake
#  2. CMAKE_CURRENT_SOURCE_DIR
#  3. CMAKE_CURRENT_BINARY_DIR
#  4. the name of the test case
set -x
set -e
cp $2/$4.c $3/$4.c
$1 $3/$4.c -- -I$2/testinclude
diff -u $2/$4.c.expected $3/$4.c

