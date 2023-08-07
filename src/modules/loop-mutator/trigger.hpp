// Copying this Rack SDK code for now
// https://github.com/VCVRack/Rack/blob/v2/include/dsp/digital.hpp
struct BooleanTrigger {
  enum State {
    LOW,
    HIGH,
    UNINITIALIZED
  };
  union {
    State s = UNINITIALIZED;
  };

  void reset() {
    s = UNINITIALIZED;
  }

  /** Returns whether the input changed from false to true. */
  bool process(bool in) {
    bool triggered = (s == LOW) && in;
    s = in ? HIGH : LOW;
    return triggered;
  }

  enum Event {
    NONE = 0,
    TRIGGERED = 1,
    UNTRIGGERED = -1
  };
  /** Returns TRIGGERED if the input changed from false to true, and UNTRIGGERED if the input changed from true to false.
   */
  Event processEvent(bool in) {
    Event event = NONE;
    if (s == LOW && in) {
      event = TRIGGERED;
    } else if (s == HIGH && !in) {
      event = UNTRIGGERED;
    }
    s = in ? HIGH : LOW;
    return event;
  }

  bool isHigh() {
    return s == HIGH;
  }
};