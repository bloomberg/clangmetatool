#ifndef INCLUDED_CLANGMETATOOL_SOURCE_UTIL_H
#define INCLUDED_CLANGMETATOOL_SOURCE_UTIL_H

#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>

#include <ostream>
#include <string>

namespace clangmetatool {

/**
 * Namespace for common clang source-related utilities
 */
struct SourceUtil {
  /**
   * Record describing a region of code
   */
  struct Region {
    std::string d_filename;
    int d_line = -1;
    int d_column = -1;
    int d_length = -1;
  };

  /**
   * Expand the given range as much as possible by resolving macros and
   * iterating past tokens.
   */
  static clang::CharSourceRange
  expandRange(const clang::SourceRange &range,
              const clang::SourceManager &sourceManager);

  /**
   * Expand the given range as much as possible by resolving macros and
   * iterating past tokens. This function will return an invalid range if no
   * contiguous expansion can be found for the range, e.g. the begin and end of
   * each expansion are in different macros.
   */
  static clang::CharSourceRange
  expandRangeIfValid(const clang::SourceRange &range,
                     const clang::SourceManager &sourceManager,
                     clang::Preprocessor &preprocessor);

  /**
   * Return a range for the given statement, using the given source manager to
   * locate code. The range will point to the location after expanding macros.
   */
  static clang::CharSourceRange
  getRangeForStatement(const clang::Stmt &statement,
                       const clang::SourceManager &sourceManager);

  /**
   * Return the source code for the given statement, using the given source
   * manager to locate code.
   */
  static std::string
  getSourceForStatement(const clang::Stmt &statement,
                        const clang::SourceManager &sourceManager);

  /**
   * Return the macro name used in the given statement, using the given source
   * manager to locate code, or an empty string if the statement does not
   * contain a macro.
   */
  static std::string
  getMacroNameForStatement(const clang::Stmt &statement,
                           const clang::SourceManager &sourceManager);

  /**
   * Return a record describing the given region, using the given source
   * manager to locate code
   */
  static Region getRegionForRange(clang::CharSourceRange range,
                                  const clang::SourceManager &sourceManager);

  /**
   * Return a record describing the region for the given statement, using the
   * given source manager to locate code.
   */
  static Region
  getRegionForStatement(const clang::Stmt &statement,
                        const clang::SourceManager &sourceManager);

  /**
   * Return true if the given statement is partially contained in a macro,
   * using the given source manager to locate code.
   */
  static bool isPartialMacro(const clang::SourceRange &sourceRange,
                             const clang::SourceManager &sourceManager,
                             clang::Preprocessor &preprocessor);
};

/**
 * Compare two regions and return 'true' if they point to the same location.
 */
bool operator==(const SourceUtil::Region &lhs, const SourceUtil::Region &rhs);

/**
 * Compare two regions and return 'true' if they do not point to the same
 * location.
 */
bool operator!=(const SourceUtil::Region &lhs, const SourceUtil::Region &rhs);

/**
 * Write the given region to the given stream in a human-readable format.
 */
std::ostream &operator<<(std::ostream &stream,
                         const SourceUtil::Region &region);

} // namespace clangmetatool

#endif

// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
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
