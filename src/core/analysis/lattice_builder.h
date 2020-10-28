//
// Created by Arseny Tolmachev on 2017/02/23.
//

#ifndef JUMANPP_LATTICE_BUILDER_H
#define JUMANPP_LATTICE_BUILDER_H

#include <vector>
#include "core/core_types.h"
#include "extra_nodes.h"
#include "lattice_types.h"
#include "util/flatset.h"
#include "util/status.hpp"
#include "util/stl_util.h"
#include "util/types.hpp"

namespace jumanpp {
namespace core {
namespace analysis {

struct LatticeNodeSeed {
  EntryPtr entryPtr;

  // these two will become the same for erased nodes
  LatticePosition codepointStart;
  LatticePosition codepointEnd;

  LatticeNodeSeed(EntryPtr entryPtr, LatticePosition codepointStart,
                  LatticePosition codepointEnd)
      : entryPtr(entryPtr),
        codepointStart(codepointStart),
        codepointEnd(codepointEnd) {}
};

inline bool operator==(const LatticeNodeSeed& a1,
                       const LatticeNodeSeed& a2) noexcept {
  return a1.entryPtr == a2.entryPtr && a1.codepointStart == a2.codepointStart &&
         a1.codepointEnd == a2.codepointEnd;
}

struct BoundaryInfo {
  /**
   * Index of first input codepoint for this boundary
   */
  LatticePosition number;

  /**
   * Number of nodes starting from this boundary
   */
  u16 startCount;

  u16 endCount;

  /**
   * If you put all nodes lattice nodes in a single array,
   * ordered by index of first codepoint, then this will
   * be the index of the first node with starting codepoint == \refitem number
   */
  u32 firstNodeOffset;
};

class LatticeConstructionContext {
 public:
  void addBos(LatticeBoundary* lb);
  void addEos(LatticeBoundary* lb, LatticePosition position);
};

/**
 * This class compacts nodes which will have the same
 * feature representation to a single node.
 *
 * Instead of group of original nodes, lattice will have
 * only one Alias extra node.
 *
 * The extra node has the same feature representation,
 * however output needs to unroll it to all nodes.
 */
class LatticeCompactor {
  std::vector<i32> features;
  dic::DictionaryEntries dicEntries;
  ExtraNodesContext* xtra;
  std::vector<i32> entries;
  std::vector<u64> hashes;
  std::vector<i32> group;
  util::FlatSet<i32> processed;

 public:
  LatticeCompactor(const dic::DictionaryEntries& dicEntries);

  Status initialize(ExtraNodesContext* ctx, const spec::AnalysisSpec& spec);
  void computeHashes(util::ArraySlice<LatticeNodeSeed> seeds);
  bool compact(util::MutableArraySlice<LatticeNodeSeed>* seeds);
  i32 numDeleted() const { return processed.size(); }

  util::ArraySlice<i32> usedFeatures() const { return features; }
};

class LatticeBuilder {
  std::vector<LatticeNodeSeed> seeds_;
  std::vector<BoundaryInfo> boundaries_;
  std::vector<bool> connectible;
  LatticePosition maxBoundaries_;

 public:
  inline void appendSeed(EntryPtr entryPtr, LatticePosition start,
                         LatticePosition end) {
    seeds_.emplace_back(entryPtr, start, end);
  }

  void sortSeeds();
  bool checkConnectability();
  void reset(LatticePosition maxCodepoints);

  util::ArraySlice<LatticeNodeSeed> seeds() const {
    return {seeds_.data(), seeds_.size()};
  }

  Status prepare();
  void compactBoundary(i32 boundary, LatticeCompactor* compactor);
  Status constructSingleBoundary(Lattice* lattice, LatticeBoundary** result,
                                 i32 numBoundary);
  bool isAccessible(i32 boundary) const { return connectible[boundary]; }
  Status makeBos(LatticeConstructionContext* ctx, Lattice* lattice);
  Status makeEos(LatticeConstructionContext* ctx, Lattice* lattice);
  Status fillEnds(Lattice* l);
  const BoundaryInfo& infoAt(i32 boundary) const;

  u64 usedMemory() const {
    return seeds_.size() * sizeof(LatticeNodeSeed) +
           boundaries_.size() * sizeof(BoundaryInfo) + connectible.size() / 8;
  }
};

}  // namespace analysis
}  // namespace core
}  // namespace jumanpp

#endif  // JUMANPP_LATTICE_CONSTRUCTION_H
