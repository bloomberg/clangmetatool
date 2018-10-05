#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_CONSTANT_INTEGER_PROPAGATOR_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_CONSTANT_INTEGER_PROPAGATOR_H

#include <cstdint>
#include <iostream>

#include <clangmetatool/propagation/propagation_result.h>

/**
 * Forward declarations for clang types
 */
namespace clang {
class CompilerInstance;
class DeclRefExpr;
class FunctionDecl;
}

namespace clangmetatool {
namespace propagation {

/**
 * Forward declaration to implementation details of the propagator.
 */
class ConstantIntegerPropagatorImpl;

/**
 * ConstantIntegerPropagator is a tool to run a propagation
 * over integer variables that are used within functions
 * and attempt to determine the integer values of those
 * variables.
 *
 * The analysis runs once per function even if runPropagation
 * is called multiple times.
 *
 * It is also important to note that this analysis assumes
 * that all loops will only be be run through the first time
 * and effectively ignores any changes that may be made from
 * reentering a block from the loop.
 */
class ConstantIntegerPropagator {
private:
  /**
    * Pointer to implementation.
    */
  ConstantIntegerPropagatorImpl *impl;

public:
  /**
   * Explicit constructor to allow for implementation details.
   *    - ci is a pointer to an instance of the clang compiler
   */
  ConstantIntegerPropagator(const clang::CompilerInstance *ci);

  /**
    * Explicit destructor.
    */
  ~ConstantIntegerPropagator();

  /**
   * Given the surrounding function and the usage of a int
   * holding variable, attempt to determine the value held by that variable.
   *
   * PropagationResult will return true for a call to `isUnresolved()` if a
   * deterministic value cannot be determined for the variable.
   */
  PropagationResult<std::intmax_t>
  runPropagation(const clang::FunctionDecl *function,
                 const clang::DeclRefExpr *variable);

  /**
   * Print out the variable contexts for all the functions that have
   * been propagated.
   */
  void dump(std::ostream &stream) const;
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
