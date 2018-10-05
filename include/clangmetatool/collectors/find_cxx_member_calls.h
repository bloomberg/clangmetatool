#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CXX_MEMBER_CALLS_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CXX_MEMBER_CALLS_H

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>

#include <clangmetatool/collectors/find_cxx_member_calls_data.h>

namespace clangmetatool {
namespace collectors {

/**
 * forward declaration to implementation details of the
 * collector.
 */
class FindCXXMemberCallsImpl;

/**
 * Find Calls data collector. Collects the caller and references
 * to the specified function and one of its arguments.
 */
class FindCXXMemberCalls {
private:
  /**
   * Pointer to Implementation
   */
  FindCXXMemberCallsImpl *impl;

public:
  /**
   * Explicit constructor, to allow for implementation details:
   *    - ci is a pointer to an instance of the clang compiler
   *    - f is a pointer to an instance of the MatchFinder class
   *    - c is a string value of the fully-qualified class/struct name to match
   * on
   *    - n is a string value of the method name to match on
   */
  FindCXXMemberCalls(clang::CompilerInstance *ci,
                     clang::ast_matchers::MatchFinder *f, const std::string &c,
                     const std::string &n);

  /**
   * Explicit destructor.
   */
  ~FindCXXMemberCalls();

  /**
   * Get the pointer to the data structure, populated or not.
   */
  FindCXXMemberCallsData *getData();
};
}
}

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
