//
// Created by Arseny Tolmachev on 2017/05/17.
//

#include "core/training/scw.h"
#include "core/training/training_test_common.h"

#include "graphviz_format.h"

using namespace jumanpp::core;
using namespace jumanpp::core::format;

using Attr = detail::Attribute;

TEST_CASE("graphviz has working builder") {
  GoldExampleEnv env{
      "a,a,a\nb,b,b\nc,c,c\nba,ba,ba\nab,an,ab\nca,fa,da\nb,z,z\n"};
  GraphVizBuilder bldr;
  bldr.row({"b", "c"}, {Attr{"color", "green"}});
  bldr.row({"a"});
  GraphVizFormat fmt;
  REQUIRE_OK(bldr.build(&fmt));
  TrainingConfig tc{};
  tc.beamSize = 3;
  tc.featureNumberExponent = 12;
  training::SoftConfidenceWeighted scw{tc};
  REQUIRE_OK(env.anaImpl()->initScorers(*scw.scorers()));
  REQUIRE_OK(fmt.initialize(env.anaImpl()->output()));
  REQUIRE(env.anaImpl()->resetForInput("bacababa"));
  REQUIRE(env.anaImpl()->prepareNodeSeeds());
  REQUIRE(env.anaImpl()->buildLattice());
  REQUIRE_OK(env.anaImpl()->bootstrapAnalysis());
  REQUIRE(env.anaImpl()->computeScores(scw.scorers()));
  fmt.markGold({5, 0});
  CHECK_OK(fmt.render(env.anaImpl()->lattice()));
  CHECK(fmt.result().size() > 0);
}