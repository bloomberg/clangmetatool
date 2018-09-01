#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>

#include <clangmetatool/collectors/files.h>

namespace clangmetatool {
namespace collectors {

class FilesImpl {
  private:
    FilesData data;

  public:
    FilesImpl (clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f) {
      clang::SourceManager& sm = ci->getSourceManager();

      clang::SourceManager::fileinfo_iterator fileIt;
      for(fileIt = sm.fileinfo_begin(); fileIt != sm.fileinfo_end(); fileIt++) {
        const clang::FileEntry* fe = fileIt->first;

        // Get file uid
        types::FileUID fuid = fe->getUID();

        // Get file path
        llvm::StringRef srpath = fe->tryGetRealPathName();
        std::string fpath = srpath.str();

        data.filePath.insert(std::pair<types::FileUID, std::string>(fuid, fpath));
      }
    }

    FilesData* getData() {
      return &data;
    }
};

Files::Files(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f) {
  impl = new FilesImpl(ci, f);
}

Files::~Files() {
  delete impl;
}

FilesData* Files::getData() {
  return impl->getData();
}

} // namespace collectors
} // namespace clangmetatool

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
