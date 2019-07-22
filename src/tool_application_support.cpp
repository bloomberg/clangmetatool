#include <clangmetatool/tool_application_support.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/ToolChain.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Option/Option.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>

#include <llvm/Config/llvm-config.h>
#if LLVM_VERSION_MAJOR >= 8
#include <llvm/Support/VirtualFileSystem.h>
#else
#include <clang/Basic/VirtualFileSystem.h>
#endif

#include <cstring>
#include <memory>
#include <sstream>

namespace clangmetatool {

void ToolApplicationSupport::verifyInstallation(
    const clang::tooling::CompilationDatabase &compilations,
    const std::vector<std::string> &sourcePathList) {
  clang::DiagnosticsEngine diagnostics(new clang::DiagnosticIDs,
                                       new clang::DiagnosticOptions,
                                       new clang::IgnoringDiagConsumer);

  // Check each compile command, each may have different settings for toolchain
  // and resource directory.

  for (const auto &source : sourcePathList) {
    for (const auto &command : compilations.getCompileCommands(source)) {
      std::vector<std::string> args = command.CommandLine;

      // Create a driver and toolchain to verify

      clang::driver::Driver driver(args[0], llvm::sys::getDefaultTargetTriple(),
                                   diagnostics);

      std::vector<const char *> argsArray;
      for (const auto &arg : args) {
        argsArray.push_back(arg.c_str());
      }

      std::unique_ptr<clang::driver::Compilation> compilation(
          driver.BuildCompilation(llvm::makeArrayRef(argsArray)));

      const clang::driver::ToolChain &toolChain =
          compilation->getDefaultToolChain();

      // Check that we can find C++ includes using the toolchain

      llvm::opt::InputArgList inputArgs;
      llvm::opt::ArgStringList cxxIncludes;
      toolChain.AddClangCXXStdlibIncludeArgs(inputArgs, cxxIncludes);

      bool found = false;
      for (const auto &include : cxxIncludes) {
        // Skip options

        if (include[0] == '-') {
          continue;
        }

        // Check if a sample C++ header can be found at this path

        llvm::SmallVector<char, 128> path(include,
                                          include + std::strlen(include));
        llvm::sys::path::append(path, "iostream");
        if (llvm::sys::fs::exists(path)) {
          found = true;
          break;
        }
      }

      if (!found) {
        std::ostringstream os;
        os << "clang could not find a C++ toolchain to use, check that the "
           << "compiler used to configure clang is installed";
        llvm::report_fatal_error(os.str().c_str(), false);
      }

      // Check that the configured resource directory exists

      if (!llvm::sys::fs::exists(driver.ResourceDir)) {
        std::ostringstream os;
        os << "clang resource files are missing from " << driver.ResourceDir
           << ", check that this application is installed properly";
        llvm::report_fatal_error(os.str().c_str(), false);
      }
    }
  }
}

} // namespace clangmetatool

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
