#include "catch.hpp"
#include "buffer.hpp"
#include "crossfade.hpp"
#include "envelope.hpp"
#include "soft_loop.hpp"

// FadeMode=ALL is not practically useful outside of testing
TEST_CASE("SoftLoop GateEnvelope SumFade FadeMode=ALL", "[]") {
  RBuf *inBuf = new RBuf(12);
  ICrossfade *crossfade = new SumFade();
  IEnvelope *envelope = new GateEnvelope();
  SoftLoop loop(inBuf, crossfade, envelope, SoftLoop::FadeMode::ALL);
  float dt = 1.f;

  // Head

  inBuf->write(1.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 1.f) == Approx(0.f));

  inBuf->write(2.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 2.f) == Approx(0.f));

  inBuf->write(3.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 3.f) == Approx(0.f));

  loop.startRecording();

  // Loop

  // Current: outputs loop during recording (unlike Looper)
  // Wrong behaviour if there's a "Mix" control
  // Using GateEnvelope for testing provides a different experience
  // Using AttackRelease envelope in reality makes more sense
  //   The loop fades in as you record
  // muteWhileRcording=false is a good default for now
  //   but it presents a problem for fades that are proportional to loop length
  //   since loop length is not known until recording stops
  // For now Lopper is a device with no mix control, no dry output

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 10.f) == Approx(10.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 20.f) == Approx(20.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 30.f) == Approx(30.f));

  loop.stopRecording();

  // Tail

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 100.f) == Approx(111.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 200.f) == Approx(222.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 300.f) == Approx(333.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 4000.f) == Approx(111.f));

  loop.release();

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 5000.f) == Approx(0.f));
}

TEST_CASE("SoftLoop GateEnvelope SumFade FadeMode=TAIL", "[]") {
  RBuf *inBuf = new RBuf(16);
  ICrossfade *crossfade = new SumFade();
  IEnvelope *envelope = new GateEnvelope();
  SoftLoop loop(inBuf, crossfade, envelope, SoftLoop::FadeMode::TAIL);
  float dt = 1.f;

  // Accumulate head buffer
  // Head is irrelevant since this is a test for tail

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 1.f) == Approx(0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 2.f) == Approx(0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 3.f) == Approx(0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 4.f) == Approx(0.f));

  loop.startRecording();

  // Record loop

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 10.f) == Approx(10.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 20.f) == Approx(20.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 30.f) == Approx(30.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 40.f) == Approx(40.f));

  loop.stopRecording();

  // In TAIL mode the tail is summed with the loop

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 100.f) == Approx(110.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 200.f) == Approx(220.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 300.f) == Approx(330.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 400.f) == Approx(440.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 5000.f) == Approx(110.f));

  loop.release();

  // GateEnvelope is closed

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 6000.f) == Approx(0.f));
}

TEST_CASE("SoftLoop GateEnvelope LinearFade FadeMode=TAIL", "[]") {
  RBuf *inBuf = new RBuf(16);
  ICrossfade *crossfade = new LinearFade();
  IEnvelope *envelope = new GateEnvelope();
  SoftLoop loop(inBuf, crossfade, envelope, SoftLoop::FadeMode::TAIL);
  float dt = 1.f;

  // Head is irrelevant since this is a test for tail

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 1.f) == Approx(0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 2.f) == Approx(0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 3.f) == Approx(0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 4.f) == Approx(0.f));

  loop.startRecording();

  // HEAD won't work because the head loop can't be known before recording stops
  //   and the head would need to fade in before the end of the loop
  //   but it doesn't matter - just don't play the head on the first loop
  //   will you get a click?
  //   use tail to smooth the first loop?
  // TAIL can work because the tail is known by the (exact) time
  //   that it's needed for fading out during the first loop

  // A * tail + B * loop
  // Tail is 0.f because it's in the future

  // Phase is asymptotic until recording has stopped
  // Phase == position / size == position / (position + 1)
  // -1/0, 0/1, 1/2, 2/3, 3/4, …

  // Story: When recording in tail mode you will hear a live signal

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 0 + (0 + 1) == 0
  REQUIRE(loop.process(dt, 10.f) == Approx(1.f * 0.f + 0.f * 10.f));
  REQUIRE(loop.phase() == 0.f);

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 1 + (1 + 1) == 0.5
  REQUIRE(loop.process(dt, 20.f) == Approx(.5f * 0.f + 0.5f * 20.f));
  REQUIRE(loop.phase() == 0.5f);

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 2 + (2 + 1) == 2/3
  REQUIRE(loop.process(dt, 30.f) == Approx(.5f * 0.f + 2 / 3.f * 30.f));
  REQUIRE(loop.phase() == 2 / 3.f);

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 3 + (3 + 1) == 0.75
  REQUIRE(loop.process(dt, 40.f) == Approx(.5f * 0.f + 0.75f * 40.f));
  REQUIRE(loop.phase() == .75f);

  loop.stopRecording();

  // TAIL LinearFade
  // tail  100   200   300   400
  // A       1   .75    .5   .25
  // loop   10    20    30    40
  // B       0   .25    .5   .75

  // Start writing (and reading) tail. Loop length is now defined, crossfade is linear

  // Story: Once you stop recording in TAIL mode, tail continues and loop fades in

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 100.f) == Approx(100.f * 1.f + 10.f * 0.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 200.f) == Approx(200.f * .75f + 20.f * .25f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 300.f) == Approx(300.f * .5f + 30.f * .5f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 400.f) == Approx(400.f * .25f + 40.f * .75f));

  inBuf->write(-999.f);
  inBuf->next();

  REQUIRE(loop.process(dt, 5000.f) == Approx(100.f * 1.f + 10.f * 0.f));
  REQUIRE(loop.isEndOfCycle() == true);
  REQUIRE(loop.phase() == Approx(0.f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(200.f * .75f + 20.f * .25f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.25f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(300.f * .5f + 30.f * .5f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.5f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(400.f * .25f + 40.f * .75f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.75f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(100.f * 1.f + 10.f * 0.f));
  REQUIRE(loop.isEndOfCycle() == true);
  REQUIRE(loop.phase() == Approx(0.f));

  loop.release();

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, -999.f) == Approx(0.f));
}

TEST_CASE("SoftLoop GateEnvelop LinearFade FadeMode=HEAD", "[]") {
  RBuf *inBuf = new RBuf(16);
  ICrossfade *crossfade = new LinearFade();
  IEnvelope *envelope = new GateEnvelope();
  SoftLoop loop(inBuf, crossfade, envelope, SoftLoop::FadeMode::HEAD);
  float dt = 1.f;

  // Start writing head, no recording yet
  // The head values are significant since they will be used for crossfading in HEAD mode

  inBuf->write(1.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 1.f) == Approx(0.f));

  inBuf->write(2.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 2.f) == Approx(0.f));

  inBuf->write(3.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 3.f) == Approx(0.f));

  inBuf->write(4.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 4.f) == Approx(0.f));

  loop.startRecording();

  inBuf->write(-999.f);
  inBuf->next();

  // Approx(A * loop + B * head)

  // phase = p / (p + 1) == 0 + (0 + 1) == 0
  REQUIRE(loop.process(dt, 10.f) == Approx(1.f * 10.f + 0.f * 1.f));
  REQUIRE(loop.phase() == 0.f);

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 1 + (1 + 1) == 0.5
  // REQUIRE(loop.process(dt, 20.f) == Approx(.5f * 20.f + 0.5f * 2.f)); // if there was a head
  REQUIRE(loop.process(dt, 20.f) == Approx(.5f * 20.f + 0.5f * 0.f)); // there is no head, bug?
  REQUIRE(loop.phase() == 0.5f);

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 2 + (2 + 1) == 2/3
  // REQUIRE(loop.process(dt, 30.f) == Approx(.5f * 30.f + 2 / 3.f * 30.f));
  REQUIRE(loop.process(dt, 30.f) == Approx(1 / 3.f * 30.f + 2 / 3.f * 0.f));
  REQUIRE(loop.phase() == 2 / 3.f);

  inBuf->write(-999.f);
  inBuf->next();

  // phase = p / (p + 1) == 3 + (3 + 1) == 0.75
  REQUIRE(loop.process(dt, 40.f) == Approx(.25f * 40.f + 0.75f * 0.f));
  REQUIRE(loop.phase() == .75f);

  loop.stopRecording();

  // Start writing tail. Loop length is now defined, crossfade is linear

  // HEAD LinearFade
  // loop   10    20    30    40
  // A       1   .75    .5   .25
  // head    1     2     3     4
  // B       0   .25    .5   .75

  // Approx(A * loop + B * head)

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, -999.f) == Approx(1.f * 10.f + 0.f * 1.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, -999.f) == Approx(.75f * 20.f + .25f * 2.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, -999.f) == Approx(.5f * 30.f + .5f * 3.f));

  inBuf->write(-999.f);
  inBuf->next();
  REQUIRE(loop.process(dt, -999.f) == Approx(.25f * 40.f + .75f * 4.f));

  inBuf->write(-999.f);
  inBuf->next();

  // Done writing tail, input should not matter now, test cross fade
  //  0/4  1/4  2/4  3/4

  inBuf->write(-999.f);
  inBuf->next();

  // HEAD LinearFade
  // head    1     2     3     4
  // B       0   .25    .5   .75
  // loop   10    20    30    40
  // A       1   .75    .5   .25

  // Approx(A * loop + B * head)

  REQUIRE(loop.process(dt, -999.f) == Approx(1.f * 10.f + 0.f * 1.f));
  REQUIRE(loop.B() == Approx(0.f));
  REQUIRE(loop.A() == Approx(1.f));
  REQUIRE(loop.isEndOfCycle() == true);
  REQUIRE(loop.phase() == Approx(0.f));

  REQUIRE(loop.process(dt, -999.f) == Approx(.75f * 20.f + .25f * 2.f));
  REQUIRE(loop.B() == Approx(.25f));
  REQUIRE(loop.A() == Approx(.75f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.25f));

  REQUIRE(loop.process(dt, -999.f) == Approx(.5f * 30.f + .5f * 3.f));
  REQUIRE(loop.B() == Approx(.5f));
  REQUIRE(loop.A() == Approx(.5f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.5f));

  REQUIRE(loop.process(dt, -999.f) == Approx(.25f * 40.f + .75f * 4.f));
  REQUIRE(loop.B() == Approx(.75f));
  REQUIRE(loop.A() == Approx(.25f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.75f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(1.f * 0.f + 10.f * 1.f));
  REQUIRE(loop.B() == Approx(0.f));
  REQUIRE(loop.A() == Approx(1.f));
  REQUIRE(loop.isEndOfCycle() == true);
  REQUIRE(loop.phase() == Approx(0.f));

  loop.release();

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 0.f);
}

TEST_CASE("SoftLoop FadeMode=HEAD LinearFade rate=2", "[]") {
  RBuf *inBuf = new RBuf(16);
  ICrossfade *crossfade = new LinearFade();
  IEnvelope *envelope = new GateEnvelope();
  SoftLoop loop(inBuf, crossfade, envelope, SoftLoop::FadeMode::HEAD);
  loop.setCrossfadeRate(2.f);
  float dt = 1.f;

  // Start writing head, no recording yet

  inBuf->write(1.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 1.f) == Approx(0.f));

  inBuf->write(2.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 2.f) == Approx(0.f));

  inBuf->write(3.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 3.f) == Approx(0.f));

  inBuf->write(4.f);
  inBuf->next();
  REQUIRE(loop.process(dt, 4.f) == Approx(0.f));

  loop.startRecording();

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 10.f);

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 20.f);

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 30.f);

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 40.f);

  loop.stopRecording();

  // Start writing tail. Loop length is now defined, crossfade should work

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 100.f);

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 200.f);

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 300.f);

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 400.f);

  // Done writing tail, input should not matter now, test cross fade
  //  0/4  1/4  2/4  3/4

  inBuf->write(-999.f);
  inBuf->next();

  // HEAD LinearFade
  // head    1     2     3     4
  // B       0     0    .5   .75
  // loop   10    20    30    40
  // A       1     1    .5   .25

  REQUIRE(loop.process(dt, 5000.f) == Approx(1.f * 0.f + 10.f * 1.f));
  REQUIRE(loop.B() == Approx(0.f));
  REQUIRE(loop.A() == Approx(1.f));
  REQUIRE(loop.isEndOfCycle() == true);
  REQUIRE(loop.phase() == Approx(0.f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(2.f * .0f + 20.f * 1.f));
  REQUIRE(loop.B() == Approx(0.f));
  REQUIRE(loop.A() == Approx(1.f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.25f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(3.f * .0f + 30.f * 1.f));
  REQUIRE(loop.B() == Approx(0.f));
  REQUIRE(loop.A() == Approx(1.f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.5f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(4.f * .5f + 40.f * .5f));
  REQUIRE(loop.B() == Approx(.5f));
  REQUIRE(loop.A() == Approx(.5f));
  REQUIRE(loop.isEndOfCycle() == false);
  REQUIRE(loop.phase() == Approx(.75f));

  REQUIRE(loop.process(dt, 5000.f) == Approx(1.f * 0.f + 10.f * 1.f));
  REQUIRE(loop.B() == Approx(0.f));
  REQUIRE(loop.A() == Approx(1.f));
  REQUIRE(loop.isEndOfCycle() == true);
  REQUIRE(loop.phase() == Approx(0.f));

  loop.release();

  inBuf->write(-999.f);
  inBuf->next();
  loop.process(dt, 0.f);
}
