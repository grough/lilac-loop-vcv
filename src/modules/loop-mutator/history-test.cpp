#pragma once

#include "catch.hpp"
#include "history.hpp"

TEST_CASE("Initial state", "[]") {
  LoopHistory h{100};
  float dt = 1.f;

  REQUIRE(h.ops.size() == 1);

  // Initial state has no size
  REQUIRE(h.ops.back()->startPos == -1);
  REQUIRE(h.ops.back()->endPos == -1);

  // Processing has no effect
  REQUIRE(h.process(dt, -999.f) == Approx(0.f));

  // Undo has no effect
  h.control.undoInput = true;
  REQUIRE(h.process(dt, -999.f) == Approx(0.f));
  REQUIRE(h.control.event == UNDO);
}

TEST_CASE("Record, play, fade loop ending into beginning", "[]") {
  LoopHistory h{100, 4.f};
  float dt = 1.f;

  // Record

  h.control.recordInput = true;

  REQUIRE(h.process(dt, 1000.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 2000.f) == Approx(2000.f));
  REQUIRE(h.process(dt, 3000.f) == Approx(3000.f));
  REQUIRE(h.process(dt, 4000.f) == Approx(4000.f));

  // Play

  h.control.recordInput = false;

  // Crossfade: feedback * (1 - env) + input * env

  REQUIRE(h.process(dt, 0.f) == Approx(250.f));  // 1000 * .25 + 0 * .75
  REQUIRE(h.process(dt, 0.f) == Approx(1000.f)); // 2000 * .5 + 0 * .5
  REQUIRE(h.process(dt, 0.f) == Approx(2250.f)); // 3000 * .75 + 0 * .25
  REQUIRE(h.process(dt, 0.f) == Approx(4000.f)); // 4000 * 1 + 0 * .25
  REQUIRE(h.process(dt, 0.f) == Approx(250.f));  // 250 * 1 + 0 * 0
  REQUIRE(h.process(dt, 0.f) == Approx(1000.f)); // 2000 * 1 + 0 * 0

  REQUIRE(h.ops.back()->size() == 4);
}

TEST_CASE("Record, play, no fade", "[]") {
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

  // Crossfade: feedback * (1 - env) + input * env

  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(3000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(4000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
}

TEST_CASE("Record, play, record, play", "[]") {
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

  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(3000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(4000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));

  // Record

  h.control.recordInput = true;

  REQUIRE(h.process(dt, 10000.f) == Approx(10000.f));
  REQUIRE(h.process(dt, 20000.f) == Approx(20000.f));
  REQUIRE(h.process(dt, 30000.f) == Approx(30000.f));

  // Play

  h.control.recordInput = false;

  REQUIRE(h.process(dt, 100.f) == Approx(10000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(20000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(30000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(10000.f));
}

TEST_CASE("Record, undo", "[]") {
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

  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(3000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(4000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));

  // Undo

  h.control.undoInput = true;

  REQUIRE(h.process(dt, -999.f) == Approx(0.f));
  REQUIRE(h.process(dt, -999.f) == Approx(0.f));
  REQUIRE(h.process(dt, -999.f) == Approx(0.f));
}

TEST_CASE("Record, play, record, play, undo, undo", "[]") {
  LoopHistory h{100};
  float dt = 1.f;

  // Record

  h.control.recordInput = true;

  REQUIRE(h.process(dt, 1000.f) == Approx(1000.f));
  RecordOp *recOp1 = dynamic_cast<RecordOp *>(h.op());
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 0);
  REQUIRE(recOp1->endPos == 0);

  REQUIRE(h.process(dt, 2000.f) == Approx(2000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 1);
  REQUIRE(recOp1->endPos == 1);

  REQUIRE(h.process(dt, 3000.f) == Approx(3000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 2);
  REQUIRE(recOp1->endPos == 2);

  REQUIRE(h.process(dt, 4000.f) == Approx(4000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 3);
  REQUIRE(recOp1->endPos == 3);

  REQUIRE(h.process(dt, 5000.f) == Approx(5000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 4);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.ops.size() == 2);

  // Play

  h.control.recordInput = false;

  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 0);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 1);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.process(dt, 100.f) == Approx(3000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 2);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.process(dt, 100.f) == Approx(4000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 3);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.process(dt, 100.f) == Approx(5000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 4);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 0);
  REQUIRE(recOp1->endPos == 4);

  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
  REQUIRE(recOp1->startPos == 0);
  REQUIRE(recOp1->pos == 1);
  REQUIRE(recOp1->endPos == 4);

  // Record

  h.control.recordInput = true;

  REQUIRE(h.process(dt, 10000.f) == Approx(10000.f));
  RecordOp *recOp2 = dynamic_cast<RecordOp *>(h.op());
  REQUIRE(recOp2->startPos == 5);
  REQUIRE(recOp2->pos == 5);
  REQUIRE(recOp2->endPos == 5);

  REQUIRE(h.process(dt, 20000.f) == Approx(20000.f));
  REQUIRE(recOp2->startPos == 5);
  REQUIRE(recOp2->pos == 6);
  REQUIRE(recOp2->endPos == 6);

  REQUIRE(h.process(dt, 30000.f) == Approx(30000.f));
  REQUIRE(recOp2->startPos == 5);
  REQUIRE(recOp2->pos == 7);
  REQUIRE(recOp2->endPos == 7);

  REQUIRE(h.ops.size() == 3);

  // Play

  h.control.recordInput = false;

  REQUIRE(h.process(dt, 100.f) == Approx(10000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(20000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(30000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(10000.f));

  // Layout

  // Undo

  h.control.undoInput = true;

  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
  REQUIRE(h.ops.size() == 2);
  REQUIRE(h.process(dt, 100.f) == Approx(2000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(3000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(4000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(5000.f));
  REQUIRE(h.process(dt, 100.f) == Approx(1000.f));
}