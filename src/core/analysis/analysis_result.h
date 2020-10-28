//
// Created by Arseny Tolmachev on 2017/03/08.
//

#ifndef JUMANPP_ANALYSIS_RESULT_H
#define JUMANPP_ANALYSIS_RESULT_H

#include "lattice_config.h"
#include "util/array_slice.h"
#include "util/status.hpp"

namespace jumanpp {
namespace core {
namespace analysis {

class Lattice;

class AnalysisPath {
  // Pointers to beam elements from EOS to BOS order (!)
  // it is reversed morhpeme order!
  // Pointers to nodes with the same score are stored in beam order.
  std::vector<const ConnectionPtr*> elems_;
  // offsets to elements of previous array
  // begin[last] == offsets[0]
  // begin[last-1] == end[last] == offsets[1]
  // ...
  // begin[first] == end[first+1] == offsets[offsets.size - 2]
  // end[first] == offsets[offsets.size - 1]
  std::vector<u32> offsets_;
  i32 currentChunk_;
  i32 currentNode_;

 public:
  bool nextBoundary() {
    currentChunk_ += 1;
    currentNode_ = -1;
    return currentChunk_ < numBoundaries();
  }

  i32 remainingNodesInChunk() const {
    return totalNodesInChunk() - currentNode_;
  }

  i32 totalNodesInChunk() const {
    auto idx = offsets_.size() - currentChunk_ - 1;
    JPP_DCHECK_IN(idx, 1, offsets_.size());
    return offsets_[idx] - offsets_[idx - 1];
  }

  i32 currentBoundary() const {
    auto beginIdx = offsets_.size() - currentChunk_ - 2;
    JPP_DCHECK_IN(beginIdx, 0, offsets_.size());
    auto begin = offsets_[beginIdx];
    return elems_[begin]->boundary;
  }

  bool nextNode(ConnectionPtr* result) {
    auto ptr = nextNodePtr();
    if (ptr != nullptr) {
      *result = *ptr;
      return true;
    }
    return false;
  }

  const ConnectionPtr* nextNodePtr() {
    currentNode_ += 1;
    bool status = remainingNodesInChunk() > 0;
    if (status) {
      auto beginIdx = offsets_.size() - currentChunk_ - 2;
      JPP_DCHECK_IN(beginIdx, 0, offsets_.size());
      auto begin = offsets_[beginIdx];
      auto idx = begin + currentNode_;
      JPP_DCHECK_IN(idx, 0, elems_.size());
      return elems_[idx];
    }
    return nullptr;
  }

  const ConnectionBeamElement* nextBeamPtr() {
    // this cast is safe because this guy is filled directly from Lattice
    return reinterpret_cast<const ConnectionBeamElement*>(nextNodePtr());
  }

  i32 curNode() const { return currentNode_; }

  i32 numBoundaries() const { return static_cast<i32>(offsets_.size()) - 1; }

  i32 totalNodes() const { return static_cast<i32>(elems_.size()); }

  void reset() { currentChunk_ = -1; }

  bool contains(ConnectionPtr step0, ConnectionPtr step1) const {
    for (auto& el : elems_) {
      if (*el == step1 && *el->previous == step0) {
        return true;
      }
    }

    return false;
  }

  bool isLastBoundary() const { return currentChunk_ == numBoundaries() - 1; }

  void moveToChunk(i32 pos) {
    JPP_DCHECK_IN(pos, 0, offsets_.size());
    currentChunk_ = pos;
    currentNode_ = -1;
  }

  bool moveToBoundary(i32 boundary) {
    while (nextBoundary()) {
      auto bnd = currentBoundary();
      if (bnd == boundary) {
        return true;
      } else if (bnd > boundary) {
        currentChunk_ -= 1;
        return false;
      }
    }
    return false;
  }

  Status fillIn(const Lattice* l);
};

class Analyzer;

class AnalysisResult {
  Lattice* ptr_;

 public:
  Status reset(const Analyzer& analyzer);
  Status fillTop1(AnalysisPath* result);
};

}  // namespace analysis
}  // namespace core
}  // namespace jumanpp

#endif  // JUMANPP_ANALYSIS_RESULT_H
