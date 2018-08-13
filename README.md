# clang-metatool - A framework for reusing code in clang tools

[![Build Status](https://travis-ci.org/bloomberg/clangmetatool.svg?branch=master)](https://travis-ci.org/bloomberg/clangmetatool)

## About clangmetatool

When we first started writing clang tools, we realized that there is a
lot of life cycle management that we had to repeat. In some cases,
people advocate the usage of global variables to manage the life-cycle
of that data, but that makes code reuse across tools even harder.

Additionally, we also learned that when writing a tool, it will be
beneficial if the code is split in two phases. First a data collection
phase, and later a post-processing phase that actually performed the
bulk of the logic of the tool.

Essentially you will only need to write a class like:

```C++
class MyTool {
private:
  SomeDataCollector collector1;
  SomeOtherDataCollector collector2;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
   :collector1(ci, f), collector2(ci, f) {
   // the individual collectors will register their callbacks in their
   // constructor, the tool doesn't really need to do anything else here.
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
   // use data from collector1 and collector2
   // generate warnings and notices
   // add replacements to replacementsMap
  }
};
```

And then you can use the `clangmetatool::MetaToolFactory` combined
with the `clangmetatool::MetaTool` in your tool's main function:

```C++
int main(int argc, const char* argv[]) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  llvm::cl::extrahelp CommonHelp
    (clang::tooling::CommonOptionsParser::HelpMessage);
  clang::tooling::CommonOptionsParser
    optionsParser(argc, argv, MyToolCategory);
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements());
  int r = tool.runAndSave(&raf);
  return r;
}
```

One way in which our initial tools got hard to write and maintain was
by trying to perform analysis or even replacements during the
callbacks. It was not immediately obvious that this would lead to
hard-to-maintain code. After we switched to the two-phase approach, we
were able to reuse a lot more code across tools.

Fork me at [github](https://github.com/bloomberg/clangmetatool/)

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

Another part of this library consists of a number of "Data
Collectors". Those will be in the `clangmetatool::collectors`
namespace.

"Data Collector" is a "design pattern" for reusing code in clang
tools. It works by having a class that takes the CompilerInstance
object as well as the match finder to the constructor and registers
all required callbacks in order to collect the data later.

The collector class will also have a "getData" method that will return
the pointer to a struct with the data. The "getData" method should
only be called in the 'post-processing' phase of the tool.

## Constant Propagation

Another part of this consists of constant propagators to assist
with analysis. Those will be in the `clangmetatool::propagation`
namespace.

More specifically, the current implementation provides a constant
C-style string propagator, which propagates constant strings through
the control flow graph so that `char*` variables may be queried for there
true value anywhere where that value is deterministic.

This could be useful for various purposes but especially for identifing
things like which database a function is actually calling out to, etc.

### `clangmetatool::propagation::ConstantCStringPropagator`

This provides infrastructure (utilizing
`clangmetatool::propagation::ConstantPropagator` and
`clangmetatool::propagation::PropagationVisitor`) to propagate constant
C-style string values over the program. Resulting in the true value of a
variable wherever the value is deterministic and "<UNRESOLVED>" anywhere else.

#### `clangmetatool::propagation::ConstantPropagator` and `clangmetatool::propagation::PropagationVisitor`

These two classes provide the boilerplate to create infrastructure
to propagate constants of arbitrary types through the control flow graph
of the program in such a way that anywhere the constant value of a variable
would be deterministic one may query its value at that point.

These classes are private to the library, but additional propagators could be easily
made using these facilities.

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

## Building

You need a full llvm+clang installation directory. Unfortunately, the
Debian and Ubuntu packages [are
broken](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=900440), so
you may need to work-around by creating some symlinks (see
.travis.Dockerfile in this repo for an example).

````bash
mkdir build
cd build
cmake -DClang_DIR=/path/to/clang/cmake ..
make
make install
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
