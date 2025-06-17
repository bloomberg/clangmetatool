#include <clangmetatool/include_graph_dependencies.h>

#include <queue>

namespace clangmetatool {

namespace {

// Returns a range of edges starts with given source file uid
inline std::pair<types::FileGraph::const_iterator,
                 types::FileGraph::const_iterator>
edgeRangeStartsWith(const collectors::IncludeGraphData *data,
                    const types::FileUID &sourceFUID) {
  // Exploit an implementation detail of the include graph being an ordered
  // set of pairs and how operator<(...) on pairs works.
  // The property in use is that operator<(...) on pairs sorts
  // sequences of pairs into buckets of the first (ascending),
  // followed by the second (ascending)
  constexpr auto MIN_FUID = std::numeric_limits<types::FileUID>::min();
  constexpr auto MAX_FUID = std::numeric_limits<types::FileUID>::max();

  return std::make_pair(
      data->include_graph.lower_bound({sourceFUID, MIN_FUID}),
      data->include_graph.upper_bound({sourceFUID, MAX_FUID}));
}

// Given the current state of traversal through 'knownEdges' and
// 'visitedNodes', verify if 'from' requires 'to'. A requires B if the edge
// {A, B} adds new declrations or references from B to A, or A
// 'sees new information' through this edge.
//
// This function depends on the current view of the graph to check an edge
// is important. There are more than one valid solutions to this problem on
// a DAG, affected by the order of traversal and the initial state provided.
bool isRequired(const collectors::IncludeGraphData *data,
                const types::FileUID &from, const types::FileUID &to,
                std::set<types::FileGraphEdge> &knownEdges,
                std::set<types::FileUID> &knownNodes) {
  std::queue<types::FileUID> filesToProcess;
  bool keepEdge = false;

  filesToProcess.push(to);

  while (!filesToProcess.empty()) {
    auto currentFUID = filesToProcess.front();
    filesToProcess.pop();

    types::FileGraphEdge currentEdge{from, currentFUID};
    auto refCountIt = data->usage_reference_count.find(currentEdge);
    if (refCountIt != data->usage_reference_count.end() &&
        refCountIt->second > 0) {
      // If this usage edge is new, it means that this traversal provided new
      // 'information', which would make it unsafe to remove in the context of
      // the current set of includes traversed
      bool inserted;
      std::tie(std::ignore, inserted) = knownEdges.insert(currentEdge);
      // If insertion fails for whatever reason, don't flip on whether this
      // include is kept or not
      keepEdge = keepEdge || inserted;
    }

    // Find the set of files included by the current file uid
    // and set those up for traversal if we haven't seen them already
    types::FileGraph::const_iterator rangeBegin, rangeEnd;
    std::tie(rangeBegin, rangeEnd) = edgeRangeStartsWith(data, currentFUID);

    for (auto edgeIt = rangeBegin; edgeIt != rangeEnd; ++edgeIt) {
      types::FileUID nextNode;
      std::tie(std::ignore, nextNode) = *edgeIt;
      if (knownNodes.find(nextNode) == knownNodes.end()) {
        filesToProcess.push(nextNode);
        knownNodes.insert(nextNode);
      }
    }
  }

  return keepEdge;
}
} // namespace

bool IncludeGraphDependencies::decrementUsageRefCount(
    collectors::IncludeGraphData *data,
    const clangmetatool::types::FileGraphEdge &edge) {
  auto it = data->usage_reference_count.find(edge);
  if (it != data->usage_reference_count.end() && it->second > 0) {
    it->second--;
    return true;
  }
  return false;
}

std::set<clangmetatool::types::FileUID>
IncludeGraphDependencies::collectAllIncludes(
    const clangmetatool::collectors::IncludeGraphData *data,
    const types::FileUID &fileUID) {
  types::FileGraph::const_iterator rangeBegin, rangeEnd;
  std::tie(rangeBegin, rangeEnd) = edgeRangeStartsWith(data, fileUID);

  std::set<clangmetatool::types::FileUID> visitedNodes;
  std::queue<clangmetatool::types::FileUID> toVisit;
  toVisit.push(fileUID);
  while (!toVisit.empty()) {
    auto currentFUID = toVisit.front();
    toVisit.pop();

    if (!visitedNodes.insert(currentFUID).second) {
      continue;
    }
    types::FileGraph::const_iterator rangeBegin, rangeEnd;
    std::tie(rangeBegin, rangeEnd) = edgeRangeStartsWith(data, currentFUID);
    for (auto it = rangeBegin; it != rangeEnd; ++it) {
      toVisit.push(it->second);
    }
  }
  return visitedNodes;
}

std::set<types::FileUID> IncludeGraphDependencies::liveDependencies(
    const collectors::IncludeGraphData *data,
    const clangmetatool::types::FileUID &fileUID) {
  std::set<types::FileUID> dependencies;
  std::set<types::FileGraphEdge> visitedEdges;
  std::set<types::FileUID> visitedNodes;

  types::FileGraph::const_iterator rangeBegin, rangeEnd;
  std::tie(rangeBegin, rangeEnd) = edgeRangeStartsWith(data, fileUID);

  for (auto it = rangeBegin; it != rangeEnd; ++it) {
    assert(it->first == fileUID);
    auto &dependency = it->second;
    if (isRequired(data, fileUID, dependency, visitedEdges, visitedNodes)) {
      dependencies.insert(dependency);
    }
  }

  return dependencies;
}

/*
 * Traverse the include graph for `forNode` start from `rootNode` to all
 * accessible node using BFS and update given DirectDependenciesMap.
 *
 * For any node that `usage_reference_count[{rootNode, toNode}] > 0`, add a
 * record
 * `{toNode: [rootNode]}` to depsMap, means that `forNode` needs to access
 * resource defined in `toNode` through `rootNode`
 *
 * Include graph should looks like:
 * forNode -> rootNode ... -> toNode
 */
void traverseFor(const types::FileUID &forNode, const types::FileUID &rootNode,
                 const clangmetatool::collectors::IncludeGraphData *data,
                 std::set<types::FileUID> &knownNodes,
                 IncludeGraphDependencies::DirectDependenciesMap &depsMap) {
  std::queue<types::FileUID> filesToProcess;

  filesToProcess.push(rootNode);

  while (!filesToProcess.empty()) {
    auto toNode = filesToProcess.front();
    filesToProcess.pop();

    types::FileGraphEdge currentEdge{forNode, toNode};
    auto refCountIt = data->usage_reference_count.find(currentEdge);
    // the include graph looks like
    // forNode -> rootNode -> ... -> toNode
    if (refCountIt != data->usage_reference_count.end() &&
        refCountIt->second > 0) {
      depsMap[toNode].emplace(rootNode);
    }

    // Find the set of files included by the current file uid
    // and set those up for traversal if we haven't seen them already
    types::FileGraph::const_iterator rangeBegin, rangeEnd;
    std::tie(rangeBegin, rangeEnd) = edgeRangeStartsWith(data, toNode);

    for (auto edgeIt = rangeBegin; edgeIt != rangeEnd; ++edgeIt) {
      types::FileUID nextNode;
      std::tie(std::ignore, nextNode) = *edgeIt;
      if (knownNodes.find(nextNode) == knownNodes.end()) {
        filesToProcess.push(nextNode);
        knownNodes.insert(nextNode);
      }
    }
  }
}

IncludeGraphDependencies::DirectDependenciesMap
IncludeGraphDependencies::liveWeakDependencies(
    const clangmetatool::collectors::IncludeGraphData *data,
    const clangmetatool::types::FileUID &fileUID) {
  IncludeGraphDependencies::DirectDependenciesMap depsMap;

  types::FileGraph::const_iterator rangeBegin, rangeEnd;
  std::tie(rangeBegin, rangeEnd) = edgeRangeStartsWith(data, fileUID);

  for (auto it = rangeBegin; it != rangeEnd; ++it) {
    assert(it->first == fileUID);
    std::set<types::FileUID> knownNodes;
    traverseFor(fileUID, it->second, data, knownNodes, depsMap);
  }

  return depsMap;
}

} // namespace clangmetatool
