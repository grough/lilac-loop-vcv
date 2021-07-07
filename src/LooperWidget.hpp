struct LargeWarmButton : SvgSwitch {
  LargeWarmButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LargeWarmButton_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LargeWarmButton_1.svg")));
  }
};

struct WarmButton : SvgSwitch {
  WarmButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WarmButton_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WarmButton_1.svg")));
  }
};

struct WarmKnob : Davies1900hKnob {
  WarmKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WarmKnob.svg")));
  }
};

struct WarmLEDButton : app::SvgSwitch {
  WarmLEDButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WarmLEDButton.svg")));
  }
};

struct LooperWidget : ModuleWidget {

  struct SwitchOrderItem : MenuItem {
    Looper *module;
    Order order;

    void onAction(const event::Action &e) override {
      module->order = order;
    }
  };

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

      if (module->size == 0) {
        osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Empty loop memory cannot be saved.");
        return;
      }

      if (module->fileSaver.busy()) {
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "An earlier save is still in progress. Try again later.");
        return;
      }

      if (module->mode == RECORDING || module->mode == OVERDUBBING) {
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "File cannot be saved while recording. Stop recording and try again.");
        return;
      }

      char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL);

      if (path)
        module->fileSaver.save(
            path,
            (int)APP->engine->getSampleRate(),
            format,
            module->depth,
            module->loop);
    }
  };

  void appendContextMenu(Menu *menu) override {
    Looper *module = dynamic_cast<Looper *>(this->module);

    menu->addChild(new MenuSeparator());

    MenuLabel *switchOrderLabel = new MenuLabel();
    switchOrderLabel->text = "Switching order …";
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
