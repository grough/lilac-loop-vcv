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

struct LilacKnob : app::SvgKnob {
  widget::SvgWidget *bg;

  LilacKnob() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    bg = new widget::SvgWidget;
    fb->addChildBelow(bg, tw);
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LilacKnob_fg.svg")));
    bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LilacKnob_bg.svg")));
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
