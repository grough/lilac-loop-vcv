#include "plugin.hpp"
#include "modules/looper-one/module.cpp"
#include "modules/looper-two/module.cpp"
#include "modules/lopper/module.cpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  p->addModel(modelLooper);
  p->addModel(modelLooperTwo);
  p->addModel(modelLopper);
}
