#define PORTS 2     // Number of main I/O ports
#define CHANNELS 16 // Polyphony per port

struct LooperTwo : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    ERASE_BUTTON_PARAM,
    UNDO_BUTTON_PARAM,
    STOP_BUTTON_PARAM,
    FEEDBACK_PARAM,
    RETURN_BUTTON_PARAM,
    MIX_PARAM,
    NUM_PARAMS
  };

  enum InputIds {
    ARM_CV_INPUT,
    MODE_CV_INPUT,
    ERASE_CV_INPUT,
    UNDO_CV_INPUT,
    STOP_CV_INPUT,
    RETURN_1_INPUT,
    RETURN_2_INPUT,
    FEEDBACK_CV_INPUT,
    MIX_CV_INPUT,
    MAIN_1_INPUT,
    MAIN_2_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    SEND_1_OUTPUT,
    SEND_2_OUTPUT,
    MAIN_1_OUTPUT,
    MAIN_2_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    ARM_STATUS_LIGHT,
    RECORD_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    RETURN_LIGHT,
    NUM_LIGHTS
  };

  engine::Input *ins[PORTS];
  engine::Input *rtrns[PORTS];
  engine::Output *snds[PORTS];
  engine::Output *outs[PORTS];

  LoopController lc{PORTS, CHANNELS};

  Order order;

  dsp::BooleanTrigger armTrigger;
  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger eraseButtonTrigger;
  dsp::BooleanTrigger rtrnButtonTrigger;

  dsp::ClockDivider lightDivider;
  dsp::ClockDivider logDivider;

  dsp::PulseGenerator restartPulse;
  dsp::PulseGenerator togglePulse;
  float blinkTime = 0.1f;

  float t = 0.0f;

  LooperTwo() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.0f, 1.0f, 0.0f, "Toggle");
    configParam(ERASE_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Erase");
    configParam(STOP_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Stop");
    configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 1.0f, "Feedback", "%", 0.0f, 100.0f);
    configParam(RETURN_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Return enabled");
    configParam(MIX_PARAM, -1.0f, 1.0f, 0.0f, "Mix");

    ins[0] = &inputs[MAIN_1_INPUT];
    ins[1] = &inputs[MAIN_2_INPUT];
    rtrns[0] = &inputs[RETURN_1_INPUT];
    rtrns[1] = &inputs[RETURN_2_INPUT];
    snds[0] = &outputs[SEND_1_OUTPUT];
    snds[1] = &outputs[SEND_2_OUTPUT];
    outs[0] = &outputs[MAIN_1_OUTPUT];
    outs[1] = &outputs[MAIN_2_OUTPUT];

    order = RECORD_PLAY_OVERDUB;

    lightDivider.setDivision(pow(2, 9));
    logDivider.setDivision(pow(2, 13));
  }

  void process(const ProcessArgs &args) override {

    for (size_t p = 0; p < PORTS; p++) {
      lc.setInputsConnected(p, ins[p]->getChannels());
      lc.setRtrnsConnected(p, rtrns[p]->getChannels());

      outs[p]->setChannels(lc.getChannels(p));
      snds[p]->setChannels(lc.getChannels(p));

      for (size_t c = 0; c < CHANNELS; c++) {
        lc.setInput(p, c, ins[p]->getVoltage(c));
        lc.setRtrn(p, c, rtrns[p]->getVoltage(c));

        outs[p]->setVoltage(lc.getOutput(p, c), c);
        snds[p]->setVoltage(lc.getSend(p, c), c);
      }
    }

    // Process toggle control

    bool toggleTriggered = toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.0f);

    if (toggleTriggered) {
      lc.toggle(order);
      togglePulse.trigger(blinkTime);
    }

    // Process stop control

    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.0f);

    if (stopTriggered) {
      lc.stop();
    }

    // Process erase control

    if (eraseTrigger.process(inputs[ERASE_CV_INPUT].getVoltage() > 0.0f)) {
      lc.erase();
    }

    if (eraseButtonTrigger.process(params[ERASE_BUTTON_PARAM].getValue() > 0.0f)) {
      lc.erase();
    }

    // Process return button param

    if (rtrnButtonTrigger.process(params[RETURN_BUTTON_PARAM].getValue() > 0.0f) &&
        (rtrns[0]->isConnected() || rtrns[1]->isConnected())) {
      lc.rtrnEnabled = !lc.rtrnEnabled;
    }

    // Process feedback param

    lc.feedback = math::clamp(params[FEEDBACK_PARAM].getValue() + inputs[FEEDBACK_CV_INPUT].getVoltage(), 0.0f, 1.0f);

    // Process mix param

    lc.mix = math::clamp(params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage() / 5, -1.0f, 1.0f);

    // Step forward

    lc.process(args.sampleTime);

    // Lights

    if (lc.position == 0) {
      restartPulse.trigger(blinkTime);
    }

    float restartBlink = (1.0f - togglePulse.process(args.sampleTime)) * restartPulse.process(args.sampleTime);

    if (lightDivider.process()) {
      lights[RECORD_STATUS_LIGHT].setBrightness(0.0f);
      lights[PLAY_STATUS_LIGHT].setBrightness(0.0f);

      if (lc.mode == RECORDING || lc.mode == OVERDUBBING) {
        lights[RECORD_STATUS_LIGHT].setBrightness(1.0f - restartBlink * .5f);
      }

      if (lc.mode == PLAYING || lc.mode == OVERDUBBING) {
        lights[PLAY_STATUS_LIGHT].setBrightness(1.0f - restartBlink);
      }

      if (lc.mode == STOPPED && lc.size > 0) {
        float w = sin(6.f * M_PI * t) / 2 + 0.5f;
        lights[PLAY_STATUS_LIGHT].setBrightness(w / 3.f);
      }

      lights[RETURN_LIGHT].value = lc.rtrnEnabled && (rtrns[0]->isConnected() || rtrns[1]->isConnected());
    }

    t += args.sampleTime;
  }
};
