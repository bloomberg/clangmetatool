#ifndef INCLUDED_CLANGMETATOOL_PROPAGATION_BLOCK_VISITOR_MANAGER_H
#define INCLUDED_CLANGMETATOOL_PROPAGATION_BLOCK_VISITOR_MANAGER_H

#include "strongly_connected_blocks.h"
#include "types/changed_in_loop.h"

#include <iostream>
#include <map>
#include <queue>
#include <tuple>

#include <clang/Analysis/CFG.h>
#include <clang/AST/ASTContext.h>

namespace clangmetatool {
namespace propagation {

/**
 * Utility class to traverse the blocks in the CFG and generate the
 * context map through the usage of the passed in visitor object type
 */
template <typename V>
class BlockVisitorManager {
public:
  using VisitorType         = V;
  using ResultType          = typename VisitorType::ResultType;
  using ValueContextMapType = typename VisitorType::ValueContextMapType;
  using StateType           = typename VisitorType::StateType;

private:
  clang::ASTContext&              context;
  StronglyConnectedBlocks         allLoops;
  types::ChangedInLoop            changedInLoop;
  ValueContextMapType             valueMap;
  std::map<unsigned, VisitorType> blockVisitorMap;

  /**
   * Given state and a block, insert a new visitor into the blockVisitorMap
   */
  inline
  void insertVisitor(StateType&& state, const clang::CFGBlock* block) {
    blockVisitorMap.emplace(std::piecewise_construct,
                            std::forward_as_tuple(block->getBlockID()),
                            std::forward_as_tuple(context,
                                                  &valueMap,
                                                  std::move(state),
                                                  block));
  }

  /**
   * Update the state to be passed into the block's visitor, by marking
   * as UNRESOLVED any variables that have changed in any loops that are closed
   * upon entry to this block.
   */
  void handleClosedLoopsState(StateType& state, const clang::CFGBlock* block) {
    unsigned loop = allLoops.getLoop(block);

    std::set<std::string> changed;

    // Go through each of the block's predecessors
    for(auto pred : block->preds()) {
      // So long as the block is valid
      if(nullptr != pred) {
        auto predLoop = allLoops.getLoop(pred);

        // If we are not in the same loop as the predecessor
        if(loop != predLoop) {
          changed.insert(changedInLoop.changedBegin(predLoop),
                         changedInLoop.changedEnd(predLoop));
        }
      }
    }

    // Add all the changed variables to the state as UNRESOLVED
    for(const auto& var : changed) {
      const auto& it = state.find(var);

      if(state.end() == it) {
        state.emplace(var, ResultType());
      } else {
        it->second = {};
      }
    }
  }

  /**
   * Handle a block with multiple predecessors.
   */
  void handleMultiPredecessorBlock(const clang::CFGBlock* block) {
    StateType newState;

    for(auto pred : block->preds()) {
      // It the predecessor is valid and not in a loop with the current block
      if(nullptr != pred && !allLoops.inALoop(block, pred)) {
        const auto& visitor = blockVisitorMap.find(pred->getBlockID())->second;

        // For every final state in the predecessor
        for(const auto& it : visitor) {
          const auto& sit = newState.find(it.first);
          if(newState.end() == sit) {
            // If the state is not already in the starting state for this
            // block, add it
            newState.insert(it);
          } else if(sit->second != it.second) {
            // Otherwise, if the state differs, mark it as unresolved
            sit->second = {};
          }
        }
      }
    }

    // Handle any closed loops
    handleClosedLoopsState(newState, block);

    // Run the StringVisitor for this block given the starting state
    insertVisitor(std::move(newState), block);
  }

  /**
   * Handle a block with only one predecessor.
   */
  void handleSinglePredecessorBlock(const clang::CFGBlock* pred, const clang::CFGBlock* block) {
    if(nullptr == pred) {
      // Note that there are no closed loops in this case

      // If the predecessor is invalid, run the StringVisitor with no starting state
      insertVisitor({}, block);
    } else {
      // Otherwise use the predecessors final state as the starting state
      StateType predState(blockVisitorMap.find(pred->getBlockID())->second.getState());

      // Handle any closed loops
      handleClosedLoopsState(predState, block);

      insertVisitor(std::move(predState), block);
    }
  }

  using Queue = std::queue<const clang::CFGBlock*>;

  /**
   * Visit the next block in the queue to build up the initial valueMap
   */
  void visitNextBlock(std::set<unsigned>& visited, Queue& queue) {
    auto block = queue.front();
    queue.pop();

    if(visited.end() == visited.find(block->getBlockID())) {
      unsigned predCount = 0;
      const clang::CFGBlock* onlyPred = nullptr;
      for(auto pred : block->preds()) {
        if(nullptr == pred || allLoops.inALoop(block, pred)) {
          // We have found an unreachable path to this node in the context of
          // this analysis
          continue;
        } else if(visited.end() == visited.find(pred->getBlockID())) {
          // We still need to analyze this predecessor
          queue.push(block);

          return;
        } else {
          onlyPred = pred;

          // Count the predecessor as valid
          ++predCount;
        }
      }

      // We are now visiting this block
      visited.insert(block->getBlockID());

      if(1 < predCount) {
        handleMultiPredecessorBlock(block);
      } else {
        handleSinglePredecessorBlock(onlyPred, block);
      }
    }
  }

  BlockVisitorManager(const BlockVisitorManager&) = delete;
  BlockVisitorManager& operator=(const BlockVisitorManager&) = delete;

public:
  /**
   * Given an ASTContext and a CFG for a function, run constant propagation
   * using the passed in (by template) visitor.
   */
  BlockVisitorManager(clang::ASTContext& AC, const clang::CFG* cfg)
    : context(AC), allLoops(cfg) {
    std::set<unsigned> visited;
    Queue queue;

    // Run through the CFG once to figure out which variables change in any loops
    for(auto block : *cfg) {
      VisitorType loopVisitor(context, &changedInLoop, allLoops.getLoop(block), block);
    }

    // Add all the blocks in the CFG to the queue
    for(auto block : *cfg) {
      queue.push(block);
    }

    // Make a top-down traversal of the CFG (ignoring loops)
    while(0 < queue.size()) {
      visitNextBlock(visited, queue);
    }

    // Simplify the value map
    valueMap.squash();
  }

  /**
   * Given a usage location, lookup the value of a variable.
   * Return false if there is no known value.
   */
  bool lookup
  (ResultType& result, const std::string& variable, const clang::SourceLocation& location) const {
    return valueMap.lookup(result, variable, location);
  }

  /**
   * Dump the contexts for all the tracked variables to a stream.
   *
   * Note that this assumes that the stream operator has been set
   * up for the Visitor's ReturnType.
   */
  void dump(std::ostream& stream, const clang::SourceManager &SM) const {
    for(const auto& it : valueMap) {
      stream << "  ** " << it.first << std::endl;

      for(const auto& ctx : it.second) {
        auto posStr = std::get<0>(ctx).printToString(SM);

        stream << "    - " << posStr.substr(posStr.find(':', 0) + 1)
               << " '" << std::get<2>(ctx) << "' ("
               << std::get<1>(ctx) << ')' << std::endl;
      }
    }
  }
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
