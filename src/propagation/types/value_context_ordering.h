#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_VALUE_CONTEXT_ORDERING_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_VALUE_CONTEXT_ORDERING_H

#include <iostream>

#include <clang/Basic/SourceLocation.h>

namespace clangmetatool {
namespace propagation {
namespace types {

struct ValueContextOrdering {
private:
  ValueContextOrdering() = delete;

public:
  enum Value { CONTROL_FLOW_MERGE = 0, CHANGED_BY_CODE = 1 };

  static void print(std::ostream &stream, Value value);
};

std::ostream &operator<<(std::ostream &stream,
                         ValueContextOrdering::Value value);

} // namespace types
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
