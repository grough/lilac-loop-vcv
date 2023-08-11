#pragma once

class SustainRelease {
public:
  enum Stage {
    OPEN,
    CLOSING,
    CLOSED
  };
  void release();
  float process(float deltaTime);
  float getOutput();
  Stage getStage();
  bool isOpen();
  bool isClosing();
  bool isClosed();
  void setReleaseTime(float releaseTime);

private:
  Stage stage = OPEN;
  bool open = false;
  bool closing = false;
  float out = 0.f;
  float target = 1.f;
  float releaseRate = 1.f;
  float clamp(float x, float a, float b);
};

void SustainRelease::setReleaseTime(float releaseTime) {
  releaseRate = 1.f / releaseTime;
}

void SustainRelease::release() {
  stage = CLOSING;
  target = 0.f;
}

float SustainRelease::process(float deltaTime) {
  // Fade to zero
  if (target > out) {
    out = target;
  } else {
    out = clamp(target, out - releaseRate * deltaTime, out);
  }
  // Close envelope after fade to zero
  if (stage == CLOSING && out == 0.f) {
    stage = CLOSED;
  }
  return out;
}

float SustainRelease::getOutput() {
  return out;
}

SustainRelease::Stage SustainRelease::getStage() {
  return stage;
}

bool SustainRelease::isOpen() {
  return stage == SustainRelease::OPEN;
};

bool SustainRelease::isClosing() {
  return stage == SustainRelease::CLOSING;
}

bool SustainRelease::isClosed() {
  return stage == SustainRelease::CLOSED;
}

float SustainRelease::clamp(float x, float a, float b) {
  return std::max(std::min(x, b), a);
}
