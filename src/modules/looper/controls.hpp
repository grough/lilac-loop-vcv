#pragma once

#include "../../plugin.hpp"

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
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LilacPort.svg")));
  }
};

struct LilacScrew : app::SvgScrew {
  LilacScrew() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LilacScrew.svg")));
  }
};
