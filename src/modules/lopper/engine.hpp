#pragma once

#include "buffer.hpp"
#include "zero_crossing.hpp"
#include "loop_stack.hpp"

class Lopper {
public:
  Lopper(int bufferSize);
  float process(float deltaTime, float toggleIn, float in);
  bool isEndOfCycle();
  float phase();
  int size();
  void setCrossfadeRate(float rate);
  void setEnvelopeAttack(float rate);
  void setEnvelopeRelease(float rate);

private:
  RBuf *inBuf;
  LoopStack stack;
  ZeroCrossingDetector zcd;
};

Lopper::Lopper(int bufferSize) : inBuf(new RBuf(bufferSize)), stack(inBuf), zcd() {}

float Lopper::process(float deltaTime, float toggleIn, float in) {
  // Handle start/stop
  int cross = zcd.process(toggleIn);
  if (cross == 1) {
    stack.startRecording();
  }
  if (cross == -1) {
    stack.stopRecording();
  }
  // Process input
  inBuf->write(in);
  inBuf->next();
  return stack.process(deltaTime, in);
}

float Lopper::phase() {
  return stack.phase();
}

bool Lopper::isEndOfCycle() {
  return stack.isEndOfCycle();
}

int Lopper::size() {
  return stack.size();
}

void Lopper::setCrossfadeRate(float rate) {
  stack.setCrossfadeRate(rate);
}

void Lopper::setEnvelopeAttack(float rate) {
  stack.setEnvelopeAttack(rate);
}

void Lopper::setEnvelopeRelease(float rate) {
  stack.setEnvelopeRelease(rate);
}
