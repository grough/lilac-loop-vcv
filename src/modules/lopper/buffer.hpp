#pragma once

#include <math.h>
#include <vector>

struct RBuf {
  std::vector<float> samples;
  int p = 0;

  RBuf(int size) {
    samples.resize(size);
  }

  void next() {
    p++;
    if (p == samples.size()) {
      p = 0;
    }
  }

  void write(float in) {
    samples[p] = in;
  }

  float read() {
    return samples[p];
  }

  // Read forward or backward only if |i| <= buffer size
  // Might not be needed anymore
  float read(int i) {
    int pi = p + i;
    if (pi < 0) {
      return samples[samples.size() + pi];
    }
    if (pi > samples.size()) {
      return samples[pi - samples.size()];
    }
    return samples[pi];
  }
};

struct LoopBuf {
  std::vector<float> samples;
  int p = -1;
  bool open = false;

  void process(int size, float in) {
    if (!open) {
      return;
    }

    if (samples.size() < size) {
      samples.push_back(in);
    }

    p++;

    if (p == samples.size()) {
      p = 0;
    }
  }

  float read() {
    if (samples.size() == 0) {
      return 0.f;
    }
    return samples[p];
  }

  void reset() {
    p = -1;
    open = false;
    samples.clear();
  }

  float phase() {
    if (samples.size() == 0) {
      return 0.f;
    }
    return (float)p / (float)samples.size();
  }
};
