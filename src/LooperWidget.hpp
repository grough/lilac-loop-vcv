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

struct LooperWidget : ModuleWidget {

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
      module->fileFormat = format;
    }
  };

  struct DepthItem : MenuItem {
    Looper *module;
    int depth;

    void onAction(const event::Action &e) override {
      module->fileBitDepth = depth;
    }
  };

  struct PolyModeItem : MenuItem {
    Looper *module;
    std::string polyMode;

    void onAction(const event::Action &e) override {
      module->filePolyMode = polyMode;
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
      wavItem->rightText = CHECKMARK(module->fileFormat == "wav");
      wavItem->module = module;
      wavItem->format = "wav";
      menu->addChild(wavItem);

      FormatItem *aifItem = new FormatItem;
      aifItem->text = "AIFF (.aif)";
      aifItem->rightText = CHECKMARK(module->fileFormat == "aif");
      aifItem->module = module;
      aifItem->format = "aif";
      menu->addChild(aifItem);

      menu->addChild(new MenuSeparator());

      MenuLabel *depthLabel = new MenuLabel();
      depthLabel->text = "Bit depth";
      menu->addChild(depthLabel);

      DepthItem *item16 = new DepthItem;
      item16->text = "16 bit";
      item16->rightText = CHECKMARK(module->fileBitDepth == 16);
      item16->module = module;
      item16->depth = 16;
      menu->addChild(item16);

      DepthItem *item24 = new DepthItem;
      item24->text = "24 bit";
      item24->rightText = CHECKMARK(module->fileBitDepth == 24);
      item24->module = module;
      item24->depth = 24;
      menu->addChild(item24);

      menu->addChild(new MenuSeparator());

      MenuLabel *polyLabel = new MenuLabel();
      polyLabel->text = "Polyphony";
      menu->addChild(polyLabel);

      PolyModeItem *poly1 = new PolyModeItem;
      poly1->text = "Sum";
      poly1->rightText = CHECKMARK(module->filePolyMode == "sum");
      poly1->module = module;
      poly1->polyMode = "sum";
      menu->addChild(poly1);

      PolyModeItem *poly2 = new PolyModeItem;
      poly2->text = "Multi-track";
      poly2->rightText = CHECKMARK(module->filePolyMode == "multi");
      poly2->module = module;
      poly2->polyMode = "multi";
      menu->addChild(poly2);

      return menu;
    }
  };

  struct SaveFileItem : MenuItem {
    Looper *module;

    void onAction(const event::Action &e) override {
      AudioFileFormat format = FILE_FORMAT.at(module->fileFormat);
      PolySaveMode polyMode = FILE_POLY_MODE.at(module->filePolyMode);

      if (module->loop.size == 0) {
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
      std::string filename;

      switch (format) {

      case AudioFileFormat::Wave:
        filename = "Untitled.wav";
        break;

      case AudioFileFormat::Aiff:
        filename = "Untitled.aif";
        break;

      default:
        return;
      }

      char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL);

      if (path)
        module->writer.save(
            path,
            format,
            module->fileBitDepth,
            (int)APP->engine->getSampleRate(),
            polyMode,
            module->loop);
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
