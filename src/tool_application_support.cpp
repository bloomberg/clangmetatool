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
#include <llvm/Support/Process.h>

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

namespace {

#ifndef CLANG_INSTALL_LOCATION
#define CLANG_INSTALL_LOCATION ""
#endif

std::string getExecutablePath(const std::string &argv0) {
  // Use the address of 'main' to locate the executable name, it is possible
  // that this may return an empty address
  std::string exePath =
      llvm::sys::fs::getMainExecutable(argv0.c_str(), nullptr);
  if (!exePath.empty()) {
    clang::DiagnosticsEngine diagnostics(new clang::DiagnosticIDs,
                                 new clang::DiagnosticOptions,
                                 new clang::IgnoringDiagConsumer);
    clang::driver::Driver driver(
        exePath, llvm::sys::getDefaultTargetTriple(),
        diagnostics);
    if (llvm::sys::fs::exists(driver.ResourceDir)) {
      return exePath;
    }
  }

  // Fall back to locating it on $PATH
  const std::string &exeFileName = llvm::sys::path::filename(argv0);
  llvm::Optional<std::string> maybeExePath =
      llvm::sys::Process::FindInEnvPath("PATH", exeFileName);

  // If we didn't find this executable on $PATH, fall back clang's
  // location as seen at configure-time
  return maybeExePath.getValueOr(CLANG_INSTALL_LOCATION);
}
} // namespace

void ToolApplicationSupport::verifyInstallation(
    const clang::tooling::CompilationDatabase &compilations,
    const std::vector<std::string> &sourcePathList,
    const std::string &invokedArgv0) {
  clang::DiagnosticsEngine diagnostics(new clang::DiagnosticIDs,
                                       new clang::DiagnosticOptions,
                                       new clang::IgnoringDiagConsumer);

  // Find the true path to the executable
  std::string exePath = getExecutablePath(invokedArgv0);

  // Check each compile command, each may have different settings for toolchain
  // and resource directory.

  for (const auto &source : sourcePathList) {
    for (const auto &command : compilations.getCompileCommands(source)) {
      std::vector<std::string> args = command.CommandLine;
      const std::string &arg0 = exePath.empty() ? args[0] : exePath;

      // Create a driver and toolchain to verify
      clang::driver::Driver driver(arg0, llvm::sys::getDefaultTargetTriple(),
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

void ToolApplicationSupport::suppressWarnings(clang::tooling::ClangTool &tool) {
  auto argAdjuster = [](const clang::tooling::CommandLineArguments &cliArgsIn,
                        llvm::StringRef unused) {
    clang::tooling::CommandLineArguments cliArgsOut = cliArgsIn;
    cliArgsOut.push_back("-Wno-everything");
    return cliArgsOut;
  };
  tool.appendArgumentsAdjuster(argAdjuster);
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
