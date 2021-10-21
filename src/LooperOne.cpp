#include "plugin.hpp"

#include "common.h"
#include "MultiLoopWriter.hpp"
#include "Looper.hpp"
#include "LooperWidget.hpp"
#include "LooperOneWidget.hpp"

Model *modelLooperOne = createModel<Looper, LooperOneWidget>("Looper");
