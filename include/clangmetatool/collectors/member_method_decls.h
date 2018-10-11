#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_MEMBER_METHOD_DECLS
#define INCLUDED_CLANGMETATOOL_COLLECTORS_MEMBER_METHOD_DECLS

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>

#include <clangmetatool/collectors/member_method_decls_data.h>

namespace clangmetatool {
namespace collectors {

/**
 * forward declaration to implementation details of the
 * collector.
 */
class MemberMethodDeclsImpl;

/**
 * Member Method Declarations data collector. Collects the information
 * related to struct member method declarations and their usages.
 */
class MemberMethodDecls {
private:
  /**
   * Pointer to Implementation
   */
  MemberMethodDeclsImpl *impl;

public:
  /**
   * Explicit constructor, to allow for implementation details.
   */
  MemberMethodDecls(clang::CompilerInstance *ci,
                    clang::ast_matchers::MatchFinder *f);

  /**
   * Explicit destructor.
   */
  ~MemberMethodDecls();

  /**
   * Get the pointer to the data structure, populated or not.
   */
  MemberMethodDeclsData *getData();
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
