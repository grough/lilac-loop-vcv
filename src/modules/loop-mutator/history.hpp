#pragma once

#include "control.hpp"
#include "operation.hpp"
#include "record.hpp"

class LoopHistory {
public:
  std::vector<Operation *> ops{new InitOp()};
  std::vector<float> samples;
  Control control;
  float in;
  float fadeTime;

  LoopHistory(int maxLoopLength, float fadeTime = 0) : samples(maxLoopLength), fadeTime(fadeTime) {
    control.process(); // prime the event processor
  }

  Operation *op() {
    return ops.back();
  }

  float process(float deltaTime, float input) {
    ControlEvent event = control.process();

    in = input;

    if (event == END) {
      ops.back()->end();
    }

    if (event == UNDO) {
      if (ops.size() > 1) {
        ops.pop_back();
        ops.back()->recover();
      }
    }

    if (event == RECORD) {
      ops.push_back(new RecordOp(ops.back(), &samples, &in, fadeTime));
    }

    return ops.back()->process(deltaTime);
  }
};