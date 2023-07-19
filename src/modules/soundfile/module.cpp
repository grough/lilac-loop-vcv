#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "../../plugin.hpp"

struct MyModule : Module {
  enum ParamId {
    PITCH_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PITCH_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SINE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    BLINK_LIGHT,
    LIGHTS_LEN
  };

  MyModule() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "");
    configInput(PITCH_INPUT, "");
    configOutput(SINE_OUTPUT, "");
  }

  void process(const ProcessArgs &args) override {
  }
};

struct MyModuleWidget : ModuleWidget {
  MyModuleWidget(MyModule *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/MyModule.svg")));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 46.063)), module, MyModule::PITCH_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 77.478)), module, MyModule::PITCH_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 108.713)), module, MyModule::SINE_OUTPUT));
    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.24, 25.81)), module, MyModule::BLINK_LIGHT));
  }
};

Model *modelMyModule = createModel<MyModule, MyModuleWidget>("MyModule");