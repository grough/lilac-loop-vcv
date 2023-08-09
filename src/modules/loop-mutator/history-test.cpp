#pragma once

#include "catch.hpp"
#include "history.hpp"

TEST_CASE("Initial state", "[]") {
  LoopHistory h{100};
  float dt = 1.f;

  REQUIRE(h.ops.size() == 1);

  // Initial state has no size
  REQUIRE(h.ops.back()->startPos == 0);
  REQUIRE(h.ops.back()->endPos == 0);

  // Processing has no effect
  REQUIRE(h.process(dt, -999.f) == Approx(0.f));

  // Undo has no effect
  h.control.undoInput = true;
  REQUIRE(h.process(dt, -999.f) == Approx(0.f));
  REQUIRE(h.control.event == UNDO);
}

TEST_CASE("Record, play", "[]") {
  LoopHistory h{100};
  float dt = 1.f;

  // Record

  h.control.recordInput = true;

  REQUIRE(h.process(dt, 1000.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 2000.f) == Approx(2000.f));
  REQUIRE(h.process(dt, 3000.f) == Approx(3000.f));
  REQUIRE(h.process(dt, 4000.f) == Approx(4000.f));

  // Play

  h.control.recordInput = false;

  REQUIRE(h.process(dt, -999.f) == Approx(1000.f));
  REQUIRE(h.process(dt, -999.f) == Approx(2000.f));
  REQUIRE(h.process(dt, -999.f) == Approx(3000.f));
  REQUIRE(h.process(dt, -999.f) == Approx(4000.f));
  REQUIRE(h.process(dt, -999.f) == Approx(1000.f));
}
