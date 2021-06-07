#include "plugin.hpp"
#include <math.h>

#define MULTI 2      // Left, right
#define POLY 16      // Polyphony channels per input/output
#define MULTIPOLY 32 // Maximum number of loop tracks (MULTI * POLY)

struct Looper : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    STOP_BUTTON_PARAM,
    ERASE_BUTTON_PARAM,
    MIX_PARAM,
    NUM_PARAMS,
  };
  enum InputIds {
    MODE_CV_INPUT,
    STOP_CV_INPUT,
    ERASE_CV_INPUT,
    LEFT_INPUT,
    RIGHT_INPUT,
    MIX_CV_INPUT,
    NUM_INPUTS,
  };
  enum OutputIds {
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
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

  enum Mode {
    STOPPED,
    RECORDING,
    PLAYING,
    OVERDUBBING,
  };

  unsigned int size = 0;
  unsigned int position = 0;
  unsigned int tracks[MULTI];
  std::vector<float> loop[MULTIPOLY];
  bool loopEmpty[MULTIPOLY];
  int start[MULTIPOLY];
  unsigned int pos[MULTIPOLY];
  Mode mode = STOPPED;
  bool overdubAfterRecord = false;
  float t = 0.f;
  float blinkTime = .1f;
  float mix = 0.f;
  engine::Input *ins[MULTI];
  engine::Output *outs[MULTI];
  dsp::SlewLimiter inputSmoother;
  dsp::SlewLimiter outputSmoother;
  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger eraseTrigger;
  //separate trigger for erase button because triggers can't be reused
  dsp::BooleanTrigger eraseButtonTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::ClockDivider lightDivider;
  dsp::ClockDivider logDivider;
  dsp::PulseGenerator restartPulse;
  dsp::PulseGenerator togglePulse;

  Looper() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.f, 1.f, 0.f, "Toggle");
    configParam(STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "Stop");
    configParam(ERASE_BUTTON_PARAM, 0.f, 1.f, 0.f, "Erase");
    configParam(MIX_PARAM, -1.f, 1.f, 0.f, "Mix");

    ins[0] = &inputs[LEFT_INPUT];
    ins[1] = &inputs[RIGHT_INPUT];
    outs[0] = &outputs[LEFT_OUTPUT];
    outs[1] = &outputs[RIGHT_OUTPUT];

    inputSmoother.setRiseFall(100.f, 50.f);
    outputSmoother.setRiseFall(100.f, 50.f);

    lightDivider.setDivision(pow(2, 9));
    logDivider.setDivision(pow(2, 13));

    for (size_t i = 0; i < MULTIPOLY; i++) {
      pos[i] = 0;
      start[i] = -1;
      loopEmpty[i] = true;
    }
  }

  Mode getNextMode(bool overdubAfterRecord) {
    if (mode == STOPPED && size == 0) {
      return RECORDING;
    }
    if (mode == RECORDING) {
      return overdubAfterRecord ? OVERDUBBING : PLAYING;
    }
    if (mode == PLAYING) {
      return OVERDUBBING;
    }
    if (mode == OVERDUBBING) {
      return PLAYING;
    }
    if (mode == STOPPED && size > 0) {
      return PLAYING;
    }
    return mode;
  }

  void process(const ProcessArgs &args) override {
    int eraseButtonTriggered = eraseButtonTrigger.process(params[ERASE_BUTTON_PARAM].getValue());
    bool toggleTriggered = toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.f);
    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.f);
    bool eraseTriggered = eraseTrigger.process(params[ERASE_BUTTON_PARAM].getValue() + inputs[ERASE_CV_INPUT].getVoltage() > 0.f);
    float mix = math::clamp(params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage() / 5, -1.f, 1.f);


    if (toggleTriggered) {
      Mode next = getNextMode(overdubAfterRecord);
      if (mode == STOPPED && next == PLAYING) {
        position = 0;
        for (size_t i = 0; i < MULTI; i++) {
          for (size_t c = 0; c < tracks[i]; c++) {
            int track = i * 16 + c;
            if (start[track] == 0) {
              pos[track] = 0;
            } else {
              pos[track] = size - start[track];
            }
          }
        }
      }
      mode = next;
      togglePulse.trigger(blinkTime);
    }

    if (stopTriggered) {
      mode = STOPPED;
    }

    //erase is polyphonic, so only the first signal is received
    //by the initial check
    bool erasing = eraseTriggered;
    bool monophonicErase = inputs[ERASE_CV_INPUT].getChannels() == 1;

    bool eraseAll = true;
    if(!eraseButtonTriggered && !monophonicErase) {
      for (size_t c = 0; c < POLY; c++) {
        //for each input erase both the left and right tracks associated with it
        if (inputs[ERASE_CV_INPUT].getVoltage(c)) {
          erasing = true;
          for (size_t i = 0; i < MULTI; i++) {
            int track = i * 16 + c;
            if(!loop[track].empty()) {
              clearTrack(track);
            }
          }
        }
      }
    } else if( (eraseButtonTriggered) || (eraseTriggered && monophonicErase)) {
      eraseAll = true;
      erasing = true;
      for (size_t i = 0; i < MULTI; i++) {
        tracks[i] = 0;
        for (size_t c = 0; c < POLY; c++) {
          int track = i * 16 + c;
          clearTrack(track);
        }
      }
    }

if(erasing) {
    //loop through all tracks and if they are all marked as erased, then stop the loop
    for (size_t c = 0; c < POLY; c++) {
      if((loopEmpty[c] == false|| loopEmpty[c+16] == false ) ) {
        eraseAll = false;
        break;
      }
    }

    //if eraseAll is true set tracks to 0, position to 0 and size to 0, and stop the loop
    if(eraseAll && erasing) {
      mode = STOPPED;
      position = 0;
      size = 0;
      for (size_t i = 0; i < MULTI; i++) {
        tracks[i] = 0;
        for (size_t c = 0; c < POLY; c++) {
          int track = i * 16 + c;
          clearTrack(track);
        }
      }
    }
  }

    float inputGate = inputSmoother.process(args.sampleTime, mode == RECORDING || mode == OVERDUBBING ? 1.f : 0.f);
    float outputGate = outputSmoother.process(args.sampleTime, mode == STOPPED ? 0.f : 1.f);
    float monitorLevel = mix > 0 ? 1 - mix : 1;
    float loopLevel = mix > 0 ? 1 : 1 + mix;
    if (mode == RECORDING) {
      size++;
    }
    // Process each input (left, right)
    for (size_t i = 0; i < MULTI; i++) {
      int channels = ins[i]->getChannels();
      if (channels > tracks[i] || size == 0) {
        tracks[i] = channels;
        outputs[i].setChannels(channels);
      }
      // Process each polyphony channel
      for (size_t channel = 0; channel < tracks[i]; channel++) {
        int track = i * 16 + channel;
        float in = ins[i]->getVoltage(channel);
        float out;
        if (start[track] == -1 && size > 0) {
          start[track] = position;
        }
        if (loop[track].size() < size) {
          loop[track].push_back(0.f);
        }
        if (loop[track].empty()) {
          out = monitorLevel * in;
        } else {
          if(std::abs(in) > .01 && loopEmpty[track]) {
            loopEmpty[track] = false;
          }
          out = monitorLevel * in + loopLevel * outputGate * loop[track][pos[track]];
          loop[track][pos[track]] += inputGate * in;
          pos[track]++;
        }
        if (pos[track] == size) {
          pos[track] = 0;
        }
        outs[i]->setVoltage(out, channel);
      }
    }
    if (size > 0) {
      position++;
    }
    if (position == size) {
      position = 0;
    }
    if (position == 0) {
      restartPulse.trigger(blinkTime);
    }
    // Blink when loop restarts, but not right after toggling
    float restartBlink = (1.f - togglePulse.process(args.sampleTime)) * restartPulse.process(args.sampleTime);

    if (lightDivider.process()) {
      lights[RECORD_STATUS_LIGHT].setBrightness(0.f);
      lights[PLAY_STATUS_LIGHT].setBrightness(0.f);
      if (mode == RECORDING || mode == OVERDUBBING) {
        lights[RECORD_STATUS_LIGHT].setBrightness(1.f - restartBlink * .5f);
      }
      if (mode == PLAYING || mode == OVERDUBBING) {
        lights[PLAY_STATUS_LIGHT].setBrightness(1.f - restartBlink);
      }
      if (mode == STOPPED && size > 0) {
        float w = sin(6.f * M_PI * t) / 2 + 0.5f;
        lights[PLAY_STATUS_LIGHT].setBrightness(w / 3.f);
      }
    }
    t += args.sampleTime;
  }

  void clearTrack(int track) {
    loop[track].clear();
    pos[track] = 0;
    start[track] = -1;
    loopEmpty[track] = true;
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

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(12.7, 21.135)), module, Looper::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.266, 50.015)), module, Looper::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.47, 65.87)), module, Looper::ERASE_BUTTON_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(18.47, 81.725)), module, Looper::MIX_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.658, 34.16)), module, Looper::MODE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.658, 50.015)), module, Looper::STOP_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.658, 65.87)), module, Looper::ERASE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.658, 81.725)), module, Looper::MIX_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.658, 98.034)), module, Looper::LEFT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.758, 98.034)), module, Looper::RIGHT_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.658, 112.24)), module, Looper::LEFT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.758, 112.24)), module, Looper::RIGHT_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(14.981, 35.219)), module, Looper::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(20.54, 35.219)), module, Looper::PLAY_STATUS_LIGHT));
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
