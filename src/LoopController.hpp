#include "Smoother.hpp"

enum Mode {
  STOPPED,
  RECORDING,
  PLAYING,
  OVERDUBBING,
};

enum Order {
  RECORD_PLAY_OVERDUB,
  RECORD_OVERDUB_PLAY,
};

struct LoopController {
  struct Loop {
    std::vector<float> samples;

    int start = -1;

    bool inputConnected = false;
    bool rtrnConnected = false;

    float input = 0.0f;
    float rtrn = 0.0f;

    float send = 0.0f;
    float output = 0.0f;

    Smoother inSmoother;
    Smoother outSmoother;
    Smoother crossSmoother;

    void erase() {
      output = 0.0f;
      send = 0.0f;
      start = -1;
      samples.clear();
    }

    bool hasOutput() {
      return inputConnected || rtrnConnected || samples.size() > 0;
    }

    void process(LoopController &lc, float sampleTime) {
      output = lc.mix > 0 ? 1 - lc.mix : 1 * input;

      bool connected = inputConnected || rtrnConnected;

      if ((connected || samples.size() > 0) && samples.size() < lc.size)
        samples.push_back(0.0f);

      if (start == -1 && samples.size() == 1) {
        start = lc.position;
      }

      if (samples.size() > 0) {
        int position = (((lc.position - start) % lc.size) + lc.size) % lc.size;
        float sample = samples[position];

        float inGate = inSmoother.process(sampleTime, lc.mode == RECORDING || lc.mode == OVERDUBBING ? 1.f : 0.f);
        float outGate = outSmoother.process(sampleTime, lc.mode == STOPPED ? 0.f : 1.f);
        float loopLevel = lc.mix > 0 ? 1 : 1 + lc.mix;

        // Set output

        output = output + loopLevel * outGate * sample;
        send = outGate * sample;

        // Update sample

        bool shouldRtrn = lc.rtrnEnabled && rtrnConnected && lc.mode != STOPPED;
        float cross = crossSmoother.process(sampleTime, shouldRtrn ? 0.0f : 1.0f);

        float fb = lc.mode == STOPPED ? 1.0f : lc.feedback;
        float newSample = sample * cross + rtrn * (1 - cross);
        samples[position] = fb * newSample + inGate * input;
      }
    }
  };

  std::vector<std::vector<Loop>> loops;

  int ports;
  int channels;

  Mode mode = STOPPED;
  int size = 0;
  int position = -1;

  float feedback = 1.0f;
  float mix = 1.0f;
  bool rtrnEnabled = true;

  LoopController(int ports, int channels) {
    this->ports = ports;
    this->channels = channels;

    loops.resize(ports);

    for (int i = 0; i < ports; i++) {
      loops[i].resize(channels);
    }
  }

  void setInputsConnected(int port, int channels) {
    for (int c = 0; c < this->channels; c++) {
      loops[port][c].inputConnected = c < channels;
    }
  }

  void setRtrnsConnected(int port, int channels) {
    for (int c = 0; c < this->channels; c++) {
      loops[port][c].rtrnConnected = c < channels;
    }
  }

  int getChannels(int port) {
    int n = 0;

    for (size_t c = 0; c < channels; c++) {
      if (loops[port][c].hasOutput())
        n++;
    }

    return n;
  }

  Mode getNextMode(Order order) {
    if (mode == STOPPED && size == 0) {
      return RECORDING;
    }
    if (mode == RECORDING && order == RECORD_PLAY_OVERDUB) {
      return PLAYING;
    }
    if (mode == RECORDING && order == RECORD_OVERDUB_PLAY) {
      return OVERDUBBING;
    }
    if (mode == PLAYING) {
      return OVERDUBBING;
    }
    if (mode == OVERDUBBING) {
      return PLAYING;
    }
    if (mode == STOPPED && size > 0) {
      return PLAYING;
    }
    return mode;
  }

  void toggle(Order order = RECORD_PLAY_OVERDUB) {
    Mode nextMode = getNextMode(order);

    if (mode == STOPPED && nextMode == PLAYING)
      position = 0;

    mode = nextMode;
  }

  void stop() {
    mode = STOPPED;
  }

  void erase() {
    mode = STOPPED;
    position = 0;
    size = 0;

    for (int p = 0; p < ports; p++) {
      for (int c = 0; c < channels; c++) {
        loops[p][c].erase();
      }
    }
  }

  void setInput(int port, int channel, float value) {
    loops[port][channel].input = value;
  }

  void setRtrn(int port, int channel, float value) {
    loops[port][channel].rtrn = value;
  }

  void process(float sampleTime = 1.0f) {
    if (mode == RECORDING) {
      size++;
    }

    if (size > 0) {
      position++;
    }

    if (position == size) {
      position = 0;
    }

    for (int p = 0; p < ports; p++) {
      for (int c = 0; c < channels; c++) {
        loops[p][c].process(*this, sampleTime);
      }
    }
  }

  float getOutput(int port, int channel) {
    return loops[port][channel].output;
  }

  float getSend(int port, int channel) {
    return loops[port][channel].send;
  }
};
