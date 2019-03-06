#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_PROPAGATION_RESULT_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_PROPAGATION_RESULT_H

#include <iostream>

namespace clangmetatool {
namespace propagation {

/**
 * Type to represent the result in propagation, since a value
 * may either be unresolved in the case that it's value at some
 * point in the code is non-deterministic, or it may have a
 * deterministic value
 */
template <typename ResultType> class PropagationResult {
private:
  bool unresolved;
  ResultType result;

public:
  /**
   * Default constructor to produce an unresolved value
   */
  PropagationResult() : unresolved(true) {}

  /**
   * Constructor taking ResultType to produce a resolved value
   */
  PropagationResult(const ResultType &result)
      : unresolved(false), result(result) {}

  /**
   * Check if the result is unresolved.
   */
  bool isUnresolved() const { return unresolved; }

  /**
   * Get the resolved result.
   *
   * Calling this function before the result has been determined to indeed
   * be resolved  with a call to `isUnresolved()` is undefined behaviour.
   */
  const ResultType &getResult() const { return result; }

  /**
   * Print the result to a stream.
   *
   * When the result is undefined, it will output "<UNRESOLVED>".
   */
  void print(std::ostream &stream) const {
    if (unresolved) {
      stream << "<UNRESOLVED>";
    } else {
      stream << result;
    }
  }

  bool operator<(const PropagationResult<ResultType> &rhs) const {
    return result < rhs.result;
  }
  bool operator==(const PropagationResult<ResultType> &rhs) const {
    if (unresolved && rhs.unresolved) {
      return true;
    } else if (unresolved == rhs.unresolved) {
      return result == rhs.result;
    }

    return false;
  }
  bool operator!=(const PropagationResult<ResultType> &rhs) const {
    return !(*this == rhs);
  }
};

} // namespace propagation
} // namespace clangmetatool

template <typename T>
std::ostream &
operator<<(std::ostream &stream,
           const clangmetatool::propagation::PropagationResult<T> &result) {
  result.print(stream);

  return stream;
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
