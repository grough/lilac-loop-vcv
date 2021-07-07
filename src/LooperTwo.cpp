#include "plugin.hpp"
#include "osdialog.h"
#include "AudioFile.h"
#include "ui.hpp"
#include "FileSaver.hpp"
#include "LooperTwo/LooperTwo.hpp"
#include "LooperTwo/LooperTwoWidget.hpp"

Model *modelLooperTwo = createModel<LooperTwo, LooperTwoWidget>("LooperTwo");
