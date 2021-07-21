struct Loop {
  std::vector<float> samples;

  int pos = -1;
  int start = -1;

  void next(int size, int position) {
    if (start == -1 && size > 0)
      start = position;

    if (samples.size() < size)
      samples.push_back(0.0f);

    if (samples.size() > 0)
      pos++;

    if (pos == size)
      pos = 0;
  }

  float read() {
    if (pos > -1)
      return samples[pos];

    return 0.0f;
  }

  void write(float value) {
    if (pos > -1)
      samples[pos] = value;
  }

  int size() {
    return samples.size();
  }

  void rewind() {
    pos = start;
  }

  void reset() {
    pos = 0;
    start = -1;
    samples.clear();
  }
};

struct MultiLoop {
  int size;
  int position;

  std::vector<std::vector<Loop>> loops;

  MultiLoop() {
    reset();
  }

  void resize(int ports) {
    loops.resize(ports);
  }

  int setChannels(int port, int channels) {
    if (channels > loops[port].size())
      loops[port].resize(channels);

    return loops[port].size();
  }

  int getChannels(int port) {
    return loops[port].size();
  }

  void next(bool grow = false) {
    if (grow)
      size++;

    if (size > 0)
      position++;

    if (position == size)
      position = 0;

    for (size_t p = 0; p < loops.size(); p++) {
      for (size_t c = 0; c < loops[p].size(); c++) {
        loops[p][c].next(size, position);
      }
    }
  }

  float read(int port, int channel) {
    return loops[port][channel].read();
  }

  void write(int port, int channel, float value) {
    loops[port][channel].write(value);
  }

  void rewind() {
    position = 0;

    for (size_t p = 0; p < loops.size(); p++) {
      for (size_t c = 0; c < loops[p].size(); c++) {
        loops[p][c].rewind();
      }
    }
  }

  void reset() {
    size = 0;
    position = -1;

    for (size_t p = 0; p < loops.size(); p++) {
      loops[p].clear();
    }
  }
};
