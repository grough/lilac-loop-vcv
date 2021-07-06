struct LooperTwoWidget : ModuleWidget {

  LooperTwoWidget(LooperTwo *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LooperTwo.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(51.971, 27.534)), module, LooperTwo::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(72.767, 62.246)), module, LooperTwo::ERASE_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(46.698, 62.277)), module, LooperTwo::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(47.25, 87.693)), module, LooperTwo::FEEDBACK_PARAM));
    addParam(createParamCentered<WarmLEDButton>(mm2px(Vec(13.723, 93.5)), module, LooperTwo::RETURN_BUTTON_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.723, 93.5)), module, LooperTwo::RETURN_LIGHT));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(46.901, 112.213)), module, LooperTwo::MIX_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.089, 40.194)), module, LooperTwo::MODE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.954, 62.246)), module, LooperTwo::ERASE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.089, 62.277)), module, LooperTwo::STOP_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.673, 87.693)), module, LooperTwo::RETURN_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.773, 87.693)), module, LooperTwo::RETURN_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.773, 96.693)), module, LooperTwo::RETURN_MOD_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.437, 87.693)), module, LooperTwo::FEEDBACK_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.089, 112.213)), module, LooperTwo::MIX_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.907, 112.423)), module, LooperTwo::MAIN_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.007, 112.423)), module, LooperTwo::MAIN_2_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.082, 87.693)), module, LooperTwo::SEND_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(73.182, 87.693)), module, LooperTwo::SEND_2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.374, 112.24)), module, LooperTwo::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(73.474, 112.24)), module, LooperTwo::MAIN_2_OUTPUT));

    addChild(createLightCentered<LargeLight<RedLight>>(mm2px(Vec(60.954, 42.772)), module, LooperTwo::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<LargeLight<GreenLight>>(mm2px(Vec(72.767, 42.772)), module, LooperTwo::PLAY_STATUS_LIGHT));
  }

  struct OrderItem : MenuItem {
    LooperTwo *module;
    Order order;

    void onAction(const event::Action &e) override {
      module->order = order;
    }
  };

  void appendContextMenu(Menu *menu) override {
    LooperTwo *module = dynamic_cast<LooperTwo *>(this->module);

    menu->addChild(new MenuSeparator());

    MenuLabel *switchOrderLabel = new MenuLabel();
    switchOrderLabel->text = "Switching order";
    menu->addChild(switchOrderLabel);

    OrderItem *playItem = new OrderItem;
    playItem->text = "Record → Play → Overdub";
    playItem->rightText = CHECKMARK(module->order == RECORD_PLAY_OVERDUB);
    playItem->order = RECORD_PLAY_OVERDUB;
    playItem->module = module;
    menu->addChild(playItem);

    OrderItem *overdubItem = new OrderItem;
    overdubItem->text = "Record → Overdub → Play";
    overdubItem->rightText = CHECKMARK(module->order == RECORD_OVERDUB_PLAY);
    overdubItem->order = RECORD_OVERDUB_PLAY;
    overdubItem->module = module;
    menu->addChild(overdubItem);
  };
};
