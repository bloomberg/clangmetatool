#ifndef INCLUDED_CLANGMETATOOL_TYPES_FILE_ATTRIBUTE_MAP_H
#define INCLUDED_CLANGMETATOOL_TYPES_FILE_ATTRIBUTE_MAP_H

#include <map>

#include <clangmetatool/types/file_attribute.h>
#include <clangmetatool/types/file_uid.h>

namespace clangmetatool {
namespace types {

template <typename OTHER> using FileAttributeMap = std::map<FileUID, OTHER>;
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
