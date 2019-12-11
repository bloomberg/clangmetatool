#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_PROPOGATION_VISITOR_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_PROPOGATION_VISITOR_H

#include "types/changed_in_loop.h"
#include "types/state.h"
#include "types/value_context_map.h"
#include "util/get_stmt_from_cfg_element.h"

#include <clang/AST/StmtVisitor.h>
#include <clang/Analysis/CFG.h>
#include <clangmetatool/propagation/propagation_result.h>

namespace clangmetatool {
namespace propagation {

/**
 * Generic statement visitor base class for propagation.
 * The template arguments are as follows:
 *    - S: The child class
 *    - T: The resulting type of the propagation
 */
template <typename S, typename T>
class PropagationVisitor : public clang::ConstStmtVisitor<S> {
public:
  using ResultType = PropagationResult<T>;
  using ValueContextMapType = types::ValueContextMap<ResultType>;
  using StateType = types::State<ResultType>;

private:
  const bool buildingLoopChanges;
  types::ChangedInLoop *changedInLoop;
  ValueContextMapType *map;
  StateType state;
  const unsigned loop;

  PropagationVisitor(const PropagationVisitor &) = delete;
  PropagationVisitor &operator=(const PropagationVisitor &) = delete;

protected:
  clang::ASTContext &context;

  /**
   * Add a new value to the map (this assumes the addition was in the context
   * of a new definition -- i.e. not a block flow merging)
   */
  void addToMap(const std::string &name, const ResultType &value,
                clang::SourceLocation start) {
    if (buildingLoopChanges) {
      if (0 != loop) {
        changedInLoop->save(loop, name);
      }
    } else {
      map->addToMap(name, value, start,
                    types::ValueContextOrdering::CHANGED_BY_CODE);

      auto it = state.find(name);

      if (state.end() == it) {
        // If the variable is not in the state, add it
        state.insert({name, value});
      } else {
        // Otherwise, update the variable's value
        it->second = value;
      }
    }
  }

public:
  /**
   * The constructor for the Propagation visitor for filling out the
   * ValueContextMap.
   *
   * Inheriting classes that implement their own constructors should try not to
   * implement their
   * own constructors and instead use the scheme found in CStringVisitor
   * (constant_cstring_propagator.cpp), i.e.
   *
   *     using PropagationVisitor<CStringVisitor,
   * std::string>::PropagationVisitor;
   *
   * which allows a child class to inherit its parent's constructors.
   */
  explicit PropagationVisitor(clang::ASTContext &AC, ValueContextMapType *map,
                              StateType &&state, const clang::CFGBlock *block)
      : buildingLoopChanges(false), map(map), state(state), loop(0),
        context(AC) {
    // Find the first statement in the block
    const clang::Stmt *startStmt = nullptr;
    for (auto elem : *block) {
      if (util::getStmtFromCFGElement(startStmt, elem)) {
        break;
      }
    }

    // If there are acutally statements in the block
    if (nullptr != startStmt) {
      for (const auto &it : state) {
        // Add all the variable values in the starting state to the top of
        // the block's context
        map->addToMap(it.first, it.second, startStmt->getBeginLoc(),
                      types::ValueContextOrdering::CONTROL_FLOW_MERGE);
      }

      // Visit all of the statments in the block to generate the valueMap
      for (auto elem : *block) {
        const clang::Stmt *stmt;

        if (util::getStmtFromCFGElement(stmt, elem)) {
          this->Visit(stmt);
        }
      }
    }
  }

  /**
   * The constructor for the Propagation visitor for filling out the
   * ChangedInLoop data structure.
   *
   * Inheriting classes that implement their own constructors should try not to
   * implement their
   * own constructors and instead use the scheme found in CStringVisitor
   * (constant_cstring_propagator.cpp), i.e.
   *
   *     using PropagationVisitor<CStringVisitor,
   * std::string>::PropagationVisitor;
   *
   * which allows a child class to inherit its parent's constructors.
   */
  explicit PropagationVisitor(clang::ASTContext &AC,
                              types::ChangedInLoop *changedInLoop,
                              unsigned loop, const clang::CFGBlock *block)
      : buildingLoopChanges(true), changedInLoop(changedInLoop), loop(loop),
        context(AC) {
    // Visit all of the statements in the block to generate changedInLoop
    for (auto elem : *block) {
      const clang::Stmt *stmt;

      if (util::getStmtFromCFGElement(stmt, elem)) {
        this->Visit(stmt);
      }
    }
  }

  auto begin() const { return state.begin(); }
  auto end() const { return state.end(); }

  const StateType &getState() const { return state; }
};

} // namespace propagation
} // namespace clangmetatool

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
