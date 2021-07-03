#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "LoopController.hpp"

TEST_CASE("Inputs are marked as connected", "[]") {
  LoopController lc{1, 3};

  REQUIRE(lc.loops[0][0].inputConnected == false);
  REQUIRE(lc.loops[0][1].inputConnected == false);
  REQUIRE(lc.loops[0][2].inputConnected == false);

  lc.setInputsConnected(0, 1);

  REQUIRE(lc.loops[0][0].inputConnected == true);
  REQUIRE(lc.loops[0][1].inputConnected == false);
  REQUIRE(lc.loops[0][2].inputConnected == false);

  lc.setInputsConnected(0, 0);

  REQUIRE(lc.loops[0][0].inputConnected == false);
  REQUIRE(lc.loops[0][1].inputConnected == false);
  REQUIRE(lc.loops[0][2].inputConnected == false);
}

TEST_CASE("Returns are marked as connected", "[]") {
  LoopController lc{1, 3};

  REQUIRE(lc.loops[0][0].rtrnConnected == false);
  REQUIRE(lc.loops[0][1].rtrnConnected == false);
  REQUIRE(lc.loops[0][2].rtrnConnected == false);

  lc.setRtrnsConnected(0, 1);

  REQUIRE(lc.loops[0][0].rtrnConnected == true);
  REQUIRE(lc.loops[0][1].rtrnConnected == false);
  REQUIRE(lc.loops[0][2].rtrnConnected == false);

  lc.setRtrnsConnected(0, 0);

  REQUIRE(lc.loops[0][0].rtrnConnected == false);
  REQUIRE(lc.loops[0][1].rtrnConnected == false);
  REQUIRE(lc.loops[0][2].rtrnConnected == false);
}

TEST_CASE("Get number of output channels", "[]") {
  LoopController lc{2, 4};

  // Connect one (of four) channel on first port

  lc.setInputsConnected(0, 1);

  // Connect two (of four) channels on second port

  lc.setInputsConnected(1, 2);

  // Stop -> Record

  lc.toggle();

  // Step

  lc.setInput(0, 0, 0.101f);
  lc.setInput(1, 0, 1.101f);
  lc.setInput(1, 1, 1.201f);
  lc.process();

  // Step

  lc.setInput(0, 0, 0.102f);
  lc.setInput(1, 0, 1.102f);
  lc.setInput(1, 1, 1.202f);
  lc.process();

  // Record -> Play

  lc.toggle();

  // Disconnect inputs

  lc.setInputsConnected(0, 0);
  lc.setInputsConnected(1, 0);

  REQUIRE(lc.getChannels(0) == 1);
  REQUIRE(lc.getChannels(1) == 2);

  // Connect returns

  lc.setRtrnsConnected(1, 4);

  REQUIRE(lc.getChannels(0) == 1);
  REQUIRE(lc.getChannels(1) == 4);
}

TEST_CASE("Record, play, overdub", "[]") {
  LoopController lc{1, 1};

  REQUIRE(lc.mode == STOPPED);

  lc.setInputsConnected(0, 1);

  lc.toggle();
  REQUIRE(lc.mode == RECORDING);
  REQUIRE(lc.size == 0);

  lc.setInput(0, 0, 0.1f);
  lc.process();
  REQUIRE(lc.size == 1);
  REQUIRE(lc.loops[0][0].samples.size() == 1);
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.setInput(0, 0, 0.2f);
  lc.process();
  REQUIRE(lc.size == 2);
  REQUIRE(lc.loops[0][0].samples.size() == 2);
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.toggle();
  REQUIRE(lc.mode == PLAYING);

  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.1f));

  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.2f));

  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.1f));

  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.2f));

  lc.toggle();
  REQUIRE(lc.mode == OVERDUBBING);

  lc.setInput(0, 0, 0.01f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.1f));

  lc.setInput(0, 0, 0.02f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.2f));

  lc.toggle();
  REQUIRE(lc.mode == PLAYING);

  lc.setInput(0, 0, 1000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.11f));

  lc.setInput(0, 0, 2000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.22f));

  lc.setInput(0, 0, 0.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.11f));

  lc.setInput(0, 0, 0.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.22f));

  /**
   * Connect a return input.
   * The signal from a connected return will replace the loop content while
   * it's connected and the loop is not stopped. The return should have no
   * effect while the loop is stopped.
   */

  lc.setRtrnsConnected(0, 1);

  lc.setInput(0, 0, 1000.0f);
  lc.setRtrn(0, 0, 1.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.11f));

  lc.setInput(0, 0, 2000.0f);
  lc.setRtrn(0, 0, 2.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.22f));

  lc.setInput(0, 0, 1000.0f);
  lc.setRtrn(0, 0, 10.0f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(1.0f));

  lc.setInput(0, 0, 2000.0f);
  lc.setRtrn(0, 0, 20.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(2.0f));

  /**
   * Disconnect the return input.
   */

  lc.setRtrnsConnected(0, 0);

  lc.setInput(0, 0, 1000.0f);
  lc.setRtrn(0, 0, 100.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(10.0f));

  lc.setInput(0, 0, 2000.0f);
  lc.setRtrn(0, 0, 200.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(20.0f));

  lc.setInput(0, 0, 1000.0f);
  lc.setRtrn(0, 0, 0.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(10.0f));

  lc.setInput(0, 0, 2000.0f);
  lc.setRtrn(0, 0, 0.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(20.0f));

  lc.stop();
  REQUIRE(lc.mode == STOPPED);

  lc.setInput(0, 0, 10000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.setInput(0, 0, 20000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.setInput(0, 0, 10000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.toggle();
  REQUIRE(lc.mode == PLAYING);

  lc.setInput(0, 0, 20000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(20.0f));

  lc.setInput(0, 0, 10000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(10.0f));

  lc.setInput(0, 0, 20000.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(20.0f));

  lc.setRtrnsConnected(0, 1);

  lc.stop();
  REQUIRE(lc.mode == STOPPED);

  lc.setInput(0, 0, 1000.0f);
  lc.setRtrn(0, 0, 1110.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.setInput(0, 0, 2000.0f);
  lc.setRtrn(0, 0, 2200.0f); // Bug: this is being recorded while stopped
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.setInput(0, 0, 1000.1f);
  lc.setRtrn(0, 0, 1111.1f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  lc.toggle();
  REQUIRE(lc.mode == PLAYING);

  lc.setRtrnsConnected(0, 0);

  lc.setInput(0, 0, 2000.0f);
  lc.setRtrn(0, 0, 2220.0f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(20.0f));

  lc.setInput(0, 0, 1000.1f);
  lc.setRtrn(0, 0, 1100.1f);
  lc.process();
  REQUIRE(lc.getOutput(0, 0) == Approx(10.0f));
}

TEST_CASE("Recordings started at different times should be synchronized", "[]") {
  LoopController lc{1, 2};

  // Connect first input

  lc.setInputsConnected(0, 1);

  REQUIRE(lc.mode == STOPPED);

  // Stop -> Record

  lc.toggle();

  REQUIRE(lc.mode == RECORDING);

  // Step

  lc.setInput(0, 0, 1.1f);
  lc.process();

  /**       a
   *        0
   *  A|  1.1
   */

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0));
  REQUIRE(lc.getOutput(0, 1) == Approx(0.0));

  // Step

  lc.setInput(0, 0, 1.2f);
  lc.process();

  /**            a
   *        0    1
   *  A|  1.1  1.2
   */

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0));
  REQUIRE(lc.getOutput(0, 1) == Approx(0.0));

  REQUIRE(lc.loops[0][1].inputConnected == false);

  // Conect second input while recording

  lc.setInputsConnected(0, 2);

  // Step

  lc.setInput(0, 0, 1.3f);
  lc.setInput(0, 1, 2.1f);
  lc.process();

  /**                 a
   *                  b 
   *        0    1    2
   *  A|  1.1  1.2  1.3
   *  B|            2.1
   */

  REQUIRE(lc.size == 3);
  REQUIRE(lc.position == 2);

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0));
  REQUIRE(lc.getOutput(0, 1) == Approx(0.0));

  // Step

  lc.setInput(0, 0, 1.4f);
  lc.setInput(0, 1, 2.2f);
  lc.process();

  /**                      a
   *                       b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2
   */

  REQUIRE(lc.size == 4);
  REQUIRE(lc.position == 3);

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0));
  REQUIRE(lc.getOutput(0, 0) == Approx(0.0));

  // Record -> Play

  lc.toggle();

  REQUIRE(lc.mode == PLAYING);

  // Step

  lc.setInput(0, 0, 1000.0f);
  lc.setInput(0, 1, 2000.0f);
  lc.process();

  /**       a                   b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0
   */

  REQUIRE(lc.position == 0);
  REQUIRE(lc.size == 4);

  REQUIRE(lc.getOutput(0, 0) == Approx(1.1));
  REQUIRE(lc.getOutput(0, 1) == Approx(0.0));

  // Step

  lc.setInput(0, 0, 1000.0f);
  lc.setInput(0, 1, 2000.0f);
  lc.process();

  /**            a                   b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0  0.0
   */

  REQUIRE(lc.position == 1);
  REQUIRE(lc.size == 4);

  REQUIRE(lc.getOutput(0, 0) == Approx(1.2));
  REQUIRE(lc.getOutput(0, 1) == Approx(0.0));

  // Step

  lc.setInput(0, 0, 1000.0f);
  lc.setInput(0, 1, 2000.0f);
  lc.process();

  /**                 a
   *                  b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0  0.0
   */

  REQUIRE(lc.position == 2);
  REQUIRE(lc.size == 4);

  REQUIRE(lc.getOutput(0, 0) == Approx(1.3f));
  REQUIRE(lc.getOutput(0, 1) == Approx(2.1f));

  // Step

  lc.setInput(0, 0, 1000.0f);
  lc.setInput(0, 1, 2000.0f);
  lc.process();

  /**                      a
   *                       b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0  0.0
   */

  REQUIRE(lc.position == 3);
  REQUIRE(lc.size == 4);

  REQUIRE(lc.getOutput(0, 0) == Approx(1.4f));
  REQUIRE(lc.getOutput(0, 1) == Approx(2.2f));

  // Play -> Stop

  lc.stop();

  REQUIRE(lc.mode == STOPPED);

  // Step

  lc.setInput(0, 0, 1000.0f);
  lc.setInput(0, 1, 2000.0f);
  lc.process();

  /**       a
   *                            b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0  0.0
   */

  REQUIRE(lc.position == 0);
  REQUIRE(lc.size == 4);

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));
  REQUIRE(lc.getOutput(0, 1) == Approx(0.0f));

  // Stop -> Play

  lc.toggle();

  /**       a
   *                  b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0  0.0
   */

  REQUIRE(lc.mode == PLAYING);
  REQUIRE(lc.position == 0);
  REQUIRE(lc.size == 4);

  // Step

  lc.setInput(0, 0, 1000.0f);
  lc.setInput(0, 1, 2000.0f);
  lc.process();

  /**
   * THIS IS A BUG
   * (Stop -> Play) skips first sample
   *        a
   *                  b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  0.0  0.0
   */

  // When fixed these cases should pass:
  // REQUIRE(lc.getOutput(0, 0) == Approx(1.1f));
  // REQUIRE(lc.getOutput(0, 1) == Approx(2.1f));
}

TEST_CASE("Erase", "[]") {
  LoopController lc{1, 1};

  // Connect one input

  lc.setInputsConnected(0, 1);

  // Stop -> Record

  lc.toggle();

  // Step

  lc.setInput(0, 0, 1.1f);
  lc.process();

  // Step

  lc.setInput(0, 0, 1.2f);
  lc.process();

  // Record -> Play

  lc.toggle();

  lc.setInput(0, 0, -10.1f);

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(1.1f));

  // Erase

  lc.erase();

  REQUIRE(lc.mode == STOPPED);
  REQUIRE(lc.position == 0);
  REQUIRE(lc.size == 0);

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(0.0f));
}

TEST_CASE("Mix", "[]") {
  LoopController lc{1, 1};

  // Set equal mix of input and loop

  lc.mix = 0.0f;

  // Connect one input

  lc.setInputsConnected(0, 1);

  // Step

  lc.setInput(0, 0, 100.0f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(100.0f));

  // Stop -> Record

  lc.toggle();

  // Step

  lc.setInput(0, 0, 1.1f);
  lc.process();

  // Step

  lc.setInput(0, 0, 1.2f);
  lc.process();

  // Record -> Play

  lc.toggle();

  // Step

  lc.setInput(0, 0, -10.1f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(-9.0f));

  // Step

  lc.setInput(0, 0, -20.2f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(-19.0f));
}

TEST_CASE("Return bypass", "[]") {
  LoopController lc{1, 1};

  // Connect one input

  lc.setInputsConnected(0, 1);

  // Stop -> Record

  lc.toggle();

  // Step 1

  lc.setInput(0, 0, 1.1f);
  lc.process();

  // Connect one return input

  lc.setRtrnsConnected(0, 1);

  // Step 2

  lc.setInput(0, 0, 1.2f);
  lc.setRtrn(0, 0, 10.2f);
  lc.process();

  // Record -> Play

  lc.toggle();

  // Step 1

  lc.setInput(0, 0, 1.11f);
  lc.setRtrn(0, 0, 10.11f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(1.1));

  // Bypass return

  lc.rtrnEnabled = false;

  // Step 2

  lc.setInput(0, 0, 100.2f);
  lc.setRtrn(0, 0, 1000.2f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(11.4));

  // Step 1

  lc.setInput(0, 0, 0.0f);
  lc.setRtrn(0, 0, 1000.0f);
  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(10.11));

  lc.setInput(0, 0, 0.0f);
  lc.setRtrn(0, 0, 0.0f);

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(11.4));

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(10.11));
}

TEST_CASE("Feedback", "[]") {
  LoopController lc{1, 1};

  lc.feedback = 0.5f;

  // Connect one input

  lc.setInputsConnected(0, 1);

  // Stop -> Record

  lc.toggle();

  // Step

  lc.setInput(0, 0, 1.1f);
  lc.process();

  // Step

  lc.setInput(0, 0, 1.2f);
  lc.process();

  // Record -> Play

  lc.toggle();
  lc.setInput(0, 0, 0.0f);

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(1.1f));

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(1.2f));

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(0.55f));

  // Step

  lc.process();

  REQUIRE(lc.getOutput(0, 0) == Approx(0.6f));
}
