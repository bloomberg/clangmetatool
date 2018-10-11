#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_REFERENCES_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_REFERENCES_DATA_H

#include <unordered_map>

#include <clang/AST/Decl.h>

#include <clangmetatool/types/file_uid.h>

namespace clangmetatool {
namespace collectors {

/**
 * The data collected by the References collector.
 */
class ReferencesData {
public:
  /**
   * Contains all definitions that reference other identifiers.
   *  Key is the definition of a class/function/variable.
   *  Value is the declaration of an identifier referenced by that
   * class/function/variable.
   */
  std::unordered_multimap<const clang::NamedDecl *, const clang::NamedDecl *>
      deps;

  /**
   * Contains all references that are used in definitions.
   * (This is the inverse of "deps".)
   *  Key is the declaration of the identifier referenced by a
   * class/function/variable.
   *  Value is the definition of a class/function/variable that references the
   * Key.
   */
  std::unordered_multimap<const clang::NamedDecl *, const clang::NamedDecl *>
      refs;
};

} // namespace collectors
} // namespace clangmetatool

#endif // INCLUDED_CLANGMETATOOL_COLLECTORS_REFERENCES_DATA_H

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
