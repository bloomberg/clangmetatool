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

/**
 * Record for a single macro expansion step, including the location of that
 * expansion and the macro information at that point, if any.
 */
struct ExpansionFrame {
  const clang::MacroInfo *macroInfo;
  clang::SourceLocation location;
};

/**
 * Return a sequence of macro expansion records for the given location at the
 * front of a range, from least expanded (the macro definition) to most
 * expanded (the macro use). Stop if any partial macro expansions are
 * encountered.
 */
std::vector<ExpansionFrame>
expandBeginLocation(clang::SourceLocation begin,
                    const clang::SourceManager &sourceManager,
                    clang::Preprocessor &preprocessor) {
  std::vector<ExpansionFrame> stack;

  while (begin.isMacroID()) {
    // Get the macro information at this location

    auto macroInfo = getMacroInfo(begin, sourceManager, preprocessor);

    // If a macro in the hierarchy uses the GCC '##' extension (see [1])
    // we can't easily trace up the context stack how the statement is formed
    // from component macros. Cop out and return.
    // [1]: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html

    if (macroInfo->isVariadic() && macroInfo->hasCommaPasting()) {
      return stack;
    }

    // Add the current location to the stack

    stack.push_back(ExpansionFrame{macroInfo, begin});

    if (sourceManager.isMacroBodyExpansion(begin)) {
      // Handle the case where the location is in a macro body

      // Check that there are only spaces or '(' between the beginning of the
      // macro and part corresponding to the beginning of the statement.

      llvm::ArrayRef<clang::Token> tokens = macroInfo->tokens();
      if (!tokens.empty()) {
        clang::SourceLocation macroStart = tokens.front().getLocation();

        // FIXME
        // There is potentially a bug here, this is unable to deal with macros
        // that expand to more than one access expression
        clang::SourceLocation statementStart =
            sourceManager.getSpellingLoc(begin);

        if (!sourceRangeContainsOnly(macroStart, statementStart, " \t(",
                                     sourceManager)) {
          return stack;
        }
      }

      // Move up one level closer to the expansion point.

      begin = sourceManager.getImmediateExpansionRange(begin).getBegin();
    } else {
      // Handle the case where the location is in an argument to a function-like
      // macro.

      // Start resolving the macro argument instead of the macro itself.

      begin = sourceManager.getImmediateSpellingLoc(begin);
    }
  }

  // Insert the fully expanded location into the stack, there is no macro
  // information at this point.

  stack.push_back(ExpansionFrame{0, begin});
  return stack;
}

/**
 * Return a sequence of macro expansion records for the given location at the
 * end of a range, from least expanded (the macro definition) to most
 * expanded (the macro use). Stop if any partial macro expansions are
 * encountered.
 */
std::vector<ExpansionFrame>
expandEndLocation(clang::SourceLocation end,
                  const clang::SourceManager &sourceManager,
                  clang::Preprocessor &preprocessor) {
  std::vector<ExpansionFrame> stack;

  while (end.isMacroID()) {
    // Get the macro information at this location

    auto macroInfo = getMacroInfo(end, sourceManager, preprocessor);

    // If a macro in the hierarchy uses the GCC '##' extension (see [1])
    // we can't easily trace up the context stack how the statement is formed
    // from component macros. Cop out and return.
    // [1]: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html

    if (macroInfo->isVariadic() && macroInfo->hasCommaPasting()) {
      return stack;
    }

    // Add the current location to the stack

    stack.push_back(ExpansionFrame{macroInfo, end});

    if (sourceManager.isMacroBodyExpansion(end)) {
      // Handle the case where the location is in a macro body

      // Check that there are only spaces or ')' between the end of the
      // macro and part corresponding to the end of the statement.

      llvm::ArrayRef<clang::Token> tokens = macroInfo->tokens();
      if (!tokens.empty()) {
        clang::SourceLocation macroEnd = tokens.back().getEndLoc();
        clang::SourceLocation statementEnd = sourceManager.getSpellingLoc(end);

        statementEnd = clang::Lexer::getLocForEndOfToken(
            statementEnd, 0, sourceManager, clang::LangOptions());

        if (!sourceRangeContainsOnly(statementEnd, macroEnd, " \t)",
                                     sourceManager)) {
          return stack;
        }
      }

      // Move up one level closer to the expansion point.

      end = sourceManager.getImmediateExpansionRange(end).getEnd();
    } else {
      // Handle the case where the location is in an argument to a function-like
      // macro.

      // Start resolving the macro argument instead of the macro itself.

      end = sourceManager.getImmediateSpellingLoc(end);
    }
  }

  // Insert the fully expanded location into the stack, there is no macro
  // information at this point.

  stack.push_back(ExpansionFrame{0, end});
  return stack;
}

} // namespace

clang::CharSourceRange
SourceUtil::expandRange(const clang::SourceRange &range,
                        const clang::SourceManager &sourceManager) {

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
SourceUtil::expandRangeIfValid(const clang::SourceRange &range,
                               const clang::SourceManager &sourceManager,
                               clang::Preprocessor &preprocessor) {
  clang::SourceLocation begin = range.getBegin();
  clang::SourceLocation end = range.getEnd();

  // Get the full set of expansion records for the front and end of the range.
  // If there are none in either case then there is something in the code that
  // we cannot handle.

  auto beginStack = expandBeginLocation(begin, sourceManager, preprocessor);
  if (beginStack.empty()) {
    return {};
  }

  auto endStack = expandEndLocation(end, sourceManager, preprocessor);
  if (endStack.empty()) {
    return {};
  }

  // Search through each record in the expansion of the front of the range,
  // starting from the last, or most expanded, record.

  for (auto beginFrameIt = beginStack.rbegin();
       beginFrameIt != beginStack.rend(); ++beginFrameIt) {
    // For each, search for a corresponding expansion record for the end of the
    // range, matching by macro information.

    auto endFrameIt =
        std::find_if(endStack.rbegin(), endStack.rend(),
                     [&beginFrameIt, &sourceManager](const auto &frame) {
                       return frame.macroInfo == beginFrameIt->macroInfo;
                     });

    // If no match is found, keep iterating

    if (endFrameIt == endStack.rend()) {
      continue;
    }

    // Traverse the rest of the stack and ensure that any macro argument
    // expansions match up.

    for (auto beginTempIt = beginFrameIt, endTempIt = endFrameIt;
         beginTempIt != beginStack.rend() && endTempIt != endStack.rend();
         ++beginTempIt, ++endTempIt) {
      if (sourceManager.isMacroArgExpansion(beginTempIt->location) !=
          sourceManager.isMacroArgExpansion(endTempIt->location)) {
        return {};
      }
    }

    // Form and return a range with the front and back locations

    begin = sourceManager.getImmediateSpellingLoc(beginFrameIt->location);
    end = sourceManager.getImmediateSpellingLoc(endFrameIt->location);
    end = clang::Lexer::getLocForEndOfToken(end, 0, sourceManager,
                                            clang::LangOptions());
    return clang::CharSourceRange::getCharRange(begin, end);
  }

  // No matching front and end records were found, return an invalid range.

  return {};
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
  clang::SourceLocation end = sourceRange.getEnd();

  if (sourceManager.isMacroArgExpansion(begin) !=
      sourceManager.isMacroArgExpansion(end)) {
    // This catches macros which might receive other macros as arguments

    return true;
  }

  // Ensure that all macros are fully expanded. That is, the expansion function
  // should return a stack with a non-macro at the end.

  auto stack = expandBeginLocation(begin, sourceManager, preprocessor);
  if (stack.empty() || stack.back().macroInfo) {
    return true;
  }

  stack = expandEndLocation(end, sourceManager, preprocessor);
  if (stack.empty() || stack.back().macroInfo) {
    return true;
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
