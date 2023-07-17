#include "catch.hpp"
#include "engine.hpp"

TEST_CASE("Engine", "[]") {
  Lopper l = Lopper(16);
  float dt = 1.f;
  l.setCrossfadeRate(1.f);
  REQUIRE(l.process(dt, -5.f, 10.f) == Approx(0.f));
  REQUIRE(l.process(dt, -5.f, 20.f) == Approx(0.f));
  REQUIRE(l.process(dt, -5.f, 30.f) == Approx(0.f));
  REQUIRE(l.process(dt, -5.f, 40.f) == Approx(0.f));
  // Start
  // REQUIRE(l.process(dt, 5.f, 100.f) == Approx(0.f));
  // REQUIRE(l.process(dt, 5.f, 200.f) == Approx(0.f));
  // REQUIRE(l.process(dt, 5.f, 300.f) == Approx(0.f));
  // REQUIRE(l.process(dt, 5.f, 400.f) == Approx(0.f));
  // // Stop
  // REQUIRE(l.process(dt, -5.f, 1000.f) == Approx(1110.f));
  // REQUIRE(l.isEndOfCycle() == true);
  // REQUIRE(l.process(dt, -5.f, 2000.f) == Approx(2220.f));
  // REQUIRE(l.isEndOfCycle() == false);
  // REQUIRE(l.process(dt, -5.f, 3000.f) == Approx(3330.f));
  // REQUIRE(l.process(dt, -5.f, 4000.f) == Approx(4440.f));
  // REQUIRE(l.process(dt, -5.f, 5000.f) == Approx(1110.f));
  // REQUIRE(l.process(dt, -5.f, 6000.f) == Approx(2220.f));
}