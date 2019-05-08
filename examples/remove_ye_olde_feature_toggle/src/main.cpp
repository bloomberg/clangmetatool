#include <iosfwd>
#include <map>
#include <tuple>
#include <optional>
#include <sstream>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>


#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>

#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/collectors/find_calls.h>
#include <clangmetatool/collectors/find_calls_data.h>
#include <clangmetatool/collectors/include_graph.h>
#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/propagation/constant_integer_propagator.h>

namespace {

  std::optional<int64_t> ye_deterministic_val(int64_t feature_id) {
    if (feature_id % 3 == 0 && feature_id % 5 == 0) {
      return {};
    } else if (feature_id % 3 == 0) {
      return 1;
    } else if (feature_id % 5 == 0) {
      return 0;
    } else {
      return feature_id;
    }
  }

  std::string get_newcode_for_ye(int64_t feature_id, int64_t value) {
    std::stringstream newcode_ss;
    newcode_ss
      << value
      << " /* removed ye_olde_feature_toggle "
      << feature_id
      << " */";
    return newcode_ss.str();
  }

}

class MyTool {
  clang::CompilerInstance* ci;
  clangmetatool::collectors::IncludeGraph ig;
  clangmetatool::collectors::FindCalls fc;
public:
  MyTool(clang::CompilerInstance *_ci, clang::ast_matchers::MatchFinder *f)
    :ig(_ci, f),
     fc(_ci, f, "ye_olde_feature_toggle_is_enabled"),
     ci(_ci) {}
  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    clangmetatool::collectors::IncludeGraphData* igdata = ig.getData();
    clang::ASTContext& ctx = ci->getASTContext();
    clang::SourceManager& sm = ctx.getSourceManager();

    // what is the file id for ye_olde_feature_toggle.h
    bool found_header = false;
    clangmetatool::types::FileUID header_fuid = 0;
    for (auto item : igdata->fuid2name) {
      if (item.second == "ye_olde_feature_toggle.h") {
        found_header = true;
        header_fuid = item.first;
        break;
      }
    }

    // bail early if the header was not used
    if (!found_header)
      return;

    // accumulate all the calls, the argument, and its optional determinstic value
    typedef std::tuple<const clang::CallExpr*, int64_t, std::optional<int64_t> > ye_call;
    std::vector<ye_call> candidates;

    // iterate over all the calls to that function
    clangmetatool::collectors::FindCallsData* fcdata = fc.getData();
    for (auto call_ctx : fcdata->call_context) {
      const clang::CallExpr* call = call_ctx.second;
      if (!sm.isWrittenInMainFile(call->getBeginLoc()))
        continue;
      if (sm.isMacroBodyExpansion(call->getBeginLoc()))
        continue;
      const clang::Expr* arg = call->getArg(0);
      llvm::APSInt argval;
      if (arg->isIntegerConstantExpr(argval, ctx)) {
        int64_t feature_id = argval.getExtValue();
        candidates.push_back
          ( ye_call{call, feature_id, ye_deterministic_val(feature_id) } );
        continue;
      }

      // let's try the const propagation.

      // First we need to see if it is a implicitcastexpr ->
      // declrefexpr -- which is the AST for using a variable
      auto arg_as_ice = llvm::dyn_cast<clang::ImplicitCastExpr>(arg);
      if (arg_as_ice == NULL)
        continue;

      const clang::Expr* subexpr = arg_as_ice->getSubExpr();
      auto subexpr_as_dre = llvm::dyn_cast<clang::DeclRefExpr>(subexpr);
      if (subexpr_as_dre == NULL)
        continue;

      clangmetatool::propagation::ConstantIntegerPropagator p(ci);
      auto r = p.runPropagation(call_ctx.first, subexpr_as_dre);
      if (r.isUnresolved())
        continue;

      int64_t feature_id = r.getResult();
      candidates.push_back
        ( ye_call{call, feature_id, ye_deterministic_val(feature_id) } );
    }

    // slightly ugly bit because we have a single Replacments object per file
    auto replacements_emp = replacementsMap.emplace
      (sm.getFileEntryForID(sm.getMainFileID())->getName(), clang::tooling::Replacements());
    clang::tooling::Replacements& replacements = replacements_emp.first->second;

    // actually work on the refactorings
    std::set<const clang::DeclRefExpr*> removed;
    for (auto ye_candidate : candidates) {
      const clang::CallExpr* call = std::get<0>(ye_candidate);
      int64_t feature_id = std::get<1>(ye_candidate);
      std::optional<int64_t> det_val = std::get<2>(ye_candidate);
      if (!det_val.has_value())
        continue;

      std::string newcode = get_newcode_for_ye(feature_id, det_val.value());
      clang::tooling::Replacement replacement(sm, call, newcode);

      llvm::Error err = replacements.add(replacement);
      // keep track of the DeclRefExpr we actually removed
      if (!err)
        removed.insert(fcdata->call_ref[call]);
    }

    // let's remove the now-unused include statements
    // since we're only changing the main file, we're only going to look at
    // main_fuid -> header_fuid edges of the include graph
    clangmetatool::types::FileUID main_fuid =
      sm.getFileEntryForID(sm.getMainFileID())->getUID();
    clangmetatool::types::FileGraphEdge relevant{ main_fuid, header_fuid };
    bool now_unused = true;
    auto refs_range = igdata->decl_references.equal_range(relevant);
    for ( auto it = refs_range.first; it != refs_range.second; it++  ) {
      // double check if we removed all references
      if ( removed.find(it->second) == removed.end() ) {
        now_unused = false;
        break;
      }
    }

    if (now_unused) {
      auto incs_range = igdata->include_statements.equal_range(relevant);
      for ( auto it = incs_range.first; it != incs_range.second; it++ ) {
        clang::SourceRange inc_r = it->second;
        unsigned int beg_line = sm.getSpellingLineNumber(inc_r.getBegin());
        unsigned int end_line = sm.getSpellingLineNumber(inc_r.getEnd());
        clang::SourceLocation beg_loc =
          sm.translateLineCol(sm.getMainFileID(), beg_line, 1);
        clang::SourceLocation end_loc =
          sm.translateLineCol(sm.getMainFileID(), end_line+1, 1);
        clang::CharSourceRange repl_range =
          clang::CharSourceRange(clang::SourceRange(beg_loc, end_loc), false);

        clang::tooling::Replacement replacement(sm, repl_range, "");
        llvm::Error err = replacements.add(replacement);
      }
    }

  }
};

int main(int argc, const char *argv[]) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  llvm::cl::extrahelp CommonHelp(
      clang::tooling::CommonOptionsParser::HelpMessage);

  clang::tooling::CommonOptionsParser optionsParser(argc, argv, MyToolCategory);

  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<MyTool>> raf(
      tool.getReplacements());

  int r = tool.runAndSave(&raf);

  return r;
}
