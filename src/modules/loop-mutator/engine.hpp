#include <vector>
#include "control.hpp"

class IOperation {
public:
  std::vector<float> *samples;
  IOperation *parentOp;
  int startPos = 0;
  int endPos = 0;

  IOperation(IOperation *parentOp, std::vector<float> *samples)
      : parentOp(parentOp), samples(samples) {

    // InitOp has no parent
    if (parentOp == nullptr) {
      return;
    }

    // New operation begins where parent operation ends
    startPos = parentOp->endPos;
  }

  virtual float process(float deltaTime, float input) = 0;
  virtual void end() = 0;
};

class InitOp : public IOperation {
public:
  InitOp(IOperation *parentOp = nullptr, std::vector<float> *samples = nullptr)
      : IOperation(parentOp, samples) {}

  float process(float deltaTime, float input) { return 0.f; }

  void end() {}
};

class LoopHistory {
public:
  std::vector<IOperation *> ops{new InitOp()};
  std::vector<float> samples;
  Control control;

  LoopHistory(int maxLoopLength) : samples(maxLoopLength) {}

  float process(float deltaTime, float input) {
    Event event = control.process();

    if (event == END) {
      ops.back()->end();
    }

    if (event == UNDO) {
      if (ops.size() > 1) {
        ops.pop_back();
      }
    }

    return ops.back()->process(deltaTime, input);
  }
};
