#include "Loop.hpp"

TEST_CASE("Inputs are marked as connected", "[]") {
  Loops lps = Loops{2, 16};
  lps.setInputConnected(0, 1);
  lps.setInputConnected(1, 2);
  REQUIRE(lps.loops[0][0].inputConnected == true);
  REQUIRE(lps.loops[0][1].inputConnected == false);
  REQUIRE(lps.loops[1][0].inputConnected == true);
  REQUIRE(lps.loops[1][1].inputConnected == true);
  REQUIRE(lps.loops[1][2].inputConnected == false);
}

TEST_CASE("Returns are marked as connected", "[]") {
  Loops lps = Loops{2, 16};
  lps.setReturnConnected(0, 3);
  lps.setReturnConnected(1, 4);
  REQUIRE(lps.loops[0][0].returnConnected == true);
  REQUIRE(lps.loops[0][1].returnConnected == true);
  REQUIRE(lps.loops[0][2].returnConnected == true);
  REQUIRE(lps.loops[0][3].returnConnected == false);
  REQUIRE(lps.loops[1][0].returnConnected == true);
  REQUIRE(lps.loops[1][1].returnConnected == true);
  REQUIRE(lps.loops[1][2].returnConnected == true);
  REQUIRE(lps.loops[1][3].returnConnected == true);
  REQUIRE(lps.loops[1][5].returnConnected == false);
}

TEST_CASE("Toggle is monophonic", "[]") {
}

TEST_CASE("Stop is monophonic", "[]") {
}

TEST_CASE("Erase can be monophonic", "[]") {
}

TEST_CASE("Erase can erase pairs polyphonically", "[]") {
}

TEST_CASE("Mix control is monophonic", "[]") {
}
