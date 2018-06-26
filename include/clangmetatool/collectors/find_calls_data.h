#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CALLS_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CALLS_DATA_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/Expr.h>

namespace clangmetatool {
  namespace collectors {

  struct FindCallsData {
    std::map<const clang::DeclRefExpr*,
             const clang::CallExpr*> call_ref;
    std::map<const clang::FunctionDecl*,
             const clang::CallExpr*> call_context;
    std::map<const clang::VarDecl*,
             const clang::DeclStmt* > declstmt_for_vardecl;
    std::map<const clang::DeclStmt*,
             const clang::CompoundStmt* > compoundstmt_for_declstmt;
    std::map<const clang::DeclRefExpr*,
             const clang::BinaryOperator* > binaryoperator_for_declrefexpr;
    std::map<const clang::BinaryOperator*,
             const clang::CompoundStmt* > compoundstmt_for_binaryoperator;
    std::map<const clang::VarDecl*,
             const clang::Decl* > nested_decls;
  };


  }
}

#endif
