#pragma once

#include "control.hpp"
#include "operation.hpp"
#include "record.hpp"

class LoopHistory {
public:
  std::vector<Operation *> ops{new InitOp()};
  std::vector<float> samples;
  Control control;

  LoopHistory(int maxLoopLength) : samples(maxLoopLength) {
    control.process(); // prime the event processor
  }

  float process(float deltaTime, float input) {
    ControlEvent event = control.process();

    if (event == END) {
      ops.back()->end();
    }

    if (event == UNDO) {
      if (ops.size() > 1) {
        ops.pop_back();
      }
    }

    if (event == RECORD) {
      ops.push_back(new RecordOp(ops.back(), &samples));
    }

    return ops.back()->process(deltaTime, input);
  }
};