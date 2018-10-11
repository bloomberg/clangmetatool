#include "strongly_connected_blocks.h"

#include <cassert>
#include <map>
#include <memory>
#include <vector>

namespace clangmetatool {
namespace propagation {
namespace {

/**
 * Data gathered for each block, used by the algorithm.
 */
struct BlockData {
  unsigned index;
  unsigned lowlink;
  bool onStack;

  BlockData(unsigned i, unsigned l, bool o)
      : index(i), lowlink(l), onStack(o) {}
};

/**
 * Utility class so we can more easily change block data.
 */
class BlockDataMap {
private:
  std::map<unsigned, BlockData *> dataMap;
  std::vector<std::unique_ptr<BlockData>> dataHolder;

public:
  // Add a new block to the map (assumes that block is on the stack)
  BlockData *addNewData(unsigned index, unsigned id) {
    dataHolder.emplace_back(std::make_unique<BlockData>(index, index, true));

    auto ptr = dataHolder.back().get();

    dataMap.insert({id, ptr});

    return ptr;
  }

  // Lookup the data for a block given its id. Return nullptr if not found
  BlockData *lookup(unsigned id) const {
    auto it = dataMap.find(id);

    if (dataMap.end() != it) {
      return it->second;
    }

    return nullptr;
  }
};

} // namespace anonymous

/**
 * Implement Tarjan's strongly connected components algorithm to detect strongly
 * connected blocks
 * https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connectecomponents_algorithm
 */
class StronglyConnectedBlocksImpl {
private:
  using Stack = std::vector<const clang::CFGBlock *>;

  // map from block id to component id
  std::map<unsigned, unsigned> componentMap;

  void strongConnect(unsigned &index, Stack &stack, BlockDataMap &blockData,
                     unsigned &component, const clang::CFGBlock *v) {
    auto vData = blockData.addNewData(index, v->getBlockID());
    index += 1;
    stack.push_back(v);

    for (auto w : v->succs()) {
      // In the CFG, if the successor is unreachable (or nonexistant)
      // the pointer will be null
      if (nullptr != w) {
        auto wData = blockData.lookup(w->getBlockID());

        if (nullptr == wData) {
          strongConnect(index, stack, blockData, component, w);
          // Now the data is determined so look it up
          wData = blockData.lookup(w->getBlockID());
          vData->lowlink = std::min(vData->lowlink, wData->lowlink);
        } else if (wData->onStack) {
          vData->lowlink = std::min(vData->lowlink, wData->index);
        }
      }
    }

    if (vData->lowlink == vData->index) {
      const auto comp = component++;
      assert(0 != comp);

      const clang::CFGBlock *w = nullptr;

      std::vector<unsigned> toAdd;

      while (w != v) {
        w = stack.back();
        stack.pop_back();

        blockData.lookup(w->getBlockID())->onStack = false;

        toAdd.push_back(w->getBlockID());
      }

      // We only care about loops in this context
      if (1 < toAdd.size()) {
        for (auto id : toAdd) {
          componentMap.insert({id, comp});
        }
      }
    }
  }

public:
  StronglyConnectedBlocksImpl(const clang::CFG *cfg) {
    unsigned index = 0;
    Stack stack;
    BlockDataMap blockData;
    unsigned component = 1;

    for (auto v : *cfg) {
      if (nullptr == blockData.lookup(v->getBlockID())) {
        strongConnect(index, stack, blockData, component, v);
      }
    }
  }

  // Determine which loop a block is in. Return false if it is not in any block
  unsigned getLoop(const clang::CFGBlock *block) const {
    auto it = componentMap.find(block->getBlockID());

    if (componentMap.end() != it) {
      return it->second;
    }

    return 0;
  }

  // Check to see if two blocks are in a loop (they are if they are in
  // the same strongly connected component)
  bool inALoop(const clang::CFGBlock *b1, const clang::CFGBlock *b2) const {
    auto it1 = componentMap.find(b1->getBlockID());
    auto it2 = componentMap.find(b2->getBlockID());

    return componentMap.end() != it1 && componentMap.end() != it2 &&
           it1->second == it2->second;
  }
};

StronglyConnectedBlocks::StronglyConnectedBlocks(const clang::CFG *cfg) {
  impl = new StronglyConnectedBlocksImpl(cfg);
}

StronglyConnectedBlocks::~StronglyConnectedBlocks() { delete impl; }

unsigned StronglyConnectedBlocks::getLoop(const clang::CFGBlock *block) const {
  return impl->getLoop(block);
}

bool StronglyConnectedBlocks::inALoop(const clang::CFGBlock *b1,
                                      const clang::CFGBlock *b2) const {
  return impl->inALoop(b1, b2);
}

} // namespace propagation
} // namespace clangmetatool

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
