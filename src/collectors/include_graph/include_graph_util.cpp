
#include <algorithm>
#include <array>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/DirectoryLookup.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Tooling.h>
#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/types/file_graph_edge.h>
#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/types/macro_reference_info.h>
#include <iosfwd>
#include <limits.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <map>
#include <ostream>
#include <sstream>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <utility>

#include "include_graph_util.h"
#include <tuple>

namespace clangmetatool {
namespace collectors {
namespace include_graph {

using namespace clangmetatool::types;
using namespace clangmetatool::collectors;

static std::pair<FileUID, bool> get_fileuid(clang::CompilerInstance *ci,
                                            IncludeGraphData *data,
                                            clang::FileID fid) {

  clang::SourceManager &sm = ci->getSourceManager();
  const clang::FileEntry *entry = sm.getFileEntryForID(fid);
  if (!(entry && entry->isValid()))
    return std::pair<FileUID, bool>(0, false);

  FileUID fuid = entry->getUID();
  clang::SourceLocation l = sm.translateLineCol(fid, 1, 1);

  data->last_include.emplace(FileAttribute<clang::SourceLocation>(fuid, l));
  data->fuid2entry.emplace(
      FileAttribute<const clang::FileEntry *>(fuid, entry));

  return std::pair<FileUID, bool>(fuid, true);
}

static std::pair<FileUID, bool> get_fileuid(clang::CompilerInstance *ci,
                                            IncludeGraphData *data,
                                            clang::SourceLocation loc) {
  clang::SourceManager &sm = ci->getSourceManager();

  clang::SourceLocation sloc = sm.getSpellingLoc(loc);
  clang::FileID fid = sm.getFileID(sloc);

  return get_fileuid(ci, data, fid);
}

static FileUID resolve_include_next(clang::CompilerInstance *ci,
                                    IncludeGraphData *data, FileUID i) {
  while (data->include_next.find(i) != data->include_next.end()) {
    i = data->include_next[i];
  }
  return i;
}

void add_include_statement(clang::CompilerInstance *ci, IncludeGraphData *data,
                           clang::SourceLocation hashLoc,
                           const clang::Token &includeToken,
                           llvm::StringRef filename, bool isAngled,
                           clang::CharSourceRange filenameRange,
                           const clang::FileEntry *file,
                           llvm::StringRef searchPath,
                           llvm::StringRef relativePath,
                           const clang::Module *imported) {

  std::string include = (std::string)relativePath;
  if (file && file->isValid()) {
    FileUID fuid = file->getUID();

    data->fuid2entry.emplace(fuid, file);
    data->fuid2name.emplace(fuid, include);

    std::pair<FileUID, bool> tuid = get_fileuid(ci, data, hashLoc);
    if (!tuid.second)
      return;

    if (includeToken.getIdentifierInfo()->getPPKeywordID() ==
        clang::tok::pp_include_next) {
      data->include_next[fuid] = tuid.second;
    }

    std::pair<FileAttributeMap<clang::SourceLocation>::iterator, bool>
        last_incl_empl = data->last_include.emplace(tuid.first, hashLoc);
    if (!last_incl_empl.second)
      last_incl_empl.first->second = hashLoc;

    data->include_graph.insert(FileGraphEdge(tuid.first, fuid));

    data->include_statements.insert(
        FileGraphEdgeMultimap<clang::SourceRange>::value_type(
            FileGraphEdge(tuid.first, fuid),
            clang::SourceRange(hashLoc, filenameRange.getEnd())));
  }
}

static std::pair<bool, std::pair<FileUID, FileUID>>
resolve_file_graph_edge(clang::CompilerInstance *ci, IncludeGraphData *data,
                        clang::SourceLocation caller,
                        clang::SourceLocation callee) {

  std::pair<FileUID, bool> a = get_fileuid(ci, data, caller);
  if (!a.second)
    return {false, {0, 0}};

  std::pair<FileUID, bool> b = get_fileuid(ci, data, callee);
  if (!b.second)
    return {false, {0, 0}};

  FileUID final = resolve_include_next(ci, data, b.first);
  if (a.first == final)
    return {false, {0, 0}};

  return {true, {a.first, final}};
}

template <typename ELEMENT, typename MULTIMAP>
static void add_usage(clang::CompilerInstance *ci, IncludeGraphData *data,
                      clang::SourceLocation caller,
                      clang::SourceLocation callee, ELEMENT &e, MULTIMAP &m) {

  std::tuple<bool, std::pair<FileUID, FileUID>> resolved =
      resolve_file_graph_edge(ci, data, caller, callee);

  if (!std::get<0>(resolved))
    return;

  m.insert({std::get<1>(resolved), e});
  data->use_graph.insert(std::get<1>(resolved));
}

void add_macro_reference(clang::CompilerInstance *ci, IncludeGraphData *data,
                         clangmetatool::types::MacroReferenceInfo m) {
  if (!std::get<1>(m))
    return;

  clang::MacroInfo *info = std::get<1>(m).getMacroInfo();
  if (!info)
    return;

  clang::SourceLocation usageLoc = std::get<0>(m).getLocation();
  clang::SourceLocation defLoc = info->getDefinitionLoc();

  add_usage(ci, data, usageLoc, defLoc, m, data->macro_references);
}

void add_redeclaration(clang::CompilerInstance *ci, IncludeGraphData *data,
                       const clang::Decl *n) {

  const clang::Decl *decl = n->getCanonicalDecl();
  if (!decl)
    return;

  add_usage(ci, data, n->getLocation(), decl->getLocation(), n,
            data->redeclarations);
}

void add_decl_reference(clang::CompilerInstance *ci, IncludeGraphData *data,
                        const clang::DeclRefExpr *e) {

  clang::SourceLocation locUse = e->getLocation();

  const clang::ValueDecl *d = e->getDecl();
  if (!d)
    return;
  clang::SourceLocation locDef = d->getLocation();

  add_usage(ci, data, locUse, locDef, e, data->decl_references);
}

template <typename T>
static clang::Decl *extract_decl_for_type(const clang::Type *t) {
  const T *inner = t->getAs<T>();
  if (inner) {
    return inner->getDecl();
  } else {
    return NULL;
  }
}

void add_type_reference(clang::CompilerInstance *ci, IncludeGraphData *data,
                        const clang::TypeLoc *n) {

  const clang::Type *t = n->getTypePtr();
  const clang::Decl *decl = NULL;
  if (!decl)
    decl = extract_decl_for_type<clang::TypedefType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::RecordType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::InjectedClassNameType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::ObjCTypeParamType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::ObjCInterfaceType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::TagType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::TemplateTypeParmType>(t);
  if (!decl)
    decl = extract_decl_for_type<clang::UnresolvedUsingType>(t);
  if (!decl)
    return;

  add_usage(ci, data, n->getBeginLoc(), decl->getLocation(), n,
            data->type_references);
}
} // namespace include_graph
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
