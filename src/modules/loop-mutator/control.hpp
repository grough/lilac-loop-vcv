#pragma once

#include "trigger.hpp"

enum ControlEvent {
  NONE,
  RECORD,
  UNDO,
  END
};

struct Control {
  bool recordInput = false;
  bool undoInput = false;
  BooleanTrigger recordTrigger;
  BooleanTrigger undoTrigger;
  ControlEvent event = NONE;

  ControlEvent process() {
    event = NONE;

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