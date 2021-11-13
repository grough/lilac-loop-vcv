#include "plugin.hpp"

#include "common.h"
#include "MultiLoopReader.hpp"
#include "MultiLoopWriter.hpp"
#include "Looper.hpp"
#include "LooperWidget.hpp"
#include "LooperTwoWidget.hpp"

Model *modelLooperTwo = createModel<Looper, LooperTwoWidget>("LooperTwo");
