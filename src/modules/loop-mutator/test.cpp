#pragma once

#include "catch.hpp"
#include "engine.hpp"

TEST_CASE("Initial state") {
  LoopHistory h{8};
  REQUIRE(h.ops.back()->startPos == 0);
  REQUIRE(h.ops.back()->endPos == 0);
  REQUIRE(h.process(1.f, -999.f) == Approx(0.f));
  h.control.undoInput = true;
  REQUIRE(h.process(1.f, -999.f) == Approx(0.f));
}
