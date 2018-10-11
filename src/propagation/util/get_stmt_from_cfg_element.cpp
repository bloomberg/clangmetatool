#include "get_stmt_from_cfg_element.h"

#include <llvm/ADT/Optional.h>

namespace clangmetatool {
namespace propagation {
namespace util {

bool getStmtFromCFGElement(const clang::Stmt *&result,
                           const clang::CFGElement &element) {
  if (llvm::Optional<clang::CFGStmt> opStmt = element.getAs<clang::CFGStmt>()) {
    result = opStmt->getStmt();

    return true;
  }

  return false;
}

} // namespace util
} // namespace propagation
} // namespace clangmetatool

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
