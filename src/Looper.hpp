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

struct Looper : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    ERASE_BUTTON_PARAM,
    UNDO_BUTTON_PARAM,
    STOP_BUTTON_PARAM,
    FEEDBACK_PARAM,
    RETURN_BUTTON_PARAM,
    RETURN_ENABLED_PARAM,
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
  Order order = RECORD_PLAY_OVERDUB;

  Mode mode = STOPPED;

  MultiLoop loop;

  float feedback = 1.0f;
  float mix = 1.0f;

  bool armed = false;

  dsp::SlewLimiter smoothInGate;
  dsp::SlewLimiter smoothOutGate;

  dsp::BooleanTrigger armTrigger;
  dsp::BooleanTrigger toggleTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger eraseButtonTrigger;
  dsp::BooleanTrigger rtrnButtonTrigger;

  dsp::ClockDivider lightDivider;
  dsp::ClockDivider uiDivider;

  dsp::PulseGenerator restartPulse;
  dsp::PulseGenerator togglePulse;
  float blinkTime = 0.1f;

  float t = 0.0f;

  Looper() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.0f, 1.0f, 0.0f, "Toggle");
    configParam(ERASE_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Erase");
    configParam(STOP_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Stop");
    configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 1.0f, "Feedback", "%", 0.0f, 100.0f);
    configParam(RETURN_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Return enabled");
    configParam(RETURN_ENABLED_PARAM, 0.0f, 1.0f, 1.0f);
    configParam(MIX_PARAM, -1.0f, 1.0f, 0.0f, "Mix");

    ins[0] = &inputs[MAIN_1_INPUT];
    ins[1] = &inputs[MAIN_2_INPUT];
    rtrns[0] = &inputs[RETURN_1_INPUT];
    rtrns[1] = &inputs[RETURN_2_INPUT];
    snds[0] = &outputs[SEND_1_OUTPUT];
    snds[1] = &outputs[SEND_2_OUTPUT];
    outs[0] = &outputs[MAIN_1_OUTPUT];
    outs[1] = &outputs[MAIN_2_OUTPUT];

    smoothInGate.setRiseFall(100.f, 50.f);
    smoothOutGate.setRiseFall(100.f, 50.f);

    lightDivider.setDivision(512);
    uiDivider.setDivision(512);

    loop.resize(PORTS);
  }

  Mode getNextMode() {
    if (mode == STOPPED && loop.size == 0)
      return RECORDING;

    if (mode == RECORDING && order == RECORD_PLAY_OVERDUB)
      return PLAYING;

    if (mode == RECORDING && order == RECORD_OVERDUB_PLAY)
      return OVERDUBBING;

    if (mode == PLAYING)
      return OVERDUBBING;

    if (mode == OVERDUBBING)
      return PLAYING;

    if (mode == STOPPED && loop.size > 0)
      return PLAYING;

    return mode;
  }

  void toggle() {
    if (inputs[ARM_CV_INPUT].isConnected() && !armed)
      return;

    Mode nextMode = getNextMode();

    if (mode == STOPPED && nextMode == PLAYING)
      loop.rewind();

    mode = nextMode;
    armed = false;
    togglePulse.trigger(blinkTime);
  }

  void stop() {
    mode = STOPPED;
  }

  void erase() {
    mode = STOPPED;
    loop.reset();
  }

  void process(const ProcessArgs &args) override {

    // Process arm control

    if (armTrigger.process(inputs[ARM_CV_INPUT].getVoltage() > 0.0f)) {
      armed = true;
    }

    // Process toggle control

    if (toggleTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.0f)) {
      toggle();
    }

    // Process stop control

    if (stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.0f)) {
      stop();
    }

    // Process erase control

    if (eraseTrigger.process(inputs[ERASE_CV_INPUT].getVoltage() > 0.0f)) {
      erase();
    }

    if (eraseButtonTrigger.process(params[ERASE_BUTTON_PARAM].getValue() > 0.0f)) {
      erase();
    }

    // Process return enable control

    if (rtrnButtonTrigger.process(params[RETURN_BUTTON_PARAM].getValue() > 0.0f)) {
      params[RETURN_ENABLED_PARAM].setValue(1.0f - params[RETURN_ENABLED_PARAM].getValue());
    }

    bool rtrnActive = mode != STOPPED && params[RETURN_ENABLED_PARAM].getValue() > 0.0f;

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

    // Gates

    float inGate = smoothInGate.process(args.sampleTime, mode == RECORDING || mode == OVERDUBBING ? 1.f : 0.f);
    float outGate = smoothOutGate.process(args.sampleTime, mode == STOPPED ? 0.f : 1.f);

    // Count inputs

    for (size_t p = 0; p < PORTS; p++) {
      int tracks = loop.setChannels(p, std::max(ins[p]->getChannels(), rtrns[p]->getChannels()));
      outs[p]->setChannels(tracks);
      snds[p]->setChannels(tracks);
    }

    // Grow

    loop.next(mode == RECORDING);

    // Process each main port (left, right)

    for (size_t p = 0; p < PORTS; p++) {

      // Process each polyphony channel

      for (size_t channel = 0; channel < loop.getChannels(p); channel++) {
        float in = ins[p]->getVoltage(channel);
        float rtrn = rtrns[p]->getVoltage(channel);

        float sample = loop.read(p, channel);
        float rtrnGate = rtrnActive && rtrns[p]->getChannels() >= (channel + 1) ? mod : 0.0f;
        float newSample = rtrnGate * rtrn + (1 - rtrnGate) * sample;

        loop.write(p, channel, feedback * newSample + inGate * in);

        float send = outGate * sample;
        float out = loopLevel * send;

        outs[p]->setVoltage(out, channel);
        snds[p]->setVoltage(send, channel);
      }
    }

    // Lights

    if (loop.position == 0)
      restartPulse.trigger(blinkTime);

    if (lightDivider.process()) {
      float lightTime = 512 * args.sampleTime;

      float restartBlink = (1.0f - togglePulse.process(lightTime)) * restartPulse.process(lightTime);

      lights[RECORD_STATUS_LIGHT].setBrightness(0.0f);
      lights[PLAY_STATUS_LIGHT].setBrightness(0.0f);

      if (mode == RECORDING || mode == OVERDUBBING) {
        lights[RECORD_STATUS_LIGHT].setBrightness(1.0f - restartBlink * .5f);
      }

      if (mode == PLAYING || mode == OVERDUBBING) {
        lights[PLAY_STATUS_LIGHT].setBrightness(1.0f - restartBlink);
      }

      if (mode == STOPPED && loop.size > 0) {
        float w = sin(6.f * M_PI * t) / 2 + 0.5f;
        lights[PLAY_STATUS_LIGHT].setBrightness(w / 3.f);
      }

      lights[ARM_STATUS_LIGHT].value = armed;
      lights[RETURN_LIGHT].value = rtrns[0]->isConnected() || rtrns[1]->isConnected() ? params[RETURN_ENABLED_PARAM].getValue() : 0.0f;
    }

    t += args.sampleTime;
  }

  void onRemove() override {
    fileSaver.wait();
  }
};
