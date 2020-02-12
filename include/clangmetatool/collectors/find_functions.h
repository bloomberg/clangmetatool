#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_FUNCTIONS_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_FUNCTIONS_H

#include <vector>

namespace clang {

class CompilerInstance;
class FunctionDecl;

namespace ast_matchers {

class MatchFinder;

} // namespace ast_matchers

} // namespace clang

namespace clangmetatool {
namespace collectors {

/**
 * The data collected by the FindFunctions collector
 */
using FindFunctionsData = std::vector<const clang::FunctionDecl *>;

/**
 * Forward declaration to the implementation details of the
 * collector.
 */
class FindFunctionsImpl;

/**
 * FindFunctions data collector. Collects all the functions that
 * are defined within a source unit. This includes anonymous/static
 * functions but not functions defined in macros or functions that
 * are declared "extern"
 */
class FindFunctions {
private:
  /**
   * Pointer to implementation
   */
  FindFunctionsImpl *impl;

public:
  /**
   * Explicit constructor, to allow for implementation details:
   *    - ci is a pointer to an instance of the clang compiler
   *    - f is a pointer to an instance of the MatchFinder class
   */
  FindFunctions(clang::CompilerInstance *ci,
                clang::ast_matchers::MatchFinder *f);

  /**
   * Explicit destructor.
   */
  ~FindFunctions();

  /**
   * Get the pointer to the data structure, populated or not
   */
  const FindFunctionsData *getData() const;
};

} // namespace collectors
} // namespace clangmetatool

#endif

// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
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
