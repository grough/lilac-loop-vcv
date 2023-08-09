#pragma once

#include "operation.hpp"

class RecordOp : public Operation {
public:
  bool firstPass = true;
  bool secondPass = false;
  int pos = -1;

  RecordOp(Operation *parent, std::vector<float> *samples)
      : Operation(parent, samples) {}

  float process(float deltaTime, float input) {
    // Move to next sample
    pos++;

    // Increase loop length while recording
    if (firstPass) {
      endPos = pos;
    }

    // When recording stops, pos will outpace endPos
    // Reset it to zero when it does
    if (pos > endPos) {
      pos = 0;
    }

    // Record input sample while recording
    if (firstPass) {
      samples->at(pos) = input;
    }

    float out = samples->at(pos);

    return out;
  }

  void end() {
    firstPass = false;
    secondPass = true;
  }
};
