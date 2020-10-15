#include "plugin.hpp"
#include <vector>

enum Mode {
  STOPPED,
  RECORDING,
  PLAYING,
  OVERDUBBING,
};

struct Loop {
private:
  std::vector<float> loop;
  unsigned int position = 0;
  unsigned int channels = 1;
  Mode mode = STOPPED;
  dsp::SlewLimiter inputSmoother;
  dsp::SlewLimiter outputSmoother;

public:
  Loop() {
    inputSmoother.setRiseFall(100.f, 50.f);
    outputSmoother.setRiseFall(100.f, 50.f);
  }

  void setChannels(int channels) {
    this->channels = channels;
  }

  Mode getMode() {
    return mode;
  }

  Mode getNextMode(bool overdubAfterRecord) {
    if (mode == STOPPED && loop.empty()) {
      return RECORDING;
    }
    if (mode == RECORDING) {
      return overdubAfterRecord ? OVERDUBBING : PLAYING;
    }
    if (mode == PLAYING) {
      return OVERDUBBING;
    }
    if (mode == OVERDUBBING) {
      return PLAYING;
    }
    if (mode == STOPPED && !loop.empty()) {
      return PLAYING;
    }
    return mode;
  }

  void toggle(bool overdubAfterRecord) {
    Mode next = getNextMode(overdubAfterRecord);
    if (mode == STOPPED && next == PLAYING) {
      position = 0;
    }
    mode = next;
  }

  void stop() {
    mode = STOPPED;
  }

  void erase() {
    mode = STOPPED;
    position = 0;
    loop.clear();
  }

  float *process(float deltaTime, float *in) {
    static float out[16];
    float inputEnv = inputSmoother.process(deltaTime, mode == RECORDING || mode == OVERDUBBING ? 1.f : 0.f);
    float outputEnv = outputSmoother.process(deltaTime, mode == STOPPED ? 0.f : 1.f);
    for (unsigned int c = 0; c < channels; c++) {
      if (mode == RECORDING) {
        loop.push_back(0.f);
      }
      if (loop.empty()) {
        out[c] = 0.f;
      } else {
        out[c] = outputEnv * loop[position];
        loop[position] += inputEnv * in[c];
        position++;
      }
    }
    if (position == loop.size()) {
      position = 0;
    }
    return out;
  }

  bool empty() {
    return loop.empty();
  }

  bool nearZero(float sampleTime, float tolerance) {
    return sampleTime * position / channels < tolerance;
  }
};
