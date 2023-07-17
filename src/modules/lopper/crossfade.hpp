#pragma once

#include <algorithm>
#include <math.h>

class ICrossfade {
public:
  void setRate(float rate);
  void setOrientation(int orientation);
  virtual void process(float x) = 0;
  virtual float getA() const = 0;
  virtual float getB() const = 0;
  virtual ~ICrossfade() {}

protected:
  float time = 0.f;
  float rate = 1.f;
  int orientation = 1;
  float a = 1.f;
  float b = 0.f;
};

void ICrossfade::setRate(float rate) {
  this->rate = rate;
}

void ICrossfade::setOrientation(int orientation) {
  this->orientation = orientation >= 0 ? 1 : -1;
}

class LinearFade : public ICrossfade {
public:
  void process(float x) override;
  float getA() const override;
  float getB() const override;

private:
  float a = 1.f;
  float b = 0.f;
};

void LinearFade::process(float x) {
  if (orientation > 0) {
    x = x * rate;
  } else {
    x = rate * x - rate + 1;
  }
  x = std::fmin(std::fmax(x, 0.f), 1.f);
  a = 1.f - x;
  b = x;
}

float LinearFade::getA() const {
  return a;
}

float LinearFade::getB() const {
  return b;
}

class ConstantPowerFade : public ICrossfade {
public:
  void process(float x) override;
  float getA() const override;
  float getB() const override;

private:
  float a = 1.f;
  float b = 0.f;
};

void ConstantPowerFade::process(float x) {
  if (orientation > 0) {
    x = x * rate;
  } else {
    x = rate * x - rate + 1;
  }

  x = std::fmin(std::fmax(x, 0.f), 1.f);
  a = sqrt((1.0 + cos(x * M_PI)) / 2.0);
  b = sqrt((1.0 - cos(x * M_PI)) / 2.0);
}

float ConstantPowerFade::getA() const {
  return a;
}

float ConstantPowerFade::getB() const {
  return b;
}

class SumFade : public ICrossfade {
public:
  void process(float x) override;
  float getA() const override;
  float getB() const override;

private:
  float a = 1.f;
  float b = 1.f;
};

void SumFade::process(float x) {}

float SumFade::getA() const {
  return 1.f;
}

float SumFade::getB() const {
  return 1.f;
}
