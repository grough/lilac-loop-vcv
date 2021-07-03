struct SwitchOrderItem : MenuItem {
  Looper *module;
  SwitchOrder order;

  void onAction(const event::Action &e) override {
    module->order = order;
  }
};

struct SaveFileItem : MenuItem {
  Looper *module;
  AudioFileFormat format;

  void onAction(const event::Action &e) override {
    std::string dir;
    std::string filename;

    switch (format) {
    case AudioFileFormat::Wave:
      filename = "Untitled.wav";
      break;
    case AudioFileFormat::Aiff:
      filename = "Untitled.aif";
      break;
    default:
      filename = "Untitled";
      break;
    }

    if (module->saveFileFuture.valid() &&
        module->saveFileFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout) {
      osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "Previous save is still in progress. Try again later.");
      return;
    }

    if (module->mode == RECORDING || module->mode == OVERDUBBING) {
      osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "File cannot be saved while recording. Stop recording and try again.");
      return;
    }

    char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL);
    if (path) {
      module->saveFileFuture = std::async(std::launch::async, &Looper::saveFile, module, path, format);
    }
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

    MenuLabel *switchOrderLabel = new MenuLabel();
    switchOrderLabel->text = "Switching order";
    menu->addChild(switchOrderLabel);

    SwitchOrderItem *playItem = new SwitchOrderItem;
    playItem->text = "Record → Play → Overdub";
    playItem->rightText = CHECKMARK(module->order == RECORD_PLAY_OVERDUB);
    playItem->order = RECORD_PLAY_OVERDUB;
    playItem->module = module;
    menu->addChild(playItem);

    SwitchOrderItem *overdubItem = new SwitchOrderItem;
    overdubItem->text = "Record → Overdub → Play";
    overdubItem->rightText = CHECKMARK(module->order == RECORD_OVERDUB_PLAY);
    overdubItem->order = RECORD_OVERDUB_PLAY;
    overdubItem->module = module;
    menu->addChild(overdubItem);

    menu->addChild(new MenuSeparator());

    MenuLabel *saveFileLabel = new MenuLabel();
    saveFileLabel->text = "Save loop";
    menu->addChild(saveFileLabel);

    struct DepthItem : MenuItem {
      Looper *module;
      int depth;
      void onAction(const event::Action &e) override {
        module->depth = depth;
      }
    };

    struct SettingsItem : MenuItem {
      Looper *module;
      Menu *createChildMenu() override {
        Menu *menu = new Menu;

        DepthItem *item16 = new DepthItem;
        item16->text = "16 bit";
        item16->rightText = CHECKMARK(module->depth == 16);
        item16->module = module;
        item16->depth = 16;
        menu->addChild(item16);

        DepthItem *item24 = new DepthItem;
        item24->text = "24 bit";
        item24->rightText = CHECKMARK(module->depth == 24);
        item24->module = module;
        item24->depth = 24;
        menu->addChild(item24);

        return menu;
      }
    };

    SettingsItem *settingsItem = new SettingsItem;
    settingsItem->text = "File settings";
    settingsItem->rightText = RIGHT_ARROW;
    settingsItem->module = module;
    menu->addChild(settingsItem);

    SaveFileItem *saveWaveFileItem = new SaveFileItem;
    saveWaveFileItem->text = "Save WAV file (.wav)";
    saveWaveFileItem->module = module;
    saveWaveFileItem->format = AudioFileFormat::Wave;
    menu->addChild(saveWaveFileItem);

    SaveFileItem *saveAiffFileItem = new SaveFileItem;
    saveAiffFileItem->text = "Save AIFF file (.aif)";
    saveAiffFileItem->module = module;
    saveAiffFileItem->format = AudioFileFormat::Aiff;
    menu->addChild(saveAiffFileItem);
  }
};
