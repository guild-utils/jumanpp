//
// Created by Arseny Tolmachev on 2017/11/07.
//

#ifndef JUMANPP_INNODE_FEATURES_H
#define JUMANPP_INNODE_FEATURES_H

#include "core/analysis/analyzer_impl.h"
#include "core/impl/feature_impl_types.h"

namespace jumanpp {
namespace core {
namespace analysis {

class InNodeFeatureComputer {
  const dic::DictionaryEntries entries_;
  const core::features::FeatureHolder& features_;
  features::impl::PrimitiveFeatureContext pfc_;

 public:
  InNodeFeatureComputer(const dic::DictionaryHolder& dic,
                        const features::FeatureHolder& features,
                        ExtraNodesContext* xtra, const AnalysisInput& input)
      : entries_{dic.entries()},
        features_{features},
        pfc_{xtra, dic.fields(), dic.entries(), input.codepoints()} {}

  bool importOneEntry(NodeInfo nfo, util::MutableArraySlice<i32> result);

  Status importEntryData(LatticeBoundary* lb) {
    auto ptrs = lb->starts()->nodeInfo();
    auto entries = lb->starts()->entryData();
    for (int i = 0; i < ptrs.size(); ++i) {
      if (!importOneEntry(ptrs[i], entries.row(i))) {
        return Status::InvalidState()
               << "failed to import entry data into lattice";
      }
    }
    return Status::Ok();
  }

  void patternFeaturesDynamic(LatticeBoundary* lb);

  void patternFeaturesEos(Lattice* l);
};

}  // namespace analysis
}  // namespace core
}  // namespace jumanpp

#endif  // JUMANPP_INNODE_FEATURES_H
