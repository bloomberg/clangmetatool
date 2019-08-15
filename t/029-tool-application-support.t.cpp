#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/tool_application_support.h>
#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>

#include <string>
#include <queue>

llvm::cl::OptionCategory& optionCategory() {
  static llvm::cl::OptionCategory s_optionCategory("test");
  return s_optionCategory;
}

class LLVMFatalError : public std::runtime_error {
  public:
    LLVMFatalError(const std::string& reason)
      : std::runtime_error(reason)
    {
    }
};

struct ToolApplicationSupportTest : public ::testing::Test {
  public:
    static void fatalErrorHandler(void *,
                                  const std::string& reason,
                                  bool generateCrashDiagnostic)
    {
      throw LLVMFatalError(reason);
    }

    ToolApplicationSupportTest()
    {
      llvm::install_fatal_error_handler(&fatalErrorHandler, 0);
    }

    ~ToolApplicationSupportTest()
    {
      llvm::remove_fatal_error_handler();
    }

    std::string verifyInstallation(const std::vector<std::string>& arguments)
    {
      try {
        std::vector<const char*> argumentPtrs;
        for (const auto& item : arguments) {
          argumentPtrs.push_back(item.c_str());
        }

        int argc = argumentPtrs.size();
        const char** argv = argumentPtrs.data();
        clang::tooling::CommonOptionsParser parser(argc, argv,
                                                   optionCategory());

        clangmetatool::ToolApplicationSupport::verifyInstallation(
          parser.getCompilations(),
          parser.getSourcePathList());
      }
      catch(const LLVMFatalError& e) {
        return e.what();
      }

      return std::string();
    }
};

TEST_F(ToolApplicationSupportTest, resource_dir)
{
  EXPECT_EQ("", verifyInstallation({"tool", "test.cpp", "--",
                                    "-resource-dir", CLANG_STD_INCLUDE_DIR}));

  EXPECT_EQ("clang resource files are missing from /non-existent/123, "
            "check that this application is installed properly",
            verifyInstallation({"tool", "test.cpp", "--",
                                "-resource-dir", "/non-existent/123"}));
}

TEST_F(ToolApplicationSupportTest, toolchain)
{
  EXPECT_EQ("clang could not find a C++ toolchain to use, check that the "
            "compiler used to configure clang is installed",
            verifyInstallation({"tool", "test.cpp", "--",
                                "-resource-dir", CLANG_STD_INCLUDE_DIR,
                                "-gcc-toolchain", "/non-existent/123"}));

  // Check using a c file

  EXPECT_EQ("", verifyInstallation({"tool", "test.c", "--",
                                    "-resource-dir", CLANG_STD_INCLUDE_DIR}));

  EXPECT_EQ("clang could not find a C++ toolchain to use, check that the "
            "compiler used to configure clang is installed",
            verifyInstallation({"tool", "test.c", "--",
                                "-resource-dir", CLANG_STD_INCLUDE_DIR,
                                "-gcc-toolchain", "/non-existent/123"}));
}

TEST_F(ToolApplicationSupportTest, compilation_database)
{
  std::string buildDir =
    CMAKE_SOURCE_DIR "/t/data/029-tool-application-support/build";

  EXPECT_EQ("", verifyInstallation({"tool", "/src/test1.cpp",
                                    "-p", buildDir}));

  EXPECT_EQ("clang resource files are missing from /non-existent/123, "
            "check that this application is installed properly",
            verifyInstallation({"tool", "/src/test2.cpp",
                                "-p", buildDir}));

  EXPECT_EQ("clang resource files are missing from /non-existent/123, "
            "check that this application is installed properly",
            verifyInstallation({"tool", "/src/test1.cpp",
                                "/src/testa2.cpp",
                                "-p", buildDir}));
}

class MyTool {
private:
  clang::CompilerInstance* ci;
  clang::ast_matchers::MatchFinder *f;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), f(f) {}
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    auto &diagnosticsEngine = ci->getDiagnostics();
    const auto id =
        diagnosticsEngine.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                          "oh noz, a thing happened!");
    diagnosticsEngine.Report(id);
  }
};

TEST(suppress_warnings, test)
{
  std::string source =
    CMAKE_SOURCE_DIR "/t/data/029-tool-application-support/warnings.cpp";

  const char* argv[] = {
    "foo",
    source.c_str(),
    "--",
    "-xc++"
  };
  int argc = sizeof argv / sizeof *argv;

  clang::tooling::CommonOptionsParser optionsParser
    ( argc, argv, optionCategory() );

  // We can't rely on 'optionsParser.getSourcePathList' because it is not
  // reset between invocations, so has all of the bogus paths from the other
  // tests. Instead we pass only the source relevant to this test.

  clang::tooling::RefactoringTool tool
    ( optionsParser.getCompilations(), { source });

  clangmetatool::ToolApplicationSupport::suppressWarnings(tool);

  clang::TextDiagnosticBuffer tdb;
  tool.setDiagnosticConsumer(&tdb);

  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements());

  int r = tool.run(&raf);

  // Ensure we have the one warning output by the tool, but not the macro
  // warning from clang.

  ASSERT_EQ(1, tdb.getNumWarnings());
  EXPECT_EQ("oh noz, a thing happened!", tdb.warn_begin()->second);

  // Ensure we have no errors

  EXPECT_EQ(0, tdb.getNumErrors());
}
