#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_CONSTANT_PROPAGATOR_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_CONSTANT_PROPAGATOR_H

#include "block_visitor_manager.h"

#include <iostream>
#include <map>
#include <string>

#include <clang/Analysis/CFG.h>
#include <clang/AST/Expr.h>
#include <clang/Frontend/CompilerInstance.h>

namespace clangmetatool {
namespace propagation {

/**
 * Class to run constant propagation given a visitor type
 */
template <typename V>
class ConstantPropagator {
public:
  using VisitorType    = V;
  using ManagerType    = BlockVisitorManager<VisitorType>;
  using ResultType     = typename VisitorType::ResultType;

private:
  const clang::CompilerInstance* ci;

  std::map<std::string, ManagerType> managers;

public:
  /**
   * We need a CompilerInstance to be able to run the propagation
   */
  ConstantPropagator(const clang::CompilerInstance* ci)
    : ci(ci) {
  }

  /**
   * Run the propagation (if not run already) on a variable usage
   * in a particular function.
   *
   * By not already run we mean to say that this class in particular
   * will attempt to cache propagations for future use.
   *
   * If a resolved (i.e. deterministic) value was not found for the
   * variable at the location it is used in particular then the
   * PropagationResult will be marked as unresolved.
   */
  ResultType runPropagation
  (const clang::FunctionDecl* func, const clang::DeclRefExpr* var) {
    ResultType result;
    bool wasFound = false;

    auto varName = var->getNameInfo().getAsString();

    auto it = managers.find(func->getQualifiedNameAsString());

    if(managers.end() == it) {
      // If the propagation was not already run
      std::unique_ptr<clang::CFG> cfg
        = clang::CFG::buildCFG(func,
                               func->getBody(),
                               &ci->getASTContext(),
                               clang::CFG::BuildOptions());

      // Store the propagation
      managers.emplace(std::piecewise_construct,
                       std::forward_as_tuple(func->getQualifiedNameAsString()),
                       std::forward_as_tuple(ci->getASTContext(), cfg.get()));

      // Run the propagation
      const auto& manager = managers.find(func->getQualifiedNameAsString())->second;

      // Lookup the result
      wasFound = manager.lookup(result, varName, var->getLocStart());
    } else {
      // Otherwise, look up the variable in the saved propagation
      wasFound = it->second.lookup(result, varName, var->getLocStart());
    }

    if(wasFound) {
      return result;
    }

    return {};
  }

  /**
   * Print out the variable contexts for all the functions that have
   * been propagated.
   *
   * Note that this assumes that the stream operator has been set
   * up for the Visitor's ReturnType.
   */
  void dump(std::ostream& stream) const {
    for(const auto& it : managers) {
      stream << it.first << " >>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
      it.second.dump(stream, ci->getSourceManager());
      stream << it.first << " <<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    }
  }
};

} // namespace propagation
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
