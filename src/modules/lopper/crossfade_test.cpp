#include "catch.hpp"
#include "crossfade.hpp"

TEST_CASE("Linear rate=1 orientation=1", "[]") {
  LinearFade crossfade;
  crossfade.setRate(1.f);
  crossfade.setOrientation(1);

  crossfade.process(0.f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(.25f);
  REQUIRE(crossfade.getA() == Approx(.75f));
  REQUIRE(crossfade.getB() == Approx(.25f));

  crossfade.process(.5f);
  REQUIRE(crossfade.getA() == Approx(.5f));
  REQUIRE(crossfade.getB() == Approx(.5f));

  crossfade.process(.75f);
  REQUIRE(crossfade.getA() == Approx(0.25f));
  REQUIRE(crossfade.getB() == Approx(.75f));

  crossfade.process(1.f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));
}

TEST_CASE("Linear rate=1 orientation=-1 should be the same as rate=1 orientation=1", "[]") {
  LinearFade crossfade;
  crossfade.setRate(1.f);
  crossfade.setOrientation(-1);

  crossfade.process(0.f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(.25f);
  REQUIRE(crossfade.getA() == Approx(.75f));
  REQUIRE(crossfade.getB() == Approx(.25f));

  crossfade.process(.5f);
  REQUIRE(crossfade.getA() == Approx(.5f));
  REQUIRE(crossfade.getB() == Approx(.5f));

  crossfade.process(.75f);
  REQUIRE(crossfade.getA() == Approx(0.25f));
  REQUIRE(crossfade.getB() == Approx(.75f));

  crossfade.process(1.f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));
}

TEST_CASE("Linear crossfade rate=2 orientation=1", "[]") {
  LinearFade crossfade;

  crossfade.setRate(2.f);
  crossfade.setOrientation(1);
  crossfade.process(0.f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(.25f);
  REQUIRE(crossfade.getA() == Approx(.5f));
  REQUIRE(crossfade.getB() == Approx(.5f));

  crossfade.process(.5f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));

  crossfade.process(.75f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));

  // Check out of bounds

  crossfade.process(-999.f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(999.f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));
}

TEST_CASE("Linear crossfade rate=2 orientation=-1", "[]") {
  LinearFade crossfade;

  crossfade.setRate(2.f);
  crossfade.setOrientation(-1);
  crossfade.process(0.f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(.25f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(.5f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(.75f);
  REQUIRE(crossfade.getA() == Approx(.5f));
  REQUIRE(crossfade.getB() == Approx(.5f));

  crossfade.process(1.f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));

  // Check out of bounds

  crossfade.process(-999.f);
  REQUIRE(crossfade.getA() == Approx(1.f));
  REQUIRE(crossfade.getB() == Approx(0.f));

  crossfade.process(999.f);
  REQUIRE(crossfade.getA() == Approx(0.f));
  REQUIRE(crossfade.getB() == Approx(1.f));
}

// TEST_CASE("Constant power crossfade", "[]") {
//   ConstantPowerFade crossfade;
//   crossfade.process(0.f);
//   REQUIRE(crossfade.getA() == Approx(1.f));
//   crossfade.process(.25f);
//   REQUIRE(crossfade.getA() == Approx(0.92388f));
//   crossfade.process(.5f);
//   REQUIRE(crossfade.getA() == Approx(0.70711f));
//   crossfade.process(.75f);
//   REQUIRE(crossfade.getA() == Approx(0.38268f));
//   crossfade.process(1.f);
//   REQUIRE(crossfade.getA() == Approx(0.f));
// }

// TEST_CASE("Switch crossfade", "[]") {
//   SwitchFade crossfade;
//   crossfade.process(0.f);
//   REQUIRE(crossfade.getA() == Approx(1.f));
//   REQUIRE(crossfade.getB() == Approx(0.f));
//   crossfade.process(.25f);
//   REQUIRE(crossfade.getA() == Approx(1.f));
//   REQUIRE(crossfade.getB() == Approx(0.f));
//   crossfade.process(.4999f);
//   REQUIRE(crossfade.getA() == Approx(1.f));
//   REQUIRE(crossfade.getB() == Approx(0.f));
//   crossfade.process(.5f);
//   REQUIRE(crossfade.getA() == Approx(0.f));
//   REQUIRE(crossfade.getB() == Approx(1.f));
//   crossfade.process(.75f);
//   REQUIRE(crossfade.getA() == Approx(0.f));
//   REQUIRE(crossfade.getB() == Approx(1.f));
//   crossfade.process(1.f);
//   REQUIRE(crossfade.getA() == Approx(0.f));
//   REQUIRE(crossfade.getB() == Approx(1.f));
// }
