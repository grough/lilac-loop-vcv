#pragma once

#include "catch.hpp"
#include "AudioFile.h"
#include "common.hpp"
#include "engine.hpp"
#include "MultiLoopReader.hpp"
#include "MultiLoopWriter.hpp"

TEST_CASE("Record and play back a loop with multiple ports and channels", "[]") {
  MultiLoop ml;

  // Set ports (left, right)

  ml.resize(2);

  // Set channels per port

  ml.setChannels(0, 2);
  ml.setChannels(1, 2);

  // Record 1

  ml.next(true);

  REQUIRE(ml.read(0, 0) == Approx(0.0f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));
  REQUIRE(ml.read(1, 0) == Approx(0.0f));
  REQUIRE(ml.read(1, 1) == Approx(0.0f));

  ml.write(0, 0, 1.1f);
  ml.write(0, 1, 2.1f);
  ml.write(1, 0, 3.1f);
  ml.write(1, 1, 4.1f);

  // Record 2

  ml.next(true);

  REQUIRE(ml.read(0, 0) == Approx(0.0f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));
  REQUIRE(ml.read(1, 0) == Approx(0.0f));
  REQUIRE(ml.read(1, 1) == Approx(0.0f));

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);
  ml.write(1, 1, 4.2f);

  // Play 1

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.1f));
  REQUIRE(ml.read(0, 1) == Approx(2.1f));
  REQUIRE(ml.read(1, 0) == Approx(3.1f));
  REQUIRE(ml.read(1, 1) == Approx(4.1f));

  // Play 2

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.2f));
  REQUIRE(ml.read(0, 1) == Approx(2.2f));
  REQUIRE(ml.read(1, 0) == Approx(3.2f));
  REQUIRE(ml.read(1, 1) == Approx(4.2f));

  // Play 1

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.1f));
  REQUIRE(ml.read(0, 1) == Approx(2.1f));
  REQUIRE(ml.read(1, 0) == Approx(3.1f));
  REQUIRE(ml.read(1, 1) == Approx(4.1f));

  // Play 2

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.2f));
  REQUIRE(ml.read(0, 1) == Approx(2.2f));
  REQUIRE(ml.read(1, 0) == Approx(3.2f));
  REQUIRE(ml.read(1, 1) == Approx(4.2f));
}

TEST_CASE("Recordings started at different times should be synchronized", "[]") {
  MultiLoop ml;

  ml.resize(1);
  ml.setChannels(0, 2);

  // Record 1

  ml.next(true);

  REQUIRE(ml.read(0, 0) == Approx(0.0f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));

  ml.write(0, 0, 1.1f);

  /**       a
   *        0
   *  A|  1.1
   */

  // Record 2

  ml.next(true);

  REQUIRE(ml.read(0, 0) == Approx(0.0f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));

  ml.write(0, 0, 1.2f);

  /**            a
   *        0    1
   *  A|  1.1  1.2
   */

  // Conect second input while recording

  ml.setChannels(0, 2);

  // Record 3

  ml.next(true);

  REQUIRE(ml.read(0, 0) == Approx(0.0f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));

  ml.write(0, 0, 1.3f);
  ml.write(0, 1, 2.1f);

  /**                 a
   *                  b
   *        0    1    2
   *  A|  1.1  1.2  1.3
   *  B|            2.1
   */

  // Record 4

  ml.next(true);

  REQUIRE(ml.read(0, 0) == Approx(0.0f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));

  ml.write(0, 0, 1.4f);
  ml.write(0, 1, 2.2f);

  /**                      a
   *                       b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2
   */

  // Play 1

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.1f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));

  ml.write(0, 1, 2.3);

  /**       a                   b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3
   */

  // Play 3

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.2f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));

  ml.write(0, 1, 2.4);

  /**            a                   b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3  2.4
   */

  // Play 4

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.3f));
  REQUIRE(ml.read(0, 1) == Approx(2.1f));

  /**                 a
   *                  b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3  2.4
   */

  // Play 4

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.4f));
  REQUIRE(ml.read(0, 1) == Approx(2.2f));

  /**                      a
   *                       b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3  2.4
   */

  // Play 1

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.1f));
  REQUIRE(ml.read(0, 1) == Approx(2.3f));

  /**       a
   *                            b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3  2.4
   */

  // Play 2

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.2f));
  REQUIRE(ml.read(0, 1) == Approx(2.4f));

  /**            a
   *                                 b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3  2.4
   */

  // Rewind

  ml.rewind();

  REQUIRE(ml.read(0, 0) == Approx(1.1f));
  REQUIRE(ml.read(0, 1) == Approx(2.3f));

  /**       a
   *                  b
   *        0    1    2    3
   *  A|  1.1  1.2  1.3  1.4
   *  B|            2.1  2.2  2.3  2.4
   */
}

TEST_CASE("Write with zero length", "[]") {
  MultiLoop ml;

  ml.resize(1);
  ml.setChannels(0, 1);

  ml.write(0, 0, 10.0f);
}

TEST_CASE("Erase a specific channel", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(0, 2);
  ml.setChannels(1, 1);

  ml.next(true);

  ml.write(0, 0, 1.1f);
  ml.write(0, 1, 2.1f);
  ml.write(1, 0, 3.1f);

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);

  REQUIRE(ml.loops[0][0].size() == 2);
  REQUIRE(ml.loops[0][1].size() == 2);
  REQUIRE(ml.loops[1][0].size() == 2);

  // Erase second channel (all ports)

  ml.erase(1);

  REQUIRE(ml.length() == 2);

  REQUIRE(ml.loops[0][0].size() == 2);
  REQUIRE(ml.loops[0][1].size() == 0);
  REQUIRE(ml.loops[1][0].size() == 2);

  ml.next();

  REQUIRE(ml.loops[0][0].size() == 2);
  REQUIRE(ml.loops[0][1].size() == 1);

  REQUIRE(ml.read(0, 0) == Approx(1.1f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));
  REQUIRE(ml.read(1, 0) == Approx(3.1f));

  ml.next();

  REQUIRE(ml.read(0, 0) == Approx(1.2f));
  REQUIRE(ml.read(0, 1) == Approx(0.0f));
  REQUIRE(ml.read(1, 0) == Approx(3.2f));

  REQUIRE(ml.loops[0][0].size() == 2);
  REQUIRE(ml.loops[0][1].size() == 2);

  // Erase first channel on both ports

  ml.erase(0);

  REQUIRE(ml.length() == 0);

  ml.read(0, 0);
  ml.read(0, 1);
  ml.read(1, 0);

  ml.write(0, 0, 10.0f);
  // ml.write(0, 1, 10.0f); // malloc: Incorrect checksum for freed object
  // ml.write(1, 0, 10.0f);

  // ml.next();

  // ml.read(0, 0);
  // ml.read(0, 1);
  // ml.read(1, 0);

  // ml.write(0, 0, 10.0f);
  // ml.write(0, 1, 10.0f);
  // ml.write(1, 0, 10.0f);
}

TEST_CASE("Write to and read from a linear multi-track AudioBuffer", "[]") {
  MultiLoop ml;

  ml.resize(3);
  ml.setChannels(0, 1);
  ml.setChannels(1, 0);
  ml.setChannels(2, 2);

  ml.next(true);

  ml.write(0, 0, 1.1f);
  ml.write(2, 0, 2.1f);
  ml.write(2, 1, 3.1f);

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(2, 0, 2.2f);
  ml.write(2, 1, 3.2f);

  ml.rewind();

  MultiLoopWriter writer;

  AudioFile<float>::AudioBuffer buffer = writer.makeLinearMultiTrackBuffer(ml);

  REQUIRE(buffer.size() == 3);

  REQUIRE(buffer[0].size() == 2);
  REQUIRE(buffer[1].size() == 2);
  REQUIRE(buffer[2].size() == 2);

  REQUIRE(buffer[0][0] == Approx(1.1f / 10));
  REQUIRE(buffer[1][0] == Approx(2.1f / 10));
  REQUIRE(buffer[2][0] == Approx(3.1f / 10));

  REQUIRE(buffer[0][1] == Approx(1.2f / 10));
  REQUIRE(buffer[1][1] == Approx(2.2f / 10));
  REQUIRE(buffer[2][1] == Approx(3.2f / 10));

  std::vector<int> layout = {ml.getChannels(0), ml.getChannels(1), ml.getChannels(2)};

  REQUIRE(layout[0] == 1);
  REQUIRE(layout[1] == 0);
  REQUIRE(layout[2] == 2);

  MultiLoopReader reader;
  MultiLoop ml2 = reader.fromLinearMultiTrackBuffer(buffer, layout);

  REQUIRE(ml2.getChannels(0) == 1);
  REQUIRE(ml2.getChannels(1) == 0);
  REQUIRE(ml2.getChannels(2) == 2);

  REQUIRE(ml2.loops[0][0].size() == 2);
  REQUIRE(ml2.loops[2][0].size() == 2);
  REQUIRE(ml2.loops[2][1].size() == 2);

  REQUIRE(ml2.loops.size() == 3);

  REQUIRE(ml2.loops[0].size() == 1);
  REQUIRE(ml2.loops[1].size() == 0);
  REQUIRE(ml2.loops[2].size() == 2);

  REQUIRE(ml2.read(0, 0) == Approx(1.1f));
  REQUIRE(ml2.read(2, 0) == Approx(2.1f));
  REQUIRE(ml2.read(2, 1) == Approx(3.1f));

  ml2.next();

  REQUIRE(ml2.read(0, 0) == Approx(1.2f));
  REQUIRE(ml2.read(2, 0) == Approx(2.2f));
  REQUIRE(ml2.read(2, 1) == Approx(3.2f));
}

TEST_CASE("Format polyphonic loop as a multi-track AudioBuffer", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(0, 2);
  ml.setChannels(1, 2);

  ml.next(true);

  ml.write(0, 0, 1.1f);
  ml.write(0, 1, 2.1f);
  ml.write(1, 0, 3.1f);
  ml.write(1, 1, 4.1f);

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);
  ml.write(1, 1, 4.2f);

  ml.rewind();

  MultiLoopWriter writer;

  AudioFile<float>::AudioBuffer buffer = writer.makeMultiTrackBuffer(ml);

  REQUIRE(buffer[0][0] == Approx(1.1f / 10));
  REQUIRE(buffer[1][0] == Approx(2.1f / 10));
  REQUIRE(buffer[2][0] == Approx(3.1f / 10));
  REQUIRE(buffer[3][0] == Approx(4.1f / 10));

  REQUIRE(buffer[0][1] == Approx(1.2f / 10));
  REQUIRE(buffer[1][1] == Approx(2.2f / 10));
  REQUIRE(buffer[2][1] == Approx(3.2f / 10));
  REQUIRE(buffer[3][1] == Approx(4.2f / 10));
}

TEST_CASE("Sum a polyphonic loop down to a mono AudioBuffer", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(1, 2);

  ml.next(true);

  ml.write(1, 0, 1.1f);
  ml.write(1, 1, 2.1f);

  ml.next(true);

  ml.write(1, 0, 1.2f);
  ml.write(1, 1, 2.2f);

  ml.rewind();

  MultiLoopWriter writer;

  AudioFile<float>::AudioBuffer buffer = writer.makeSummedBuffer(ml);

  REQUIRE(buffer.size() == 1);
  REQUIRE(buffer[0].size() == ml.size);

  REQUIRE(buffer[0][0] == Approx(3.2f / 10));
  REQUIRE(buffer[0][1] == Approx(3.4f / 10));
}

TEST_CASE("Sum a polyphonic loop down to a stereo AudioBuffer", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(0, 2);
  ml.setChannels(1, 2);

  ml.next(true);

  ml.write(0, 0, 1.1f);
  ml.write(0, 1, 2.1f);
  ml.write(1, 0, 3.1f);
  ml.write(1, 1, 4.1f);

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);
  ml.write(1, 1, 4.2f);

  ml.rewind();

  MultiLoopWriter writer;

  AudioFile<float>::AudioBuffer buffer = writer.makeSummedBuffer(ml);

  REQUIRE(buffer.size() == 2);
  REQUIRE(buffer[0].size() == ml.size);
  REQUIRE(buffer[1].size() == ml.size);

  REQUIRE(buffer[0][0] == Approx(3.2f / 10));
  REQUIRE(buffer[1][0] == Approx(7.2f / 10));

  REQUIRE(buffer[0][1] == Approx(3.4f / 10));
  REQUIRE(buffer[1][1] == Approx(7.4f / 10));
}

TEST_CASE("Handle partially recorded track when creating AudioBuffer", "[]") {
  MultiLoop ml;

  ml.resize(1);
  ml.setChannels(0, 1);

  ml.next(true);
  ml.write(0, 0, 1.1f);

  ml.next(true);
  ml.write(0, 0, 1.2f);

  ml.setChannels(0, 2);

  ml.next(true);
  ml.write(0, 0, 1.3f);
  ml.write(0, 1, 2.3f);

  ml.next(true);
  ml.write(0, 0, 1.4f);
  ml.write(0, 1, 2.4f);

  ml.rewind();

  MultiLoopWriter writer;

  AudioFile<float>::AudioBuffer buffer = writer.makeMultiTrackBuffer(ml);

  REQUIRE(buffer.size() == 2);
  REQUIRE(buffer[0].size() == ml.size);
  REQUIRE(buffer[1].size() == ml.size);

  REQUIRE(buffer[0][0] == Approx(1.1f / 10));
  REQUIRE(buffer[1][0] == Approx(0.0f));

  REQUIRE(buffer[0][1] == Approx(1.2f / 10));
  REQUIRE(buffer[1][1] == Approx(0.0f));

  REQUIRE(buffer[0][2] == Approx(1.3f / 10));
  REQUIRE(buffer[1][2] == Approx(2.3f / 10));

  REQUIRE(buffer[0][3] == Approx(1.4f / 10));
  REQUIRE(buffer[1][3] == Approx(2.4f / 10));
}

TEST_CASE("Format a polyphonic loop as separate stereo buffers", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(0, 2);
  ml.setChannels(1, 2);

  ml.next(true);

  ml.write(0, 0, 1.1f); // left 1
  ml.write(0, 1, 2.1f); // left 2
  ml.write(1, 0, 3.1f); // right 1
  ml.write(1, 1, 4.1f); // right 2

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);
  ml.write(1, 1, 4.2f);

  ml.rewind();

  MultiLoopWriter writer;

  std::vector<AudioFile<float>::AudioBuffer> buffers = writer.makeSeparateBuffers(ml);

  REQUIRE(buffers.size() == 2);

  REQUIRE(buffers[0].size() == 2);
  REQUIRE(buffers[1].size() == 2);

  REQUIRE(buffers[0][0].size() == 2);
  REQUIRE(buffers[0][1].size() == 2);
  REQUIRE(buffers[1][0].size() == 2);
  REQUIRE(buffers[1][1].size() == 2);

  // File 1
  REQUIRE(buffers[0][0][0] == Approx(1.1f / 10));
  REQUIRE(buffers[0][1][0] == Approx(3.1f / 10));
  REQUIRE(buffers[0][0][1] == Approx(1.2f / 10));
  REQUIRE(buffers[0][1][1] == Approx(3.2f / 10));

  // File 2
  REQUIRE(buffers[1][0][0] == Approx(2.1f / 10));
  REQUIRE(buffers[1][1][0] == Approx(4.1f / 10));
  REQUIRE(buffers[1][0][1] == Approx(2.2f / 10));
  REQUIRE(buffers[1][1][1] == Approx(4.2f / 10));
}

// TEST_CASE("Format a polyphonic loop as separate mono buffers", "[.]") {}

// TEST_CASE("Format a polyphonic loop when L/R channels are not equal", "[.]") {}

TEST_CASE("Read a loop from an AudioBuffer", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(0, 2);
  ml.setChannels(1, 2);

  ml.next(true);

  ml.write(0, 0, 1.1f);
  ml.write(0, 1, 2.1f);
  ml.write(1, 0, 3.1f);
  ml.write(1, 1, 4.1f);

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);
  ml.write(1, 1, 4.2f);

  ml.rewind();

  MultiLoopWriter writer;
  AudioFile<float>::AudioBuffer buffer = writer.makeMultiTrackBuffer(ml);

  REQUIRE(buffer.size() == 4);
  REQUIRE(buffer[0].size() == 2);

  MultiLoopReader reader;
  MultiLoop ml2 = reader.fromBuffer(buffer);

  REQUIRE(ml2.read(0, 0) == Approx(1.1f));
  ml2.next();
  REQUIRE(ml2.read(0, 0) == Approx(1.2f));
  ml2.next();
  REQUIRE(ml2.read(0, 0) == Approx(1.1f));
}

TEST_CASE("Write a file", "[]") {
  MultiLoop ml;

  ml.resize(2);
  ml.setChannels(0, 2);
  ml.setChannels(1, 2);

  ml.next(true);

  ml.write(0, 0, 1.1f);
  ml.write(0, 1, 2.1f);
  ml.write(1, 0, 3.1f);
  ml.write(1, 1, 4.1f);

  ml.next(true);

  ml.write(0, 0, 1.2f);
  ml.write(0, 1, 2.2f);
  ml.write(1, 0, 3.2f);
  ml.write(1, 1, 4.2f);

  ml.rewind();

  MultiLoopWriter writer;
  AudioFile<float>::AudioBuffer buffer = writer.makeMultiTrackBuffer(ml);

  AudioFile<float> file;
  file.setBitDepth(16);
  file.setSampleRate(44100);
  file.setAudioBuffer(buffer);
  file.save("./test/output.wav", AudioFileFormat::Wave);
}
