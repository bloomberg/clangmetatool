#ifndef INCLUDED_CLANGMETATOOL_MATCH_FORWARDER_H
#define INCLUDED_CLANGMETATOOL_MATCH_FORWARDER_H

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <functional>
#include <list>

namespace clangmetatool {

/**
 * Forwards clang matches to a function object, allowing one to handle matches
 * without creating a new class for each.
 */
class MatchForwarder {
public:
  typedef clang::ast_matchers::MatchFinder::MatchResult ResultType;
  typedef std::function<void(const ResultType &result)> FunctionType;

private:
  class MatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
  private:
    FunctionType d_function;

  public:
    MatchCallback(const FunctionType &function) : d_function(function) {}

    virtual void run(const ResultType &result) override { d_function(result); }
  };

  clang::ast_matchers::MatchFinder *d_matchFinder_p;
  std::list<MatchCallback> d_callbacks;

public:
  /**
   * Create a forwarder that wraps the given match finder.
   */
  MatchForwarder(clang::ast_matchers::MatchFinder *matchFinder)
      : d_matchFinder_p(matchFinder) {}

  /**
   * Add the given matcher to the match finder. When a match is found, the
   * result will be forwarded to the given function.
   */
  template <typename MatchType>
  void addMatcher(const MatchType &match, const FunctionType &function) {
    d_callbacks.emplace_back(function);
    d_matchFinder_p->addMatcher(match, &d_callbacks.back());
  }
};

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
