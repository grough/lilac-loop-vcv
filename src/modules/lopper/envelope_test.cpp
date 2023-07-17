#include "catch.hpp"
#include "envelope.hpp"

TEST_CASE("Envelope", "[]") {
  GateEnvelope envelope;

  REQUIRE(envelope.getOutput() == Approx(0.f));
  REQUIRE(envelope.isDone() == false);

  envelope.attack();

  REQUIRE(envelope.getOutput() == Approx(1.f));
  REQUIRE(envelope.isDone() == false);

  envelope.release();

  REQUIRE(envelope.getOutput() == Approx(0.f));
  REQUIRE(envelope.isDone() == true);
}

TEST_CASE("AttackReleaseEnvelope", "[]") {
  AttackReleaseEnvelope envelope;
  envelope.setAttackRate(0.25f);
  envelope.setReleaseRate(0.25f);
  float dt = 1.f;

  REQUIRE(envelope.process(dt) == Approx(0.f));
  REQUIRE(envelope.isDone() == false);
  
  REQUIRE(envelope.process(dt) == Approx(0.f));
  REQUIRE(envelope.isDone() == false);

  envelope.attack();

  REQUIRE(envelope.process(dt) == Approx(.25f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(.5f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(.75f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(1.f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(1.f));
  REQUIRE(envelope.isDone() == false);

  envelope.release();

  REQUIRE(envelope.process(dt) == Approx(.75f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(.5f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(.25f));
  REQUIRE(envelope.isDone() == false);

  REQUIRE(envelope.process(dt) == Approx(0.f));
  REQUIRE(envelope.isDone() == true);

  // envelope.release();

  // REQUIRE(envelope.getOutput() == Approx(0.f));
  // REQUIRE(envelope.isDone() == true);
}
