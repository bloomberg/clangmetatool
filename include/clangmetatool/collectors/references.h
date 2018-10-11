#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_REFERENCES_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_REFERENCES_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clangmetatool/collectors/references_data.h>

namespace clangmetatool {
namespace collectors {

/**
 * Forward declaration to implementation details of the collector.
 */
class ReferencesImpl;

/**
 * References data collector. Collects References of functions and variables
 * with global storage.
 */
class References {
private:
  /**
   * Pointer to implementation.
   */
  ReferencesImpl *impl;

public:
  /**
   * Explicit constructor to allow for implementation details.
   *    - ci is a pointer to an instance of the clang compiler
   *    - f is a pointer to an instance of the MatchFinder class
   */
  References(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f);

  /**
   * Explicit destructor.
   */
  ~References();

  /**
   * Get the pointer to the object containing the data; populated or not.
   */
  ReferencesData *getData();
};

} // namespace collectors
} // namespace clangmetatool

#endif // INCLUDED_CLANGMETATOOL_COLLECTORS_REFERENCES_H

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
