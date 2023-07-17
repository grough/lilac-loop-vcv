#pragma once

#include "crossfade.hpp"
#include "envelope.hpp"
#include "buffer.hpp"

class SoftLoop {
public:
  enum FadeMode {
    ALL,
    NONE,
    HEAD,
    TAIL,
  };
  SoftLoop(RBuf *inBuf, ICrossfade *crossfade, IEnvelope *envelope, FadeMode fadeMode);
  float process(float deltaTime, float input);
  void startRecording();
  void stopRecording();
  bool isEndOfCycle();
  float phase();
  void release();
  bool isReleased() const;
  void setCrossfadeRate(float rate);
  void setEnvelopeAttack(float rate);
  void setEnvelopeRelease(float rate);
  float A(); // for testing
  float B(); // for testing
  int debugSize();
  int debugP();

private:
  RBuf *inBuf;
  ICrossfade *crossfade;
  IEnvelope *envelope;
  FadeMode fadeMode;
  LoopBuf loop;
  LoopBuf head;
  LoopBuf tail;
  bool open = false;
  int size = 0;
};

SoftLoop::SoftLoop(RBuf *inBuf, ICrossfade *crossfade, IEnvelope *envelope, FadeMode fadeMode) : inBuf(inBuf), crossfade(crossfade), envelope(envelope), fadeMode(fadeMode) {}

float SoftLoop::process(float deltaTime, float in) {
  if (open) {
    size++;
  }

  loop.process(size, in);
  head.process(size, inBuf->read((-size * 2) - 1));
  tail.process(size, in);

  float loopOut = loop.read();
  float headOut = head.read();
  float tailOut = tail.read();

  if (size > 0) {
    crossfade->process(phase());
  }

  float out = 0.f;

  switch (fadeMode) {
  case NONE:
    out = loopOut;
    break;
  case ALL:
    out = loopOut + headOut + tailOut;
    break;
  case TAIL:
    crossfade->setOrientation(1);
    out = tailOut * crossfade->getA() + loopOut * crossfade->getB();
    break;
  case HEAD:
    crossfade->setOrientation(-1);
    out = headOut * crossfade->getB() + loopOut * crossfade->getA();
    break;
  default:
    out = 42.f;
  }

  return out * envelope->process(deltaTime);
}

float SoftLoop::A() {
  return crossfade->getA();
}
float SoftLoop::B() {
  return crossfade->getB();
}

void SoftLoop::startRecording() {
  open = true;
  loop.open = true;
  envelope->attack();
}

void SoftLoop::stopRecording() {
  open = false;
  head.open = true;
  tail.open = true;
}

bool SoftLoop::isEndOfCycle() {
  return loop.p == 0;
}

float SoftLoop::phase() {
  if (loop.samples.size() < 1) {
    return 0.f;
  }
  return (float)loop.p / (float)loop.samples.size();
}

void SoftLoop::release() {
  envelope->release();
}

bool SoftLoop::isReleased() const {
  return envelope->isDone();
}

void SoftLoop::setCrossfadeRate(float rate) {
  crossfade->setRate(rate);
}

void SoftLoop::setEnvelopeAttack(float rate) {
  envelope->setAttackRate(rate);
}

void SoftLoop::setEnvelopeRelease(float rate) {
  envelope->setReleaseRate(rate);
}

int SoftLoop::debugSize() {
  return loop.samples.size();
}

int SoftLoop::debugP() {
  return loop.p;
}
