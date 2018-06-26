#ifndef INCLUDED_CLANGMETATOOL_TYPES_FILE_GRAPH_H
#define INCLUDED_CLANGMETATOOL_TYPES_FILE_GRAPH_H

#include <iosfwd>
#include <iostream>
#include <set>
#include <vector>

#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/types/file_graph_edge.h>

namespace clangmetatool {
  namespace types {
    class FileGraph : public std::set<FileGraphEdge> {

      /**
       * prints this graph on the given ostream in the "dot"
       * syntax. Users the given name as the naem of the graph.
       */
      void print_graph(const char* name, std::ostream &s);

      /**
       * returns true whether this edge is in this graph.
       */
      bool is_in_graph(const FileGraphEdge &e);

      /**
       * Returns the list of file uids that point to the given file
       * uid on the given graph.
       */
      std::vector<clangmetatool::types::FileUID>
      points_to(const clangmetatool::types::FileGraph &g,
                clangmetatool::types::FileUID fuid);
    };
  }
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
