
#pragma once

#include "../../plugin.hpp"
#include "osdialog.h"
#include "../looper/AudioFile.h"
#include "../looper/engine.hpp"
#include "../looper/common.hpp"
#include "../looper/MultiLoopReader.hpp"
#include "../looper/MultiLoopWriter.hpp"

struct LooperTwoModule : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    ERASE_BUTTON_PARAM,
    UNDO_BUTTON_PARAM,
    STOP_BUTTON_PARAM,
    FEEDBACK_PARAM,
    RETURN_BUTTON_PARAM,
    RETURN_ENABLED_PARAM,
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
    ARM_CV_INPUT,
    UNDO_CV_INPUT,
    RETURN_1_INPUT,
    RETURN_2_INPUT,
    FEEDBACK_CV_INPUT,
    RETURN_MOD_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    MAIN_1_OUTPUT,
    MAIN_2_OUTPUT,
    SEND_1_OUTPUT,
    SEND_2_OUTPUT,
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    RECORD_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    ARM_STATUS_LIGHT,
    RETURN_LIGHT,
    NUM_LIGHTS
  };

  dsp::BooleanTrigger armTrigger;
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
  unsigned int rtrns[PORTS];
  unsigned int snds[PORTS];
  unsigned int outs[PORTS];

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

  LooperTwoModule() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configButton(MODE_TOGGLE_PARAM, "Toggle");
    configButton(ERASE_BUTTON_PARAM, "Erase");
    configButton(STOP_BUTTON_PARAM, "Stop");
    configButton(RETURN_BUTTON_PARAM, "Return enabled");
    configButton(RETURN_ENABLED_PARAM);

    configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 1.0f, "Feedback", "%", 0.0f, 100.0f);
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

    rtrns[0] = RETURN_1_INPUT;
    rtrns[1] = RETURN_2_INPUT;

    snds[0] = SEND_1_OUTPUT;
    snds[1] = SEND_2_OUTPUT;

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
    if (inputs[ARM_CV_INPUT].isConnected() && !armed)
      return;

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

    // Process arm control

    if (armTrigger.process(inputs[ARM_CV_INPUT].getVoltage() > 0.0f)) {
      armed = true;
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

    // Process return enable control

    if (rtrnButtonTrigger.process(params[RETURN_BUTTON_PARAM].getValue() > 0.0f)) {
      params[RETURN_ENABLED_PARAM].setValue(1.0f - params[RETURN_ENABLED_PARAM].getValue());
    }

    bool rtrnActive = mode != STOPPED && params[RETURN_ENABLED_PARAM].getValue() > 0.0f;

    // Process feedback param

    if (mode == STOPPED) {
      feedback = 1.0f;
    } else {
      feedback = math::clamp(params[FEEDBACK_PARAM].getValue() + inputs[FEEDBACK_CV_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
    }

    // Process mix param

    mix = math::clamp(params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage() / 5, -1.0f, 1.0f);

    float monitorLevel = mix > 0 ? 1 - mix : 1;
    float loopLevel = mix > 0 ? 1 : 1 + mix;

    // Process return mod input

    float mod = inputs[RETURN_MOD_INPUT].isConnected() ? inputs[RETURN_MOD_INPUT].getVoltage() : 1.0f;

    // Gates

    float inGate = smoothInGate.process(args.sampleTime, mode == RECORDING || mode == OVERDUBBING ? 1.f : 0.f);
    float outGate = smoothOutGate.process(args.sampleTime, mode == STOPPED ? 0.f : 1.f);

    // Count inputs

    for (size_t p = 0; p < PORTS; p++) {
      int tracks = loop.setChannels(p, std::max(inputs[ins[p]].getChannels(), inputs[rtrns[p]].getChannels()));
      outputs[outs[p]].setChannels(tracks);
      outputs[snds[p]].setChannels(tracks);
    }

    // Grow

    loop.next(mode == RECORDING);

    // Process each main port (left, right)

    for (size_t p = 0; p < PORTS; p++) {

      // Process each polyphony channel

      for (int channel = 0; channel < loop.getChannels(p); channel++) {
        float in = inputs[ins[p]].getVoltage(channel);
        float rtrn = inputs[rtrns[p]].getVoltage(channel);

        float sample = loop.read(p, channel);
        float rtrnGate = rtrnActive && inputs[rtrns[p]].getChannels() >= (signed)(channel + 1) ? mod : 0.0f;
        float newSample = rtrnGate * rtrn + (1 - rtrnGate) * sample;

        loop.write(p, channel, feedback * newSample + inGate * in);

        float send = outGate * sample;
        float out = loopLevel * send + monitorLevel * in;

        outputs[outs[p]].setVoltage(out, channel);
        outputs[snds[p]].setVoltage(send, channel);
      }
    }

    if (loop.tick()) {
      clockPulse.trigger();
    }
    outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(args.sampleTime) ? 10.f : 0.f);
    outputs[PHASE_OUTPUT].setVoltage(loop.phase() * 10.f);

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

      lights[ARM_STATUS_LIGHT].value = armed;
      lights[RETURN_LIGHT].value = inputs[rtrns[0]].isConnected() || inputs[rtrns[1]].isConnected() ? params[RETURN_ENABLED_PARAM].getValue() : 0.0f;
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

struct LooperTwoWidget : ModuleWidget {
  LooperTwoWidget(LooperTwoModule *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LooperTwo.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(51.971, 27.534)), module, LooperTwoModule::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(72.767, 62.246)), module, LooperTwoModule::ERASE_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(46.698, 62.277)), module, LooperTwoModule::STOP_BUTTON_PARAM));
    addParam(createParamCentered<LilacKnob>(mm2px(Vec(47.25, 87.693)), module, LooperTwoModule::FEEDBACK_PARAM));
    addParam(createParamCentered<WarmLEDButton>(mm2px(Vec(13.723, 93.5)), module, LooperTwoModule::RETURN_BUTTON_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.723, 93.5)), module, LooperTwoModule::RETURN_LIGHT));
    addParam(createParamCentered<LilacKnob>(mm2px(Vec(46.901, 112.213)), module, LooperTwoModule::MIX_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.089, 40.194)), module, LooperTwoModule::MODE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(8, 40.194)), module, LooperTwoModule::ARM_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(60.954, 62.246)), module, LooperTwoModule::ERASE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.089, 62.277)), module, LooperTwoModule::STOP_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.673, 87.693)), module, LooperTwoModule::RETURN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(19.773, 87.693)), module, LooperTwoModule::RETURN_2_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.437, 87.693)), module, LooperTwoModule::FEEDBACK_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.089, 112.213)), module, LooperTwoModule::MIX_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.907, 112.423)), module, LooperTwoModule::MAIN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(20.007, 112.423)), module, LooperTwoModule::MAIN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(61.082, 87.693)), module, LooperTwoModule::SEND_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(73.182, 87.693)), module, LooperTwoModule::SEND_2_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(61.374, 112.24)), module, LooperTwoModule::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(73.474, 112.24)), module, LooperTwoModule::MAIN_2_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(47.086, 123.23)), module, LooperTwoModule::CLOCK_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(57.599, 123.23)), module, LooperTwoModule::PHASE_OUTPUT));

    addChild(createLightCentered<LargeLight<RedLight>>(mm2px(Vec(60.954, 42.772)), module, LooperTwoModule::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<LargeLight<GreenLight>>(mm2px(Vec(72.767, 42.772)), module, LooperTwoModule::PLAY_STATUS_LIGHT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20, 40.194)), module, LooperTwoModule::ARM_STATUS_LIGHT));
  }

  struct AutosaveItem : MenuItem {
    LooperTwoModule *module;
    bool enabled;

    void onAction(const event::Action &e) override {
      module->autoSaveEnabled = enabled;
    }
  };

  struct SwitchingOrderItem : MenuItem {
    LooperTwoModule *module;
    SwitchingOrder switchingOrder;

    void onAction(const event::Action &e) override {
      module->switchingOrder = switchingOrder;
    }
  };

  struct FormatItem : MenuItem {
    LooperTwoModule *module;
    std::string format;

    void onAction(const event::Action &e) override {
      module->writer.format = format;
    }
  };

  struct DepthItem : MenuItem {
    LooperTwoModule *module;
    int depth;

    void onAction(const event::Action &e) override {
      module->writer.depth = depth;
    }
  };

  struct PolyModeItem : MenuItem {
    LooperTwoModule *module;
    std::string polyMode;

    void onAction(const event::Action &e) override {
      module->writer.polyMode = polyMode;
    }
  };

  struct SettingsItem : MenuItem {
    LooperTwoModule *module;
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
    LooperTwoModule *module;

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
    LooperTwoModule *module = dynamic_cast<LooperTwoModule *>(this->module);

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
  }
};

Model *modelLooperTwo = createModel<LooperTwoModule, LooperTwoWidget>("LooperTwo");