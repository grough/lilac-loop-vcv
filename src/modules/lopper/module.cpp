#include "../../plugin.hpp"
#include "engine.hpp"

struct LopperModule : Module {
  enum ParamId {
    CROSSFADE_PARAM,
    ATTACK_PARAM,
    RELEASE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    TOGGLE_INPUT,
    MAIN_1_INPUT,
    CROSSFADE_INPUT,
    ATTACK_INPUT,
    RELEASE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    MAIN_1_OUTPUT,
    DEBUG_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    RECORD_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    LIGHTS_LEN
  };
  enum LooperMode {
    RECORD_PLAY_OVERDUB,
    RECORD_OVERDUB_PLAY,
    GATE_1,
  };

  dsp::ClockDivider debugDiv;
  dsp::PulseGenerator clockPulse;
  dsp::BooleanTrigger toggleTrigger;
  Lopper lop{48000 * 8};

  LopperModule() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CROSSFADE_PARAM, 1.f, 1000.f, 1.f, "Crossfade");
    configParam(ATTACK_PARAM, .01f, 40.f, 1.f, "");
    configParam(RELEASE_PARAM, .01f, 40.f, 1.f, "");
    configInput(TOGGLE_INPUT, "");
    configInput(MAIN_1_INPUT, "");
    configInput(ATTACK_INPUT, "");
    configInput(RELEASE_INPUT, "");
    configOutput(CLOCK_OUTPUT, "");
    configOutput(PHASE_OUTPUT, "");
    configOutput(DEBUG_OUTPUT, "Debug");
    debugDiv.setDivision(8192);
  }

  void process(const ProcessArgs &args) override {
    // Handle crossfade controls
    float fade = 0.f;
    if (inputs[CROSSFADE_INPUT].isConnected()) {
      fade = inputs[CROSSFADE_INPUT].getVoltage() / 10.f;
    } else {
      fade = params[CROSSFADE_PARAM].getValue();
    }
    lop.setCrossfadeRate(fade);

    // Handle envelope attack controls
    float attack = params[ATTACK_PARAM].getValue();
    if (inputs[ATTACK_INPUT].isConnected()) {
      attack = attack * inputs[ATTACK_INPUT].getVoltage() / 10.f;
    }
    lop.setEnvelopeAttack(attack);

    // Handle envelope release controls
    float release = params[RELEASE_PARAM].getValue();
    if (inputs[RELEASE_INPUT].isConnected()) {
      release = release * inputs[RELEASE_INPUT].getVoltage() / 10.f;
    }
    lop.setEnvelopeRelease(release);

    float out = lop.process(args.sampleTime, inputs[TOGGLE_INPUT].getVoltage(), inputs[MAIN_1_INPUT].getVoltage());

    outputs[MAIN_1_OUTPUT].setVoltage(out);

    if (lop.isEndOfCycle()) {
      clockPulse.trigger();
    }

    outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(args.sampleTime));
    outputs[PHASE_OUTPUT].setVoltage(lop.phase() * 10.f);

    if (debugDiv.process()) {
      DEBUG("Lopper\tstack:%d\tfade:%f", lop.size(), fade);
    }
  }
};

struct LopperWidget : ModuleWidget {
  LopperWidget(LopperModule *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Lopper.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(52.474, 86.607)), module, LopperModule::CROSSFADE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(78.933, 65.44)), module, LopperModule::ATTACK_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(89.516, 65.44)), module, LopperModule::RELEASE_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.422, 67.443)), module, LopperModule::TOGGLE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.422, 109.776)), module, LopperModule::MAIN_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.586, 86.607)), module, LopperModule::CROSSFADE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.933, 75.495)), module, LopperModule::ATTACK_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(89.516, 75.495)), module, LopperModule::RELEASE_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(103.87, 108.947)), module, LopperModule::CLOCK_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(119.745, 108.947)), module, LopperModule::PHASE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.138, 109.776)), module, LopperModule::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(119.745, 70.0)), module, LopperModule::DEBUG_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(50.909, 67.359)), module, LopperModule::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(56.201, 67.359)), module, LopperModule::PLAY_STATUS_LIGHT));
  }
};

Model *modelLopper = createModel<LopperModule, LopperWidget>("Lopper");
