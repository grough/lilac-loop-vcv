struct LooperOneWidget : LooperWidget {
  LooperOneWidget(Looper *module) {
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
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.658, 98.034)), module, Looper::MAIN_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.758, 98.034)), module, Looper::MAIN_2_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.658, 112.24)), module, Looper::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.758, 112.24)), module, Looper::MAIN_2_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(14.981, 35.219)), module, Looper::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(20.54, 35.219)), module, Looper::PLAY_STATUS_LIGHT));
  }
};
