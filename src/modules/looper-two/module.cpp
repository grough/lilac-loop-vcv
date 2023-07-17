#include "../../plugin.hpp"
#include "../looper/module.hpp"

struct LooperTwoWidget : LooperWidget {
  LooperTwoWidget(LooperModule *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LooperTwo.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(51.971, 27.534)), module, LooperModule::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(72.767, 62.246)), module, LooperModule::ERASE_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(46.698, 62.277)), module, LooperModule::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(47.25, 87.693)), module, LooperModule::FEEDBACK_PARAM));
    addParam(createParamCentered<WarmLEDButton>(mm2px(Vec(13.723, 93.5)), module, LooperModule::RETURN_BUTTON_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.723, 93.5)), module, LooperModule::RETURN_LIGHT));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(46.901, 112.213)), module, LooperModule::MIX_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.089, 40.194)), module, LooperModule::MODE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(8, 40.194)), module, LooperModule::ARM_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(60.954, 62.246)), module, LooperModule::ERASE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.089, 62.277)), module, LooperModule::STOP_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.673, 87.693)), module, LooperModule::RETURN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(19.773, 87.693)), module, LooperModule::RETURN_2_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.437, 87.693)), module, LooperModule::FEEDBACK_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(35.089, 112.213)), module, LooperModule::MIX_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.907, 112.423)), module, LooperModule::MAIN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(20.007, 112.423)), module, LooperModule::MAIN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(61.082, 87.693)), module, LooperModule::SEND_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(73.182, 87.693)), module, LooperModule::SEND_2_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(61.374, 112.24)), module, LooperModule::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(73.474, 112.24)), module, LooperModule::MAIN_2_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(47.086, 123.23)), module, LooperModule::CLOCK_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(57.599, 123.23)), module, LooperModule::PHASE_OUTPUT));

    addChild(createLightCentered<LargeLight<RedLight>>(mm2px(Vec(60.954, 42.772)), module, LooperModule::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<LargeLight<GreenLight>>(mm2px(Vec(72.767, 42.772)), module, LooperModule::PLAY_STATUS_LIGHT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20, 40.194)), module, LooperModule::ARM_STATUS_LIGHT));
  }
};

Model *modelLooperTwo = createModel<LooperModule, LooperTwoWidget>("LooperTwo");