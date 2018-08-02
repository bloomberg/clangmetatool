#ifndef INCLUDED_CLANGMETATOOL_META_TOOL_FACTORY_H
#define INCLUDED_CLANGMETATOOL_META_TOOL_FACTORY_H

#include <string>
#include <map>
#include <tuple>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Tooling.h>

namespace clangmetatool {

  /**
   * MetaToolFactory wraps around FrontendAction class that takes a
   * replacementsMap as argument to the construtor. You can use it in
   * conjunction with the MetaTool class to reduce boilerplate in the
   * code required to write a clang tool.
   */
  template <class T, class... Args>
  class MetaToolFactory
    : public clang::tooling::FrontendActionFactory {
  private:

    /**
     * List of replacements to be used in the run.
     */
    std::map<std::string, clang::tooling::Replacements> &replacements;
    std::tuple<Args...> additionalArgs;
  public:

    /**
     * Metatool factory takes a reference to the replacements map that
     * will be used for this run.
     */
    MetaToolFactory
    (std::map<std::string, clang::tooling::Replacements> &replacements,
     Args... args)
      : replacements(replacements),
        additionalArgs(std::tuple<Args...>(args...))
    {}
    
    /**
     * This will create the object of your tool giving the
     * replacemnets map as an argument.
     */
    virtual clang::FrontendAction* create() {
      return new T(replacements, additionalArgs);
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
