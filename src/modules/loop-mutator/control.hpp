#include "trigger.hpp"

enum Event {
  NOOP,
  RECORD,
  UNDO,
  END
};

struct Control {
  bool recordInput = false;
  bool undoInput = false;
  BooleanTrigger recordTrigger;
  BooleanTrigger undoTrigger;
  Event event = NOOP;

  Event process() {
    event = NOOP;

    int recordEvent = recordTrigger.processEvent(recordInput);
    if (recordEvent > 0) {
      event = RECORD;
    }
    if (recordEvent < 0) {
      event = END;
    }

    int undoEvent = undoTrigger.processEvent(undoInput);
    if (undoEvent > 0) {
      event = UNDO;
    }

    return event;
  }
};