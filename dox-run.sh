#!/bin/sh
set -e
git stash
git checkout -B gh-pages master
doxygen Doxyfile
mv doxygen/html/* .
git add .
git commit -am "Doxygen run over $(git rev-parse HEAD)"
git checkout master
git stash pop
