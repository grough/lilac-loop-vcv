#pragma once

#include <vector>

class Operation {
public:
  std::vector<float> *samples;
  Operation *parent;
  int startPos = 0;
  int endPos = 0;

  Operation(Operation *parent, std::vector<float> *samples)
      : parent(parent), samples(samples) {

    // InitOp has no parent
    if (parent == nullptr) {
      return;
    }

    // New operation begins where parent operation ends
    startPos = parent->endPos;
  }

  virtual float process(float deltaTime, float input) = 0;
  virtual void end() = 0;
};

class InitOp : public Operation {
public:
  InitOp(Operation *parent = nullptr, std::vector<float> *samples = nullptr)
      : Operation(parent, samples) {}

  float process(float deltaTime, float input) { return 0.f; }

  void end() {}
};
