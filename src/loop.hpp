#include "plugin.hpp"
#include <vector>

struct Recorder {
private:
public:
  unsigned int position = 0;
  bool growing = false;
  std::vector<float> loop;

  void start() {
    growing = true;
  }

  void end() {
    growing = false;
  }

  bool hasRecording() {
    return !loop.empty();
  }

  void erase() {
    loop.clear();
    growing = false;
    position = 0;
  }

  float process(float voltage) {
    if (growing) {
      loop.push_back(0.f);
    }
    if (loop.empty()) {
      return 0.f;
    }
    if (position == loop.size()) {
      position = 0;
    }
    float output = loop[position];
    loop[position] += voltage;
    ++position;
    return output;
  }
};

enum Mode {
  STOPPED,
  RECORDING,
  PLAYING,
  OVERDUBBING,
};

struct Loop {
private:
  Mode mode = STOPPED;
  int position = 0;
  int channels = 1;
  Recorder recorder;
  dsp::SlewLimiter inputSmoother;
  dsp::SlewLimiter outputSmoother;

  Mode nextMode(Mode mode, bool hasRecording, bool overdubAfterRecord) {
    if (mode == STOPPED && !hasRecording) {
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
    if (mode == STOPPED && hasRecording) {
      return PLAYING;
    }
    return mode;
  }

public:
  Loop() {
    inputSmoother.setRiseFall(100.0f, 50.0f);
    outputSmoother.setRiseFall(100.0f, 50.0f);
  }

  float *process(float deltaTime, float *in) {
    static float out[16];
    float inputEnv = inputSmoother.process(deltaTime, recording() || overdubbing() ? 1.0f : 0.0f);
    float outputEnv = outputSmoother.process(deltaTime, stopped() ? 0.0f : 1.0f);
    for (int c = 0; c < channels; c++) {
      out[c] = outputEnv * recorder.process(inputEnv * in[c]);
    }
    return out;
  }

  void setChannels(int channels) {
    this->channels = channels;
  }

  Mode getNextMode(bool overdubAfterRecord) {
    return nextMode(mode, recorder.hasRecording(), overdubAfterRecord);
  }

  void toggle(bool overdubAfterRecord) {
    if (recording()) {
      recorder.end();
    }
    mode = nextMode(mode, recorder.hasRecording(), overdubAfterRecord);
    if (recording()) {
      recorder.start();
    }
  }

  void stop() {
    mode = STOPPED;
  }

  void erase() {
    stop();
    recorder.erase();
  }

  bool recording() {
    return mode == RECORDING;
  }

  bool overdubbing() {
    return mode == OVERDUBBING;
  }

  bool playing() {
    return mode == PLAYING;
  }

  bool stopped() {
    return mode == STOPPED;
  }

  bool hasRecording() {
    return recorder.hasRecording();
  }
};
