#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CALLS_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CALLS_DATA_H

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/CompilerInstance.h>

namespace clangmetatool {
namespace collectors {

/**
 * The data collected by the FindCalls collector
 */
struct FindCallsData {

  /**
   * The declaration of the function and the context in which it occurs
   */
  std::multimap<const clang::FunctionDecl *, const clang::CallExpr *>
      call_context;

  /**
   * The reference to the function and the context in which it occurs
   */
  std::map<const clang::CallExpr *, const clang::DeclRefExpr *> call_ref;
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
