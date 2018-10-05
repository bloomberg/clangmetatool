#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_CHANGED_IN_LOOP_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_TYPES_CHANGED_IN_LOOP_H

#include <map>
#include <set>
#include <string>

namespace clangmetatool {
namespace propagation {
namespace types {

/**
 * Map from a loop's id to the names of all the variables that
 * are modified within that loop
 */
class ChangedInLoop {
private:
  std::map<unsigned, std::set<std::string>> changed;

public:
  void save(unsigned loop, const std::string &name) {
    changed[loop].insert(name);
  }

  auto changedBegin(unsigned loop) { return changed[loop].begin(); }
  auto changedEnd(unsigned loop) { return changed[loop].end(); }
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
