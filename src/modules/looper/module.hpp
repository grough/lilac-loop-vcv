#pragma once

#include "../../plugin.hpp"
#include "osdialog.h"
#include "AudioFile.h"
#include "engine.hpp"
#include "common.hpp"
#include "MultiLoopReader.hpp"
#include "MultiLoopWriter.hpp"

struct LooperModule : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    ERASE_BUTTON_PARAM,
    UNDO_BUTTON_PARAM,
    STOP_BUTTON_PARAM,
    FEEDBACK_PARAM,       // Deprecated
    RETURN_BUTTON_PARAM,  // Deprecated
    RETURN_ENABLED_PARAM, // Deprecated
    MIX_PARAM,
    NUM_PARAMS
  };

  enum InputIds {
    MODE_CV_INPUT,
    STOP_CV_INPUT,
    ERASE_CV_INPUT,
    MAIN_1_INPUT,
    MAIN_2_INPUT,
    MIX_CV_INPUT,
    RETURN_1_DUMMY_INPUT,
    RETURN_2_DUMMY_INPUT,
    FEEDBACK_DUMMY_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    MAIN_1_OUTPUT,
    MAIN_2_OUTPUT,
    SEND_1_DUMMY_OUTPUT,
    SEND_2_DUMMY_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    RECORD_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    NUM_LIGHTS
  };

  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::BooleanTrigger eraseButtonTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger erasePolyTrigger[CHANNELS];
  dsp::BooleanTrigger rtrnButtonTrigger;

  dsp::SlewLimiter smoothInGate;
  dsp::SlewLimiter smoothOutGate;

  dsp::ClockDivider lightDivider;
  dsp::ClockDivider uiDivider;

  dsp::PulseGenerator restartPulse;
  dsp::PulseGenerator togglePulse;
  dsp::PulseGenerator clockPulse;
  float phase = 0.f;
  float blinkTime = 0.1f;

  unsigned int ins[PORTS];
  unsigned int outs[PORTS];

  rack::engine::Input feedbackInput;
  rack::engine::Param feedbackParam;
  rack::engine::Input *returnInputs[PORTS];
  rack::engine::Output *sendOutputs[PORTS];

  MultiLoopReader reader;
  MultiLoopWriter writer;
  MultiLoopWriter autoWriter;
  SwitchingOrder switchingOrder = RECORD_PLAY_OVERDUB;
  Mode mode = STOPPED;
  MultiLoop loop;

  bool armed = false;
  float feedback = 1.0f;
  float mix = 1.0f;

  bool autoSaveEnabled = false;
  std::string autoSaveDir = asset::user("LilacLoop");
  std::string autoSavePath;
  std::vector<int> autoSaveLayout;

  std::string exportFileType = "wav";
  std::string exportPolyMode = "sum";
  int exportBitDepth = 16;

  float t = 0.0f;

  LooperModule() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configButton(MODE_TOGGLE_PARAM, "Toggle");
    configButton(ERASE_BUTTON_PARAM, "Erase");
    configButton(STOP_BUTTON_PARAM, "Stop");

    configParam(MIX_PARAM, -1.0f, 1.0f, 0.0f, "Mix");

    configInput(MAIN_1_INPUT, "Left");
    configInput(MAIN_2_INPUT, "Right");
    configInput(MODE_CV_INPUT, "Toggle");
    configInput(STOP_CV_INPUT, "Stop");
    configInput(ERASE_CV_INPUT, "Erase");
    configInput(MIX_CV_INPUT, "Mix");

    configOutput(MAIN_1_OUTPUT, "Left");
    configOutput(MAIN_2_OUTPUT, "Right");

    configOutput(MAIN_1_OUTPUT, "Clock");
    configOutput(MAIN_2_OUTPUT, "Phase");

    configLight(RECORD_STATUS_LIGHT, "Record");
    configLight(PLAY_STATUS_LIGHT, "Play");

    configBypass(MAIN_1_INPUT, MAIN_1_OUTPUT);
    configBypass(MAIN_2_INPUT, MAIN_2_OUTPUT);

    ins[0] = MAIN_1_INPUT;
    ins[1] = MAIN_2_INPUT;

    outs[0] = MAIN_1_OUTPUT;
    outs[1] = MAIN_2_OUTPUT;

    smoothInGate.setRiseFall(100.f, 50.f);
    smoothOutGate.setRiseFall(100.f, 50.f);

    lightDivider.setDivision(512);
    uiDivider.setDivision(512);

    loop.resize(PORTS);
  }

  Mode getNextMode() {
    if (mode == STOPPED && loop.size == 0)
      return RECORDING;

    if (mode == RECORDING && switchingOrder == RECORD_PLAY_OVERDUB)
      return PLAYING;

    if (mode == RECORDING && switchingOrder == RECORD_OVERDUB_PLAY)
      return OVERDUBBING;

    if (mode == PLAYING)
      return OVERDUBBING;

    if (mode == OVERDUBBING)
      return PLAYING;

    if (mode == STOPPED && loop.size > 0)
      return PLAYING;

    return mode;
  }

  void toggle() {
    Mode nextMode = getNextMode();

    if (mode == STOPPED && nextMode == PLAYING)
      loop.rewind();

    mode = nextMode;
    armed = false;
    togglePulse.trigger(0.1f);
  }

  void stop() {
    mode = STOPPED;
  }

  void erase() {
    mode = STOPPED;
    loop.reset();
    system::remove(autoSavePath);
    autoSavePath = "";
  }

  void erase(int channel) {
    loop.erase(channel);

    if (loop.position == -1) {
      mode = STOPPED;
      system::remove(autoSavePath);
      autoSavePath = "";
    }
  }

  json_t *dataToJson() override {
    json_t *root = json_object();
    json_object_set_new(root, "switchingOrder", json_integer(switchingOrder));
    json_object_set_new(root, "exportFileType", json_string(writer.format.c_str()));
    json_object_set_new(root, "exportBitDepth", json_integer(writer.depth));
    json_object_set_new(root, "exportPolyMode", json_string(writer.polyMode.c_str()));
    json_object_set_new(root, "autoSaveEnabled", json_boolean(autoSaveEnabled));
    json_object_set_new(root, "autoSavePath", json_string(autoSavePath.c_str()));

    json_t *autoSaveLayoutJ = json_array();
    for (size_t p = 0; p < PORTS; p++) {
      json_array_append_new(autoSaveLayoutJ, json_integer(loop.getChannels(p)));
    }
    json_object_set_new(root, "autoSaveLayout", autoSaveLayoutJ);

    return root;
  }

  void dataFromJson(json_t *root) override {
    json_t *switchingOrderJson = json_object_get(root, "switchingOrder");
    if (switchingOrderJson)
      switchingOrder = (SwitchingOrder)json_number_value(switchingOrderJson);

    json_t *exportFileTypeJson = json_object_get(root, "exportFileType");
    if (exportFileTypeJson)
      writer.format = json_string_value(exportFileTypeJson);

    json_t *exportBitDepthJson = json_object_get(root, "exportBitDepth");
    if (exportBitDepthJson)
      writer.depth = json_number_value(exportBitDepthJson);

    json_t *exportPolyModeJson = json_object_get(root, "exportPolyMode");
    if (exportPolyModeJson)
      writer.polyMode = json_string_value(exportPolyModeJson);

    json_t *autoSaveEnabledJson = json_object_get(root, "autoSaveEnabled");
    if (autoSaveEnabledJson)
      autoSaveEnabled = json_boolean_value(autoSaveEnabledJson);

    json_t *autoSavePathJson = json_object_get(root, "autoSavePath");
    if (autoSavePathJson)
      autoSavePath = json_string_value(autoSavePathJson);

    json_t *autoSaveLayoutJson = json_object_get(root, "autoSaveLayout");
    if (autoSavePathJson) {
      size_t i;
      json_t *sizeJ;
      autoSaveLayout = {};
      json_array_foreach(autoSaveLayoutJson, i, sizeJ) {
        int size = json_number_value(sizeJ);
        autoSaveLayout.push_back(size);
      }
    }
  }

  void process(const ProcessArgs &args) override {
    Module *rightModule = getRightExpander().module;
    bool feedbackExpanded = rightModule && rightModule->model == modelLooperFeedbackExpander && !rightModule->isBypassed();
    returnInputs[0] = &(getInput(RETURN_1_DUMMY_INPUT));
    returnInputs[1] = &(getInput(RETURN_2_DUMMY_INPUT));
    sendOutputs[0] = &(getOutput(SEND_1_DUMMY_OUTPUT));
    sendOutputs[1] = &(getOutput(SEND_2_DUMMY_OUTPUT));

    if (feedbackExpanded) {
      returnInputs[0] = &(rightModule->getInput(LooperFeedbackExpander::InputId::RETURN_1_INPUT));
      returnInputs[1] = &(rightModule->getInput(LooperFeedbackExpander::InputId::RETURN_2_INPUT));
      sendOutputs[0] = &(rightModule->getOutput(LooperFeedbackExpander::OutputId::SEND_1_OUTPUT));
      sendOutputs[1] = &(rightModule->getOutput(LooperFeedbackExpander::OutputId::SEND_2_OUTPUT));
    }

    // Process toggle control

    if (toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.0f)) {
      toggle();
    }

    // Process stop control

    if (stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.0f)) {
      stop();
    }

    // Process erase control

    if (inputs[ERASE_CV_INPUT].isPolyphonic()) {
      for (int c = 0; c < inputs[ERASE_CV_INPUT].getChannels(); c++) {
        if (erasePolyTrigger[c].process(inputs[ERASE_CV_INPUT].getVoltage(c) > 0.0f)) {
          erase(c);
        }
      }
    } else {
      if (eraseTrigger.process(inputs[ERASE_CV_INPUT].getVoltage() > 0.0f)) {
        erase();
      }
    }

    if (eraseButtonTrigger.process(params[ERASE_BUTTON_PARAM].getValue() > 0.0f)) {
      erase();
    }

    // bool rtrnActive = mode != STOPPED && params[RETURN_ENABLED_PARAM].getValue() > 0.0f;
    bool rtrnActive = mode != STOPPED;

    // Process feedback param

    if (feedbackExpanded && mode != STOPPED) {
      feedback = rightModule->getParam(LooperFeedbackExpander::ParamId::FEEDBACK_PARAM).getValue() * (rightModule->getInput(LooperFeedbackExpander::InputId::FEEDBACK_CV_INPUT).isConnected() ? rightModule->getInput(LooperFeedbackExpander::InputId::FEEDBACK_CV_INPUT).getVoltage() / 10.f : 1.f);
    } else {
      feedback = 1.f;
    }

    // Process mix param

    mix = math::clamp(params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage() / 5, -1.0f, 1.0f);

    float monitorLevel = mix > 0 ? 1 - mix : 1;
    float loopLevel = mix > 0 ? 1 : 1 + mix;

    // Gates

    float inGate = smoothInGate.process(args.sampleTime, mode == RECORDING || mode == OVERDUBBING ? 1.f : 0.f);
    float outGate = smoothOutGate.process(args.sampleTime, mode == STOPPED ? 0.f : 1.f);

    // The number of "tracks" is whichever is the largest of main input channels and return input channels

    for (size_t p = 0; p < PORTS; p++) {
      int tracks = loop.setChannels(p, std::max(inputs[ins[p]].getChannels(), returnInputs[p]->getChannels()));
      outputs[outs[p]].setChannels(tracks);
      sendOutputs[p]->setChannels(tracks);
    }

    // Grow

    loop.next(mode == RECORDING);

    // Process each main port (left, right)

    for (size_t p = 0; p < PORTS; p++) {

      // Process each polyphony channel

      for (int channel = 0; channel < loop.getChannels(p); channel++) {
        float in = inputs[ins[p]].getVoltage(channel);
        float rtrn = returnInputs[p]->getVoltage(channel);

        float sample = loop.read(p, channel);
        float rtrnGate = rtrnActive && returnInputs[p]->getChannels() >= (signed)(channel + 1) ? 1.f : 0.f;
        float newSample = rtrnGate * rtrn + (1 - rtrnGate) * sample;

        loop.write(p, channel, feedback * newSample + inGate * in);

        float send = outGate * sample;
        float out = loopLevel * send + monitorLevel * in;

        outputs[outs[p]].setVoltage(out, channel);
        sendOutputs[p]->setVoltage(send, channel);
      }
    }

    if (loop.tick()) {
      clockPulse.trigger();
    }
    // outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(args.sampleTime) ? 10.f : 0.f);
    // outputs[PHASE_OUTPUT].setVoltage(loop.phase() * 10.f);

    // Lights

    if (loop.position == 0)
      restartPulse.trigger(0.1f);

    if (lightDivider.process()) {
      float lightTime = 512 * args.sampleTime;

      float restartBlink = (1.0f - togglePulse.process(lightTime)) * restartPulse.process(lightTime);

      lights[RECORD_STATUS_LIGHT].setBrightness(0.0f);
      lights[PLAY_STATUS_LIGHT].setBrightness(0.0f);

      if (mode == RECORDING || mode == OVERDUBBING) {
        lights[RECORD_STATUS_LIGHT].setBrightness(1.0f - restartBlink * .5f);
      }

      if (mode == PLAYING || mode == OVERDUBBING) {
        lights[PLAY_STATUS_LIGHT].setBrightness(1.0f - restartBlink);
      }

      if (mode == STOPPED && loop.size > 0) {
        float w = sin(6.f * M_PI * t) / 2 + 0.5f;
        lights[PLAY_STATUS_LIGHT].setBrightness(w / 3.f);
      }

      // lights[ARM_STATUS_LIGHT].value = armed;
      // lights[RETURN_LIGHT].value = returnInputs[0]->isConnected() || returnInputs[1]->isConnected() ? params[RETURN_ENABLED_PARAM].getValue() : 0.0f;
    }

    t += args.sampleTime;
  }

  std::string randomString(const int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(length);

    for (int i = 0; i < length; ++i) {
      tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
  }

  void onAdd() override {
    if (system::isFile(autoSavePath)) {
      char *path = strdup(autoSavePath.c_str());
      std::future<MultiLoop> future = reader.read(path, autoSaveLayout);
      MultiLoop ml = future.get();
      loop = ml;
    }
  }

  void onSave(const SaveEvent &e) override {
    if (!autoSaveEnabled)
      return;

    if (loop.length() == 0)
      return;

    if (autoWriter.busy())
      return;

    if (autoSavePath.empty())
      autoSavePath = system::join(autoSaveDir, "loop_" + randomString(7) + ".wav");

    system::createDirectory(autoSaveDir);
    char *path = strdup(autoSavePath.c_str());
    autoWriter.sampleRate = APP->engine->getSampleRate();
    autoWriter.polyMode = "multi";
    autoWriter.save(path, loop);

    autoSaveLayout = {};
    for (size_t p = 0; p < PORTS; p++) {
      autoSaveLayout.push_back(loop.getChannels(p));
    }
  }

  void onReset() override {
    erase();
  }

  void onRemove(const RemoveEvent &e) override {
    writer.wait();
  }
};

struct LooperWidget : ModuleWidget {

  LooperWidget(LooperModule *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Looper.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(12.7, 21.135)), module, LooperModule::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.266, 49.817)), module, LooperModule::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.47, 65.473)), module, LooperModule::ERASE_BUTTON_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(18.47, 81.129)), module, LooperModule::MIX_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 34.16)), module, LooperModule::MODE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 49.817)), module, LooperModule::STOP_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 65.473)), module, LooperModule::ERASE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 81.129)), module, LooperModule::MIX_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 97.091)), module, LooperModule::MAIN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(18.758, 97.091)), module, LooperModule::MAIN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(6.666, 112.441)), module, LooperModule::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(18.766, 112.441)), module, LooperModule::MAIN_2_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.978, 35.219)), module, LooperModule::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(20.651, 35.219)), module, LooperModule::PLAY_STATUS_LIGHT));
  }

  struct AutosaveItem : MenuItem {
    LooperModule *module;
    bool enabled;

    void onAction(const event::Action &e) override {
      module->autoSaveEnabled = enabled;
    }
  };

  struct SwitchingOrderItem : MenuItem {
    LooperModule *module;
    SwitchingOrder switchingOrder;

    void onAction(const event::Action &e) override {
      module->switchingOrder = switchingOrder;
    }
  };

  struct FormatItem : MenuItem {
    LooperModule *module;
    std::string format;

    void onAction(const event::Action &e) override {
      module->writer.format = format;
    }
  };

  struct DepthItem : MenuItem {
    LooperModule *module;
    int depth;

    void onAction(const event::Action &e) override {
      module->writer.depth = depth;
    }
  };

  struct PolyModeItem : MenuItem {
    LooperModule *module;
    std::string polyMode;

    void onAction(const event::Action &e) override {
      module->writer.polyMode = polyMode;
    }
  };

  struct SettingsItem : MenuItem {
    LooperModule *module;
    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      MenuLabel *formatLabel = new MenuLabel();
      formatLabel->text = "Format";
      menu->addChild(formatLabel);

      FormatItem *wavItem = new FormatItem;
      wavItem->text = "WAV (.wav)";
      wavItem->rightText = CHECKMARK(module->writer.format == "wav");
      wavItem->module = module;
      wavItem->format = "wav";
      menu->addChild(wavItem);

      FormatItem *aifItem = new FormatItem;
      aifItem->text = "AIFF (.aif)";
      aifItem->rightText = CHECKMARK(module->writer.format == "aif");
      aifItem->module = module;
      aifItem->format = "aif";
      menu->addChild(aifItem);

      menu->addChild(new MenuSeparator());

      MenuLabel *depthLabel = new MenuLabel();
      depthLabel->text = "Bit depth";
      menu->addChild(depthLabel);

      DepthItem *item16 = new DepthItem;
      item16->text = "16 bit";
      item16->rightText = CHECKMARK(module->writer.depth == 16);
      item16->module = module;
      item16->depth = 16;
      menu->addChild(item16);

      DepthItem *item24 = new DepthItem;
      item24->text = "24 bit";
      item24->rightText = CHECKMARK(module->writer.depth == 24);
      item24->module = module;
      item24->depth = 24;
      menu->addChild(item24);

      menu->addChild(new MenuSeparator());

      MenuLabel *polyLabel = new MenuLabel();
      polyLabel->text = "Polyphony";
      menu->addChild(polyLabel);

      PolyModeItem *poly1 = new PolyModeItem;
      poly1->text = "Sum";
      poly1->rightText = CHECKMARK(module->writer.polyMode == "sum");
      poly1->module = module;
      poly1->polyMode = "sum";
      menu->addChild(poly1);

      PolyModeItem *poly2 = new PolyModeItem;
      poly2->text = "Multi-track";
      poly2->rightText = CHECKMARK(module->writer.polyMode == "multi");
      poly2->module = module;
      poly2->polyMode = "multi";
      menu->addChild(poly2);

      return menu;
    }
  };

  struct SaveFileItem : MenuItem {
    LooperModule *module;

    void onAction(const event::Action &e) override {

      if (module->loop.length() == 0) {
        osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Empty loop memory cannot be saved.");
        return;
      }

      if (module->writer.busy()) {
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "An earlier save is still in progress. Try again later.");
        return;
      }

      if (module->mode == RECORDING || module->mode == OVERDUBBING) {
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "File cannot be saved while recording.");
        return;
      }

      std::string dir;
      std::string filename = module->writer.defaultFileName();

      char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL);

      if (path) {
        module->writer.sampleRate = APP->engine->getSampleRate();
        module->writer.save(path, module->loop);
      }
    }
  };

  void appendContextMenu(Menu *menu) override {
    LooperModule *module = dynamic_cast<LooperModule *>(this->module);

    menu->addChild(new MenuSeparator());

    MenuLabel *switchingOrderLabel = new MenuLabel();
    switchingOrderLabel->text = "Switching order";
    menu->addChild(switchingOrderLabel);

    SwitchingOrderItem *recPlayOver = new SwitchingOrderItem;
    recPlayOver->text = "Record → Play → Overdub";
    recPlayOver->rightText = CHECKMARK(module->switchingOrder == RECORD_PLAY_OVERDUB);
    recPlayOver->switchingOrder = RECORD_PLAY_OVERDUB;
    recPlayOver->module = module;
    menu->addChild(recPlayOver);

    SwitchingOrderItem *recOverPlay = new SwitchingOrderItem;
    recOverPlay->text = "Record → Overdub → Play";
    recOverPlay->rightText = CHECKMARK(module->switchingOrder == RECORD_OVERDUB_PLAY);
    recOverPlay->switchingOrder = RECORD_OVERDUB_PLAY;
    recOverPlay->module = module;
    menu->addChild(recOverPlay);

    menu->addChild(new MenuSeparator());

    MenuLabel *autoSaveLabel = new MenuLabel();
    autoSaveLabel->text = "Save audio with patch";
    menu->addChild(autoSaveLabel);

    AutosaveItem *autoSaveOnItem = new AutosaveItem;
    autoSaveOnItem->text = "On";
    autoSaveOnItem->rightText = CHECKMARK(module->autoSaveEnabled);
    autoSaveOnItem->enabled = true;
    autoSaveOnItem->module = module;
    menu->addChild(autoSaveOnItem);

    AutosaveItem *autoSaveOffItem = new AutosaveItem;
    autoSaveOffItem->text = "Off";
    autoSaveOffItem->rightText = CHECKMARK(!module->autoSaveEnabled);
    autoSaveOffItem->enabled = false;
    autoSaveOffItem->module = module;
    menu->addChild(autoSaveOffItem);

    menu->addChild(new MenuSeparator());

    MenuLabel *saveFileLabel = new MenuLabel();
    saveFileLabel->text = "Export loop";
    menu->addChild(saveFileLabel);

    SettingsItem *settingsItem = new SettingsItem;
    settingsItem->text = "File settings";
    settingsItem->rightText = RIGHT_ARROW;
    settingsItem->module = module;
    menu->addChild(settingsItem);

    SaveFileItem *saveWaveFileItem = new SaveFileItem;
    saveWaveFileItem->text = "Export audio file…";
    saveWaveFileItem->module = module;
    menu->addChild(saveWaveFileItem);

    // menu->addChild(new MenuSeparator());

    // menu->addChild(createMenuItem("Show hidden modules…", "", [=]() {
    //   system::openBrowser("https://grough.github.io/lilac-loop-vcv#hidden-modules");
    // }));
  }
};

Model *modelLooper = createModel<LooperModule, LooperWidget>("Looper");