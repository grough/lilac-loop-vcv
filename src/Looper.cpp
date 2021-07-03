#include "plugin.hpp"
#include <future>
#include "osdialog.h"
#include "AudioFile.h"
#include "ui.hpp"
#include "Looper/Looper.hpp"
#include "Looper/LooperWidget.hpp"

Model *modelLooper = createModel<Looper, LooperWidget>("Looper");
