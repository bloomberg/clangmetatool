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

#if __APPLE__
#warning Some tests will be disabled on the macOS platform
#define ARCH_DEPENDENT(test_name) DISABLED_##test_name
#else
#define ARCH_DEPENDENT(test_name) test_name
#endif

llvm::cl::OptionCategory& optionCategory() {
  static llvm::cl::OptionCategory s_optionCategory("test");
  return s_optionCategory;
}

class ToolApplicationSupportTest : public ::testing::Test {
protected:
  const std::string buildDir =
    CMAKE_SOURCE_DIR "/t/data/029-tool-application-support/build";

  bool verifyInstallation(const std::vector<std::string>& arguments)
  {
    std::vector<const char*> argumentPtrs;
    for (const auto& item : arguments) {
      argumentPtrs.push_back(item.c_str());
    }

    int argc = argumentPtrs.size();
    const char** argv = argumentPtrs.data();
    clang::tooling::CommonOptionsParser parser(argc, argv,
                                               optionCategory());

    // ToolApplicationSupport::verifyInstallation crashes with exit(1)
    // if the required headers aren't found
    clangmetatool::ToolApplicationSupport::verifyInstallation(
      parser.getCompilations(),
      parser.getSourcePathList(),
      // The parameter doesn't HAVE to be main's address. Any function
      // in the same binary will also suffice.
      argumentPtrs[0]);

    return true;
  }
};


TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(ResourceDirUnspecified))
{
  EXPECT_TRUE(verifyInstallation({"tool", "test.cpp", "--", "-x", "c++"}));
}

TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(ResourceDirFound))
{
  EXPECT_TRUE(verifyInstallation({"tool", "test.cpp", "--",
                                 "-resource-dir", CLANG_RESOURCE_DIR}))
    << "verification failed with CLANG_RESOURCE_DIR=" CLANG_RESOURCE_DIR
    << "\n";
}

TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(ResourceDirNotFound))
{
  const char *messageRegex = ".*"
                             "clang resource files are missing from "
                             "/non-existent/123, check that this application "
                             "is installed properly";
  ASSERT_DEATH(
    verifyInstallation({"tool", "test.cpp", "--",
                        "-resource-dir", "/non-existent/123"}),
    messageRegex);
}

TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(GCCToolChain))
{
  const char *messageRegex = ".*"
                             "clang could not find a C\\+\\+ toolchain to use, "
                             "check that the compiler used to configure "
                             "clang is installed";
  ASSERT_DEATH(
      verifyInstallation({"tool", "test.cpp", "--",
                          "-resource-dir", CLANG_RESOURCE_DIR,
                          "-gcc-toolchain", "/non-existent/123"}),
      messageRegex);
}

TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(CFilesWithResourceDir))
{
  EXPECT_TRUE(verifyInstallation({"tool", "test.c", "--",
                                  "-resource-dir", CLANG_RESOURCE_DIR}));
}

TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(CFilesWithGCCToolChain))
{
  const char *messageRegex = ".*"
                             "clang could not find a C\\+\\+ toolchain to use, "
                             "check that the compiler used to configure "
                             "clang is installed";
  ASSERT_DEATH(
      verifyInstallation({"tool", "test.c", "--",
                          "-resource-dir", CLANG_RESOURCE_DIR,
                          "-gcc-toolchain", "/non-existent/123"}),
      messageRegex);
}

TEST_F(ToolApplicationSupportTest, ARCH_DEPENDENT(ValidCompDBEntry) )
{
  EXPECT_TRUE(verifyInstallation({"tool", "/src/test1.cpp",
                                  "-p", buildDir}));
}

TEST_F(ToolApplicationSupportTest,
       ARCH_DEPENDENT(CompDBEntryWithInvalidResourceDir))
{

  const char *messageRegex = ".*"
                             "clang resource files are missing from "
                             "/non-existent/123, check that this application "
                             "is installed properly";
  ASSERT_DEATH(
    verifyInstallation({"tool", "/src/test2.cpp", "-p", buildDir}),
    messageRegex);
}

TEST_F(ToolApplicationSupportTest,
       ARCH_DEPENDENT(MixingValidAndInvalidCompdbEntries))
{
  const char *messageRegex = ".*"
                             "clang resource files are missing from "
                             "/non-existent/123, check that this application "
                             "is installed properly";
  ASSERT_DEATH(
    verifyInstallation({"tool", "/src/test1.cpp", "/src/test2.cpp",
                        "-p", buildDir}),
    messageRegex);
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
