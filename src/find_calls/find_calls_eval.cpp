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

    private:
      clang::CompilerInstance* ci;
      clangmetatool::collectors::VariableRefs vr;

      std::set<const clang::DeclRefExpr*> var_used_in_call;

    public:
      std::pair<bool, clang::APValue>
      try_to_evaluate(const clang::Expr* expr,
                      const clang::DeclRefExpr* declrefexpr) {
        std::pair<bool, clang::APValue> ret(false, clang::APValue(0));

      // if we know the expr is a declrefexpr, let's just use that.
      if (declrefexpr != NULL)
        expr = declrefexpr;

      if (expr == NULL)
        return ret;

      clang::Expr::EvalResult eval_r;
      bool eval = expr->EvaluateAsRValue(eval_r, ci->getASTContext());

      if (eval) {
        ret.first = true;
        ret.second = eval_r.Val;
        return ret;

      } else if (declrefexpr != NULL) {
        clangmetatool::collectors::VariableRefsData *data = vr.getData();

        // shoot, the input isn't a const. But not all is lost. We can
        // still handle cases where the variable has a deterministic
        // value.
        const clang::ValueDecl* dre_v = declrefexpr->getDecl();
        const clang::VarDecl*   var =
          llvm::dyn_cast_or_null<clang::VarDecl>(dre_v);
        if (var != NULL) {
          const clang::Expr* init = var->getAnyInitializer();

          bool has_init = false;
          bool has_assign = false;

          if (init != NULL) {
            has_init = true;
            eval = init->EvaluateAsRValue(eval_r, ci->getASTContext());
            if (!eval)
              return ret;
            ret.second = eval_r.Val;
          }

          // now we need to look at all the declrefexpr for this
          // variable and see if there are clear assignments, if
          // they're used as rvalues or if they're arguments to
          // figlbit calls. If everything goes well, we'd be able to
          // find a deterministic value.

          auto vit = data->refs.lower_bound(var);
          while (vit != data->refs.end() &&
                 vit->first == var) {
            const clang::DeclRefExpr* usage = vit->second;
            auto aeit = data->clear_assignment_refs.find(usage);
            if (data->rvalue_refs.find(usage) != data->rvalue_refs.end()) {
              // this is a rvalue usage, not interesting...
            } else if (aeit != data->clear_assignment_refs.end()) {
              // this is a clear assignment.
              if (has_assign) {
                // second assignment, not deterministic.
                return ret;
              }
              if (has_init) {
                // initializer then assignment, not deterministic.
                return ret;
              }
              eval = aeit->second->EvaluateAsRValue(eval_r, ci->getASTContext());
              if (!eval) {
                // expr is not constant, not determistic
                return ret;
              }
              ret.second = eval_r.Val;
              has_assign = true;
            } else if (var_used_in_call.find(usage) !=
                       var_used_in_call.end()) {
              // this is used in fglblchk call, so we know it's a
              // "rvalue" even if would be a pointer.
            } else {
              // a usage that we can't identify, so we can't consider
              // the variable as having a deterministic value.
              return ret;
            }
            vit++;
          }

          if (has_assign || has_init) {
            ret.first = true;
          }
        }
      }
      return ret;
    }
    };
    }
  }
}
