#pragma once

#include "catch.hpp"
#include "sr-envelope.hpp"

TEST_CASE("Sustain-Release Envelope", "[]") {
  SustainRelease sr{};
  sr.setReleaseTime(4.f);
  float dt = 1.f;

  REQUIRE(sr.process(dt) == Approx(1.f));
  REQUIRE(sr.getOutput() == Approx(1.f));
  REQUIRE(sr.getStage() == SustainRelease::OPEN);

  REQUIRE(sr.process(dt) == Approx(1.f));
  REQUIRE(sr.getOutput() == Approx(1.f));
  REQUIRE(sr.getStage() == SustainRelease::OPEN);

  sr.release();

  REQUIRE(sr.process(dt) == Approx(0.75f));
  REQUIRE(sr.getOutput() == Approx(0.75f));
  REQUIRE(sr.getStage() == SustainRelease::CLOSING);

  REQUIRE(sr.process(dt) == Approx(0.5f));
  REQUIRE(sr.getOutput() == Approx(0.5f));
  REQUIRE(sr.getStage() == SustainRelease::CLOSING);

  REQUIRE(sr.process(dt) == Approx(0.25f));
  REQUIRE(sr.getOutput() == Approx(0.25f));
  REQUIRE(sr.getStage() == SustainRelease::CLOSING);

  REQUIRE(sr.process(dt) == Approx(0.f));
  REQUIRE(sr.getOutput() == Approx(0.f));
  REQUIRE(sr.getStage() == SustainRelease::CLOSED);

  REQUIRE(sr.process(dt) == Approx(0.f));
  REQUIRE(sr.getOutput() == Approx(0.f));
  REQUIRE(sr.getStage() == SustainRelease::CLOSED);
}
