//
// Created by Arseny Tolmachev on 2017/03/01.
//

#include "core/spec/spec_dsl.h"
#include "testing/standalone_test.h"

using namespace jumanpp::core::spec;
using namespace jumanpp::core::spec::dsl;

#define VALID(x) CHECK_OK((x).validate());
#define NOT_VALID(x) CHECK(!(x).validate().isOk());

using namespace jumanpp;

namespace jumanpp {
namespace util {

template <class Char, class Traits, class T>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os, const ArraySlice<T>& as) {
  os << '{';
  for (int i = 0; i < as.size(); ++i) {
    auto& obj = as[i];
    os << obj;
    if (i != as.size() - 1) {
      os << ", ";
    }
  }
  os << '}';
  return os;
}
}  // namespace util
}  // namespace jumanpp

template <typename T1, typename T2>
void CheckEqImpl(const util::ArraySlice<T1>& s1,
                 const util::ArraySlice<T2>& s2) {
  INFO("left : " << s1);
  INFO("right : " << s2);
  CHECK(s1.size() == s2.size());
  for (int i = 0; i < s1.size(); ++i) {
    INFO("index : " << i);
    CHECK(s1[i] == s2[i]);
  }
}

template <typename C1, typename C2>
void CheckEq(const C1& s1, const C2& s2) {
  util::ArraySlice<typename C1::value_type> sl1{s1};
  util::ArraySlice<typename C2::value_type> sl2{s2};
  CheckEqImpl(sl1, sl2);
}

template <typename C1, typename C2>
void SeqEq(const C1& c1, const std::initializer_list<C2>& ilist) {
  util::ArraySlice<typename C1::value_type> sl1{c1};
  util::ArraySlice<C2> sl2{ilist};
  CheckEqImpl(sl1, sl2);
}

void SeqEq(const std::vector<std::string>& c1,
           std::initializer_list<StringPiece> c2) {
  std::vector<StringPiece> copy;
  for (auto& s : c1) {
    copy.emplace_back(s);
  }

  util::ArraySlice<StringPiece> sl1{copy.data(), copy.size()};
  util::ArraySlice<StringPiece> sl2{c2.begin(), c2.size()};

  CheckEqImpl(sl1, sl2);
}

TEST_CASE("simple dsl creates descrptor") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  bldr.unigram({f1});
  AnalysisSpec spec{};
  CHECK_OK(bldr.build(&spec));
  CHECK(spec.dictionary.fields.size() == 1);
  CHECK(spec.features.totalPrimitives == 1);
  SeqEq(spec.features.primitive[0].references, {0});
  CHECK(spec.features.ngram.size() == 1);
}

TEST_CASE("simple dsl with two fields and two final features") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").strings();
  bldr.unigram({f1});
  bldr.unigram({f1, f2});
  AnalysisSpec spec{};
  CHECK_OK(bldr.build(&spec));
  CHECK(spec.dictionary.fields.size() == 2);
  CHECK(spec.features.totalPrimitives == 2);
  SeqEq(spec.features.primitive[0].references, {0});
  SeqEq(spec.features.primitive[1].references, {1});
  CHECK(spec.features.ngram.size() == 2);
  CHECK(spec.features.pattern.size() == 2);
  SeqEq(spec.features.pattern[1].references, {0, 1});
}

TEST_CASE("simple dsl with two fields and matcher") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").strings();
  auto& ft1 = bldr.feature("ft1").matchData(f1, "x");
  bldr.unigram({ft1});
  bldr.unigram({f1, f2});
  AnalysisSpec spec{};
  CHECK_OK(bldr.build(&spec));
  CHECK(spec.dictionary.fields.size() == 3);
  REQUIRE(spec.features.numDicFeatures == 3);
  REQUIRE(spec.features.numDicData == 0);
  REQUIRE(spec.features.totalPrimitives == 3);
  SeqEq(spec.features.primitive[0].references, {2, 0});
  CHECK(spec.features.primitive[0].kind == PrimitiveFeatureKind::SingleBit);
  SeqEq(spec.features.primitive[1].references, {0});
  SeqEq(spec.features.primitive[2].references, {1});
  CHECK(spec.features.pattern.size() == 2);
  CHECK(spec.features.ngram.size() == 2);
  SeqEq(spec.features.pattern[1].references, {1, 2});
}

TEST_CASE("simple dsl with two fields and list matcher") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").stringLists();
  auto& ft1 = bldr.feature("ft1").matchData(f2, "x");
  bldr.unigram({ft1});
  bldr.unigram({f1});
  AnalysisSpec spec{};
  CHECK_OK(bldr.build(&spec));
  CHECK(spec.dictionary.fields.size() == 3);
  CHECK(spec.features.totalPrimitives == 2);
  SeqEq(spec.features.primitive[0].references, {1, 0});
  CHECK(spec.features.primitive[0].kind == PrimitiveFeatureKind::SingleBit);
  SeqEq(spec.features.primitive[1].references, {0});
}

TEST_CASE("simple dsl with two fields and length matcher") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").strings();
  auto& ft1 = bldr.feature("ft1").length(f2);
  bldr.unigram({ft1});
  bldr.unigram({f1});
  AnalysisSpec spec{};
  CHECK_OK(bldr.build(&spec));
  CHECK(spec.dictionary.fields.size() == 2);
  CHECK(spec.features.totalPrimitives == 3);
  SeqEq(spec.features.primitive[0].references, {1});
  CHECK(spec.features.primitive[0].kind == PrimitiveFeatureKind::ByteLength);
  SeqEq(spec.features.primitive[1].references, {0});
}

TEST_CASE("dsl should fail to use string_list as ngram feature") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").stringLists();
  bldr.unigram({f1});
  bldr.unigram({f2});
  AnalysisSpec spec{};
  CHECK_FALSE(bldr.build(&spec));
}

TEST_CASE("dsl should fail to use kvlist as ngram feature") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").kvLists();
  bldr.unigram({f1});
  bldr.unigram({f2});
  AnalysisSpec spec{};
  CHECK_FALSE(bldr.build(&spec));
}

TEST_CASE("simple dsl with two fields and placeholder") {
  ModelSpecBuilder bldr;
  auto& f1 = bldr.field(1, "f1").strings().trieIndex();
  auto& f2 = bldr.field(2, "f2").stringLists();
  auto& ft1 = bldr.feature("ft1").placeholder();
  bldr.unk("wat", 1)
      .chunking(chars::CharacterClass::KATAKANA)
      .writeFeatureTo(ft1);
  bldr.unigram({ft1});
  bldr.unigram({f1});
  AnalysisSpec spec{};
  CHECK_OK(bldr.build(&spec));
  CHECK(spec.dictionary.fields.size() == 2);
  CHECK(spec.features.totalPrimitives == 2);
  CHECK(spec.features.primitive[0].kind == PrimitiveFeatureKind::Provided);
  SeqEq(spec.features.primitive[0].references, {0});
}

TEST_CASE("dic feature counts are resolved correctly") {
  ModelSpecBuilder msb;
  auto& f1 = msb.field(1, "f1").strings().trieIndex();
  auto& f2 = msb.feature("f2").matchData(f1, "test");
  auto& f3 = msb.field(2, "f3").strings();
  auto& f4 = msb.field(3, "f4").strings();
  auto& f5 = msb.feature("f5").matchAnyRowOfCsv("test,test", {f3, f1});
  msb.unigram({f2, f5});
  AnalysisSpec spec{};
  REQUIRE_OK(msb.build(&spec));
  CHECK(spec.features.numDicData == 1);
  CHECK(spec.features.numDicFeatures == 3);
}