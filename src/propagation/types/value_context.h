#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_VALUE_CONTEXT_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_VALUE_CONTEXT_H

#include "value_context_ordering.h"

#include <tuple>

#include <clang/Basic/SourceLocation.h>

namespace clangmetatool {
namespace propagation {
namespace types {

/**
 * A value as well as its context.
 *    - Location of definition
 *    - How the ordering is defined
 *    - The value at that context
 */
template <typename T>
using ValueContext = std::tuple<clang::SourceLocation, ValueContextOrdering::Value, T>;

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
