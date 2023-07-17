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

struct WarmKnob : RoundKnob {
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

struct LilacPort : app::SvgPort {
  LilacPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port.svg")));
  }
};

struct LooperWidget : ModuleWidget {

  struct AutosaveItem : MenuItem {
    Looper *module;
    bool enabled;

    void onAction(const event::Action &e) override {
      module->autoSaveEnabled = enabled;
    }
  };

  struct SwitchingOrderItem : MenuItem {
    Looper *module;
    SwitchingOrder switchingOrder;

    void onAction(const event::Action &e) override {
      module->switchingOrder = switchingOrder;
    }
  };

  struct FormatItem : MenuItem {
    Looper *module;
    std::string format;

    void onAction(const event::Action &e) override {
      module->writer.format = format;
    }
  };

  struct DepthItem : MenuItem {
    Looper *module;
    int depth;

    void onAction(const event::Action &e) override {
      module->writer.depth = depth;
    }
  };

  struct PolyModeItem : MenuItem {
    Looper *module;
    std::string polyMode;

    void onAction(const event::Action &e) override {
      module->writer.polyMode = polyMode;
    }
  };

  struct SettingsItem : MenuItem {
    Looper *module;
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
    Looper *module;

    void onAction(const event::Action &e) override {

      if (module->loop.length() == 0) {
#ifdef USING_CARDINAL_NOT_RACK
        async_dialog_message("Empty loop memory cannot be saved.");
#else
        osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Empty loop memory cannot be saved.");
#endif
        return;
      }

      if (module->writer.busy()) {
#ifdef USING_CARDINAL_NOT_RACK
        async_dialog_message("An earlier save is still in progress. Try again later.");
#else
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "An earlier save is still in progress. Try again later.");
#endif
        return;
      }

      if (module->mode == RECORDING || module->mode == OVERDUBBING) {
#ifdef USING_CARDINAL_NOT_RACK
        async_dialog_message("File cannot be saved while recording.");
#else
        osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "File cannot be saved while recording.");
#endif
        return;
      }

      std::string dir;
      std::string filename = module->writer.defaultFileName();
      const float sampleRate = APP->engine->getSampleRate();

#ifdef USING_CARDINAL_NOT_RACK
      Looper *module = this->module;
      async_dialog_filebrowser(true, filename.c_str(), dir.c_str(), "Export audio file...", [module, sampleRate](char *path) {
        pathSelected(module, sampleRate, path);
      });
#else
      char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL);
      pathSelected(module, sampleRate, path);
#endif
    }

    static void pathSelected(Looper *module, float sampleRate, char *path) {
      if (path) {
        module->writer.sampleRate = sampleRate;
        module->writer.save(path, module->loop);
      }
    }
  };

  void appendContextMenu(Menu *menu) override {
    Looper *module = dynamic_cast<Looper *>(this->module);

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
