#ifndef INCLUDED_CLANGMETATOOL_MAIN_SUPPORT_H
#define INCLUDED_CLANGMETATOOL_MAIN_SUPPORT_H

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <string>
#include <vector>

namespace clangmetatool {

/**
 * Functions and utilities to support clangmetatool-based applications.
 */
struct ToolApplicationSupport {
  /**
   * Using the given compilations and source list (as can be obtained from a
   * tooling::CommonOptionsParser), check that the toolchain needed to start
   * clang is installed, and that this clang-based application has been
   * installed along with the required resource files. If the installation is
   * broken, report a fatal error and exit.
   *
   * \param compilations Compilation Database to use for this run
   * \param sourcePathList List of files to process during this run
   * \param invokedArgv0 Path to the invoked clangmetatool based executable
   *                     This must be as it appears in 'argv[0]'
   * \param mainAddr The address of the 'main' function from the calling
   *                 executable
   */
  static void
  verifyInstallation(const clang::tooling::CompilationDatabase &compilations,
                     const std::vector<std::string> &sourcePathList,
                     const std::string &invokedArgv0, void *mainAddr = nullptr);
  /**
   * For the given tool, append an ArgumentsAdjuster that adds '-w',
   * This will supress all of clang's warnings, but still allow the tool to
   * output its own warnings.
   */
  static void suppressWarnings(clang::tooling::ClangTool &tool);
};

} // namespace clangmetatool

#endif

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
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
