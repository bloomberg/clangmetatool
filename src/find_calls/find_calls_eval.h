#ifndef INCLUDED_FIND_CALLS_EVAL_H
#define INCLUDED_FIND_CALLS_EVAL_H

#include <string>
#include <clangmetatool/collectors/find_calls_data.h>
#include <clangmetatool/collectors/find_calls.h>
#include <clangmetatool/collectors/variable_refs.h>

namespace clangmetatool {
  namespace collectors {
    namespace find_calls {

    using namespace clang::ast_matchers;

    class FindCallsEval {

      typedef std::tuple<const clang::Expr*, const clang::DeclRefExpr*> GenArg;
      typedef std::tuple<const clang::CallExpr*, GenArg> GenCall;

    private:
      clang::CompilerInstance* ci;
      clangmetatool::collectors::VariableRefs vr;

      std::set<const clang::DeclRefExpr*> var_used_in_call;
      std::set<GenCall> gen_calls;

    public:
      FindCallsEval(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
      :ci(ci), vr(ci, f) {
      }

      ~FindCallsEval() {
      }

      void insert_gen_call(GenCall call);

      void insert_used_var(const clang::DeclRefExpr* var);

      std::pair<bool, clang::APValue>
      try_to_evaluate(const clang::Expr* expr,
                      const clang::DeclRefExpr* declrefexpr);
    };
    }
  }
}
#endif
