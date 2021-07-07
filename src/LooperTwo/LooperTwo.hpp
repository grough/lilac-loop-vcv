#define PORTS 2     // Number of main I/O ports
#define CHANNELS 16 // Polyphony per port

enum Mode {
  STOPPED,
  RECORDING,
  PLAYING,
  OVERDUBBING,
};

enum Order {
  RECORD_PLAY_OVERDUB,
  RECORD_OVERDUB_PLAY,
};

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
    RETURN_MOD_INPUT,
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

  FileSaver fileSaver;
  int depth = 16;

  Order order;

  Mode mode = STOPPED;
  int size = 0;
  int position = -1;

  std::vector<std::vector<float>> loop;

  int tracks[PORTS * CHANNELS];
  int start[PORTS * CHANNELS];
  int pos[PORTS * CHANNELS];

  float feedback = 1.0f;
  float mix = 1.0f;
  bool rtrnEnabled = true;

  dsp::SlewLimiter inputSmoother;
  dsp::SlewLimiter outputSmoother;

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

    inputSmoother.setRiseFall(100.f, 50.f);
    outputSmoother.setRiseFall(100.f, 50.f);

    lightDivider.setDivision(pow(2, 9));
    logDivider.setDivision(pow(2, 13));

    loop.resize(PORTS * CHANNELS);

    for (size_t i = 0; i < PORTS * CHANNELS; i++) {
      pos[i] = 0;
      start[i] = -1;
    }
  }

  Mode getNextMode() {
    if (mode == STOPPED && size == 0)
      return RECORDING;

    if (mode == RECORDING && order == RECORD_PLAY_OVERDUB)
      return PLAYING;

    if (mode == RECORDING && order == RECORD_OVERDUB_PLAY)
      return OVERDUBBING;

    if (mode == PLAYING)
      return OVERDUBBING;

    if (mode == OVERDUBBING)
      return PLAYING;

    if (mode == STOPPED && size > 0)
      return PLAYING;

    return mode;
  }

  Mode toggle() {
    Mode nextMode = getNextMode();

    if (mode == STOPPED && nextMode == PLAYING)
      position = 0;

    mode = nextMode;
    return mode;
  }

  void stop() {
    mode = STOPPED;
  }

  void erase() {
    mode = STOPPED;
    position = 0;
    size = 0;

    for (size_t p = 0; p < PORTS; p++) {
      tracks[p] = 0;

      for (size_t c = 0; c < CHANNELS; c++) {
        int track = p * CHANNELS + c;
        loop[track].clear();
        pos[track] = 0;
        start[track] = -1;
      }
    }
  }

  void process(const ProcessArgs &args) override {

    // Process toggle control

    bool toggleTriggered = toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.0f);

    if (toggleTriggered) {
      toggle();
      togglePulse.trigger(blinkTime);
    }

    // Process stop control

    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.0f);

    if (stopTriggered) {
      stop();
    }

    // Process erase control

    if (eraseTrigger.process(inputs[ERASE_CV_INPUT].getVoltage() > 0.0f)) {
      erase();
    }

    if (eraseButtonTrigger.process(params[ERASE_BUTTON_PARAM].getValue() > 0.0f)) {
      erase();
    }

    // Process return button param

    if (rtrnButtonTrigger.process(params[RETURN_BUTTON_PARAM].getValue() > 0.0f) &&
        (rtrns[0]->isConnected() || rtrns[1]->isConnected())) {
      rtrnEnabled = !rtrnEnabled;
    }

    bool rtrnActive = mode != STOPPED && rtrnEnabled;

    // Process feedback param

    if (mode == STOPPED) {
      feedback = 1.0f;
    } else {
      feedback = math::clamp(params[FEEDBACK_PARAM].getValue() + inputs[FEEDBACK_CV_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
    }

    // Process mix param

    mix = math::clamp(params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage() / 5, -1.0f, 1.0f);

    float monitorLevel = mix > 0 ? 1 - mix : 1;
    float loopLevel = mix > 0 ? 1 : 1 + mix;

    // Process return mod input

    float mod = inputs[RETURN_MOD_INPUT].isConnected() ? inputs[RETURN_MOD_INPUT].getVoltage() : 1.0f;

    // Grow

    if (mode == RECORDING)
      size++;

    // Gates

    float inGate = inputSmoother.process(args.sampleTime, mode == RECORDING || mode == OVERDUBBING ? 1.f : 0.f);
    float outGate = outputSmoother.process(args.sampleTime, mode == STOPPED ? 0.f : 1.f);

    // Process each main port

    for (size_t p = 0; p < PORTS; p++) {

      // Count channels

      int inChannels = ins[p]->getChannels();
      int rtrnChannels = rtrns[p]->getChannels();
      int totalChannels = std::max(inChannels, rtrnChannels);

      // Increase track count as inputs are are connected

      if (totalChannels > tracks[p] || size == 0) {
        tracks[p] = totalChannels;
        outs[p]->setChannels(totalChannels);
        snds[p]->setChannels(totalChannels);
      }

      // Process each polyphony channel

      for (size_t channel = 0; channel < tracks[p]; channel++) {
        int track = p * CHANNELS + channel;

        float in = ins[p]->getVoltage(channel);
        float rtrn = rtrns[p]->getVoltage(channel);
        float out = monitorLevel * in;
        float send = 0.0f;

        if (start[track] == -1 && size > 0)
          start[track] = position;

        if (loop[track].size() < size)
          loop[track].push_back(0.f);

        if (loop[track].size() > 0) {
          float sample = loop[track][pos[track]];
          float rtrnGate = rtrnActive && rtrnChannels >= (channel + 1) ? mod : 0.0f;
          float modRtrn = rtrnGate * rtrn + (1 - rtrnGate) * sample;
          float newSample = modRtrn;
          send = outGate * sample;
          out = out + loopLevel * send;
          loop[track][pos[track]] = feedback * newSample + inGate * in;
          pos[track]++;
        }

        if (pos[track] == size)
          pos[track] = 0;

        outs[p]->setVoltage(out, channel);
        snds[p]->setVoltage(send, channel);
      }
    }

    if (size > 0)
      position++;

    if (position == size)
      position = 0;

    // Lights

    if (position == 0) {
      restartPulse.trigger(blinkTime);
    }

    float restartBlink = (1.0f - togglePulse.process(args.sampleTime)) * restartPulse.process(args.sampleTime);

    if (lightDivider.process()) {
      lights[RECORD_STATUS_LIGHT].setBrightness(0.0f);
      lights[PLAY_STATUS_LIGHT].setBrightness(0.0f);

      if (mode == RECORDING || mode == OVERDUBBING) {
        lights[RECORD_STATUS_LIGHT].setBrightness(1.0f - restartBlink * .5f);
      }

      if (mode == PLAYING || mode == OVERDUBBING) {
        lights[PLAY_STATUS_LIGHT].setBrightness(1.0f - restartBlink);
      }

      if (mode == STOPPED && size > 0) {
        float w = sin(6.f * M_PI * t) / 2 + 0.5f;
        lights[PLAY_STATUS_LIGHT].setBrightness(w / 3.f);
      }

      lights[RETURN_LIGHT].value = rtrnEnabled && (rtrns[0]->isConnected() || rtrns[1]->isConnected());
    }

    t += args.sampleTime;
  }

  void onRemove() override {
    fileSaver.wait();
  }
};
