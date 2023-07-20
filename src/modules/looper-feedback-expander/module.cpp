#include "../../plugin.hpp"

struct LooperFeedbackExpander : Module {
  enum ParamId {
    BYPASS_PARAM,
    FEEDBACK_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    FEEDBACK_CV_INPUT,
    RETURN_1_INPUT,
    RETURN_2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SEND_1_OUTPUT,
    SEND_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    BYPASS_LIGHT,
    LIGHTS_LEN
  };

  dsp::ClockDivider lightDivider;

  LooperFeedbackExpander() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 1.0f, "Feedback", "%", 0.0f, 100.0f);
    configInput(RETURN_1_INPUT, "Left return");
    configInput(RETURN_2_INPUT, "Right return");
    configInput(FEEDBACK_CV_INPUT, "Feedback attenuator");
    configOutput(SEND_1_OUTPUT, "Left send");
    configOutput(SEND_2_OUTPUT, "Right send");
    lightDivider.setDivision(512);
  }

  void process(const ProcessArgs &args) override {
    if (lightDivider.process()) {
      Module *leftModule = getLeftExpander().module;
      if (leftModule && leftModule->model == modelLooper && !leftModule->isBypassed()) {
        getLight(LooperFeedbackExpander::LightId::BYPASS_LIGHT).setBrightness(1.f);
      } else {
        getLight(LooperFeedbackExpander::LightId::BYPASS_LIGHT).setBrightness(0.f);
      }
    }
  }
};

struct LooperFeedbackExpanderWidget : ModuleWidget {
  LooperFeedbackExpanderWidget(LooperFeedbackExpander *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/LooperFeedbackExpander.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<WarmKnob>(mm2px(Vec(7.62, 37.196)), module, LooperFeedbackExpander::FEEDBACK_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 49.841)), module, LooperFeedbackExpander::FEEDBACK_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 100.267)), module, LooperFeedbackExpander::RETURN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 112.367)), module, LooperFeedbackExpander::RETURN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 69.047)), module, LooperFeedbackExpander::SEND_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 81.155)), module, LooperFeedbackExpander::SEND_2_OUTPUT));

    addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(7.62, 8.7)), module, LooperFeedbackExpander::BYPASS_LIGHT));
  }
};

Model *modelLooperFeedbackExpander = createModel<LooperFeedbackExpander, LooperFeedbackExpanderWidget>("LooperFeedbackExpander");