#include <clangmetatool/source_util.h>

#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

#include <algorithm>
#include <cassert>

namespace clangmetatool {

namespace {
const clang::MacroInfo *getMacroInfo(clang::SourceLocation location,
                                     const clang::SourceManager &sourceManager,
                                     clang::Preprocessor &preprocessor) {
  // Get macro identifier for the given location

  llvm::StringRef macroName = preprocessor.getImmediateMacroName(location);
  if (macroName.empty()) {
    return nullptr;
  }

  clang::IdentifierInfo *macroId = preprocessor.getIdentifierInfo(macroName);
  if (!macroId) {
    return nullptr;
  }

  // The macro may have been redefined, so we need to search through history
  // by location.

  clang::MacroDirective *history =
      preprocessor.getLocalMacroDirectiveHistory(macroId);
  if (!history) {
    return nullptr;
  }

  return history->findDirectiveAtLoc(location, sourceManager).getMacroInfo();
}

llvm::ArrayRef<clang::Token>
getMacroTokens(clang::SourceLocation location,
               const clang::SourceManager &sourceManager,
               clang::Preprocessor &preprocessor) {
  // Get macro identifier for the given location
  auto *macroInfo = getMacroInfo(location, sourceManager, preprocessor);
  return macroInfo ? macroInfo->tokens() : llvm::ArrayRef<clang::Token>();
}

bool sourceRangeContainsOnly(clang::SourceLocation beginLocation,
                             clang::SourceLocation endLocation,
                             const std::string &allowed,
                             const clang::SourceManager &sourceManager) {
  for (const char *it = sourceManager.getCharacterData(beginLocation),
                  *end = sourceManager.getCharacterData(endLocation);
       it != end; ++it) {
    if (*it == '\\') {
      ++it;
      if (it == end && !(*it == '\n' || *it == '\r')) {
        return false;
      }
    } else if (allowed.find(*it) == std::string::npos) {
      return false;
    }
  }
  return true;
}

} // namespace

clang::CharSourceRange
SourceUtil::expandRange(const clang::SourceRange &range,
                        const clang::SourceManager &sourceManager) {
  // Get the start location, resolving from macro definition to macro call
  // location. The loop is adapted from 'clang::SourceManager::getFileLoc'.

  clang::SourceLocation begin = range.getBegin();
  while (!begin.isFileID()) {
    if (sourceManager.isMacroArgExpansion(begin)) {
      begin = sourceManager.getImmediateSpellingLoc(begin);
    } else {
      begin = sourceManager.getImmediateExpansionRange(begin).getBegin();
    }
  }

  // Get the end location, resolving from macro definition to macro call
  // location. The end location of a statement points to the beginning of the
  // last token in it, so we must use the lexer to traverse the token too.

  clang::SourceLocation end = range.getEnd();
  while (!end.isFileID()) {
    if (sourceManager.isMacroArgExpansion(end)) {
      end = sourceManager.getImmediateSpellingLoc(end);
    } else {
      end = sourceManager.getImmediateExpansionRange(end).getEnd();
    }
  }
  end = clang::Lexer::getLocForEndOfToken(end, 0, sourceManager,
                                          clang::LangOptions());

  return clang::CharSourceRange::getCharRange(begin, end);
}

clang::CharSourceRange
SourceUtil::getRangeForStatement(const clang::Stmt &statement,
                                 const clang::SourceManager &sourceManager) {
  return expandRange(statement.getSourceRange(), sourceManager);
}

std::string
SourceUtil::getSourceForStatement(const clang::Stmt &statement,
                                  const clang::SourceManager &sourceManager) {
  clang::CharSourceRange sourceRange =
      getRangeForStatement(statement, sourceManager);
  auto text = clang::Lexer::getSourceText(sourceRange, sourceManager,
                                          clang::LangOptions());
  // If used incorrectly, or on an 'implicit' element getSourceText will return
  // an empty string. This was probably unintentional. Fail fast
  assert(!text.empty() && "Lexer::getSourceText returned an empty string!");
  return std::string(text);
}

std::string SourceUtil::getMacroNameForStatement(
    const clang::Stmt &statement, const clang::SourceManager &sourceManager) {
  clang::SourceLocation loc = statement.getBeginLoc();

  while (loc.isMacroID()) {
    if (sourceManager.isMacroBodyExpansion(loc)) {
      return clang::Lexer::getImmediateMacroName(loc, sourceManager,
                                                 clang::LangOptions())
          .str();
    }

    loc = sourceManager.getImmediateMacroCallerLoc(loc);
  }

  return std::string();
}

SourceUtil::Region
SourceUtil::getRegionForRange(clang::CharSourceRange range,
                              const clang::SourceManager &sourceManager) {
  clang::SourceLocation begin = range.getBegin();
  clang::SourceLocation end = range.getEnd();

  // Create and return a record.

  Region newRegion = {};

  // Get filename without path

  std::string filename =
      sourceManager.getFilename(sourceManager.getSpellingLoc(begin)).str();
  std::string::size_type sep = filename.rfind('/');
  if (std::string::npos != sep) {
    filename.erase(0, sep + 1);
  }

  newRegion.d_filename = filename;
  newRegion.d_line = sourceManager.getSpellingLineNumber(begin);
  newRegion.d_column = sourceManager.getSpellingColumnNumber(begin);
  newRegion.d_length = sourceManager.getCharacterData(end) -
                       sourceManager.getCharacterData(begin);
  return newRegion;
}

SourceUtil::Region
SourceUtil::getRegionForStatement(const clang::Stmt &statement,
                                  const clang::SourceManager &sourceManager) {
  clang::CharSourceRange sourceRange =
      getRangeForStatement(statement, sourceManager);
  return getRegionForRange(sourceRange, sourceManager);
}

bool SourceUtil::isPartialMacro(const clang::SourceRange &sourceRange,
                                const clang::SourceManager &sourceManager,
                                clang::Preprocessor &preprocessor) {
  // Trace through levels of macros that are expanded by the beginning of
  // the statement.

  clang::SourceLocation begin = sourceRange.getBegin();

  auto usesGccVarargExtensionAtLoc = [&sourceManager,
                                      &preprocessor](const auto &loc) {
    if (auto *macroInfo = getMacroInfo(loc, sourceManager, preprocessor)) {
      return macroInfo->isVariadic() && macroInfo->hasCommaPasting();
    }
    return false;
  };

  while (begin.isMacroID()) {
    // If a macro in the heierarchy uses the GCC '##' extension (see [1])
    // we can't easily trace up the context stack how the statement is formed
    // from component macros. Cop out, return true
    // [1]: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
    if (usesGccVarargExtensionAtLoc(begin)) {
      return true;
    }
    // Only process macros where the statement is in the body, not ones where
    // it is an argument.

    if (sourceManager.isMacroBodyExpansion(begin)) {
      // Check that there are only spaces or '(' between the beginning of the
      // macro and part corresponding to the beginning of the statement.

      llvm::ArrayRef<clang::Token> tokens =
          getMacroTokens(begin, sourceManager, preprocessor);
      if (!tokens.empty()) {
        clang::SourceLocation macroStart = tokens.front().getLocation();

        // FIXME
        // There is potentially a bug here, this is unable to deal with macros
        // that expand to more than one access expression
        clang::SourceLocation statementStart =
            sourceManager.getSpellingLoc(begin);

        if (!sourceRangeContainsOnly(macroStart, statementStart, " \t(",
                                     sourceManager)) {
          return true;
        }
      }
    }

    // Move up one level closer to the expansion point. This code is adapted
    // from 'clang::SourceManager::getImmediateMacroCallerLoc'.

    if (sourceManager.isMacroArgExpansion(begin)) {
      begin = sourceManager.getImmediateSpellingLoc(begin);
    } else {
      begin = sourceManager.getImmediateExpansionRange(begin).getBegin();
    }
  }

  // Trace through levels of macros that are expanded by the end of the
  // statement.

  clang::SourceLocation end = sourceRange.getEnd();
  const clang::MacroInfo *prevMacro = nullptr;

  while (end.isMacroID()) {
    // If a macro in the heierarchy uses the GCC '##' extension (see [1])
    // we can't easily trace up the context stack how the statement is formed
    // from component macros. Cop out, return true
    // [1]: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
    if (usesGccVarargExtensionAtLoc(end)) {
      return true;
    }
    // Only process macros where the statement is in the body, not ones where
    // it is an argument.

    if (sourceManager.isMacroBodyExpansion(end)) {
      // Check that there are only spaces or '(' between the beginning of the
      // macro and part corresponding to the beginning of the statement.

      llvm::ArrayRef<clang::Token> tokens =
          getMacroTokens(end, sourceManager, preprocessor);
      if (!tokens.empty()) {
        clang::SourceLocation macroEnd = tokens.back().getEndLoc();
        clang::SourceLocation statementEnd = sourceManager.getSpellingLoc(end);

        statementEnd = clang::Lexer::getLocForEndOfToken(
            statementEnd, 0, sourceManager, clang::LangOptions());

        if (!sourceRangeContainsOnly(statementEnd, macroEnd, " \t)",
                                     sourceManager)) {
          return true;
        }
      }
    }

    // Move up one level closer to the expansion point. This code is adapted
    // from 'clang::SourceManager::getImmediateMacroCallerLoc'.

    if (sourceManager.isMacroArgExpansion(end)) {
      end = sourceManager.getImmediateSpellingLoc(end);
    } else {
      end = sourceManager.getImmediateExpansionRange(end).getEnd();
    }
  }

  return false;
}

bool operator==(const SourceUtil::Region &lhs, const SourceUtil::Region &rhs) {
  return lhs.d_line == rhs.d_line && lhs.d_column == rhs.d_column &&
         lhs.d_length == rhs.d_length && lhs.d_filename == rhs.d_filename;
}

bool operator!=(const SourceUtil::Region &lhs, const SourceUtil::Region &rhs) {
  return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &stream,
                         const SourceUtil::Region &region) {
  stream << "{ " << region.d_filename << ", " << region.d_line << ", "
         << region.d_column << ", " << region.d_length << " }";
  return stream;
}

} // namespace clangmetatool

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
