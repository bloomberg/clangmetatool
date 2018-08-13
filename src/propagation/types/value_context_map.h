#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_VALUE_CONTEXT_MAP_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_VALUE_CONTEXT_MAP_H

#include "value_context.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace clangmetatool {
namespace propagation {
namespace types {

/**
 * Mapping from a variable name and its location of usage to its value
 * at that point.
 */
template <typename ResultType>
class ValueContextMap {
private:
  using ValueContextType = ValueContext<ResultType>;
  using ValueContextSet  = std::set<ValueContextType>;

  // Use set for automatic sorting
  std::map<std::string, ValueContextSet> map;

public:
  /**
   * Add a new value to the the map.
   *    - The name of the variable
   *    - Its value
   *    - The location where it first has this value
   *    - Is this value defined by the code itself (not a control flow merge)?
   */
  void addToMap
  (const std::string& name, const ResultType& value, clang::SourceLocation start, ValueContextOrdering::Value ordering) {
    auto it = map.find(name);

    if(map.end() == it) {
      // Create a new context set if it doesn't exist
      map.insert({name, ValueContextSet({ValueContextType(start, ordering, value)})});
    } else {
      // If it exists, add it to the set
      it->second.insert(ValueContextType(start, ordering, value));
    }
  }

  /**
   * Squash the value map so that it is as conscise as possible.
   *
   * This is done by removing the second context if two subsequent contexts
   * have the same value. And removing the first context if two contexts have
   * the same source location.
   */
  void squash() {
    for(auto& it : map) {
      std::vector<ValueContextType> vec(it.second.begin(), it.second.end());

      ValueContextSet newSet;

      unsigned i = 0;
      unsigned j = 1;

      while(j < vec.size()) {
        // If two subsequent contexts have the same value, ignore the second
        if(std::get<2>(vec[i]) == std::get<2>(vec[j])) {
          ++j;
        } else {
          // If two subsequent contexts have the same line number, only
          // save the second
          if(std::get<0>(vec[i]) != std::get<0>(vec[j])) {
            newSet.insert(vec[i]);
          }

          i = j;
          ++j;
        }
      }

      newSet.insert(vec[i]);

      map[it.first] = newSet;
    }
  }

  /**
   * Lookup the value of a variable given its name and usage location in the source.
   * Return false if no context is found.
   */
  bool lookup
  (ResultType& result, const std::string& variable, const clang::SourceLocation& location) const {
    auto it = map.find(variable);

    if(map.end() != it) {
      // Find the last definition before the location
      for(auto sit = it->second.rbegin(); sit != it->second.rend(); ++sit) {
        if(std::get<0>(*sit) < location) {
          result = std::get<2>(*sit);

          return true;
        }
      }
    }

    return false;
  }

  auto begin() const { return map.begin(); }
  auto end() const { return map.end(); }
};

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
