struct ZeroCrossingDetector {
  float last = 0.f;
  int process(float input) {
    int direction = 0;
    if (last < 0.f && input > 0.f) {
      direction = 1;
    } else if (last > 0.f && input < 0.f) {
      direction = -1;
    } else if (last == 0.f && input < 0.f) {
      direction = -1;
    } else if (last == 0.f && input > 0.f) {
      direction = 1;
    }
    last = input;
    return direction;
  };
};

struct ZeroCrossGate {
  ZeroCrossingDetector zcd;
  bool open = false;
  bool process(float input) {
    int direction = zcd.process(input);
    if (direction == 1) {
      open = true;
    }
    if (direction == -1) {
      open = false;
    }
    return open;
  };
};
