#include "loop.hpp"
#include "plugin.hpp"
#include <math.h>

struct Looper : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
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

  unsigned int channels = 1;
  Loop loop;
  Mode mode, nextMode;
  bool overdubAfterRecord = false;
  float t = 0.f;
  float blinkTime = .1f;
  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::ClockDivider lightDivider;
  dsp::ClockDivider logDivider;
  dsp::PulseGenerator atZeroPulse;
  dsp::PulseGenerator togglePulse;

  Looper() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    configParam(ERASE_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    lightDivider.setDivision(pow(2, 9));
    logDivider.setDivision(pow(2, 13));
  }

  void process(const ProcessArgs &args) override {

    bool toggleTriggered = toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.f);
    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.f);
    bool eraseTriggered = eraseTrigger.process(params[ERASE_BUTTON_PARAM].getValue() + inputs[ERASE_CV_INPUT].getVoltage() > 0.f);

    if (toggleTriggered) {
      loop.toggle(overdubAfterRecord);
      togglePulse.trigger(blinkTime);
    }

    if (stopTriggered) {
      loop.stop();
    }

    if (eraseTriggered) {
      loop.erase();
    }

    mode = loop.getMode();
    nextMode = loop.getNextMode(overdubAfterRecord);

    if (toggleTriggered && mode == RECORDING) {
      channels = inputs[MAIN_INPUT].getChannels();
      outputs[MAIN_OUTPUT].setChannels(channels);
      loop.setChannels(channels);
    }

    outputs[MAIN_OUTPUT].writeVoltages(loop.process(args.sampleTime, inputs[MAIN_INPUT].getVoltages()));

    if (loop.atZero()) {
      atZeroPulse.trigger(blinkTime);
    }
    // Blink when loop restarts, but not right after toggling
    float atZeroBlink = (1.f - togglePulse.process(args.sampleTime)) * atZeroPulse.process(args.sampleTime);

    if (lightDivider.process()) {
      lights[RECORD_STATUS_LIGHT].setBrightness(0.f);
      lights[PLAY_STATUS_LIGHT].setBrightness(0.f);
      if (mode == RECORDING || mode == OVERDUBBING) {
        lights[RECORD_STATUS_LIGHT].setBrightness(1.f - atZeroBlink * .5f);
      }
      if (mode == PLAYING || mode == OVERDUBBING) {
        lights[PLAY_STATUS_LIGHT].setBrightness(1.f - atZeroBlink);
      }
      if (mode == STOPPED && !loop.empty()) {
        float w = sin(6.f * M_PI * t) / 2 + 0.5f;
        lights[PLAY_STATUS_LIGHT].setBrightness(w / 3.f);
      }
    }
    t += args.sampleTime;
  }
};

struct LargeWarmButton : SvgSwitch {
  LargeWarmButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button_warm_large_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button_warm_large_1.svg")));
  }
};

struct WarmButton : SvgSwitch {
  WarmButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button_warm_medium_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button_warm_medium_1.svg")));
  }
};

struct AfterRecordItem : MenuItem {
  Looper *module;
  bool overdubAfterRecord;
  void onAction(const event::Action &e) override {
    module->overdubAfterRecord = overdubAfterRecord;
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

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(12.7, 28.687)), module, Looper::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.266, 68.457)), module, Looper::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.47, 89.149)), module, Looper::ERASE_BUTTON_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.761, 44.215)), module, Looper::MODE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.761, 68.457)), module, Looper::STOP_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.761, 89.149)), module, Looper::ERASE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.761, 112.651)), module, Looper::MAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.657, 112.429)), module, Looper::MAIN_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(14.981, 45.273)), module, Looper::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(20.54, 45.273)), module, Looper::PLAY_STATUS_LIGHT));
  }

  void appendContextMenu(Menu *menu) override {
    Looper *module = dynamic_cast<Looper *>(this->module);

    menu->addChild(new MenuSeparator());

    MenuLabel *afterRecordLabel = new MenuLabel();
    afterRecordLabel->text = "After first loop…";
    menu->addChild(afterRecordLabel);

    AfterRecordItem *playItem = new AfterRecordItem;
    playItem->text = "Play";
    playItem->rightText = CHECKMARK(!module->overdubAfterRecord);
    playItem->overdubAfterRecord = false;
    playItem->module = module;
    menu->addChild(playItem);

    AfterRecordItem *overdubItem = new AfterRecordItem;
    overdubItem->text = "Overdub";
    overdubItem->rightText = CHECKMARK(module->overdubAfterRecord);
    overdubItem->overdubAfterRecord = true;
    overdubItem->module = module;
    menu->addChild(overdubItem);
  }
};

Model *modelLooper = createModel<Looper, LooperWidget>("Looper");
