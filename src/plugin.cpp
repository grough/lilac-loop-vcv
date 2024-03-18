#include "plugin.hpp"
#include "modules/looper/controls.hpp"
#include "modules/looper-feedback-expander/module.cpp"
#include "modules/looper/module.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  p->addModel(modelLooper);
  p->addModel(modelLooperFeedbackExpander);
}
