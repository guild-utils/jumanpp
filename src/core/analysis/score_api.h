//
// Created by Arseny Tolmachev on 2017/03/07.
//

#ifndef JUMANPP_SCORE_API_H
#define JUMANPP_SCORE_API_H

#include <memory>
#include "core/analysis/lattice_config.h"
#include "core/impl/model_format.h"
#include "util/array_slice.h"
#include "util/quantized_weights.h"
#include "util/sliceable_array.h"
#include "util/status.hpp"

namespace jumanpp {
namespace core {
namespace analysis {

class Lattice;
class ExtraNodesContext;

class ScorerBase {
 public:
  virtual ~ScorerBase() = default;
  virtual Status load(const model::ModelInfo& model) = 0;
};

struct FloatBufferWeights {
  util::ArraySlice<float> weights;
  FloatBufferWeights(const util::ArraySlice<float>& weights)
      : weights(weights) {}
  float at(size_t idx) const { return weights.at(idx); }
  size_t size() const { return weights.size(); }
  template <util::PrefetchHint Hint>
  void prefetch(size_t idx) const {
    util::prefetch<Hint>(weights.data() + idx);
  }
};

// using WeightBuffer = util::Float8BitLinearQ;
using WeightBuffer = FloatBufferWeights;

class FeatureScorer : public ScorerBase {
 public:
  virtual void compute(util::MutableArraySlice<float> result,
                       util::ConstSliceable<u32> features) const = 0;
  virtual void add(util::ArraySlice<float> source,
                   util::MutableArraySlice<float> result,
                   util::ConstSliceable<u32> features) const = 0;
  virtual const WeightBuffer& weights() const = 0;
};

class ScoreComputer {
 public:
  virtual ~ScoreComputer() = default;
  virtual Status scoreLattice(Lattice* l, const ExtraNodesContext* xtra,
                              u32 scorerIdx) = 0;
};

class ScorerFactory : public ScorerBase {
 public:
  virtual Status makeInstance(std::unique_ptr<ScoreComputer>* result) = 0;
};

struct ScorerDef {
  FeatureScorer* feature;
  std::vector<ScorerFactory*> others;
  std::vector<Score> scoreWeights;

  i32 numScorers() const { return static_cast<i32>(1 + others.size()); }
};

}  // namespace analysis
}  // namespace core
}  // namespace jumanpp

#endif  // JUMANPP_SCORE_API_H
