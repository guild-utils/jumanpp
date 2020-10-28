//
// Created by Arseny Tolmachev on 2017/11/01.
//

#ifndef JUMANPP_DIC_BUILD_DETAIL_H
#define JUMANPP_DIC_BUILD_DETAIL_H

#include <memory>
#include <vector>
#include "core/dic/progress.h"
#include "core/spec/spec_serialization.h"
#include "core/spec/spec_types.h"
#include "core_config.h"
#include "entry_builder.h"
#include "field_reader.h"
#include "util/inlined_vector.h"
#include "util/serialization.h"
#include "util/string_piece.h"

namespace jumanpp {
namespace core {
namespace dic {

struct BuiltDictionary;

namespace s = core::spec;

struct DictionaryBuilderStorage {
  std::vector<impl::StringStorage> storage;
  std::vector<ColumnImportContext> importers;
  std::vector<util::CodedBuffer> stringBuffers;
  std::vector<util::CodedBuffer> intBuffers;
  EntryTableBuilder entries;
  util::CodedBuffer builtDicData;
  util::CodedBuffer builtSpecData;
  std::unique_ptr<spec::AnalysisSpec> restoredSpec;
  std::vector<StringPiece> builtStrings;
  std::vector<StringPiece> builtInts;

  i32 maxUsedCol = -1;
  StringPiece maxFieldName;
  i32 indexColumn = -1;

  Status initialize(const s::DictionarySpec& dicSpec);
  Status initDicFeatures(const s::FeaturesSpec& dicSpec);
  Status computeStats(StringPiece name, util::CsvReader* csv,
                      ProgressCallback* callback);
  Status makeStorage(ProgressCallback* callback);
  i32 importActualData(util::CsvReader* csv, ProgressCallback* callback);
  Status buildTrie(ProgressCallback* callback);
  void fillResult(BuiltDictionary* dic_);
  Status initGroupingFields(const s::AnalysisSpec& spec);
};

}  // namespace dic
}  // namespace core
}  // namespace jumanpp

#endif  // JUMANPP_DIC_BUILD_DETAIL_H
