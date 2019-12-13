#ifndef INCLUDED_CLANGMETATOOL_INCLUDE_GRAPH_DEPENDENCIES_H
#define INCLUDED_CLANGMETATOOL_INCLUDE_GRAPH_DEPENDENCIES_H

#include <clangmetatool/collectors/include_graph_data.h>

namespace clangmetatool {
struct IncludeGraphDependencies {
  /**
   * Decrement the reference count tracked for this edge, notifying the caller
   * if the operation was successful.
   *
   * Return a boolean if indicating we were able to decrement the count:
   *  - false if the edge doesn't exist or has no balance to decrement
   *  - true otherwise. This will always reduce the count by 1
   */
  static bool
  decrementUsageRefCount(clangmetatool::collectors::IncludeGraphData *data,
                         const clangmetatool::types::FileGraphEdge &edge);

  /**
   * Get the 'live' dependencies of a header within the given include graph.
   * This is a subset of the set of transitive dependencies.
   *
   * Header B is called a 'live' dependency of B if A uses or references a
   * name originating from B or any of its transitive dependencies, i.e.:
   * - A references a macro provided through B
   * - A references a variable declaration provided through B
   * - A references a type declaration provided through B
   * - A redeclares a named declaration in B.
   */
  static std::set<clangmetatool::types::FileUID>
  liveDependencies(const clangmetatool::collectors::IncludeGraphData *data,
                   const clangmetatool::types::FileUID &headerFUID);
}; // struct IncludeGraphDependencies
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
