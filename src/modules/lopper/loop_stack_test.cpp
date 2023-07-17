#include "catch.hpp"
#include "buffer.hpp"
#include "loop_stack.hpp"

TEST_CASE("LoopStack", "[]") {
  RBuf *inBuf = new RBuf(16);
  LoopStack stack = LoopStack(inBuf);
  // FIX: HEAD mode is not a smooth crossfade
  stack.setHeadMode();
  float dt = 1.f;

  inBuf->write(1.f);
  inBuf->next();
  stack.process(dt, 1.f);

  inBuf->write(2.f);
  inBuf->next();
  stack.process(dt, 2.f);

  inBuf->write(3.f);
  inBuf->next();
  stack.process(dt, 3.f);

  inBuf->write(4.f);
  inBuf->next();
  stack.process(dt, 4.f);

  stack.startRecording();

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 10.f);

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 20.f);

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 30.f);

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 40.f);

  stack.stopRecording();

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 100.f);

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 200.f);

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 300.f);

  inBuf->write(-999.f);
  inBuf->next();
  stack.process(dt, 400.f);

  // ---

  // inBuf->write(-999.f);
  // inBuf->next();
  // // REQUIRE(stack.process(dt, 100.f) == Approx(110.f)); // SumFade
  // REQUIRE(stack.process(dt, 100.f) == Approx(100.f)); // LinearFade
  // // REQUIRE(stack.process(dt, 100.f) == Approx(0.f)); // AttackRelaseEnvelope

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 200.f) == Approx(220.f)); // SumFade
  // REQUIRE(stack.process(dt, 200.f) == Approx(155.f)); // LinearFade
  // // REQUIRE(stack.process(dt, 200.f) == Approx(0.f)); // AttackRelaseEnvelope

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 300.f) == Approx(330.f)); // SumFade
  // REQUIRE(stack.process(dt, 300.f) == Approx(165.f)); // LinearFade

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 400.f) == Approx(440.f)); // SunFade
  // REQUIRE(stack.process(dt, 400.f) == Approx(130.f));

  // stack.startRecording();

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 1000.f) == Approx(1000.f)); //SumFade
  // REQUIRE(stack.process(dt, 1000.f) == Approx(0.f));

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 2000.f) == Approx(2000.f)); // SumFade
  // REQUIRE(stack.process(dt, 2000.f) == Approx(1000.f));

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 3000.f) == Approx(3000.f)); // SumFade
  // REQUIRE(stack.process(dt, 3000.f) == Approx(2000.f));

  // stack.stopRecording();

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 10000.f) == Approx(11000.f)); // SumFade
  // REQUIRE(stack.process(dt, 10000.f) == Approx(10000.f));

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 20000.f) == Approx(22000.f)); // SumFade
  // REQUIRE(stack.process(dt, 20000.f) == Approx(14000.f));

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 30000.f) == Approx(33000.f)); // SumFade
  // REQUIRE(stack.process(dt, 30000.f) == Approx(12000.f));

  // inBuf->write(-999.f);
  // inBuf->next();
  // //   REQUIRE(stack.process(dt, 30000.f) == Approx(11000.f)); // SumFade
  // REQUIRE(stack.process(dt, 30000.f) == Approx(10000.f));
}
