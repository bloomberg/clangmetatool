# clang-metatool - A framework for reusing code in clang tools

The point of this library is to establish a framework for reusing code
in clang tools.

Source control at [https://github.com/bloomberg/clangmetatool/](https://github.com/bloomberg/clangmetatool/)

## Building

````bash
mkdir build
cd build
cmake -DClang_DIR=/path/to/clang/cmake
make
make install
````

## Infrastructure

### `clangmetatool::MetaToolFactory`

This provides the boilerplate for a refactoring tool action, since you
need a factory that passes the replacementsMap in to the frontend
action class.

### `clangmetatool::MetaTool`

This provides the boilerplate of a FrontendAction class that will
perform data gathering and then run a post-processing phase that may
do replacements. This simplifies the writing of the code into a
constructor that registers preprocessor callbacks or ast matchers and
a postprocessing phase.

### clangmetatool cmake module

When building a clang tool you are expected to ship the builtin headers from the compiler with the tool, otherwise the tool will fail to find headers like stdarg.h. Clang expects to find the builtin headers relative to the absolute path of where the tool is installed. This cmake module will provide a function called `clangmetatool_install` which will handle all of that for you, example at [skeleton/CMakeLists.txt](skeleton/CMakeLists.txt).

## Reusable data types

This defines types that can be used as building blocks, those will be
in the `clangmetatool::types` namespace.

## Reusable data collection

The other part of this library consists of a number of "Data
Collectors". Those will be in the `clangmetatool::collectors`
namespace.

"Data Collector" is a "design pattern" for reusing code in clang
tools. It works by having a class that takes the CompilerInstance
object as well as the match finder to the constructor and registers
all required callbacks in order to collect the data later.

The collector class will also have a "getData" method that will return
the pointer to a struct with the data. The "getData" method should
only be called in the 'post-processing' phase of the tool.

## Skeleton for a new project

After you "git init" into an empty directory, copy the contents of the
skeleton directory. To build that project, do something like:

````bash
( mkdir -p build && cd build/ && \
  cmake \
  -DClang_DIR=/path/to/clang/ \
  -Dclangmetatool_DIR=/path/to/clang/ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
  .. )
make -C build
````

# License and Copyright

```
// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------

```
