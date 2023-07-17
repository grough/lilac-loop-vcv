#pragma once

#include <vector>
#include "soft_loop.hpp"
#include "envelope.hpp"
#include "buffer.hpp"

class LoopStack {
public:
  LoopStack(RBuf *inBuf);
  float process(float deltaTime, float input);
  void startRecording();
  void stopRecording();
  bool isEndOfCycle();
  float phase();
  void setCrossfadeRate(float rate);
  void clean();
  int size() const;
  void setHeadMode();
  void setTailMode();
  void setNoneMode();
  void setAllMode();
  void setEnvelopeAttack(float rate);
  void setEnvelopeRelease(float rate);

private:
  RBuf *inBuf;
  std::vector<SoftLoop> loops;
  SoftLoop::FadeMode fadeMode = SoftLoop::FadeMode::TAIL;
};

LoopStack::LoopStack(RBuf *inBuf) : inBuf(inBuf) {}

void LoopStack::startRecording() {
  for (size_t i = 0; i < loops.size(); i++) {
    loops[i].release();
  }
  ICrossfade *crossfade = new ConstantPowerFade();
  // ICrossfade *crossfade = new LinearFade();
  // GateEnvelope *envelope = new GateEnvelope();
  AttackReleaseEnvelope *envelope = new AttackReleaseEnvelope();
  SoftLoop loop = SoftLoop(inBuf, crossfade, envelope, fadeMode);
  loops.push_back(loop);
  loops.back().startRecording();
}

float LoopStack::process(float deltaTime, float in) {
  inBuf->write(in);
  inBuf->next();
  clean();
  float out = 0.f;
  for (size_t i = 0; i < loops.size(); i++) {
    out += loops[i].process(deltaTime, in);
  }
  return out;
}

void LoopStack::stopRecording() {
  if (loops.size() > 0) {
    loops.back().stopRecording();
  }
}

bool LoopStack::isEndOfCycle() {
  if (loops.size() > 0) {
    return loops.back().isEndOfCycle();
  }
  return false;
}

float LoopStack::phase() {
  if (loops.size() > 0) {
    return loops.back().phase();
  }
  return 0.f;
}

void LoopStack::clean() {
  loops.erase(std::remove_if(loops.begin(), loops.end(), [](const SoftLoop &loop) {
                return loop.isReleased();
              }),
              loops.end());
}

int LoopStack::size() const {
  return loops.size();
}

void LoopStack::setCrossfadeRate(float rate) {
  for (size_t i = 0; i < loops.size(); i++) {
    loops[i].setCrossfadeRate(rate);
  }
}

void LoopStack::setEnvelopeAttack(float rate) {
  for (size_t i = 0; i < loops.size(); i++) {
    loops[i].setEnvelopeAttack(rate);
  }
}
void LoopStack::setEnvelopeRelease(float rate) {
  for (size_t i = 0; i < loops.size(); i++) {
    loops[i].setEnvelopeRelease(rate);
  }
}

void LoopStack::setHeadMode() {
  fadeMode = SoftLoop::FadeMode::HEAD;
}

void LoopStack::setTailMode() {
  fadeMode = SoftLoop::FadeMode::TAIL;
}

void LoopStack::setNoneMode() {
  fadeMode = SoftLoop::FadeMode::NONE;
}

void LoopStack::setAllMode() {
  fadeMode = SoftLoop::FadeMode::ALL;
}
