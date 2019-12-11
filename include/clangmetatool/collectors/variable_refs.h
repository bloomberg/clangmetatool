#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_VARIABLES_REFS_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_VARIABLES_REFS_H

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>

#include <clangmetatool/collectors/variable_refs_data.h>

namespace clangmetatool {
namespace collectors {

/**
 * forward declaration to implementation details of the
 * collector.
 */
class VariableRefsImpl;

/**
 * Variable Refs data collector. Collects the information related
 * to variable declarations and their usages.
 */
class VariableRefs {
private:
  /**
   * Pointer to Implementation
   */
  VariableRefsImpl *impl;

public:
  /**
   * Explicit constructor, to allow for implementation details.
   */
  VariableRefs(clang::CompilerInstance *ci,
               clang::ast_matchers::MatchFinder *f);

  /**
   * Explicit destructor.
   */
  ~VariableRefs();

  /**
   * Get the pointer to the data structure, populated or not.
   */
  VariableRefsData *getData();
};
} // namespace collectors
} // namespace clangmetatool

#endif

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
