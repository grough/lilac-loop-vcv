#include "../../plugin.hpp"

struct LooperFeedbackExpander : Module {
  enum ParamId {
    BYPASS_PARAM,
    FEEDBACK_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RETURN_1_INPUT,
    RETURN_2_INPUT,
    FEEDBACK_CV_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SEND_1_OUTPUT,
    SEND_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  LooperFeedbackExpander() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(BYPASS_PARAM, 0.f, 1.f, 0.f, "");
    configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 1.0f, "Feedback", "%", 0.0f, 100.0f);
    configInput(RETURN_1_INPUT, "Left return");
    configInput(RETURN_2_INPUT, "Right return");
    configInput(FEEDBACK_CV_INPUT, "Feedback attenuator");
    configOutput(SEND_1_OUTPUT, "Left send");
    configOutput(SEND_2_OUTPUT, "Right send");
  }
};

struct LooperFeedbackExpanderWidget : ModuleWidget {
  LooperFeedbackExpanderWidget(LooperFeedbackExpander *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/LooperFeedbackExpander.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // addParam(createParamCentered<WarmButton>(mm2px(Vec(7.62, 18.505)), module, LooperFeedbackExpander::BYPASS_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(7.62, 39.159)), module, LooperFeedbackExpander::FEEDBACK_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 52.338)), module, LooperFeedbackExpander::FEEDBACK_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.538, 100.677)), module, LooperFeedbackExpander::RETURN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.538, 112.777)), module, LooperFeedbackExpander::RETURN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.538, 69.986)), module, LooperFeedbackExpander::SEND_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.538, 82.086)), module, LooperFeedbackExpander::SEND_2_OUTPUT));
  }
};

Model *modelLooperFeedbackExpander = createModel<LooperFeedbackExpander, LooperFeedbackExpanderWidget>("LooperFeedbackExpander");