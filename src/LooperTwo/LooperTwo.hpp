struct LooperTwo : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    ERASE_BUTTON_PARAM,
    UNDO_BUTTON_PARAM,
    STOP_BUTTON_PARAM,
    FEEDBACK_PARAM,
    MIX_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    ARM_CV_INPUT,
    MODE_CV_INPUT,
    ERASE_CV_INPUT,
    UNDO_CV_INPUT,
    STOP_CV_INPUT,
    RETURN_1_INPUT,
    RETURN_2_INPUT,
    FEEDBACK_CV_INPUT,
    MIX_CV_INPUT,
    MAIN_INPUT,
    MAIN_2_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SEND_1_OUTPUT,
    SEND_2_OUTPUT,
    MAIN_OUTPUT,
    MAIN_2_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    ARM_STATUS_LIGHT,
    RECORD_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    NUM_LIGHTS
  };

  float t = 0.f;
  float mix = 0.f;
  dsp::BooleanTrigger armTrigger;
  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::ClockDivider logDivider;

  LooperTwo() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.f, 1.f, 0.f, "Toggle");
    configParam(ERASE_BUTTON_PARAM, 0.f, 1.f, 0.f, "Erase");
    // configParam(UNDO_BUTTON_PARAM, 0.f, 1.f, 0.f, "Undo");
    configParam(STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "Stop");
    configParam(FEEDBACK_PARAM, 0.f, 1.f, 1.f, "Feedback");
    configParam(MIX_PARAM, -1.f, 1.f, 0.f, "Mix");
    logDivider.setDivision(pow(2, 13));
  }

  void process(const ProcessArgs &args) override {
    bool armTriggered = armTrigger.process(inputs[ARM_CV_INPUT].getVoltage() > 0.f);
    bool toggleTriggered = toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.f);
    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.f);
    bool eraseTriggered = eraseTrigger.process(params[ERASE_BUTTON_PARAM].getValue() + inputs[ERASE_CV_INPUT].getVoltage() > 0.f);
    float feedback = math::clamp(params[FEEDBACK_PARAM].getValue() + inputs[FEEDBACK_CV_INPUT].getVoltage(), 0.f, 1.f);
    float mix = math::clamp(params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage() / 5, -1.f, 1.f);
    t += args.sampleTime;
  }
};

struct LargeWarmButton : SvgSwitch {
  LargeWarmButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigButton_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigButton_1.svg")));
  }
};

struct WarmButton : SvgSwitch {
  WarmButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button_1.svg")));
  }
};

struct WarmKnob : Davies1900hKnob {
  WarmKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knob.svg")));
  }
};

struct LooperTwoWidget : ModuleWidget {
  LooperTwoWidget(LooperTwo *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LooperTwo.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(51.971, 27.534)), module, LooperTwo::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(72.767, 62.246)), module, LooperTwo::ERASE_BUTTON_PARAM));
    // addParam(createParamCentered<WarmButton>(mm2px(Vec(19.628, 62.277)), module, LooperTwo::UNDO_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(46.698, 62.277)), module, LooperTwo::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(47.25, 87.693)), module, LooperTwo::FEEDBACK_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(46.901, 112.213)), module, LooperTwo::MIX_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.001, 40.194)), module, LooperTwo::ARM_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.089, 40.194)), module, LooperTwo::MODE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.954, 62.246)), module, LooperTwo::ERASE_CV_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.02, 62.277)), module, LooperTwo::UNDO_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.089, 62.277)), module, LooperTwo::STOP_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.673, 87.693)), module, LooperTwo::RETURN_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.773, 87.693)), module, LooperTwo::RETURN_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.437, 87.693)), module, LooperTwo::FEEDBACK_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.089, 112.213)), module, LooperTwo::MIX_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.907, 112.423)), module, LooperTwo::MAIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.007, 112.423)), module, LooperTwo::MAIN_2_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.082, 87.693)), module, LooperTwo::SEND_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(73.182, 87.693)), module, LooperTwo::SEND_2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.374, 112.24)), module, LooperTwo::MAIN_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(73.474, 112.24)), module, LooperTwo::MAIN_2_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(19.61, 40.194)), module, LooperTwo::ARM_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(64.314, 42.772)), module, LooperTwo::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(69.872, 42.772)), module, LooperTwo::PLAY_STATUS_LIGHT));
  }
};
