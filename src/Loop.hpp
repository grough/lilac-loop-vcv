#include <iostream>

struct Loop {
  bool inputConnected = false;
  bool returnConnected = false;
  float input = 0.0f;
  float rtrn = 0.0f;
  float feedback = 1.0f;
  float mix = 0.0f;

  Loop() {}

  void setInputConnected(bool connected) {
    inputConnected = connected;
  }

  void setReturnConnected(bool connected) {
    returnConnected = connected;
  }

  void arm() {
  }

  void toggle() {
  }

  void stop() {
  }

  void erase() {
  }

  void process() {
  }

  float getOutput() {
    return 0.0f;
  }

  float getSend() {
    return 0.0f;
  }
};

struct Loops {
  std::vector<std::vector<Loop>> loops;

  Loops(int ports, int channels) {
    loops.resize(ports);

    for (int i = 0; i < loops.size(); i++) {
      loops[i].resize(channels);
    }
  }

  //=============================================================
  // Mark input connection for each channel

  void setInputConnected(int port, int channels) {
    for (int c = 0; c < 16; c++) {
      loops[port][c].setInputConnected(c < channels);
    }
  }

  //=============================================================
  // Mark return connection for each channel

  void setReturnConnected(int port, int channels) {
    for (int c = 0; c < 16; c++) {
      loops[port][c].setReturnConnected(c < channels);
    }
  }

  //=============================================================
  // Toggle is monophonic. All channels are toggled together

  void arm() {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
        loops[p][c].arm();
      }
    }
  }

  //=============================================================
  // Arm is monophonic. All channels are armed together

  void toggle() {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
        loops[p][c].toggle();
      }
    }
  }

  //=============================================================
  // Stop is monophonic. All channels are stopped together

  void stop() {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
        loops[p][c].stop();
      }
    }
  }

  //=============================================================
  // All channels can be erased at once

  void erase() {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
        loops[p][c].erase();
      }
    }
  }

  //=============================================================
  // Channel pairs can be erased polyphonically

  void erase(int channel) {
    for (int p = 0; p < 2; p++) {
      loops[p][channel].erase();
    }
  }

  //=============================================================
  // Feedback control is monophonic

  void setFeedback(float value) {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
        loops[p][c].feedback = value;
      }
    }
  }

  //=============================================================
  // Mix control is monophonic

  void setMix(float value) {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
        loops[p][c].mix = value;
      }
    }
  }

  //=============================================================
  // Input signal is set per channel

  void setInput(int port, int channel, float value) {
    loops[port][channel].input = value;
  }

  //=============================================================
  // Return signal is set per channel

  void setReturn(int port, int channel, float value) {
    loops[port][channel].rtrn = value;
  }

  //=============================================================
  // Step all loop channels forward together

  void process() {
    for (int p = 0; p < 2; p++) {
      for (int c = 0; c < 16; c++) {
      }
    }
  }

  //=============================================================
  // Get a channel's output value

  float getOutput(int port, int channel) {
    return loops[port][channel].getOutput();
  }

  //=============================================================
  // Get a channel's send value

  float getSend(int port, int channel) {
    return loops[port][channel].getSend();
  }
};
