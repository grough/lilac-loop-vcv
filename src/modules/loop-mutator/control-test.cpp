#pragma once

#include "catch.hpp"
#include "control.hpp"

TEST_CASE("Control", "[]") {
  Control c;

  REQUIRE(c.recordInput == false);
  REQUIRE(c.undoInput == false);
  REQUIRE(c.event == NONE);

  // Do nothing

  c.process();
  REQUIRE(c.recordInput == false);
  REQUIRE(c.undoInput == false);
  REQUIRE(c.event == NONE);

  // Trigger undo

  c.undoInput = true;
  REQUIRE(c.process() == UNDO);
  c.undoInput = false;
  REQUIRE(c.process() == NONE);

  // Trigger record

  c.recordInput = true;
  REQUIRE(c.process() == RECORD);
  c.recordInput = false;
  REQUIRE(c.process() == END);
  REQUIRE(c.process() == NONE);
}

// Document an edge case
TEST_CASE("Does not trigger on first call to process()", "[]") {
  Control c;

  // Setting record input to true will not trigger on first call to process

  c.recordInput = true;
  c.process();
  REQUIRE(c.event == NONE);

  c.recordInput = true;
  c.process();
  REQUIRE(c.event == NONE);

  c.recordInput = false;
  c.process();
  REQUIRE(c.event == END);
}

// Document an edge case
TEST_CASE("Need a minimum of two calls process() to trigger an event", "[]") {
  Control c;

  c.process();
  c.recordInput = true;
  c.process();

  REQUIRE(c.event == RECORD);
}