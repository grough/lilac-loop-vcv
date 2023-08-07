#include "../../plugin.hpp"
#include "engine.hpp"

struct LoopMutator : Module {
  enum ParamId {
    RECORD_BUTTON_PARAM,
    OVERDUB_BUTTON_PARAM,
    UNDO_BUTTON_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RECORD_INPUT,
    OVERDUB_INPUT,
    UNDO_INPUT,
    MAIN_1_INPUT,
    MAIN_2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    MAIN_1_OUTPUT,
    MAIN_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  dsp::ClockDivider logDivider;
  LoopHistory history{48000 * 120};

  LoopMutator() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(RECORD_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    // configParam(OVERDUB_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    configParam(UNDO_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    configInput(RECORD_INPUT, "");
    configInput(OVERDUB_INPUT, "");
    configInput(UNDO_INPUT, "");
    configInput(MAIN_1_INPUT, "");
    configInput(MAIN_2_INPUT, "");
    configOutput(MAIN_1_OUTPUT, "");
    configOutput(MAIN_2_OUTPUT, "");
    logDivider.setDivision(8192);
  }

  void debug() {
    DEBUG("Event %d\tHistory %d\tStart %d\tEnd %d\t",
          history.control.event,
          history.ops.size(),
          history.ops.back()->startPos,
          history.ops.back()->endPos);
  }

  void process(const ProcessArgs &args) override {
    history.control.recordInput = (getParam(RECORD_BUTTON_PARAM).getValue() + getInput(RECORD_INPUT).getVoltage()) > 0.f;
    history.control.undoInput = (getParam(UNDO_BUTTON_PARAM).getValue() + getInput(UNDO_INPUT).getVoltage()) > 0.f;
    getOutput(MAIN_1_OUTPUT).setVoltage(history.process(args.sampleTime, getInput(MAIN_1_INPUT).getVoltage()));

    if (history.control.event != NOOP) {
      debug();
    }
    if (logDivider.process()) {
      debug();
    }
  }
};

struct LoopMutatorWidget : ModuleWidget {
  LoopMutatorWidget(LoopMutator *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/LoopMutator.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.266, 33.412)), module, LoopMutator::RECORD_BUTTON_PARAM));
    // addParam(createParamCentered<WarmButton>(mm2px(Vec(18.47, 49.069)), module, LoopMutator::OVERDUB_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.47, 64.944)), module, LoopMutator::UNDO_BUTTON_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 33.412)), module, LoopMutator::RECORD_INPUT));
    // addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 49.069)), module, LoopMutator::OVERDUB_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 64.944)), module, LoopMutator::UNDO_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 97.091)), module, LoopMutator::MAIN_1_INPUT));
    // addInput(createInputCentered<LilacPort>(mm2px(Vec(18.758, 97.091)), module, LoopMutator::MAIN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(6.666, 112.441)), module, LoopMutator::MAIN_1_OUTPUT));
    // addOutput(createOutputCentered<LilacPort>(mm2px(Vec(18.766, 112.441)), module, LoopMutator::MAIN_2_OUTPUT));
  }
};

Model *modelLoopMutator = createModel<LoopMutator, LoopMutatorWidget>("LoopMutator");