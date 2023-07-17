#pragma once

class IEnvelope {
public:
  virtual float process(float deltaTime) = 0;
  virtual void attack() = 0;
  virtual void release() = 0;
  virtual bool isDone() const = 0;
  virtual float getOutput() = 0;
  virtual void setAttackRate(float rate) = 0;
  virtual void setReleaseRate(float rate) = 0;
};

class GateEnvelope : public IEnvelope {
public:
  float process(float deltaTime) override;
  void attack() override;
  void release() override;
  bool isDone() const override;
  float getOutput() override;
  void setAttackRate(float rate) override;
  void setReleaseRate(float rate) override;

private:
  bool open = false;
  bool done = false;
};

void GateEnvelope::attack() {
  open = true;
}

void GateEnvelope::release() {
  open = false;
  done = true;
}

bool GateEnvelope::isDone() const {
  return done;
}

float GateEnvelope::getOutput() {
  return open ? 1.f : 0.f;
}

float GateEnvelope::process(float deltaTime) {
  return getOutput();
}

void GateEnvelope::setAttackRate(float rate) {}
void GateEnvelope::setReleaseRate(float rate) {}

struct Slew {
  float out = 0.f;
  float rise = 0.f;
  float fall = 0.f;

  float clamp(float x, float a, float b) {
    return std::max(std::min(x, b), a);
  }

  void reset() {
    out = 0.f;
  }

  void setRiseFall(float rise, float fall) {
    this->rise = rise;
    this->fall = fall;
  }

  float process(float deltaTime, float in) {
    out = clamp(in, out - fall * deltaTime, out + rise * deltaTime);
    return out;
  }
};

class AttackReleaseEnvelope : public IEnvelope {
public:
  AttackReleaseEnvelope();
  float process(float deltaTime) override;
  void attack() override;
  void release() override;
  bool isDone() const override;
  float getOutput() override;
  void setAttackRate(float rate) override;
  void setReleaseRate(float rate) override;

private:
  bool released = false;
  bool done = false;
  float target = 0.f;
  Slew slew;
};

AttackReleaseEnvelope::AttackReleaseEnvelope() {
  slew.setRiseFall(1.f, 1.f);
}

void AttackReleaseEnvelope::attack() {
  slew.reset();
  target = 1.f;
}

float AttackReleaseEnvelope::process(float deltaTime) {
  slew.process(deltaTime, target);
  return getOutput();
}

void AttackReleaseEnvelope::release() {
  released = true;
  target = 0.f;
}

bool AttackReleaseEnvelope::isDone() const {
  if (released && slew.out < 0.001f) {
    return true;
  }
  return false;
}

float AttackReleaseEnvelope::getOutput() {
  return slew.out;
}

void AttackReleaseEnvelope::setAttackRate(float rate) {
  slew.rise = rate;
}

void AttackReleaseEnvelope::setReleaseRate(float rate) {
  slew.fall = rate;
}