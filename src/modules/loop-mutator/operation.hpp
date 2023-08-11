#pragma once

#include <vector>

class Operation {
public:
  std::vector<float> *samples;
  Operation *parent;
  int startPos = -1;
  int endPos = -1;
  float *input;
  float fadeTime;

  Operation(Operation *parent, std::vector<float> *samples, float *input, float fadeTime = 0.f)
      : parent(parent), samples(samples), input(input) {}

  virtual float process(float deltaTime) = 0;
  virtual void end() = 0;
  virtual void recover() = 0;

  int size() {
    return endPos - startPos + 1;
  }
};

class InitOp : public Operation {
public:
  InitOp(Operation *parent = nullptr, std::vector<float> *samples = nullptr, float *input = nullptr)
      : Operation(parent, samples, input) {
  }

  float process(float deltaTime) { return 0.f; }
  void end() {}
  void recover () {}
};
