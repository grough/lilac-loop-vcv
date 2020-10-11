#include "loop.hpp"
#include "plugin.hpp"

struct Looper : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    AFTER_RECORD_PARAM,
    STOP_BUTTON_PARAM,
    ERASE_BUTTON_PARAM,
    NUM_PARAMS,
  };
  enum InputIds {
    MODE_CV_INPUT,
    STOP_CV_INPUT,
    ERASE_CV_INPUT,
    MAIN_INPUT,
    NUM_INPUTS,
  };
  enum OutputIds {
    MAIN_OUTPUT,
    NUM_OUTPUTS,
  };
  enum LightIds {
    RECORD_STATUS_LIGHT,
    OVERDUB_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    STOP_STATUS_LIGHT,
    TOGGLE_WAIT_LIGHT,
    NUM_LIGHTS,
  };

  float t = 0;
  unsigned int channels = 1;
  Loop loop;
  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::ClockDivider lightDivider;
  dsp::ClockDivider logDivider;

  Looper() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(AFTER_RECORD_PARAM, 0.f, 1.f, 1.f, "");
    configParam(STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    configParam(ERASE_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    lightDivider.setDivision(pow(2, 9));
    logDivider.setDivision(pow(2, 13));
  }

  void process(const ProcessArgs &args) override {
    bool toggleTriggered = toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.f);
    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.f);
    bool eraseTriggered = eraseTrigger.process(params[ERASE_BUTTON_PARAM].getValue() + inputs[ERASE_CV_INPUT].getVoltage() > 0.f);
    bool overdubAfterRecord = params[AFTER_RECORD_PARAM].getValue() == 0.f;

    if (toggleTriggered) {
      loop.toggle(overdubAfterRecord);
    }

    if (stopTriggered) {
      loop.stop();
    }

    if (eraseTriggered) {
      loop.erase();
    }

    if (toggleTriggered && loop.recording()) {
      channels = inputs[MAIN_INPUT].getChannels();
      outputs[MAIN_OUTPUT].setChannels(channels);
      loop.setChannels(channels);
    }

    outputs[MAIN_OUTPUT].writeVoltages(loop.process(args.sampleTime, inputs[MAIN_INPUT].getVoltages()));

    if (lightDivider.process()) {
      lights[RECORD_STATUS_LIGHT].value = 0.f;
      lights[PLAY_STATUS_LIGHT].value = 0.f;
      lights[OVERDUB_STATUS_LIGHT].value = 0.f;
      lights[STOP_STATUS_LIGHT].value = 0.f;

      if (loop.recording()) {
        lights[RECORD_STATUS_LIGHT].value = 1.f;
      }
      if (loop.playing()) {
        lights[PLAY_STATUS_LIGHT].value = 1.0;
      }
      if (loop.overdubbing()) {
        lights[OVERDUB_STATUS_LIGHT].value = 1.0;
      }
      if (loop.stopped() && loop.hasRecording()) {
        lights[STOP_STATUS_LIGHT].value = 0.8f;
      }
    }

    t += args.sampleTime;
  }
};

struct LooperWidget : ModuleWidget {
  LooperWidget(Looper *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Looper.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<CKD6>(mm2px(Vec(21.967, 26.937)), module, Looper::MODE_TOGGLE_PARAM));
    addParam(createParam<CKSS>(mm2px(Vec(20.737, 49.43)), module, Looper::AFTER_RECORD_PARAM));
    addParam(createParamCentered<CKD6>(mm2px(Vec(21.967, 69.484)), module, Looper::STOP_BUTTON_PARAM));
    addParam(createParamCentered<CKD6>(mm2px(Vec(21.951, 88.486)), module, Looper::ERASE_BUTTON_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.455, 26.937)), module, Looper::MODE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.455, 74.776)), module, Looper::STOP_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.471, 93.826)), module, Looper::ERASE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.98, 112.3)), module, Looper::MAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.462, 112.3)), module, Looper::MAIN_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.097, 43.728)), module, Looper::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12.938, 43.728)), module, Looper::PLAY_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(5.147, 50.608)), module, Looper::STOP_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(12.938, 50.608)), module, Looper::OVERDUB_STATUS_LIGHT));
  }
};

Model *modelLooper = createModel<Looper, LooperWidget>("Looper");
