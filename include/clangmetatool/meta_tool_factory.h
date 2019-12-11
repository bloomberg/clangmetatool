#ifndef INCLUDED_CLANGMETATOOL_META_TOOL_FACTORY_H
#define INCLUDED_CLANGMETATOOL_META_TOOL_FACTORY_H

#include <map>
#include <string>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Tooling.h>

namespace clangmetatool {
/**
 * MetaToolFactory wraps around FrontendAction class that takes a
 * replacementsMap as argument to the construtor. You can use it in
 * conjunction with the MetaTool class to reduce boilerplate in the
 * code requjired to write a clang tool.
 */
template <class T>
class MetaToolFactory : public clang::tooling::FrontendActionFactory {
private:
  // T *must* provide this
  typename T::ArgTypes args;

  /**
   * List of replacements to be used in the run.
   */
  std::map<std::string, clang::tooling::Replacements> &replacements;

public:
  /**
   * Metatool factory takes a reference to the replacements map that
   * will be used for this run in, along with any additional arguments that
   * need to be passed on to the Tool
   */
  MetaToolFactory(
      std::map<std::string, clang::tooling::Replacements> &replacements,
      typename T::ArgTypes &args)
      : replacements(replacements), args(args) {}

  MetaToolFactory(
      std::map<std::string, clang::tooling::Replacements> &replacements)
      : replacements(replacements) {}

  /**
   * This will create the object of your tool giving the
   * replacemnets map as an argument.
   */
  virtual clang::FrontendAction *create() { return new T(replacements, args); }
};
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
