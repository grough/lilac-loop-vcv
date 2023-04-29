#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;
  p->addModel(modelLooperOne);
  p->addModel(modelLooperTwo);
}
