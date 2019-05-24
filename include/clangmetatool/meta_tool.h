#ifndef INCLUDED_CLANGMETATOOL_META_TOOL_H
#define INCLUDED_CLANGMETATOOL_META_TOOL_H

#include <assert.h>
#include <map>
#include <memory>
#include <stddef.h>
#include <string>

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <llvm/ADT/StringRef.h>

namespace clangmetatool {
namespace {

// Code to check if there exists a member T::ArgTypes that was created as a
// typedef
template <typename... Ts> using void_t = void;

template <typename T, typename = void>
struct has_typedef_ArgTypes : std::false_type {};

template <typename T>
struct has_typedef_ArgTypes<T, void_t<typename T::ArgTypes>> : std::true_type {
};
}
/**
 * MetaTool is a template that reduces the amount of boilerplate
 * required to write a clang tool. The WrappedTool is a class that
 * is expected to have:
 *
 *   * A constructor:
 *     - That takes as parameters:
 *       - A pointer to the compiler instance object.
 *       - A pointer to the MatchFinder object
 *     - Registers PPCallbacks on the preprocessor and/or
 *       add matchers to the match finder.
 *
 *   * A postProcessing method
 *     - That takes as parameter:
 *       - The replacementsMap for this run
 *
 */
template <class WrappedTool> class MetaTool : public clang::ASTFrontendAction {
private:
  static constexpr bool providesArgTypes =
      has_typedef_ArgTypes<WrappedTool>::value;
  struct NoArgs {
    typedef void *ArgTypes;
  };

public:
  typedef typename std::conditional_t<providesArgTypes, WrappedTool,
                                      NoArgs>::ArgTypes ArgTypes;

private:
  std::map<std::string, clang::tooling::Replacements> &replacementsMap;
  clang::ast_matchers::MatchFinder f;
  WrappedTool *tool;

  ArgTypes &args;

  template <class A>
  WrappedTool *create_tool(clang::CompilerInstance &ci, A args) {
    return new WrappedTool(&ci, &f, args);
  }
  WrappedTool *create_tool(clang::CompilerInstance &ci,
                           typename NoArgs::ArgTypes &args) {
    return new WrappedTool(&ci, &f);
  }

public:
  MetaTool(std::map<std::string, clang::tooling::Replacements> &replacementsMap,
           ArgTypes &args)
      : replacementsMap(replacementsMap), tool(NULL), args(args) {}

  MetaTool(std::map<std::string, clang::tooling::Replacements> &replacementsMap)
      : replacementsMap(replacementsMap), tool(NULL) {}

  ~MetaTool() {
    if (tool)
      delete tool;
  }

  virtual bool BeginSourceFileAction(clang::CompilerInstance &ci) override {
    // we don't expect to ever have the metatool be invoked more
    // than once, it would eventually result in us holding
    // references to unused compiler instance objects, and
    // eventually segfaulting, so assert here.
    assert(tool == NULL);
    tool = create_tool(ci, args);
    return true;
  }

  virtual void ExecuteAction() override {
    ASTFrontendAction::ExecuteAction();
    tool->postProcessing(replacementsMap);
  }

  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    llvm::StringRef file) override {
    return f.newASTConsumer();
  }
};
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
