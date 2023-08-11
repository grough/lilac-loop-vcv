#pragma once

#include "operation.hpp"
#include "sr-envelope.hpp"

class RecordOp : public Operation {
public:
  int pos;
  SustainRelease inputGate;

  RecordOp(Operation *parent, std::vector<float> *samples, float *input, float fadeTime)
      : Operation(parent, samples, input, fadeTime) {
    inputGate.setReleaseTime(fadeTime);
    startPos = parent->endPos + 1;
    endPos = startPos - 1;
    pos = startPos - 1;
  }

  float process(float deltaTime) {
    // Move to next sample
    pos++;

    // Update input gate envelope
    inputGate.process(deltaTime);

    // Increase loop length while recording
    if (inputGate.isOpen()) {
      endPos++;
    }

    // When recording stops, pos will reach the end and wrap around
    if (pos > endPos) {
      pos = startPos;
    }

    // Overwrite samples during recording
    if (inputGate.isOpen()) {
      samples->at(pos) = *input;
    }

    // Crossfade loop ending into beginning
    if (inputGate.isClosing()) {
      samples->at(pos) = samples->at(pos) * (1 - inputGate.getOutput()) + *input * inputGate.getOutput();
      ;
    }

    float out = samples->at(pos);

    return out;
  }

  void end() {
    inputGate.release();
  }

  void recover() {
    pos = startPos - 1;
  }
};
